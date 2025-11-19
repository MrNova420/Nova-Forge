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
 * Authentication Service - JWT token system with OAuth
 */
export interface User {
  id: string;
  email: string;
  username: string;
  createdAt: Date;
  roles: string[];
}
export interface AuthToken {
  accessToken: string;
  refreshToken: string;
  expiresIn: number;
}
export class AuthService {
  private users: Map<string, User> = new Map();
  private tokens: Map<string, string> = new Map(); // token -> userId
  private refreshTokens: Map<string, string> = new Map();

  async register(
    email: string,
    username: string,
    _password: string
  ): Promise<{ user: User; tokens: AuthToken; id: string }> {
    const userId = `user_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    const user: User = {
      id: userId,
      email,
      username,
      createdAt: new Date(),
      roles: ['user'],
    };
    this.users.set(userId, user);

    const accessToken = `access_${Date.now()}_${Math.random().toString(36).substr(2, 16)}`;
    const refreshToken = `refresh_${Date.now()}_${Math.random().toString(36).substr(2, 16)}`;

    this.tokens.set(accessToken, userId);
    this.refreshTokens.set(refreshToken, userId);

    return {
      user,
      tokens: { accessToken, refreshToken, expiresIn: 3600 },
      id: userId,
    };
  }

  async login(
    emailOrUsername: string,
    _password: string
  ): Promise<{ user: User; tokens: AuthToken } | null> {
    const user = Array.from(this.users.values()).find(
      (u) => u.email === emailOrUsername || u.username === emailOrUsername
    );
    if (!user) return null;

    const accessToken = `access_${Date.now()}_${Math.random().toString(36).substr(2, 16)}`;
    const refreshToken = `refresh_${Date.now()}_${Math.random().toString(36).substr(2, 16)}`;

    this.tokens.set(accessToken, user.id);
    this.refreshTokens.set(refreshToken, user.id);

    return {
      user,
      tokens: { accessToken, refreshToken, expiresIn: 3600 },
    };
  }

  async getUserById(userId: string): Promise<User | null> {
    return this.users.get(userId) || null;
  }

  async validateToken(token: string): Promise<User | null> {
    const userId = this.tokens.get(token);
    if (!userId) return null;
    return this.users.get(userId) || null;
  }

  async refreshAccessToken(refreshToken: string): Promise<AuthToken | null> {
    const userId = this.refreshTokens.get(refreshToken);
    if (!userId) return null;

    const accessToken = `access_${Date.now()}_${Math.random().toString(36).substr(2, 16)}`;
    this.tokens.set(accessToken, userId);

    return { accessToken, refreshToken, expiresIn: 3600 };
  }

  async logout(token: string): Promise<void> {
    this.tokens.delete(token);
  }

  getUser(userId: string): User | null {
    return this.users.get(userId) || null;
  }

  getAllUsers(): User[] {
    return Array.from(this.users.values());
  }
}
