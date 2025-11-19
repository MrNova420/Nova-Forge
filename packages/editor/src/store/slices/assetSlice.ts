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
 * Asset Slice
 * Manages asset browser state and asset metadata
 */

import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export interface Asset {
  id: string;
  name: string;
  type: 'folder' | 'model' | 'texture' | 'audio' | 'material' | 'script' | 'scene' | 'prefab';
  path: string;
  size: number;
  modifiedDate: number;
  metadata?: Record<string, any>;
}

export interface AssetState {
  assets: Record<string, Asset>;
  currentPath: string;
  selectedAssetIds: string[];
  viewMode: 'grid' | 'list';
  sortBy: 'name' | 'type' | 'size' | 'date';
  sortOrder: 'asc' | 'desc';
  searchQuery: string;
  filterType: string | null;
  recentAssets: string[];
  favorites: string[];
}

const initialState: AssetState = {
  assets: {},
  currentPath: '/',
  selectedAssetIds: [],
  viewMode: 'grid',
  sortBy: 'name',
  sortOrder: 'asc',
  searchQuery: '',
  filterType: null,
  recentAssets: [],
  favorites: [],
};

const assetSlice = createSlice({
  name: 'asset',
  initialState,
  reducers: {
    addAsset: (state, action: PayloadAction<Asset>) => {
      state.assets[action.payload.id] = action.payload;
    },

    removeAsset: (state, action: PayloadAction<string>) => {
      delete state.assets[action.payload];
      state.selectedAssetIds = state.selectedAssetIds.filter(id => id !== action.payload);
      state.recentAssets = state.recentAssets.filter(id => id !== action.payload);
      state.favorites = state.favorites.filter(id => id !== action.payload);
    },

    updateAsset: (state, action: PayloadAction<{ id: string; updates: Partial<Asset> }>) => {
      const { id, updates } = action.payload;
      const asset = state.assets[id];
      
      if (asset) {
        Object.assign(asset, updates);
      }
    },

    setCurrentPath: (state, action: PayloadAction<string>) => {
      state.currentPath = action.payload;
      state.selectedAssetIds = [];
    },

    selectAsset: (state, action: PayloadAction<string>) => {
      state.selectedAssetIds = [action.payload];
      
      // Add to recent assets
      if (!state.recentAssets.includes(action.payload)) {
        state.recentAssets.unshift(action.payload);
        if (state.recentAssets.length > 20) {
          state.recentAssets.pop();
        }
      }
    },

    selectAssets: (state, action: PayloadAction<string[]>) => {
      state.selectedAssetIds = action.payload;
    },

    toggleAssetSelection: (state, action: PayloadAction<string>) => {
      const index = state.selectedAssetIds.indexOf(action.payload);
      if (index >= 0) {
        state.selectedAssetIds.splice(index, 1);
      } else {
        state.selectedAssetIds.push(action.payload);
      }
    },

    clearAssetSelection: (state) => {
      state.selectedAssetIds = [];
    },

    setViewMode: (state, action: PayloadAction<'grid' | 'list'>) => {
      state.viewMode = action.payload;
    },

    setSortBy: (state, action: PayloadAction<'name' | 'type' | 'size' | 'date'>) => {
      state.sortBy = action.payload;
    },

    setSortOrder: (state, action: PayloadAction<'asc' | 'desc'>) => {
      state.sortOrder = action.payload;
    },

    setSearchQuery: (state, action: PayloadAction<string>) => {
      state.searchQuery = action.payload;
    },

    setFilterType: (state, action: PayloadAction<string | null>) => {
      state.filterType = action.payload;
    },

    addToFavorites: (state, action: PayloadAction<string>) => {
      if (!state.favorites.includes(action.payload)) {
        state.favorites.push(action.payload);
      }
    },

    removeFromFavorites: (state, action: PayloadAction<string>) => {
      state.favorites = state.favorites.filter(id => id !== action.payload);
    },

    toggleFavorite: (state, action: PayloadAction<string>) => {
      const index = state.favorites.indexOf(action.payload);
      if (index >= 0) {
        state.favorites.splice(index, 1);
      } else {
        state.favorites.push(action.payload);
      }
    },

    clearRecentAssets: (state) => {
      state.recentAssets = [];
    },

    loadAssets: (state, action: PayloadAction<Asset[]>) => {
      action.payload.forEach(asset => {
        state.assets[asset.id] = asset;
      });
    },

    clearAssets: (state) => {
      state.assets = {};
      state.selectedAssetIds = [];
    },
  },
});

export const {
  addAsset,
  removeAsset,
  updateAsset,
  setCurrentPath,
  selectAsset,
  selectAssets,
  toggleAssetSelection,
  clearAssetSelection,
  setViewMode,
  setSortBy,
  setSortOrder,
  setSearchQuery,
  setFilterType,
  addToFavorites,
  removeFromFavorites,
  toggleFavorite,
  clearRecentAssets,
  loadAssets,
  clearAssets,
} = assetSlice.actions;

export default assetSlice.reducer;
