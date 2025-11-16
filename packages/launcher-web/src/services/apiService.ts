import axios, { AxiosInstance } from 'axios';

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
    const response = await this.api.get('/games', { params });
    return response.data;
  }

  async getFeaturedGames() {
    const response = await this.api.get('/games/featured');
    return response.data;
  }

  async getNewGames() {
    const response = await this.api.get('/games/new');
    return response.data;
  }

  async getTopRatedGames() {
    const response = await this.api.get('/games/top-rated');
    return response.data;
  }

  async getGameById(id: string) {
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
