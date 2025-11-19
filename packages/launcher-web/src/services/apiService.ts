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

import axios, { AxiosInstance } from 'axios';
import {
  getMockGames,
  getMockFeaturedGames,
  getMockNewGames,
  getMockTopRatedGames,
  getMockGameById,
} from './mockData';

const USE_MOCK_DATA = import.meta.env.MODE === 'development' || true; // Always use mock data for now

class APIService {
  private api: AxiosInstance;

  constructor() {
    this.api = axios.create({
      baseURL: '/api',
      headers: {
        'Content-Type': 'application/json',
      },
    });

    // Add auth token to requests
    this.api.interceptors.request.use((config) => {
      const token = localStorage.getItem('auth_token');
      if (token) {
        config.headers.Authorization = `Bearer ${token}`;
      }
      return config;
    });
  }

  // Game APIs
  async getGames(params?: {
    category?: string;
    search?: string;
    sort?: string;
    limit?: number;
    offset?: number;
  }) {
    if (USE_MOCK_DATA) {
      // Simulate network delay
      await new Promise((resolve) => setTimeout(resolve, 300));
      return getMockGames(params?.category, params?.search);
    }
    const response = await this.api.get('/games', { params });
    return response.data;
  }

  async getFeaturedGames() {
    if (USE_MOCK_DATA) {
      await new Promise((resolve) => setTimeout(resolve, 300));
      return getMockFeaturedGames();
    }
    const response = await this.api.get('/games/featured');
    return response.data;
  }

  async getNewGames() {
    if (USE_MOCK_DATA) {
      await new Promise((resolve) => setTimeout(resolve, 300));
      return getMockNewGames();
    }
    const response = await this.api.get('/games/new');
    return response.data;
  }

  async getTopRatedGames() {
    if (USE_MOCK_DATA) {
      await new Promise((resolve) => setTimeout(resolve, 300));
      return getMockTopRatedGames();
    }
    const response = await this.api.get('/games/top-rated');
    return response.data;
  }

  async getGameById(id: string) {
    if (USE_MOCK_DATA) {
      await new Promise((resolve) => setTimeout(resolve, 200));
      const game = getMockGameById(id);
      if (!game) throw new Error('Game not found');
      return game;
    }
    const response = await this.api.get(`/games/${id}`);
    return response.data;
  }

  async playGame(id: string) {
    const response = await this.api.get(`/games/${id}/play`);
    return response.data;
  }

  async downloadGame(id: string, platform: string) {
    const response = await this.api.get(`/games/${id}/download/${platform}`, {
      responseType: 'blob',
    });
    return response.data;
  }

  async rateGame(id: string, rating: number) {
    const response = await this.api.post(`/games/${id}/rate`, { rating });
    return response.data;
  }

  // User APIs
  async getUserProfile() {
    const response = await this.api.get('/auth/me');
    return response.data;
  }

  async getUserLibrary() {
    const response = await this.api.get('/library');
    return response.data;
  }

  async addToLibrary(gameId: string) {
    const response = await this.api.post('/library', { gameId });
    return response.data;
  }

  async removeFromLibrary(gameId: string) {
    const response = await this.api.delete(`/library/${gameId}`);
    return response.data;
  }

  // Multiplayer APIs
  async createRoom(gameId: string, maxPlayers: number) {
    const response = await this.api.post('/multiplayer/rooms', {
      gameId,
      maxPlayers,
    });
    return response.data;
  }

  async getRooms(gameId: string) {
    const response = await this.api.get(`/multiplayer/rooms/${gameId}`);
    return response.data;
  }

  async joinRoom(roomId: string) {
    const response = await this.api.post('/multiplayer/rooms/join', { roomId });
    return response.data;
  }

  async quickMatch(gameId: string) {
    const response = await this.api.post('/multiplayer/quickmatch', { gameId });
    return response.data;
  }
}

export const apiService = new APIService();
