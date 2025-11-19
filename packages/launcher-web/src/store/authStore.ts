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
import axios from 'axios';

interface User {
  id: string;
  email: string;
  username: string;
  createdAt: string;
}

interface AuthState {
  user: User | null;
  token: string | null;
  isAuthenticated: boolean;
  isLoading: boolean;
  error: string | null;

  initialize: () => Promise<void>;
  login: (email: string, password: string) => Promise<void>;
  register: (
    email: string,
    username: string,
    password: string
  ) => Promise<void>;
  logout: () => void;
  clearError: () => void;
}

export const useAuthStore = create<AuthState>((set) => ({
  user: null,
  token: null,
  isAuthenticated: false,
  isLoading: false,
  error: null,

  initialize: async () => {
    const token = localStorage.getItem('auth_token');
    if (token) {
      try {
        const response = await axios.get('/api/auth/me', {
          headers: { Authorization: `Bearer ${token}` },
        });
        set({ user: response.data, token, isAuthenticated: true });
        // eslint-disable-next-line @typescript-eslint/no-unused-vars
      } catch (_error) {
        localStorage.removeItem('auth_token');
        set({ user: null, token: null, isAuthenticated: false });
      }
    }
  },

  login: async (email: string, password: string) => {
    set({ isLoading: true, error: null });
    try {
      const response = await axios.post('/api/auth/login', { email, password });
      const { token, user } = response.data;
      localStorage.setItem('auth_token', token);
      set({ user, token, isAuthenticated: true, isLoading: false });
    } catch (error: unknown) {
      const errorMessage =
        error &&
        typeof error === 'object' &&
        'response' in error &&
        error.response &&
        typeof error.response === 'object' &&
        'data' in error.response &&
        error.response.data &&
        typeof error.response.data === 'object' &&
        'message' in error.response.data
          ? String(error.response.data.message)
          : 'Login failed';
      set({
        error: errorMessage,
        isLoading: false,
      });
      throw error;
    }
  },

  register: async (email: string, username: string, password: string) => {
    set({ isLoading: true, error: null });
    try {
      const response = await axios.post('/api/auth/register', {
        email,
        username,
        password,
      });
      const { token, user } = response.data;
      localStorage.setItem('auth_token', token);
      set({ user, token, isAuthenticated: true, isLoading: false });
    } catch (error: unknown) {
      const errorMessage =
        error &&
        typeof error === 'object' &&
        'response' in error &&
        error.response &&
        typeof error.response === 'object' &&
        'data' in error.response &&
        error.response.data &&
        typeof error.response.data === 'object' &&
        'message' in error.response.data
          ? String(error.response.data.message)
          : 'Registration failed';
      set({
        error: errorMessage,
        isLoading: false,
      });
      throw error;
    }
  },

  logout: () => {
    localStorage.removeItem('auth_token');
    set({ user: null, token: null, isAuthenticated: false });
  },

  clearError: () => set({ error: null }),
}));
