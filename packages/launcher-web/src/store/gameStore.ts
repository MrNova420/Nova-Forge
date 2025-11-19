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

import { create } from 'zustand';

export interface Game {
  id: string;
  title: string;
  description: string;
  thumbnail: string;
  screenshots: string[];
  author: string;
  category: string;
  tags: string[];
  playCount: number;
  downloadCount: number;
  rating: number;
  ratingCount: number;
  version: string;
  platforms: string[];
  publishedAt: string;
  size: number;
}

interface GameState {
  games: Game[];
  selectedGame: Game | null;
  isLoading: boolean;
  error: string | null;
  searchQuery: string;
  categoryFilter: string | null;
  
  setGames: (games: Game[]) => void;
  setSelectedGame: (game: Game | null) => void;
  setSearchQuery: (query: string) => void;
  setCategoryFilter: (category: string | null) => void;
  setLoading: (loading: boolean) => void;
  setError: (error: string | null) => void;
  clearError: () => void;
}

export const useGameStore = create<GameState>((set) => ({
  games: [],
  selectedGame: null,
  isLoading: false,
  error: null,
  searchQuery: '',
  categoryFilter: null,
  
  setGames: (games) => set({ games }),
  setSelectedGame: (game) => set({ selectedGame: game }),
  setSearchQuery: (query) => set({ searchQuery: query }),
  setCategoryFilter: (category) => set({ categoryFilter: category }),
  setLoading: (loading) => set({ isLoading: loading }),
  setError: (error) => set({ error }),
  clearError: () => set({ error: null }),
}));
