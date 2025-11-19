/**
 * NOVA ENGINE - HUB DASHBOARD V2
 * Complete redesign matching Image 2 mockup
 * Features:
 * - Orbital planet centerpiece with glowing effects
 * - Dark purple/pink theme
 * - Card-based game browsing
 * - Category filters
 * - Advanced search and discovery
 */

import React, { useState, useEffect } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';
import { getAllDemoGames, type DemoGame } from '../demo-games';
import { apiClient } from '../services/ApiClient';
import './styles/HubModuleV2.css';

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

interface HubModuleV2Props {
  platform: UnifiedPlatformCore;
}

export const HubModuleV2: React.FC<HubModuleV2Props> = ({ platform }) => {
  const [games, setGames] = useState<Game[]>([]);
  const [selectedGame, setSelectedGame] = useState<Game | null>(null);
  const [category, setCategory] = useState<string>('all');
  const [searchQuery, setSearchQuery] = useState('');
  const [loading, setLoading] = useState(true);

  const categories = [
    { id: 'all', name: 'All Games', icon: '🎮' },
    { id: 'trending', name: 'Trending', icon: '🔥' },
    { id: 'new', name: 'New Releases', icon: '✨' },
    { id: 'action', name: 'Action', icon: '⚔️' },
    { id: 'adventure', name: 'Adventure', icon: '🗺️' },
    { id: 'rpg', name: 'RPG', icon: '🐉' },
    { id: 'strategy', name: 'Strategy', icon: '♟️' },
    { id: 'indie', name: 'Indie', icon: '💎' },
  ];

  useEffect(() => {
    loadGames();
  }, [category, searchQuery]);

  const loadGames = async () => {
    setLoading(true);
    try {
      // Fetch from backend API - no fallbacks, production ready
      const apiGames = await apiClient.getGames({
        category: category !== 'all' ? category : undefined,
        search: searchQuery || undefined,
        limit: 100,
      });

      // Map API response to Game format
      const gameList: Game[] = Array.isArray(apiGames)
        ? apiGames.map((apiGame: any) => ({
            id: apiGame.id,
            name: apiGame.title || apiGame.name,
            description: apiGame.description,
            developer:
              apiGame.creator?.username || apiGame.developer || 'Nova Studios',
            category: apiGame.genre || apiGame.category || 'action',
            rating: apiGame.rating || 0,
            downloads: apiGame.downloads || apiGame.play_count || 0,
            thumbnail:
              apiGame.thumbnail ||
              apiGame.thumbnail_url ||
              'https://via.placeholder.com/300x200',
            screenshots: apiGame.screenshots || [],
            releaseDate:
              apiGame.created_at ||
              apiGame.releaseDate ||
              new Date().toISOString(),
            version: apiGame.version || '1.0.0',
            isDemo: false,
          }))
        : [];

      setGames(gameList);
    } catch (error) {
      console.error('Failed to load games from backend API:', error);
      // Show error to user instead of hiding it with demo data
      throw new Error(
        'Unable to connect to backend server. Please ensure the backend is running.'
      );
    } finally {
      setLoading(false);
    }
  };

  const handlePlayGame = (game: Game) => {
    if (game.isDemo && game.demoGame) {
      platform.switchMode('launcher');
    }
  };

  return (
    <div className="hub-v2-container">
      {/* NOVA Header */}
      <div className="hub-v2-header">
        <div className="hub-v2-logo">
          <span className="nova-text">NOVA</span>
        </div>
        <div className="hub-v2-nav">
          <button className="hub-v2-nav-btn active">Home</button>
          <button className="hub-v2-nav-btn">Discover</button>
          <button className="hub-v2-nav-btn">Featured</button>
        </div>
        <div className="hub-v2-user">
          <button className="hub-v2-sterger-btn">Sterger</button>
        </div>
      </div>

      {/* Main Content */}
      <div className="hub-v2-main">
        {/* Left Sidebar */}
        <aside className="hub-v2-sidebar">
          <div className="hub-v2-sidebar-logo">
            <span className="nova-text-lg">NOVA</span>
          </div>
          <nav className="hub-v2-sidebar-nav">
            <button className="hub-v2-sidebar-btn active">
              <span className="icon">🏠</span>
              <span>Home</span>
            </button>
            <button className="hub-v2-sidebar-btn">
              <span className="icon">🔍</span>
              <span>Search</span>
            </button>
            <button className="hub-v2-sidebar-btn">
              <span className="icon">📚</span>
              <span>Library</span>
            </button>
            <button className="hub-v2-sidebar-btn">
              <span className="icon">⭐</span>
              <span>Favorites</span>
            </button>
          </nav>
        </aside>

        {/* Center Content */}
        <div className="hub-v2-content">
          {/* Orbital Planet Display */}
          <div className="hub-v2-planet-section">
            <div className="hub-v2-planet-container">
              <div className="hub-v2-planet-orbit">
                <div className="hub-v2-planet-glow"></div>
                <div className="hub-v2-planet-core"></div>
                <div className="hub-v2-planet-ring"></div>
              </div>
            </div>
            <h1 className="hub-v2-title">Hub Dashboard</h1>
            <p className="hub-v2-subtitle">
              Discover amazing games from talented developers around the world
            </p>
          </div>

          {/* Category Filters */}
          <div className="hub-v2-categories">
            {categories.map((cat) => (
              <button
                key={cat.id}
                className={`hub-v2-category-btn ${
                  category === cat.id ? 'active' : ''
                }`}
                onClick={() => setCategory(cat.id)}
              >
                <span className="icon">{cat.icon}</span>
                <span>{cat.name}</span>
              </button>
            ))}
          </div>

          {/* Search Bar */}
          <div className="hub-v2-search">
            <input
              type="text"
              className="hub-v2-search-input"
              placeholder="Search games..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
            <button className="hub-v2-search-btn">🔍</button>
          </div>

          {/* Game Grid */}
          <div className="hub-v2-games-section">
            <h2 className="section-title">Available Games</h2>
            {loading ? (
              <div className="hub-v2-loading">
                <div className="spinner"></div>
                <p>Loading games...</p>
              </div>
            ) : games.length === 0 ? (
              <div className="hub-v2-empty">
                <p>No games found</p>
              </div>
            ) : (
              <div className="hub-v2-game-grid">
                {games.map((game) => (
                  <div key={game.id} className="hub-v2-game-card">
                    <div className="game-card-image">
                      <img src={game.thumbnail} alt={game.name} />
                      <div className="game-card-overlay">
                        <button
                          className="play-btn"
                          onClick={() => handlePlayGame(game)}
                        >
                          ▶ Play
                        </button>
                        <button
                          className="info-btn"
                          onClick={() => setSelectedGame(game)}
                        >
                          ℹ Info
                        </button>
                      </div>
                    </div>
                    <div className="game-card-content">
                      <h3 className="game-card-title">{game.name}</h3>
                      <p className="game-card-dev">{game.developer}</p>
                      <div className="game-card-stats">
                        <span className="rating">
                          ⭐ {game.rating.toFixed(1)}
                        </span>
                        <span className="downloads">
                          ⬇ {(game.downloads / 1000).toFixed(1)}K
                        </span>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            )}
          </div>

          {/* Featured Section */}
          <div className="hub-v2-featured-section">
            <h2 className="section-title">Featured Content</h2>
            <div className="hub-v2-featured-cards">
              <div className="featured-card">
                <div className="featured-icon">🏆</div>
                <h3>Community Choice</h3>
                <p>Top rated by our community</p>
              </div>
              <div className="featured-card">
                <div className="featured-icon">🎯</div>
                <h3>Developer Spotlight</h3>
                <p>Featured indie developers</p>
              </div>
              <div className="featured-card">
                <div className="featured-icon">🌟</div>
                <h3>Editor's Pick</h3>
                <p>Handpicked by our team</p>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Game Detail Modal */}
      {selectedGame && (
        <div
          className="hub-v2-modal-overlay"
          onClick={() => setSelectedGame(null)}
        >
          <div className="hub-v2-modal" onClick={(e) => e.stopPropagation()}>
            <button
              className="modal-close"
              onClick={() => setSelectedGame(null)}
            >
              ✕
            </button>
            <div className="modal-header">
              <img src={selectedGame.thumbnail} alt={selectedGame.name} />
            </div>
            <div className="modal-content">
              <h2>{selectedGame.name}</h2>
              <p className="modal-dev">by {selectedGame.developer}</p>
              <div className="modal-stats">
                <span>⭐ {selectedGame.rating.toFixed(1)}</span>
                <span>⬇ {selectedGame.downloads.toLocaleString()}</span>
                <span>
                  📅 {new Date(selectedGame.releaseDate).toLocaleDateString()}
                </span>
              </div>
              <p className="modal-description">{selectedGame.description}</p>
              <div className="modal-actions">
                <button
                  className="btn-primary"
                  onClick={() => {
                    handlePlayGame(selectedGame);
                    setSelectedGame(null);
                  }}
                >
                  ▶ Play Now
                </button>
                <button className="btn-secondary">+ Add to Library</button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default HubModuleV2;
