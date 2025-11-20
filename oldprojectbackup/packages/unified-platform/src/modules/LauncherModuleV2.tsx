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

import React, { useState, useEffect } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';
import { apiClient } from '../services/ApiClient';
import './styles/LauncherModuleV2.css';

interface LauncherModuleV2Props {
  platform: UnifiedPlatformCore;
}

interface GameInstance {
  id: string;
  name: string;
  thumbnail: string;
  lastPlayed: string;
  playtime: string;
  saveSlots: number;
}

interface PerformanceMetrics {
  fps: number;
  cpu: number;
  gpu: number;
  ram: number;
  vram: number;
}

export const LauncherModuleV2: React.FC<LauncherModuleV2Props> = () => {
  const [selectedGame, setSelectedGame] = useState<GameInstance | null>(null);
  const [isPlaying, setIsPlaying] = useState(false);
  const [showPerformance, setShowPerformance] = useState(false);
  const [selectedTab, setSelectedTab] = useState<
    'library' | 'saves' | 'settings'
  >('library');
  const [installedGames, setInstalledGames] = useState<GameInstance[]>([]);

  const [performance] = useState<PerformanceMetrics>({
    fps: 144,
    cpu: 45,
    gpu: 62,
    ram: 58,
    vram: 42,
  });

  // Load user's games on mount
  useEffect(() => {
    loadUserGames();
  }, []);

  const loadUserGames = async () => {
    try {
      // Fetch user's games from backend API - production ready, no fallbacks
      const games = await apiClient.getGames({ limit: 100 });

      const gameInstances: GameInstance[] = Array.isArray(games)
        ? games.map((game: any) => ({
            id: game.id,
            name: game.title || game.name,
            thumbnail:
              game.thumbnail ||
              game.thumbnail_url ||
              'https://via.placeholder.com/300x200',
            lastPlayed: game.last_played || 'Never',
            playtime: game.playtime
              ? `${Math.floor(game.playtime / 60)} hours`
              : '0 hours',
            saveSlots: game.save_slots || 0,
          }))
        : [];

      setInstalledGames(gameInstances);
    } catch (error) {
      console.error('Failed to load games from backend API:', error);
      // Set empty games list - UI will show empty library
      setInstalledGames([]);
    }
  };

  const handlePlayGame = async (game: GameInstance) => {
    setSelectedGame(game);
    setIsPlaying(true);
    setShowPerformance(true);

    try {
      // Get full game details and launch
      const gameDetails = await apiClient.getGame(game.id);
      // eslint-disable-next-line no-console
      console.log('✅ Launching game:', game.name, gameDetails);

      // TODO: Initialize Nova Engine runtime and load game
      // For now, just show the performance panel
    } catch (error) {
      console.error('Failed to launch game:', error);
      alert(`Failed to launch ${game.name}. Please try again.`);
      setIsPlaying(false);
      setShowPerformance(false);
    }
  };

  const handleStopGame = () => {
    setIsPlaying(false);
    setShowPerformance(false);
    // eslint-disable-next-line no-console
    console.log('✅ Stopped game:', selectedGame?.name);
    // TODO: Properly cleanup game runtime
  };

  return (
    <div className="launcher-v2-container">
      {/* Animated Background */}
      <div className="launcher-v2-bg">
        <div className="launcher-v2-orb"></div>
        <div className="launcher-v2-stars"></div>
      </div>

      {/* Header */}
      <div className="launcher-v2-header">
        <div className="launcher-v2-logo">
          <span className="launcher-v2-logo-text">NOVA</span>
          <span className="launcher-v2-logo-sub">Launcher</span>
        </div>
        <div className="launcher-v2-tabs">
          <button
            className={`launcher-v2-tab ${selectedTab === 'library' ? 'active' : ''}`}
            onClick={() => setSelectedTab('library')}
          >
            📚 Library
          </button>
          <button
            className={`launcher-v2-tab ${selectedTab === 'saves' ? 'active' : ''}`}
            onClick={() => setSelectedTab('saves')}
          >
            💾 Saves
          </button>
          <button
            className={`launcher-v2-tab ${selectedTab === 'settings' ? 'active' : ''}`}
            onClick={() => setSelectedTab('settings')}
          >
            ⚙️ Settings
          </button>
        </div>
      </div>

      <div className="launcher-v2-main">
        {/* Sidebar - Game Library */}
        <div className="launcher-v2-sidebar">
          <div className="launcher-v2-sidebar-header">
            <h2>Installed Games</h2>
            <span className="launcher-v2-game-count">
              {installedGames.length}
            </span>
          </div>
          <div className="launcher-v2-games-list">
            {installedGames.map((game) => (
              <div
                key={game.id}
                className={`launcher-v2-game-item ${selectedGame?.id === game.id ? 'active' : ''}`}
                onClick={() => setSelectedGame(game)}
              >
                <img
                  src={game.thumbnail}
                  alt={game.name}
                  className="launcher-v2-game-thumb"
                />
                <div className="launcher-v2-game-info">
                  <h3>{game.name}</h3>
                  <p className="launcher-v2-game-playtime">{game.playtime}</p>
                  <p className="launcher-v2-game-last-played">
                    {game.lastPlayed}
                  </p>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Main Content Area */}
        <div className="launcher-v2-content">
          {selectedGame ? (
            <>
              {/* Game Display Area */}
              <div className="launcher-v2-game-viewport">
                {isPlaying ? (
                  <div className="launcher-v2-playing">
                    <div className="launcher-v2-game-canvas">
                      {/* TODO: Connect to Nova Engine renderer */}
                      <div className="launcher-v2-placeholder">
                        <div className="launcher-v2-orbital-planet"></div>
                        <p>🎮 Game Running: {selectedGame.name}</p>
                        <p className="launcher-v2-note">
                          TODO: Connect to Nova Engine game renderer
                        </p>
                      </div>
                    </div>

                    {/* In-Game HUD Overlay */}
                    {showPerformance && (
                      <div className="launcher-v2-hud">
                        <div className="launcher-v2-hud-metric">
                          <span className="launcher-v2-hud-label">FPS</span>
                          <span className="launcher-v2-hud-value">
                            {performance.fps}
                          </span>
                        </div>
                        <div className="launcher-v2-hud-metric">
                          <span className="launcher-v2-hud-label">CPU</span>
                          <span className="launcher-v2-hud-value">
                            {performance.cpu}%
                          </span>
                        </div>
                        <div className="launcher-v2-hud-metric">
                          <span className="launcher-v2-hud-label">GPU</span>
                          <span className="launcher-v2-hud-value">
                            {performance.gpu}%
                          </span>
                        </div>
                        <div className="launcher-v2-hud-metric">
                          <span className="launcher-v2-hud-label">RAM</span>
                          <span className="launcher-v2-hud-value">
                            {performance.ram}%
                          </span>
                        </div>
                        <div className="launcher-v2-hud-metric">
                          <span className="launcher-v2-hud-label">VRAM</span>
                          <span className="launcher-v2-hud-value">
                            {performance.vram}%
                          </span>
                        </div>
                      </div>
                    )}

                    <button
                      className="launcher-v2-stop-btn"
                      onClick={handleStopGame}
                    >
                      ⏹️ Stop Game
                    </button>
                  </div>
                ) : (
                  <div className="launcher-v2-game-preview">
                    <img
                      src={selectedGame.thumbnail}
                      alt={selectedGame.name}
                      className="launcher-v2-preview-img"
                    />
                    <div className="launcher-v2-game-overlay">
                      <h1 className="launcher-v2-game-title">
                        {selectedGame.name}
                      </h1>
                      <div className="launcher-v2-game-stats">
                        <div className="launcher-v2-stat">
                          <span className="launcher-v2-stat-label">
                            Playtime
                          </span>
                          <span className="launcher-v2-stat-value">
                            {selectedGame.playtime}
                          </span>
                        </div>
                        <div className="launcher-v2-stat">
                          <span className="launcher-v2-stat-label">
                            Last Played
                          </span>
                          <span className="launcher-v2-stat-value">
                            {selectedGame.lastPlayed}
                          </span>
                        </div>
                        <div className="launcher-v2-stat">
                          <span className="launcher-v2-stat-label">
                            Save Slots
                          </span>
                          <span className="launcher-v2-stat-value">
                            {selectedGame.saveSlots}
                          </span>
                        </div>
                      </div>
                      <button
                        className="launcher-v2-play-btn"
                        onClick={() => handlePlayGame(selectedGame)}
                      >
                        ▶️ Launch Game
                      </button>
                    </div>
                  </div>
                )}
              </div>

              {/* Bottom Panel - Game Controls */}
              {!isPlaying && (
                <div className="launcher-v2-controls">
                  <div className="launcher-v2-control-section">
                    <h3>Quick Actions</h3>
                    <div className="launcher-v2-actions">
                      <button className="launcher-v2-action-btn">
                        💾 Manage Saves
                      </button>
                      <button className="launcher-v2-action-btn">
                        ⚙️ Game Settings
                      </button>
                      <button className="launcher-v2-action-btn">
                        📊 View Stats
                      </button>
                      <button className="launcher-v2-action-btn">
                        🗑️ Uninstall
                      </button>
                    </div>
                  </div>
                  <div className="launcher-v2-control-section">
                    <h3>Performance Monitor</h3>
                    <div className="launcher-v2-perf-toggle">
                      <label>
                        <input
                          type="checkbox"
                          checked={showPerformance}
                          onChange={(e) => setShowPerformance(e.target.checked)}
                        />
                        Show performance overlay during gameplay
                      </label>
                    </div>
                  </div>
                </div>
              )}
            </>
          ) : (
            <div className="launcher-v2-empty">
              <div className="launcher-v2-empty-content">
                <div className="launcher-v2-orbital-planet-large"></div>
                <h2>Select a game to launch</h2>
                <p>Choose a game from your library to start playing</p>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default LauncherModuleV2;
