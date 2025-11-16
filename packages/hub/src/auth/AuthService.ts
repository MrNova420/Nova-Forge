/**
 * Authentication Service
 */

import bcrypt from 'bcryptjs';
import { v4 as uuidv4 } from 'uuid';
import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';

export interface User {
  id: string;
  email: string;
  username: string;
  displayName?: string;
  avatarUrl?: string;
  createdAt: Date;
  isActive: boolean;
  isVerified: boolean;
}

export interface RegisterData {
  email: string;
  username: string;
  password: string;
  displayName?: string;
}

export interface LoginData {
  emailOrUsername: string;
  password: string;
}

export class AuthService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();

  async register(data: RegisterData): Promise<User> {
    // Hash password
    const passwordHash = await bcrypt.hash(data.password, 10);

    // Insert user
    const result = await this.db.query<User>(
      `INSERT INTO users (email, username, password_hash, display_name)
       VALUES ($1, $2, $3, $4)
       RETURNING id, email, username, display_name as "displayName", 
                 avatar_url as "avatarUrl", created_at as "createdAt",
                 is_active as "isActive", is_verified as "isVerified"`,
      [
        data.email,
        data.username,
        passwordHash,
        data.displayName || data.username,
      ]
    );

    return result.rows[0];
  }

  async login(data: LoginData): Promise<User | null> {
    // Find user by email or username
    const result = await this.db.query<User & { password_hash: string }>(
      `SELECT id, email, username, password_hash, display_name as "displayName",
              avatar_url as "avatarUrl", created_at as "createdAt",
              is_active as "isActive", is_verified as "isVerified"
       FROM users
       WHERE (email = $1 OR username = $1) AND is_active = true`,
      [data.emailOrUsername]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const user = result.rows[0];

    // Verify password
    const isValid = await bcrypt.compare(data.password, user.password_hash);
    if (!isValid) {
      return null;
    }

    // Update last login
    await this.db.query('UPDATE users SET last_login = NOW() WHERE id = $1', [
      user.id,
    ]);

    // Remove password_hash from response
    const { password_hash, ...userWithoutPassword } = user;
    return userWithoutPassword;
  }

  async getUserById(userId: string): Promise<User | null> {
    // Check cache first
    const cached = await this.redis.getJson<User>(`user:${userId}`);
    if (cached) {
      return cached;
    }

    // Query database
    const result = await this.db.query<User>(
      `SELECT id, email, username, display_name as "displayName",
              avatar_url as "avatarUrl", created_at as "createdAt",
              is_active as "isActive", is_verified as "isVerified"
       FROM users WHERE id = $1`,
      [userId]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const user = result.rows[0];

    // Cache for 5 minutes
    await this.redis.setJson(`user:${userId}`, user, 300);

    return user;
  }

  async updateUser(
    userId: string,
    updates: Partial<Pick<User, 'displayName' | 'avatarUrl'>>
  ): Promise<User | null> {
    const sets: string[] = [];
    const values: any[] = [];
    let paramIndex = 1;

    if (updates.displayName !== undefined) {
      sets.push(`display_name = $${paramIndex++}`);
      values.push(updates.displayName);
    }

    if (updates.avatarUrl !== undefined) {
      sets.push(`avatar_url = $${paramIndex++}`);
      values.push(updates.avatarUrl);
    }

    if (sets.length === 0) {
      return this.getUserById(userId);
    }

    sets.push(`updated_at = NOW()`);
    values.push(userId);

    const result = await this.db.query<User>(
      `UPDATE users SET ${sets.join(', ')}
       WHERE id = $${paramIndex}
       RETURNING id, email, username, display_name as "displayName",
                 avatar_url as "avatarUrl", created_at as "createdAt",
                 is_active as "isActive", is_verified as "isVerified"`,
      values
    );

    if (result.rows.length === 0) {
      return null;
    }

    const user = result.rows[0];

    // Invalidate cache
    await this.redis.del(`user:${userId}`);

    return user;
  }

  async deleteUser(userId: string): Promise<boolean> {
    const result = await this.db.query('DELETE FROM users WHERE id = $1', [
      userId,
    ]);

    if (result.rowCount && result.rowCount > 0) {
      await this.redis.del(`user:${userId}`);
      return true;
    }

    return false;
  }
}
