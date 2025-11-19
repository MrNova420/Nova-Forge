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
 * NOVA ENGINE - HUB DASHBOARD
 * Redesigned to match Image 2 mockup - Hub Dashboard panel
 * Features orbital planet design with card-based game discovery
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
  rating: number;
  downloads: number;
  thumbnail: string;
  screenshots: string[];
  releaseDate: string;
  version: string;
  isDemo?: boolean;
  demoGame?: DemoGame;
}

interface HubModuleProps {
  platform: UnifiedPlatformCore;
}

export const HubModuleRedesigned: React.FC<HubModuleProps> = ({ platform }) => {
  const [games, setGames] = useState<Game[]>([]);
  const [featuredGames, setFeaturedGames] = useState<Game[]>([]);
  const [selectedGame, setSelectedGame] = useState<Game | null>(null);
  const [category, setCategory] = useState<string>('all');
  const [searchQuery, setSearchQuery] = useState('');
  const [sortBy, setSortBy] = useState<'popular' | 'recent' | 'rating'>(
    'popular'
  );
  const [viewMode, setViewMode] = useState<'grid' | 'list'>('grid');
  const [loading, setLoading] = useState(true);

  const categories = [
    { id: 'all', name: 'All Games', icon: '🎮' },
    { id: 'action', name: 'Action', icon: '⚔️' },
    { id: 'adventure', name: 'Adventure', icon: '🗺️' },
    { id: 'rpg', name: 'RPG', icon: '🐉' },
    { id: 'strategy', name: 'Strategy', icon: '♟️' },
    { id: 'puzzle', name: 'Puzzle', icon: '🧩' },
    { id: 'simulation', name: 'Simulation', icon: '🌍' },
    { id: 'sports', name: 'Sports', icon: '⚽' },
    { id: 'racing', name: 'Racing', icon: '🏎️' },
  ];

  useEffect(() => {
    loadGames();
    loadFeaturedGames();
  }, [category, searchQuery, sortBy]);

  const loadGames = async () => {
    setLoading(true);
    try {
      const demoGames = getAllDemoGames();
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

      let gameList: Game[] = demoGames.map((demo) => {
        const stats = statsMap.get(demo.id) as
          | { downloads?: number; averageRating?: number }
          | undefined;
        return {
          id: demo.id,
          name: demo.title,
          description: demo.description,
          developer: 'Nova Engine Studios',
          category: demo.category.toLowerCase(),
          rating: stats?.averageRating || 4.5,
          downloads: stats?.downloads || Math.floor(Math.random() * 10000),
          isDemo: demo.isDemo,
          thumbnail: demo.coverImage,
          screenshots: [demo.coverImage],
          releaseDate: demo.lastUpdated,
          version: demo.version,
          demoGame: demo,
        };
      });

      // Filter by category
      if (category !== 'all') {
        gameList = gameList.filter((g) => g.category === category);
      }

      // Filter by search
      if (searchQuery) {
        const query = searchQuery.toLowerCase();
        gameList = gameList.filter(
          (g) =>
            g.name.toLowerCase().includes(query) ||
            g.description.toLowerCase().includes(query)
        );
      }

      // Sort
      if (sortBy === 'popular') {
        gameList.sort((a, b) => b.downloads - a.downloads);
      } else if (sortBy === 'recent') {
        gameList.sort(
          (a, b) =>
            new Date(b.releaseDate).getTime() -
            new Date(a.releaseDate).getTime()
        );
      } else if (sortBy === 'rating') {
        gameList.sort((a, b) => b.rating - a.rating);
      }

      setGames(gameList);
    } catch (error) {
      console.error('Failed to load games:', error);
    } finally {
      setLoading(false);
    }
  };

  const loadFeaturedGames = async () => {
    const demoGames = getAllDemoGames();
    const featured = demoGames.slice(0, 3).map((demo) => ({
      id: demo.id,
      name: demo.title,
      description: demo.description,
      developer: 'Nova Engine Studios',
      category: demo.category.toLowerCase(),
      rating: 4.8,
      downloads: Math.floor(Math.random() * 50000),
      thumbnail: demo.coverImage,
      screenshots: [demo.coverImage],
      releaseDate: demo.lastUpdated,
      version: demo.version,
      demoGame: demo,
    }));
    setFeaturedGames(featured);
  };

  const handlePlayGame = async (game: Game) => {
    if (game.demoGame) {
      try {
        await fetch('/api/games-stats/play', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ gameId: game.id }),
        });
      } catch (error) {
        console.error('Failed to track game play:', error);
      }

      platform.switchMode('launcher');
      platform.emit('playGame', { gameId: game.id, game: game.demoGame });
      platform.showNotification({
        type: 'success',
        message: `Launching ${game.name}...`,
      });
    }
  };

  return (
    <div className="hub-redesigned">
      {/* Hero Section with Featured Games */}
      <section className="hub-hero">
        <div className="hero-background">
          <div className="stars-bg"></div>
        </div>
        <div className="hero-content">
          <div className="hero-branding">
            <h1 className="nova-logo-hero">NOVA</h1>
            <span className="engine-text-hero">ENGINE</span>
            <p className="hero-tagline">Discover Amazing Games</p>
          </div>

          {/* Featured Carousel */}
          <div className="featured-carousel">
            {featuredGames.map((game, idx) => (
              <div
                key={game.id}
                className="featured-card"
                style={{ '--index': idx } as any}
              >
                <div className="featured-image">
                  <img src={game.thumbnail} alt={game.name} />
                  <div className="featured-overlay">
                    <div className="featured-info">
                      <h2>{game.name}</h2>
                      <p>{game.description}</p>
                      <div className="featured-meta">
                        <span className="rating">
                          ⭐ {game.rating.toFixed(1)}
                        </span>
                        <span className="downloads">
                          📥 {game.downloads.toLocaleString()} plays
                        </span>
                      </div>
                      <button
                        className="play-featured-btn"
                        onClick={() => handlePlayGame(game)}
                      >
                        ▶️ Play Now
                      </button>
                    </div>
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      </section>

      {/* Search and Filters */}
      <section className="hub-filters">
        <div className="filter-container">
          {/* Search Bar */}
          <div className="search-bar">
            <span className="search-icon">🔍</span>
            <input
              type="text"
              placeholder="Search games..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>

          {/* View Mode Toggle */}
          <div className="view-toggle">
            <button
              className={viewMode === 'grid' ? 'active' : ''}
              onClick={() => setViewMode('grid')}
              title="Grid View"
            >
              ▦
            </button>
            <button
              className={viewMode === 'list' ? 'active' : ''}
              onClick={() => setViewMode('list')}
              title="List View"
            >
              ☰
            </button>
          </div>

          {/* Sort Options */}
          <div className="sort-options">
            <label>Sort by:</label>
            <select
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value as any)}
            >
              <option value="popular">Most Popular</option>
              <option value="recent">Recently Added</option>
              <option value="rating">Highest Rated</option>
            </select>
          </div>
        </div>
      </section>

      {/* Category Pills */}
      <section className="categories-section">
        <div className="categories-scroll">
          {categories.map((cat) => (
            <button
              key={cat.id}
              className={`category-pill ${category === cat.id ? 'active' : ''}`}
              onClick={() => setCategory(cat.id)}
            >
              <span className="cat-icon">{cat.icon}</span>
              <span>{cat.name}</span>
            </button>
          ))}
        </div>
      </section>

      {/* Games Grid/List */}
      <section className="games-section">
        {loading ? (
          <div className="loading-state">
            <div className="spinner-large"></div>
            <p>Loading games...</p>
          </div>
        ) : games.length === 0 ? (
          <div className="empty-state">
            <span className="empty-icon">🎮</span>
            <h3>No games found</h3>
            <p>Try adjusting your search or filters</p>
          </div>
        ) : (
          <div className={`games-${viewMode}`}>
            {games.map((game) => (
              <div
                key={game.id}
                className="game-card"
                onClick={() => setSelectedGame(game)}
              >
                <div className="game-thumbnail">
                  <img src={game.thumbnail} alt={game.name} />
                  <div className="game-overlay">
                    <button
                      className="quick-play-btn"
                      onClick={(e) => {
                        e.stopPropagation();
                        handlePlayGame(game);
                      }}
                    >
                      ▶️ Play
                    </button>
                  </div>
                </div>
                <div className="game-info">
                  <h3>{game.name}</h3>
                  <p className="game-developer">{game.developer}</p>
                  <p className="game-description">{game.description}</p>
                  <div className="game-stats">
                    <span className="stat">
                      <span className="stat-icon">⭐</span>
                      {game.rating.toFixed(1)}
                    </span>
                    <span className="stat">
                      <span className="stat-icon">📥</span>
                      {game.downloads.toLocaleString()}
                    </span>
                    <span className="stat">
                      <span className="stat-icon">📅</span>
                      {new Date(game.releaseDate).toLocaleDateString()}
                    </span>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </section>

      {/* Game Detail Modal */}
      {selectedGame && (
        <div
          className="game-detail-modal"
          onClick={() => setSelectedGame(null)}
        >
          <div className="modal-content" onClick={(e) => e.stopPropagation()}>
            <button
              className="close-modal"
              onClick={() => setSelectedGame(null)}
            >
              ✕
            </button>
            <div className="modal-header">
              <img src={selectedGame.thumbnail} alt={selectedGame.name} />
              <div className="modal-header-info">
                <h2>{selectedGame.name}</h2>
                <p className="modal-developer">{selectedGame.developer}</p>
                <div className="modal-ratings">
                  <span className="large-rating">
                    ⭐ {selectedGame.rating.toFixed(1)}/5.0
                  </span>
                  <span className="large-downloads">
                    📥 {selectedGame.downloads.toLocaleString()} plays
                  </span>
                </div>
                <button
                  className="play-large-btn"
                  onClick={() => handlePlayGame(selectedGame)}
                >
                  ▶️ Play Game
                </button>
              </div>
            </div>
            <div className="modal-body">
              <h3>About This Game</h3>
              <p>{selectedGame.description}</p>
              <div className="modal-details">
                <div className="detail-item">
                  <span className="detail-label">Category</span>
                  <span className="detail-value">{selectedGame.category}</span>
                </div>
                <div className="detail-item">
                  <span className="detail-label">Version</span>
                  <span className="detail-value">{selectedGame.version}</span>
                </div>
                <div className="detail-item">
                  <span className="detail-label">Release Date</span>
                  <span className="detail-value">
                    {new Date(selectedGame.releaseDate).toLocaleDateString()}
                  </span>
                </div>
                <div className="detail-item">
                  <span className="detail-label">Engine</span>
                  <span className="detail-value">Nova Engine</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      )}

      <style>{`
        .hub-redesigned {
          width: 100%;
          min-height: 100vh;
          background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
          color: white;
          overflow-y: auto;
        }

        /* Hero Section */
        .hub-hero {
          position: relative;
          height: 500px;
          overflow: hidden;
          margin-bottom: 40px;
        }

        .hero-background {
          position: absolute;
          top: 0;
          left: 0;
          width: 100%;
          height: 100%;
          background: linear-gradient(180deg, transparent 0%, rgba(0,0,0,0.8) 100%);
        }

        .stars-bg {
          position: absolute;
          width: 100%;
          height: 100%;
          background-image: 
            radial-gradient(2px 2px at 20px 30px, white, transparent),
            radial-gradient(2px 2px at 60px 70px, white, transparent),
            radial-gradient(1px 1px at 50px 50px, white, transparent),
            radial-gradient(1px 1px at 130px 80px, white, transparent);
          background-size: 200px 200px;
          animation: twinkle 3s infinite;
        }

        .hero-content {
          position: relative;
          height: 100%;
          padding: 40px;
          display: flex;
          flex-direction: column;
          justify-content: space-between;
        }

        .hero-branding {
          text-align: center;
        }

        .nova-logo-hero {
          font-size: 72px;
          font-weight: 900;
          letter-spacing: 16px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          margin: 0;
          animation: glow 2s ease-in-out infinite alternate;
        }

        .engine-text-hero {
          display: block;
          font-size: 24px;
          font-weight: 300;
          letter-spacing: 8px;
          color: #a0a0ff;
          margin-top: 10px;
        }

        .hero-tagline {
          font-size: 20px;
          color: rgba(255, 255, 255, 0.7);
          margin-top: 20px;
        }

        /* Featured Carousel */
        .featured-carousel {
          display: grid;
          grid-template-columns: repeat(3, 1fr);
          gap: 20px;
          max-width: 1400px;
          margin: 0 auto;
        }

        .featured-card {
          position: relative;
          border-radius: 16px;
          overflow: hidden;
          transform: translateY(0);
          transition: transform 0.3s ease;
          animation: fadeInUp 0.6s ease calc(var(--index) * 0.1s) backwards;
        }

        .featured-card:hover {
          transform: translateY(-10px);
        }

        @keyframes fadeInUp {
          from {
            opacity: 0;
            transform: translateY(30px);
          }
          to {
            opacity: 1;
            transform: translateY(0);
          }
        }

        .featured-image {
          position: relative;
          height: 250px;
          overflow: hidden;
        }

        .featured-image img {
          width: 100%;
          height: 100%;
          object-fit: cover;
        }

        .featured-overlay {
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          background: linear-gradient(180deg, transparent 0%, rgba(0,0,0,0.95) 100%);
          display: flex;
          align-items: flex-end;
          opacity: 0;
          transition: opacity 0.3s ease;
        }

        .featured-card:hover .featured-overlay {
          opacity: 1;
        }

        .featured-info {
          padding: 20px;
          width: 100%;
        }

        .featured-info h2 {
          font-size: 24px;
          margin: 0 0 10px 0;
        }

        .featured-info p {
          font-size: 14px;
          color: rgba(255, 255, 255, 0.7);
          margin: 0 0 15px 0;
          display: -webkit-box;
          -webkit-line-clamp: 2;
          -webkit-box-orient: vertical;
          overflow: hidden;
        }

        .featured-meta {
          display: flex;
          gap: 20px;
          margin-bottom: 15px;
          font-size: 13px;
        }

        .play-featured-btn {
          width: 100%;
          padding: 12px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 25px;
          color: white;
          font-weight: 700;
          cursor: pointer;
          transition: transform 0.2s ease;
        }

        .play-featured-btn:hover {
          transform: scale(1.05);
        }

        /* Filters */
        .hub-filters {
          padding: 0 40px 20px 40px;
        }

        .filter-container {
          max-width: 1400px;
          margin: 0 auto;
          display: flex;
          gap: 20px;
          align-items: center;
        }

        .search-bar {
          flex: 1;
          position: relative;
        }

        .search-icon {
          position: absolute;
          left: 15px;
          top: 50%;
          transform: translateY(-50%);
          font-size: 18px;
        }

        .search-bar input {
          width: 100%;
          padding: 14px 14px 14px 45px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 25px;
          color: white;
          font-size: 16px;
          transition: all 0.3s ease;
        }

        .search-bar input:focus {
          outline: none;
          border-color: rgba(123, 47, 247, 0.8);
          box-shadow: 0 0 20px rgba(123, 47, 247, 0.3);
        }

        .view-toggle {
          display: flex;
          gap: 5px;
          background: rgba(26, 0, 51, 0.6);
          border-radius: 12px;
          padding: 5px;
        }

        .view-toggle button {
          width: 40px;
          height: 40px;
          background: transparent;
          border: none;
          color: rgba(255, 255, 255, 0.6);
          font-size: 20px;
          cursor: pointer;
          border-radius: 8px;
          transition: all 0.2s ease;
        }

        .view-toggle button.active {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          color: white;
        }

        .sort-options {
          display: flex;
          align-items: center;
          gap: 10px;
        }

        .sort-options label {
          font-size: 14px;
          color: rgba(255, 255, 255, 0.7);
        }

        .sort-options select {
          padding: 10px 15px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          cursor: pointer;
        }

        /* Categories */
        .categories-section {
          padding: 0 40px 30px 40px;
        }

        .categories-scroll {
          max-width: 1400px;
          margin: 0 auto;
          display: flex;
          gap: 10px;
          overflow-x: auto;
          padding: 10px 0;
        }

        .category-pill {
          display: flex;
          align-items: center;
          gap: 8px;
          padding: 12px 24px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 25px;
          color: white;
          cursor: pointer;
          white-space: nowrap;
          transition: all 0.3s ease;
        }

        .category-pill:hover {
          background: rgba(123, 47, 247, 0.3);
          border-color: rgba(123, 47, 247, 0.6);
        }

        .category-pill.active {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border-color: transparent;
        }

        .cat-icon {
          font-size: 20px;
        }

        /* Games Grid */
        .games-section {
          padding: 0 40px 60px 40px;
        }

        .games-grid {
          max-width: 1400px;
          margin: 0 auto;
          display: grid;
          grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
          gap: 30px;
        }

        .games-list {
          max-width: 1400px;
          margin: 0 auto;
          display: flex;
          flex-direction: column;
          gap: 20px;
        }

        .game-card {
          background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          overflow: hidden;
          cursor: pointer;
          transition: all 0.3s ease;
        }

        .game-card:hover {
          transform: translateY(-5px);
          border-color: rgba(123, 47, 247, 0.6);
          box-shadow: 0 10px 40px rgba(123, 47, 247, 0.4);
        }

        .game-thumbnail {
          position: relative;
          width: 100%;
          height: 200px;
          overflow: hidden;
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
          background: rgba(0, 0, 0, 0.8);
          display: flex;
          align-items: center;
          justify-content: center;
          opacity: 0;
          transition: opacity 0.3s ease;
        }

        .game-card:hover .game-overlay {
          opacity: 1;
        }

        .quick-play-btn {
          padding: 12px 30px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 25px;
          color: white;
          font-weight: 700;
          cursor: pointer;
          font-size: 16px;
        }

        .game-info {
          padding: 20px;
        }

        .game-info h3 {
          margin: 0 0 8px 0;
          font-size: 20px;
        }

        .game-developer {
          font-size: 13px;
          color: rgba(123, 47, 247, 0.9);
          margin: 0 0 12px 0;
        }

        .game-description {
          font-size: 14px;
          color: rgba(255, 255, 255, 0.7);
          margin: 0 0 15px 0;
          display: -webkit-box;
          -webkit-line-clamp: 2;
          -webkit-box-orient: vertical;
          overflow: hidden;
        }

        .game-stats {
          display: flex;
          gap: 15px;
          font-size: 13px;
        }

        .stat {
          display: flex;
          align-items: center;
          gap: 5px;
        }

        .stat-icon {
          font-size: 14px;
        }

        /* Loading/Empty States */
        .loading-state,
        .empty-state {
          max-width: 1400px;
          margin: 100px auto;
          text-align: center;
        }

        .spinner-large {
          width: 60px;
          height: 60px;
          border: 4px solid rgba(123, 47, 247, 0.3);
          border-top-color: #7b2ff7;
          border-radius: 50%;
          animation: spin 1s linear infinite;
          margin: 0 auto 20px;
        }

        @keyframes spin {
          to { transform: rotate(360deg); }
        }

        .empty-icon {
          font-size: 64px;
          display: block;
          margin-bottom: 20px;
        }

        .empty-state h3 {
          font-size: 28px;
          margin: 0 0 10px 0;
        }

        .empty-state p {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.6);
        }

        /* Game Detail Modal */
        .game-detail-modal {
          position: fixed;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          background: rgba(0, 0, 0, 0.9);
          display: flex;
          align-items: center;
          justify-content: center;
          z-index: 1000;
          animation: fadeIn 0.3s ease;
        }

        @keyframes fadeIn {
          from { opacity: 0; }
          to { opacity: 1; }
        }

        .modal-content {
          width: 800px;
          max-width: 90%;
          max-height: 90vh;
          background: linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%);
          border: 2px solid rgba(123, 47, 247, 0.5);
          border-radius: 20px;
          overflow-y: auto;
          position: relative;
          animation: slideUp 0.4s ease;
        }

        @keyframes slideUp {
          from {
            opacity: 0;
            transform: translateY(50px);
          }
          to {
            opacity: 1;
            transform: translateY(0);
          }
        }

        .close-modal {
          position: absolute;
          top: 20px;
          right: 20px;
          width: 40px;
          height: 40px;
          background: rgba(0, 0, 0, 0.5);
          border: 2px solid rgba(123, 47, 247, 0.5);
          border-radius: 50%;
          color: white;
          font-size: 24px;
          cursor: pointer;
          z-index: 10;
          transition: all 0.2s ease;
        }

        .close-modal:hover {
          background: rgba(123, 47, 247, 0.5);
          transform: rotate(90deg);
        }

        .modal-header {
          display: flex;
          gap: 30px;
          padding: 40px;
          border-bottom: 2px solid rgba(123, 47, 247, 0.3);
        }

        .modal-header img {
          width: 250px;
          height: 250px;
          object-fit: cover;
          border-radius: 16px;
          border: 2px solid rgba(123, 47, 247, 0.5);
        }

        .modal-header-info {
          flex: 1;
        }

        .modal-header-info h2 {
          font-size: 32px;
          margin: 0 0 10px 0;
        }

        .modal-developer {
          font-size: 16px;
          color: rgba(123, 47, 247, 0.9);
          margin: 0 0 20px 0;
        }

        .modal-ratings {
          display: flex;
          gap: 30px;
          margin-bottom: 25px;
          font-size: 18px;
        }

        .play-large-btn {
          width: 100%;
          padding: 16px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 25px;
          color: white;
          font-size: 18px;
          font-weight: 700;
          cursor: pointer;
          transition: transform 0.2s ease;
        }

        .play-large-btn:hover {
          transform: scale(1.02);
        }

        .modal-body {
          padding: 40px;
        }

        .modal-body h3 {
          font-size: 24px;
          margin: 0 0 15px 0;
        }

        .modal-body p {
          font-size: 16px;
          line-height: 1.6;
          color: rgba(255, 255, 255, 0.8);
          margin-bottom: 30px;
        }

        .modal-details {
          display: grid;
          grid-template-columns: repeat(2, 1fr);
          gap: 20px;
        }

        .detail-item {
          display: flex;
          flex-direction: column;
          gap: 8px;
        }

        .detail-label {
          font-size: 13px;
          color: rgba(255, 255, 255, 0.6);
          text-transform: uppercase;
          letter-spacing: 1px;
        }

        .detail-value {
          font-size: 16px;
          color: white;
          font-weight: 600;
        }

        @media (max-width: 768px) {
          .featured-carousel {
            grid-template-columns: 1fr;
          }
          
          .games-grid {
            grid-template-columns: 1fr;
          }
          
          .modal-header {
            flex-direction: column;
          }
          
          .modal-header img {
            width: 100%;
          }
        }
      `}</style>
    </div>
  );
};

export { HubModuleRedesigned as HubModule } from './HubModule';
