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
 * Nova Engine Unified Platform - API Client
 * Centralized API client that connects frontend to backend services
 */

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:3001';
const HUB_API_URL = import.meta.env.VITE_HUB_API_URL || 'http://localhost:3001';

class ApiClient {
  private baseUrl: string;
  private hubUrl: string;

  constructor() {
    this.baseUrl = API_BASE_URL;
    this.hubUrl = HUB_API_URL;
  }

  /**
   * Get auth token from localStorage
   */
  private getToken(): string | null {
    return localStorage.getItem('auth_token');
  }

  /**
   * Get default headers including auth
   */
  private getHeaders(includeAuth: boolean = true): HeadersInit {
    const headers: HeadersInit = {
      'Content-Type': 'application/json',
    };

    if (includeAuth) {
      const token = this.getToken();
      if (token) {
        headers['Authorization'] = `Bearer ${token}`;
      }
    }

    return headers;
  }

  /**
   * Validate and sanitize path parameter
   */
  private validatePathParam(
    param: string,
    paramName: string = 'parameter'
  ): string {
    if (!param || typeof param !== 'string') {
      throw new Error(`Invalid ${paramName}`);
    }
    // Only allow alphanumeric characters, hyphens, and underscores
    if (!/^[a-zA-Z0-9-_]+$/.test(param)) {
      throw new Error(`${paramName} contains invalid characters`);
    }
    return encodeURIComponent(param);
  }

