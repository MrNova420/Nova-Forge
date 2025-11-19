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
 * Hub Module - Game Discovery & Browsing
 *
 * Complete implementation of game discovery interface with:
 * - Featured games carousel
 * - Category filtering
 * - Search functionality
 * - Game cards with ratings
 * - New releases
 * - Top rated games
 */

import React, { useState, useEffect } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';
import { getAllDemoGames, type DemoGame } from '../demo-games';

interface Game {
  id: string;
  name: string;
  description: string;
  developer: string;
  category: string;
  rating: number; // Real rating from user reviews
  downloads: number; // Real download/play count
  thumbnail: string;
  screenshots: string[];
  releaseDate: string;
  version: string;
  isDemo?: boolean; // Flag for demo games
  // Add demo game data
  demoGame?: DemoGame;
}

interface HubModuleProps {
  platform: UnifiedPlatformCore;
}

export const HubModule: React.FC<HubModuleProps> = ({ platform }) => {
  const [games, setGames] = useState<Game[]>([]);
  const [featuredGames, setFeaturedGames] = useState<Game[]>([]);
  const [category, setCategory] = useState<string>('all');
  const [searchQuery, setSearchQuery] = useState('');
  const [loading, setLoading] = useState(true);

  const categories = [
    'all',
    'action',
    'adventure',
    'rpg',
    'strategy',
    'puzzle',
    'simulation',
    'sports',
    'racing',
  ];

  useEffect(() => {
    loadGames();
    loadFeaturedGames();
  }, [category, searchQuery]);

  const loadGames = async () => {
    setLoading(true);
    try {
      // Load REAL demo games built with Nova Engine
      const demoGames = getAllDemoGames();

      // Fetch real stats from backend (optional, fallback to defaults)
      let statsMap = new Map();
      try {
        const statsResponse = await fetch('/api/games-stats/all');
        if (statsResponse.ok) {
          const statsData = await statsResponse.json();
          statsMap = new Map(statsData.map((s: any) => [s.gameId, s]));
        }
      } catch (error) {
        console.warn('Could not load game stats:', error);
      }

      const gameList: Game[] = demoGames.map((demo) => {
        const stats = statsMap.get(demo.id) as
          | { downloads?: number; averageRating?: number }
          | undefined;
        return {
          id: demo.id,
          name: demo.title,
          description: demo.description,
          developer: 'Nova Engine Studios',
          category: demo.category.toLowerCase(),
          rating: stats?.averageRating || 0, // Real rating from backend
          downloads: stats?.downloads || 0, // Real download count from backend
          isDemo: demo.isDemo,
          thumbnail: demo.coverImage,
          screenshots: [demo.coverImage],
          releaseDate: demo.lastUpdated,
          version: demo.version,
          demoGame: demo, // Store the actual game object
        };
      });

      // Filter by category
      let filtered = gameList;
      if (category !== 'all') {
        filtered = filtered.filter(
          (g) => g.category.toLowerCase() === category.toLowerCase()
        );
      }

      // Filter by search
      if (searchQuery) {
        const query = searchQuery.toLowerCase();
        filtered = filtered.filter(
          (g) =>
            g.name.toLowerCase().includes(query) ||
            g.description.toLowerCase().includes(query)
        );
      }

      setGames(filtered);

      platform.showNotification({
        type: 'success',
        message: `Loaded ${filtered.length} demo games built with Nova Engine`,
      });
    } catch (error) {
      console.error('Failed to load games:', error);
      platform.showNotification({
        type: 'error',
        message: 'Failed to load games',
      });
    } finally {
      setLoading(false);
    }
  };

  const loadFeaturedGames = async () => {
    try {
      // Featured = demo games with real stats
      const demoGames = getAllDemoGames();

      // Fetch real stats from backend (optional, fallback to defaults)
      let statsMap = new Map();
      try {
        const statsResponse = await fetch('/api/games-stats/all');
        if (statsResponse.ok) {
          const statsData = await statsResponse.json();
          statsMap = new Map(statsData.map((s: any) => [s.gameId, s]));
        }
      } catch (error) {
        console.warn('Could not load game stats:', error);
      }

      const featured: Game[] = demoGames
        .map((demo) => {
          const stats = statsMap.get(demo.id) as
            | { downloads?: number; averageRating?: number }
            | undefined;
          return {
            id: demo.id,
            name: demo.title,
            description: demo.description,
            developer: 'Nova Engine Studios',
            category: demo.category.toLowerCase(),
            rating: stats?.averageRating || 0, // Real rating from backend
            downloads: stats?.downloads || 0, // Real downloads from backend
            thumbnail: demo.coverImage,
            screenshots: [demo.coverImage],
            releaseDate: demo.lastUpdated,
            version: demo.version,
            demoGame: demo,
          };
        })
        .slice(0, 3); // Show first 3 games

      setFeaturedGames(featured);
    } catch (error) {
      console.error('Failed to load featured games:', error);
    }
  };

  const handleGameClick = (game: Game) => {
    platform.emit('gameSelected', game);
    platform.showNotification({
      type: 'info',
      message: `Selected: ${game.name}`,
    });
  };

  const handlePlayGame = async (game: Game) => {
    if (game.demoGame) {
      // Track real play/download in backend
      try {
        await fetch('/api/games-stats/play', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ gameId: game.id }),
        });
      } catch (error) {
        console.error('Failed to track game play:', error);
      }

      // Pass the REAL game object to launcher
      platform.switchMode('launcher');
      platform.emit('playGame', {
        gameId: game.id,
        game: game.demoGame, // Pass actual DemoGame object
      });
      platform.showNotification({
        type: 'success',
        message: `Launching ${game.name} with Nova Engine...`,
      });
    } else {
      platform.showNotification({
        type: 'error',
        message: 'Game not available',
      });
    }
  };

  return (
    <div className="hub-module">
      {/* Hero Section with Featured Games */}
      <section className="hub-hero">
        <h1>Discover Amazing Games</h1>
        <p>Browse thousands of games created with Nova Engine</p>

        {/* Featured Games Carousel */}
        <div className="featured-carousel">
          {featuredGames.map((game) => (
            <div
              key={game.id}
              className="featured-game"
              onClick={() => handleGameClick(game)}
            >
              <img src={game.thumbnail} alt={game.name} />
              <div className="featured-info">
                <h2>{game.name}</h2>
                <p>{game.description}</p>
                <div className="featured-meta">
                  <span className="rating">
                    {game.rating > 0
                      ? `⭐ ${game.rating.toFixed(1)}`
                      : '⭐ Not rated yet'}
                  </span>
                  <span className="downloads">
                    📥 {game.downloads.toLocaleString()}{' '}
                    {game.downloads === 0 ? 'downloads' : 'plays'}
                  </span>
                </div>
                <button
                  className="play-btn"
                  onClick={(e) => {
                    e.stopPropagation();
                    handlePlayGame(game);
                  }}
                >
                  Play Now
                </button>
              </div>
            </div>
          ))}
        </div>
      </section>

      {/* Filter Bar */}
      <section className="hub-filters">
        <div className="category-filters">
          {categories.map((cat) => (
            <button
              key={cat}
              className={`category-btn ${category === cat ? 'active' : ''}`}
              onClick={() => setCategory(cat)}
            >
              {cat.charAt(0).toUpperCase() + cat.slice(1)}
            </button>
          ))}
        </div>

        <div className="search-bar">
          <input
            type="text"
            placeholder="Search games..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
          <button className="search-btn">🔍</button>
        </div>
      </section>

      {/* Game Grid */}
      <section className="hub-games">
        {loading ? (
          <div className="loading">Loading games...</div>
        ) : (
          <div className="game-grid">
            {games.map((game) => (
              <div
                key={game.id}
                className="game-card"
                onClick={() => handleGameClick(game)}
              >
                <div className="game-thumbnail">
                  <img src={game.thumbnail} alt={game.name} />
                  <div className="game-overlay">
                    <button
                      className="quick-play"
                      onClick={(e) => {
                        e.stopPropagation();
                        handlePlayGame(game);
                      }}
                    >
                      ▶ Play
                    </button>
                  </div>
                </div>
                <div className="game-info">
                  <h3>{game.name}</h3>
                  <p className="game-developer">{game.developer}</p>
                  <div className="game-meta">
                    <span className="category">{game.category}</span>
                    <span className="rating">
                      {game.rating > 0
                        ? `⭐ ${game.rating.toFixed(1)}`
                        : '⭐ Not rated'}
                    </span>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </section>

      {/* New Releases Section */}
      <section className="hub-section">
        <h2>New Releases</h2>
        <div className="game-horizontal-scroll">
          {games
            .sort(
              (a, b) =>
                new Date(b.releaseDate).getTime() -
                new Date(a.releaseDate).getTime()
            )
            .slice(0, 10)
            .map((game) => (
              <div
                key={game.id}
                className="game-card-small"
                onClick={() => handleGameClick(game)}
              >
                <img src={game.thumbnail} alt={game.name} />
                <div className="game-info-small">
                  <h4>{game.name}</h4>
                  <span className="rating">
                    {game.rating > 0
                      ? `⭐ ${game.rating.toFixed(1)}`
                      : '⭐ Not rated'}
                  </span>
                </div>
              </div>
            ))}
        </div>
      </section>

      {/* Top Rated Section */}
      <section className="hub-section">
        <h2>Top Rated</h2>
        <div className="game-horizontal-scroll">
          {games
            .sort((a, b) => b.rating - a.rating)
            .slice(0, 10)
            .map((game) => (
              <div
                key={game.id}
                className="game-card-small"
                onClick={() => handleGameClick(game)}
              >
                <img src={game.thumbnail} alt={game.name} />
                <div className="game-info-small">
                  <h4>{game.name}</h4>
                  <span className="rating">
                    {game.rating > 0
                      ? `⭐ ${game.rating.toFixed(1)}`
                      : '⭐ Not rated'}
                  </span>
                </div>
              </div>
            ))}
        </div>
      </section>

      {/* Trending Section */}
      <section className="hub-section">
        <h2>Trending Now</h2>
        <div className="game-horizontal-scroll">
          {games
            .sort((a, b) => b.downloads - a.downloads)
            .slice(0, 10)
            .map((game) => (
              <div
                key={game.id}
                className="game-card-small"
                onClick={() => handleGameClick(game)}
              >
                <img src={game.thumbnail} alt={game.name} />
                <div className="game-info-small">
                  <h4>{game.name}</h4>
                  <span className="downloads">
                    📥 {game.downloads.toLocaleString()}
                  </span>
                </div>
              </div>
            ))}
        </div>
      </section>

      <style>{`
        .hub-module {
          width: 100%;
          height: 100%;
          overflow-y: auto;
          padding: 20px;
          background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
          color: #ffffff;
        }

        .hub-hero {
          margin-bottom: 40px;
          text-align: center;
        }

        .hub-hero h1 {
          font-size: 3em;
          margin-bottom: 10px;
          background: linear-gradient(45deg, #00d4ff, #7b2ff7);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
        }

        .featured-carousel {
          display: flex;
          gap: 20px;
          overflow-x: auto;
          padding: 20px 0;
        }

        .featured-game {
          min-width: 600px;
          height: 300px;
          border-radius: 12px;
          overflow: hidden;
          cursor: pointer;
          position: relative;
          transition: transform 0.3s;
        }

        .featured-game:hover {
          transform: scale(1.02);
        }

        .featured-game img {
          width: 100%;
          height: 100%;
          object-fit: cover;
        }

        .featured-info {
          position: absolute;
          bottom: 0;
          left: 0;
          right: 0;
          padding: 20px;
          background: linear-gradient(transparent, rgba(0, 0, 0, 0.9));
        }

        .hub-filters {
          display: flex;
          justify-content: space-between;
          margin-bottom: 30px;
          gap: 20px;
        }

        .category-filters {
          display: flex;
          gap: 10px;
          flex-wrap: wrap;
        }

        .category-btn {
          padding: 8px 16px;
          border: none;
          border-radius: 20px;
          background: rgba(255, 255, 255, 0.1);
          color: white;
          cursor: pointer;
          transition: all 0.3s;
        }

        .category-btn.active {
          background: #7b2ff7;
        }

        .game-grid {
          display: grid;
          grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
          gap: 20px;
          margin-bottom: 40px;
        }

        .game-card {
          background: rgba(255, 255, 255, 0.05);
          border-radius: 12px;
          overflow: hidden;
          cursor: pointer;
          transition: transform 0.3s;
        }

        .game-card:hover {
          transform: translateY(-5px);
        }

        .game-thumbnail {
          width: 100%;
          height: 180px;
          position: relative;
        }

        .game-thumbnail img {
          width: 100%;
          height: 100%;
          object-fit: cover;
        }

        .game-overlay {
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          background: rgba(0, 0, 0, 0.6);
          display: flex;
          align-items: center;
          justify-content: center;
          opacity: 0;
          transition: opacity 0.3s;
        }

        .game-card:hover .game-overlay {
          opacity: 1;
        }

        .quick-play {
          padding: 10px 20px;
          background: #7b2ff7;
          border: none;
          border-radius: 20px;
          color: white;
          cursor: pointer;
          font-size: 16px;
        }

        .game-info {
          padding: 15px;
        }

        .game-info h3 {
          margin: 0 0 5px 0;
          font-size: 1.2em;
        }

        .game-developer {
          color: #888;
          font-size: 0.9em;
          margin: 0 0 10px 0;
        }

        .game-meta {
          display: flex;
          justify-content: space-between;
          align-items: center;
        }

        .hub-section {
          margin-bottom: 40px;
        }

        .hub-section h2 {
          margin-bottom: 20px;
          font-size: 1.8em;
        }

        .game-horizontal-scroll {
          display: flex;
          gap: 15px;
          overflow-x: auto;
        }

        .game-card-small {
          min-width: 180px;
          cursor: pointer;
          transition: transform 0.3s;
        }

        .game-card-small:hover {
          transform: scale(1.05);
        }

        .game-card-small img {
          width: 100%;
          height: 120px;
          object-fit: cover;
          border-radius: 8px;
        }

        .game-info-small {
          padding: 10px 0;
        }

        .game-info-small h4 {
          margin: 0 0 5px 0;
          font-size: 0.9em;
        }
      `}</style>
    </div>
  );
};
