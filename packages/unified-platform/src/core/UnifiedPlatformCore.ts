/**
 * Nova Engine Unified Platform Core
 *
 * Central system that integrates ALL modules:
 * - Hub, Editor, Launcher, Multiplayer, Social, Settings
 *
 * Provides unified authentication, state management, and communication
 * Single source of truth for the entire platform
 */

import { EventEmitter } from 'events';

// Import all integrated services
import { friendsService } from '../../launcher-web/src/services/FriendsService';
import { achievementsService } from '../../launcher-web/src/services/AchievementsService';
import { multiplayerLobbyService } from '../../launcher-web/src/services/MultiplayerLobbyService';

export type PlatformMode =
  | 'hub'
  | 'editor'
  | 'launcher'
  | 'multiplayer'
  | 'social'
  | 'settings';

export interface UnifiedUser {
  id: string;
  username: string;
  email: string;
  displayName: string;
  avatarUrl?: string;
  level: number;
  xp: number;
  points: number;
  badges: string[];
  createdAt: Date;
  lastLogin: Date;
}

export interface UnifiedNotification {
  id: string;
  type:
    | 'info'
    | 'success'
    | 'warning'
    | 'error'
    | 'achievement'
    | 'friend'
    | 'invite'
    | 'update';
  title: string;
  message: string;
  icon?: string;
  actionUrl?: string;
  actionText?: string;
  timestamp: Date;
  read: boolean;
  persistent: boolean;
}

export interface PlatformState {
  mode: PlatformMode;
  isLoggedIn: boolean;
  user: UnifiedUser | null;
  isOnline: boolean;
  notifications: UnifiedNotification[];

  // Current activity
  currentGame: {
    id: string;
    name: string;
    mode: 'playing' | 'editing' | 'spectating';
    startedAt: Date;
  } | null;

  currentProject: {
    id: string;
    name: string;
    lastSaved: Date;
  } | null;

  currentLobby: {
    id: string;
    gameId: string;
    players: number;
  } | null;

  // Statistics
  stats: {
    gamesPlayed: number;
    gamesCreated: number;
    achievementsUnlocked: number;
    friendsCount: number;
    totalPlaytime: number; // minutes
  };
}

export class UnifiedPlatformCore extends EventEmitter {
  private state: PlatformState;
  private wsConnections: Map<string, WebSocket> = new Map();
  private syncInterval: number | null = null;

  constructor() {
    super();

    this.state = {
      mode: 'hub',
      isLoggedIn: false,
      user: null,
      isOnline: navigator.onLine,
      notifications: [],
      currentGame: null,
      currentProject: null,
      currentLobby: null,
      stats: {
        gamesPlayed: 0,
        gamesCreated: 0,
        achievementsUnlocked: 0,
        friendsCount: 0,
        totalPlaytime: 0,
      },
    };
  }

  /**
   * Initialize the unified platform
   */
  async initialize(): Promise<void> {
    console.log('🚀 Initializing Nova Engine Unified Platform...');

    // Setup online/offline detection
    this.setupOnlineDetection();

    // Check for existing session
    const token = localStorage.getItem('auth_token');
    if (token) {
      await this.restoreSession();
    }

    // Setup global event listeners
    this.setupGlobalEventListeners();

    console.log('✅ Nova Engine Platform Ready');
  }

