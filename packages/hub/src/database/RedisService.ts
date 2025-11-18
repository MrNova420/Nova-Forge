/**
 * Redis Caching Service
 */

import { createClient, RedisClientType } from 'redis';
import { logger } from '../utils/logger';

export class RedisService {
  private static instance: RedisService;
  private client: RedisClientType | null = null;

  private constructor() {}

  static getInstance(): RedisService {
    if (!RedisService.instance) {
      RedisService.instance = new RedisService();
    }
    return RedisService.instance;
  }

  async connect(): Promise<void> {
    if (this.client) {
      logger.warn('Redis already connected');
      return;
    }

    this.client = createClient({
      url: process.env.REDIS_URL || 'redis://localhost:6379',
      socket: {
        reconnectStrategy: false, // Disable auto-reconnect
      },
    });

    this.client.on('error', (err) => {
      // Only log once, don't spam
      if (!this.client?.isOpen) {
        logger.warn('Redis unavailable - running without cache');
      }
    });
    this.client.on('connect', () => logger.info('Redis connecting...'));
    this.client.on('ready', () => logger.info('Redis ready'));

    await this.client.connect();
  }

  async disconnect(): Promise<void> {
    if (this.client) {
      try {
        // Check if client is still connected before trying to quit
        if (this.client.isOpen) {
          await this.client.quit();
        }
      } catch (error) {
        logger.warn('Redis disconnect error (ignored):', error);
      } finally {
        this.client = null;
        logger.info('Redis disconnected');
      }
    }
  }

  async get(key: string): Promise<string | null> {
    if (!this.client) throw new Error('Redis not connected');
    return this.client.get(key);
  }

  async set(key: string, value: string, ttl?: number): Promise<void> {
    if (!this.client) throw new Error('Redis not connected');
    if (ttl) {
      await this.client.setEx(key, ttl, value);
    } else {
      await this.client.set(key, value);
    }
  }

  async del(key: string): Promise<void> {
    if (!this.client) throw new Error('Redis not connected');
    await this.client.del(key);
  }

  async exists(key: string): Promise<boolean> {
    if (!this.client) throw new Error('Redis not connected');
    return (await this.client.exists(key)) === 1;
  }

  async getJson<T>(key: string): Promise<T | null> {
    const value = await this.get(key);
    return value ? JSON.parse(value) : null;
  }

  async setJson<T>(key: string, value: T, ttl?: number): Promise<void> {
    await this.set(key, JSON.stringify(value), ttl);
  }
}