  /**
   * Make API request with error handling
   */
  private async request<T>(url: string, options: RequestInit = {}): Promise<T> {
    try {
      const response = await fetch(url, {
        ...options,
        headers: {
          ...this.getHeaders(),
          ...options.headers,
        },
      });

      if (!response.ok) {
        const error = await response.json().catch(() => ({
          message: response.statusText,
        }));
        throw new Error(error.message || `HTTP ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error('API Request failed:', url, error);
      throw error;
    }
  }

  /**
   * GET request
   */
  async get<T>(endpoint: string): Promise<T> {
    return this.request<T>(`${this.baseUrl}${endpoint}`, {
      method: 'GET',
    });
  }

  /**
   * POST request
   */
  async post<T>(endpoint: string, data?: any): Promise<T> {
    return this.request<T>(`${this.baseUrl}${endpoint}`, {
      method: 'POST',
      body: data ? JSON.stringify(data) : undefined,
    });
  }

  /**
   * PUT request
   */
  async put<T>(endpoint: string, data?: any): Promise<T> {
    return this.request<T>(`${this.baseUrl}${endpoint}`, {
      method: 'PUT',
      body: data ? JSON.stringify(data) : undefined,
    });
  }

  /**
   * DELETE request
   */
  async delete<T>(endpoint: string): Promise<T> {
    return this.request<T>(`${this.baseUrl}${endpoint}`, {
      method: 'DELETE',
    });
  }

  /**
   * GET request to Hub API
   */
  async hubGet<T>(endpoint: string): Promise<T> {
    return this.request<T>(`${this.hubUrl}${endpoint}`, {
      method: 'GET',
    });
  }

  /**
   * POST request to Hub API
   */
  async hubPost<T>(endpoint: string, data?: any): Promise<T> {
    return this.request<T>(`${this.hubUrl}${endpoint}`, {
      method: 'POST',
      body: data ? JSON.stringify(data) : undefined,
    });
  }

  // ============ AUTHENTICATION ============

  async login(email: string, password: string) {
    return this.post('/api/auth/login', { email, password });
  }

  async register(username: string, email: string, password: string) {
    return this.post('/api/auth/register', { username, email, password });
  }

  async getMe() {
    return this.get('/api/auth/me');
  }

  // ============ GAMES ============

  async getGames(params?: {
    category?: string;
    search?: string;
    sort?: string;
    limit?: number;
  }) {
    const query = new URLSearchParams(params as any).toString();
    return this.get(`/api/games${query ? `?${query}` : ''}`);
  }

  async getGame(gameId: string) {
    const safeGameId = this.validatePathParam(gameId, 'game ID');
    return this.get(`/api/games/${safeGameId}`);
  }

  async createGame(gameData: any) {
    return this.post('/api/games', gameData);
  }

  async updateGame(gameId: string, gameData: any) {
    const safeGameId = this.validatePathParam(gameId, 'game ID');
    return this.put(`/api/games/${safeGameId}`, gameData);
  }

  async deleteGame(gameId: string) {
    const safeGameId = this.validatePathParam(gameId, 'game ID');
    return this.delete(`/api/games/${safeGameId}`);
  }

  async getGameStats(gameId: string) {
    const safeGameId = this.validatePathParam(gameId, 'game ID');
    return this.get(`/api/games/${safeGameId}/stats`);
  }

  // ============ USER / SOCIAL ============

  async getUserProfile(userId: string) {
    const safeUserId = this.validatePathParam(userId, 'user ID');
    return this.get(`/api/users/${safeUserId}`);
  }

  async getUserStats() {
    return this.get('/api/users/me/stats');
  }

  async getFriends() {
    return this.get('/api/social/friends');
  }

  async addFriend(friendId: string) {
    const safeFriendId = this.validatePathParam(friendId, 'friend ID');
    return this.post(`/api/social/friends/${safeFriendId}`);
  }

  async removeFriend(friendId: string) {
    const safeFriendId = this.validatePathParam(friendId, 'friend ID');
    return this.delete(`/api/social/friends/${safeFriendId}`);
  }

  async getAchievements() {
    return this.get('/api/social/achievements');
  }

  // ============ MULTIPLAYER ============

  async getLobbies(gameId?: string) {
    const query = gameId
      ? `?gameId=${this.validatePathParam(gameId, 'game ID')}`
      : '';
    return this.get(`/api/multiplayer/lobbies${query}`);
  }

  async createLobby(lobbyData: any) {
    return this.post('/api/multiplayer/lobbies', lobbyData);
  }

  async joinLobby(lobbyId: string) {
    const safeLobbyId = this.validatePathParam(lobbyId, 'lobby ID');
    return this.post(`/api/multiplayer/lobbies/${safeLobbyId}/join`);
  }

  async leaveLobby(lobbyId: string) {
    const safeLobbyId = this.validatePathParam(lobbyId, 'lobby ID');
    return this.post(`/api/multiplayer/lobbies/${safeLobbyId}/leave`);
  }

  async quickMatch(gameId: string, options?: any) {
    return this.post('/api/multiplayer/quick-match', { gameId, ...options });
  }

  // ============ PROJECTS (EDITOR) ============

  async getProjects() {
    return this.hubGet('/api/projects');
  }

  async getProject(projectId: string) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');
    return this.hubGet(`/api/projects/${safeProjectId}`);
  }

  async createProject(projectData: any) {
    return this.hubPost('/api/projects', projectData);
  }

  async updateProject(projectId: string, projectData: any) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');
    return this.hubPost(`/api/projects/${safeProjectId}`, projectData);
  }

  async deleteProject(projectId: string) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');
    return this.request(`${this.hubUrl}/api/projects/${safeProjectId}`, {
      method: 'DELETE',
    });
  }

  async saveProject(projectId: string, sceneData: any) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');
    return this.hubPost(`/api/projects/${safeProjectId}/save`, sceneData);
  }

  // ============ ASSETS ============

  async getAssets(projectId: string) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');
    return this.hubGet(`/api/assets?projectId=${safeProjectId}`);
  }

  async uploadAsset(projectId: string, file: File, metadata?: any) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');

    // No file size or type restrictions - users need to upload large game assets
    const formData = new FormData();
    formData.append('file', file);
    if (metadata) {
      formData.append('metadata', JSON.stringify(metadata));
    }

    const response = await fetch(
      `${this.hubUrl}/api/assets/upload?projectId=${safeProjectId}`,
      {
        method: 'POST',
        headers: {
          Authorization: `Bearer ${this.getToken()}`,
        },
        body: formData,
      }
    );

    if (!response.ok) {
      throw new Error('Asset upload failed');
    }

    return response.json();
  }

  // ============ BUILDS ============

  async buildProject(projectId: string, buildConfig: any) {
    const safeProjectId = this.validatePathParam(projectId, 'project ID');
    return this.hubPost(`/api/builds`, {
      projectId: safeProjectId,
      ...buildConfig,
    });
  }

  async getBuildStatus(buildId: string) {
    const safeBuildId = this.validatePathParam(buildId, 'build ID');
    return this.hubGet(`/api/builds/${safeBuildId}/status`);
  }

  // ============ NOTIFICATIONS ============

  async getNotifications() {
    return this.get('/api/notifications');
  }

  async markNotificationRead(notificationId: string) {
    const safeNotificationId = this.validatePathParam(
      notificationId,
      'notification ID'
    );
    return this.put(`/api/notifications/${safeNotificationId}/read`, {});
  }

  async clearNotifications() {
    return this.delete('/api/notifications');
  }
}

// Export singleton instance
export const apiClient = new ApiClient();
export default apiClient;