  /**
   * Login to the platform
   */
  async login(email: string, password: string): Promise<boolean> {
    try {
      const response = await fetch('/api/auth/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password }),
      });

      if (!response.ok) throw new Error('Login failed');

      const data = await response.json();

      // Store auth data
      localStorage.setItem('auth_token', data.token);
      localStorage.setItem('user_id', data.user.id);
      localStorage.setItem('refresh_token', data.refreshToken);

      // Update state
      this.state.isLoggedIn = true;
      this.state.user = data.user;

      // Initialize user features
      await this.initializeUserFeatures();

      // Emit login event
      this.emit('login', data.user);

      console.log(`✅ Welcome back, ${data.user.username}!`);
      return true;
    } catch (error) {
      console.error('❌ Login failed:', error);
      return false;
    }
  }

  /**
   * Register new account
   */
  async register(
    username: string,
    email: string,
    password: string
  ): Promise<boolean> {
    try {
      const response = await fetch('/api/auth/register', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, email, password }),
      });

      if (!response.ok) throw new Error('Registration failed');

      const data = await response.json();

      // Store auth data
      localStorage.setItem('auth_token', data.token);
      localStorage.setItem('user_id', data.user.id);

      // Update state
      this.state.isLoggedIn = true;
      this.state.user = data.user;

      // Initialize user features
      await this.initializeUserFeatures();

      // Emit event
      this.emit('register', data.user);

      console.log(`✅ Account created! Welcome, ${data.user.username}!`);
      return true;
    } catch (error) {
      console.error('❌ Registration failed:', error);
      return false;
    }
  }

  /**
   * Logout from platform
   */
  async logout(): Promise<void> {
    // Close all WebSocket connections
    this.wsConnections.forEach((ws) => ws.close());
    this.wsConnections.clear();

    // Stop sync
    if (this.syncInterval) {
      clearInterval(this.syncInterval);
      this.syncInterval = null;
    }

    // Clear auth data
    localStorage.removeItem('auth_token');
    localStorage.removeItem('user_id');
    localStorage.removeItem('refresh_token');

    // Reset state
    this.state.isLoggedIn = false;
    this.state.user = null;
    this.state.currentGame = null;
    this.state.currentProject = null;
    this.state.currentLobby = null;

    // Switch to hub
    this.switchMode('hub');

    // Emit event
    this.emit('logout');

    console.log('✅ Logged out successfully');
  }

  /**
   * Restore session from stored token
   */
  async restoreSession(): Promise<boolean> {
    try {
      const token = localStorage.getItem('auth_token');
      if (!token) return false;

      const response = await fetch('/api/auth/me', {
        headers: { Authorization: `Bearer ${token}` },
      });

      if (!response.ok) throw new Error('Session invalid');

      const user = await response.json();

      this.state.isLoggedIn = true;
      this.state.user = user;

      await this.initializeUserFeatures();

      this.emit('sessionRestored', user);
      console.log(`✅ Session restored for ${user.username}`);

      return true;
    } catch (error) {
      console.error('❌ Session restore failed:', error);
      localStorage.removeItem('auth_token');
      return false;
    }
  }

  /**
   * Initialize user-specific features after login
   */
  private async initializeUserFeatures(): Promise<void> {
    console.log('Initializing user features...');

    // Load user stats
    await this.loadUserStats();

    // Initialize integrated services
    friendsService.initializeRealtimeUpdates();

    // Connect to platform WebSocket
    this.connectPlatformWebSocket();

    // Load notifications
    await this.loadNotifications();

    // Start background sync
    this.startBackgroundSync();

    // Load friends list
    await friendsService.getFriends();

    console.log('✅ User features initialized');
  }

  /**
   * Load user statistics
   */
  private async loadUserStats(): Promise<void> {
    try {
      const response = await fetch('/api/users/me/stats', {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (response.ok) {
        this.state.stats = await response.json();
      }
    } catch (error) {
      console.error('Failed to load stats:', error);
    }
  }

  /**
   * Connect to platform WebSocket for real-time updates
   */
  private connectPlatformWebSocket(): void {
    const token = this.getToken();
    const ws = new WebSocket(
      `ws://localhost:3000/api/platform/ws?token=${token}`
    );

    ws.onopen = () => {
      console.log('✅ Connected to platform');
    };

    ws.onmessage = (event) => {
      const message = JSON.parse(event.data);
      this.handlePlatformMessage(message);
    };

    ws.onerror = (error) => {
      console.error('Platform WebSocket error:', error);
    };

    ws.onclose = () => {
      console.log('Disconnected from platform, reconnecting...');
      setTimeout(() => this.connectPlatformWebSocket(), 5000);
    };

    this.wsConnections.set('platform', ws);
  }

  /**
   * Handle platform messages
   */
  private handlePlatformMessage(message: any): void {
    switch (message.type) {
      case 'notification':
        this.addNotification(message.notification);
        break;
      case 'friend_online':
        friendsService.updateFriendStatus(message.friendId, 'online');
        break;
      case 'friend_offline':
        friendsService.updateFriendStatus(message.friendId, 'offline');
        break;
      case 'achievement_progress':
        achievementsService.updateProgress(
          message.achievementId,
          message.progress
        );
        break;
      case 'game_invite':
        this.addNotification({
          type: 'invite',
          title: 'Game Invite',
          message: `${message.fromUser} invited you to play ${message.gameName}`,
          actionUrl: `/play/${message.gameId}?lobby=${message.lobbyId}`,
          actionText: 'Join Game',
          persistent: true,
        });
        break;
      case 'system_update':
        this.addNotification({
          type: 'update',
          title: 'System Update Available',
          message: message.message,
          actionUrl: '/settings/updates',
          actionText: 'Update Now',
          persistent: true,
        });
        break;
    }
  }

  /**
   * Load notifications
   */
  private async loadNotifications(): Promise<void> {
    try {
      const response = await fetch('/api/notifications', {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (response.ok) {
        this.state.notifications = await response.json();
        this.emit('notificationsLoaded');
      }
    } catch (error) {
      console.error('Failed to load notifications:', error);
    }
  }

  /**
   * Add notification
   */
  private addNotification(
    notification: Omit<UnifiedNotification, 'id' | 'timestamp' | 'read'>
  ): void {
    const fullNotification: UnifiedNotification = {
      ...notification,
      id: `notif_${Date.now()}_${Math.random().toString(36)}`,
      timestamp: new Date(),
      read: false,
      persistent: notification.persistent || false,
    };

    this.state.notifications.unshift(fullNotification);

    // Limit non-persistent notifications
    const nonPersistent = this.state.notifications.filter((n) => !n.persistent);
    if (nonPersistent.length > 50) {
      this.state.notifications = [
        ...this.state.notifications.filter((n) => n.persistent),
        ...nonPersistent.slice(0, 50),
      ];
    }

    this.emit('notification', fullNotification);
  }

  /**
   * Show notification (public wrapper for addNotification)
   */
  public showNotification(
    notification: Omit<UnifiedNotification, 'id' | 'timestamp' | 'read'>
  ): void {
    this.addNotification(notification);
  }

  /**
   * Start background sync
   */
  private startBackgroundSync(): void {
    // Sync every 5 minutes
    this.syncInterval = window.setInterval(
      () => {
        this.syncCloudData();
      },
      5 * 60 * 1000
    );

    // Initial sync
    this.syncCloudData();
  }

  /**
   * Sync cloud data
   */
  private async syncCloudData(): Promise<void> {
    if (!this.state.isOnline || !this.state.isLoggedIn) return;

    try {
      await Promise.all([
        this.syncGameSaves(),
        this.syncProjectFiles(),
        this.syncSettings(),
        this.syncAchievements(),
      ]);

      console.log('✅ Cloud sync complete');
    } catch (error) {
      console.error('❌ Cloud sync failed:', error);
    }
  }

  private async syncGameSaves(): Promise<void> {
    // Sync game save data
    const response = await fetch('/api/sync/saves', {
      method: 'POST',
      headers: { Authorization: `Bearer ${this.getToken()}` },
    });
  }

  private async syncProjectFiles(): Promise<void> {
    // Sync editor project files
    const response = await fetch('/api/sync/projects', {
      method: 'POST',
      headers: { Authorization: `Bearer ${this.getToken()}` },
    });
  }

  private async syncSettings(): Promise<void> {
    // Sync user settings
    const response = await fetch('/api/sync/settings', {
      method: 'POST',
      headers: { Authorization: `Bearer ${this.getToken()}` },
    });
  }

  private async syncAchievements(): Promise<void> {
    // Sync achievement progress
    const response = await fetch('/api/sync/achievements', {
      method: 'POST',
      headers: { Authorization: `Bearer ${this.getToken()}` },
    });
  }

  /**
   * Switch between platform modes
   */
  switchMode(mode: PlatformMode): void {
    const previousMode = this.state.mode;
    this.state.mode = mode;

    this.emit('modeChanged', { from: previousMode, to: mode });
    console.log(`Switched to ${mode} mode`);
  }

  /**
   * Setup online/offline detection
   */
  private setupOnlineDetection(): void {
    window.addEventListener('online', () => {
      this.state.isOnline = true;
      this.emit('online');
      console.log('✅ Connection restored');

      if (this.state.isLoggedIn) {
        this.connectPlatformWebSocket();
        this.syncCloudData();
      }
    });

    window.addEventListener('offline', () => {
      this.state.isOnline = false;
      this.emit('offline');
      console.log('⚠️  Offline mode');
    });
  }

  /**
   * Setup global event listeners
   */
  private setupGlobalEventListeners(): void {
    // Handle before unload
    window.addEventListener('beforeunload', (e) => {
      if (this.state.currentProject) {
        e.preventDefault();
        e.returnValue =
          'You have unsaved changes. Are you sure you want to leave?';
      }
    });

    // Handle visibility change
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        console.log('App hidden');
      } else {
        console.log('App visible');
        // Refresh data when app becomes visible
        if (this.state.isLoggedIn) {
          this.loadNotifications();
        }
      }
    });
  }

  /**
   * Get current state
   */
  getState(): PlatformState {
    return { ...this.state };
  }

  /**
   * Get auth token
   */
  private getToken(): string {
    return localStorage.getItem('auth_token') || '';
  }

  /**
   * Quick access methods for integrated modules
   */

  // Hub methods
  async browseGames(filters?: any) {
    this.switchMode('hub');
    // Implementation
  }

  async searchGames(query: string) {
    // Implementation
  }

  // Editor methods
  async createProject(name: string) {
    this.switchMode('editor');
    // Implementation
  }

  async openProject(projectId: string) {
    this.switchMode('editor');
    // Implementation
  }

  // Launcher methods
  async playGame(gameId: string) {
    this.switchMode('launcher');
    // Implementation
  }

  // Multiplayer methods
  async joinMultiplayer(gameId: string) {
    this.switchMode('multiplayer');
    return multiplayerLobbyService.quickMatch({ gameId, region: 'auto' });
  }

  // Social methods
  async viewSocial() {
    this.switchMode('social');
  }

  // Settings
  async openSettings() {
    this.switchMode('settings');
  }
}

// Export singleton instance
export const unifiedPlatform = new UnifiedPlatformCore();

// Make globally accessible
if (typeof window !== 'undefined') {
  (window as any).NovaEngine = unifiedPlatform;
}
