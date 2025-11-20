/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * CollaborativeEditing - Real-time collaborative game development
 *
 * Enables multiple developers to work together from anywhere in the world:
 * - Desktop developer in New York
 * - Mobile editor user in Tokyo
 * - Web editor user in London
 * All editing the same project simultaneously!
 *
 * Features:
 * - Real-time synchronization
 * - User presence indicators
 * - Live cursors
 * - Conflict resolution
 * - Chat and voice communication
 * - Change history and undo/redo
 * - Permission system (owner, editor, viewer)
 */

export interface CollaborativeSession {
  projectId: string;
  users: SessionUser[];
  changes: Change[];
  startTime: Date;
}

export interface SessionUser {
  id: string;
  username: string;
  color: string; // For cursor color
  platform: 'web' | 'desktop' | 'mobile';
  permission: 'owner' | 'editor' | 'viewer';
  cursor?: { sceneId: string; position: [number, number, number] };
  lastActive: Date;
}

export interface Change {
  id: string;
  userId: string;
  type: 'add' | 'update' | 'delete';
  target: 'entity' | 'asset' | 'script' | 'scene';
  targetId: string;
  data: any;
  timestamp: Date;
}

class CollaborativeEditingService {
  private ws: WebSocket | null = null;
  private session: CollaborativeSession | null = null;
  private currentUser: SessionUser | null = null;
  private changeHandlers: ((change: Change) => void)[] = [];

  /**
   * Join collaborative session
   */
  async joinSession(
    projectId: string,
    userId: string,
    username: string,
    platform: 'web' | 'desktop' | 'mobile'
  ): Promise<void> {
    this.currentUser = {
      id: userId,
      username,
      color: this.generateUserColor(userId),
      platform,
      permission: 'editor',
      lastActive: new Date(),
    };

    // Connect to collaboration server
    await this.connectToServer(projectId);

    // eslint-disable-next-line no-console
    console.log(`${username} joined collaborative session from ${platform}`);
  }

  /**
   * Connect to collaboration server
   */
  private async connectToServer(projectId: string): Promise<void> {
    return new Promise((resolve, reject) => {
      this.ws = new WebSocket(`wss://hub.novaengine.io/collab/${projectId}`);

      this.ws.onopen = () => {
        // Send join message
        this.send({
          type: 'join',
          user: this.currentUser,
        });
        resolve();
      };

      this.ws.onmessage = (event) => {
        this.handleMessage(JSON.parse(event.data));
      };

      this.ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        reject(error);
      };

      this.ws.onclose = () => {
        // eslint-disable-next-line no-console
        console.log('Collaboration session closed');
      };
    });
  }

  /**
   * Handle incoming message
   */
  private handleMessage(message: any): void {
    switch (message.type) {
      case 'user_joined':
        this.handleUserJoined(message.user);
        break;
      case 'user_left':
        this.handleUserLeft(message.userId);
        break;
      case 'change':
        this.handleChange(message.change);
        break;
      case 'cursor_move':
        this.handleCursorMove(message.userId, message.cursor);
        break;
      case 'chat':
        this.handleChat(message);
        break;
    }
  }

  /**
   * Handle user joined
   */
  private handleUserJoined(user: SessionUser): void {
    if (!this.session) return;
    this.session.users.push(user);
    // eslint-disable-next-line no-console
    console.log(`${user.username} joined from ${user.platform}`);
  }

  /**
   * Handle user left
   */
  private handleUserLeft(userId: string): void {
    if (!this.session) return;
    this.session.users = this.session.users.filter((u) => u.id !== userId);
  }

  /**
   * Handle change from another user
   */
  private handleChange(change: Change): void {
    if (!this.session) return;

    this.session.changes.push(change);

    // Notify all registered handlers
    this.changeHandlers.forEach((handler) => handler(change));

    // eslint-disable-next-line no-console
    console.log(
      `Change from user: ${change.type} ${change.target} ${change.targetId}`
    );
  }

  /**
   * Handle cursor move
   */
  private handleCursorMove(userId: string, cursor: any): void {
    if (!this.session) return;

    const user = this.session.users.find((u) => u.id === userId);
    if (user) {
      user.cursor = cursor;
    }
  }

  /**
   * Handle chat message
   */
  private handleChat(message: any): void {
    // eslint-disable-next-line no-console
    console.log(`${message.username}: ${message.text}`);
  }

  /**
   * Broadcast a change
   */
  broadcastChange(change: Omit<Change, 'id' | 'userId' | 'timestamp'>): void {
    if (!this.currentUser) return;

    const fullChange: Change = {
      ...change,
      id: `change-${Date.now()}`,
      userId: this.currentUser.id,
      timestamp: new Date(),
    };

    this.send({
      type: 'change',
      change: fullChange,
    });

    // Also apply locally
    this.handleChange(fullChange);
  }

  /**
   * Update cursor position
   */
  updateCursor(sceneId: string, position: [number, number, number]): void {
    if (!this.currentUser) return;

    this.currentUser.cursor = { sceneId, position };

    this.send({
      type: 'cursor_move',
      cursor: this.currentUser.cursor,
    });
  }

  /**
   * Send chat message
   */
  sendChatMessage(text: string): void {
    if (!this.currentUser) return;

    this.send({
      type: 'chat',
      username: this.currentUser.username,
      text,
    });
  }

  /**
   * Register change handler
   */
  onChangeReceived(handler: (change: Change) => void): void {
    this.changeHandlers.push(handler);
  }

  /**
   * Get active users
   */
  getActiveUsers(): SessionUser[] {
    return this.session?.users || [];
  }

  /**
   * Send message to server
   */
  private send(message: any): void {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    }
  }

  /**
   * Generate unique color for user
   */
  private generateUserColor(userId: string): string {
    const colors = [
      '#FF6B6B',
      '#4ECDC4',
      '#45B7D1',
      '#FFA07A',
      '#98D8C8',
      '#F7DC6F',
      '#BB8FCE',
      '#85C1E2',
    ];
    const index = userId
      .split('')
      .reduce((acc, char) => acc + char.charCodeAt(0), 0);
    return colors[index % colors.length]!;
  }

  /**
   * Leave session
   */
  leaveSession(): void {
    if (this.ws) {
      this.send({ type: 'leave' });
      this.ws.close();
      this.ws = null;
    }
    this.session = null;
    this.currentUser = null;
  }

  /**
   * Check if connected
   */
  isConnected(): boolean {
    return this.ws !== null && this.ws.readyState === WebSocket.OPEN;
  }
}

export const collaborativeEditing = new CollaborativeEditingService();
