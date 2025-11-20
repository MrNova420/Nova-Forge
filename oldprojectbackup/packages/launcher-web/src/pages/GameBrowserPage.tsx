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
import { useSearchParams } from 'react-router-dom';
import { useQuery } from '@tanstack/react-query';
import { apiService } from '../services/apiService';
import GameGrid from '../components/GameGrid';

const CATEGORIES = [
  'All',
  'Action',
  'Adventure',
  'RPG',
  'Strategy',
  'Puzzle',
  'Platformer',
  'Shooter',
  'Racing',
  'Sports',
  'Simulation',
];

const GameBrowserPage: React.FC = () => {
  const [searchParams, setSearchParams] = useSearchParams();
  const [searchQuery, setSearchQuery] = useState(searchParams.get('q') || '');
  const [selectedCategory, setSelectedCategory] = useState(
    searchParams.get('category') || 'All'
  );

  const { data: games = [], isLoading } = useQuery({
    queryKey: ['games', selectedCategory, searchQuery],
    queryFn: () =>
      apiService.getGames({
        category: selectedCategory !== 'All' ? selectedCategory : undefined,
        search: searchQuery || undefined,
      }),
  });

  const handleSearch = (e: React.FormEvent) => {
    e.preventDefault();
    setSearchParams({
      ...(selectedCategory !== 'All' && { category: selectedCategory }),
      ...(searchQuery && { q: searchQuery }),
    });
  };

  const handleCategoryChange = (category: string) => {
    setSelectedCategory(category);
    setSearchParams({
      ...(category !== 'All' && { category }),
      ...(searchQuery && { q: searchQuery }),
    });
  };

  return (
    <div className="container py-8">
      <h1 className="text-4xl font-bold mb-8">Browse Games</h1>

      {/* Search and Filters */}
      <div className="mb-8">
        <form onSubmit={handleSearch} className="mb-4">
          <div className="flex gap-4">
            <input
              type="text"
              placeholder="Search games..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="flex-1 px-4 py-3 bg-bg-secondary border border-border rounded-lg focus:border-accent"
            />
            <button type="submit" className="btn btn-primary">
              🔍 Search
            </button>
          </div>
        </form>

        {/* Category Filters */}
        <div className="flex gap-2 flex-wrap">
          {CATEGORIES.map((category) => (
            <button
              key={category}
              onClick={() => handleCategoryChange(category)}
              className={`px-4 py-2 rounded-lg transition ${
                selectedCategory === category
                  ? 'bg-accent text-white'
                  : 'bg-bg-secondary hover:bg-bg-tertiary'
              }`}
            >
              {category}
            </button>
          ))}
        </div>
      </div>

      {/* Results Count */}
      {!isLoading && (
        <p className="text-text-secondary mb-4">
          Found {games.length} game{games.length !== 1 ? 's' : ''}
        </p>
      )}

      {/* Games Grid */}
      <GameGrid games={games} isLoading={isLoading} />
    </div>
  );
};

export default GameBrowserPage;
