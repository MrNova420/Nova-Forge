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

import React, { useState } from 'react';
import { useQuery } from '@tanstack/react-query';
import { apiService } from '../services/apiService';
import GameGrid from '../components/GameGrid';

const LibraryPage: React.FC = () => {
  const [activeTab, setActiveTab] = useState<'all' | 'played' | 'favorites'>('all');

  const { data: libraryGames = [], isLoading } = useQuery({
    queryKey: ['library'],
    queryFn: () => apiService.getUserLibrary(),
  });

  const filteredGames = React.useMemo(() => {
    switch (activeTab) {
      case 'played':
        return libraryGames.filter((g: any) => g.lastPlayed);
      case 'favorites':
        return libraryGames.filter((g: any) => g.isFavorite);
      default:
        return libraryGames;
    }
  }, [libraryGames, activeTab]);

  return (
    <div className="container py-8">
      <h1 className="text-4xl font-bold mb-8">My Library</h1>

      {/* Tabs */}
      <div className="flex gap-4 mb-8 border-b border-border">
        <button
          onClick={() => setActiveTab('all')}
          className={`px-6 py-3 font-semibold transition ${
            activeTab === 'all'
              ? 'border-b-2 border-accent text-accent'
              : 'text-text-secondary hover:text-text-primary'
          }`}
        >
          All Games ({libraryGames.length})
        </button>
        <button
          onClick={() => setActiveTab('played')}
          className={`px-6 py-3 font-semibold transition ${
            activeTab === 'played'
              ? 'border-b-2 border-accent text-accent'
              : 'text-text-secondary hover:text-text-primary'
          }`}
        >
          Recently Played
        </button>
        <button
          onClick={() => setActiveTab('favorites')}
          className={`px-6 py-3 font-semibold transition ${
            activeTab === 'favorites'
              ? 'border-b-2 border-accent text-accent'
              : 'text-text-secondary hover:text-text-primary'
          }`}
        >
          Favorites
        </button>
      </div>

      {/* Games Grid */}
      <GameGrid games={filteredGames} isLoading={isLoading} />

      {/* Empty State */}
      {!isLoading && filteredGames.length === 0 && (
        <div className="text-center py-12">
          <div className="text-6xl mb-4">📚</div>
          <h2 className="text-2xl font-bold mb-2">Your library is empty</h2>
          <p className="text-text-secondary mb-6">
            Start playing games to add them to your library
          </p>
          <a href="/games" className="btn btn-primary">
            Browse Games
          </a>
        </div>
      )}
    </div>
  );
};

export default LibraryPage;
