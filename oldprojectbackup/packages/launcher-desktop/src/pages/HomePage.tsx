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
 * HomePage - Main landing page for Nova Launcher Desktop
 */

import { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import { localGameStorage, LocalGame } from '../services/LocalGameStorage';
import { updateManager, GameUpdate } from '../services/UpdateManager';

export default function HomePage() {
  const [recentGames, setRecentGames] = useState<LocalGame[]>([]);
  const [updates, setUpdates] = useState<GameUpdate[]>([]);
  const [isOnline, setIsOnline] = useState(navigator.onLine);

  useEffect(() => {
    loadData();

    // Listen for online/offline changes
    const handleOnline = () => setIsOnline(true);
    const handleOffline = () => setIsOnline(false);

    window.addEventListener('online', handleOnline);
    window.addEventListener('offline', handleOffline);

    return () => {
      window.removeEventListener('online', handleOnline);
      window.removeEventListener('offline', handleOffline);
    };
  }, []);

  async function loadData() {
    try {
      // Get recently played games
      const games = await localGameStorage.getInstalledGames();
      const recent = games
        .filter((g) => g.lastPlayedAt)
        .sort((a, b) => {
          const aTime = a.lastPlayedAt?.getTime() || 0;
          const bTime = b.lastPlayedAt?.getTime() || 0;
          return bTime - aTime;
        })
        .slice(0, 5);
      setRecentGames(recent);

      // Check for updates
      const availableUpdates = updateManager.getAvailableUpdates();
      setUpdates(availableUpdates);
    } catch (error) {
      console.error('Failed to load data:', error);
    }
  }

  async function playGame(game: LocalGame) {
    try {
      await localGameStorage.updateLastPlayed(game.id);
      // Launch game (implementation would open game)
      alert(`Launching ${game.name}...`);
    } catch (error) {
      console.error('Failed to play game:', error);
    }
  }

  return (
    <div className="page home-page">
      <header className="page-header">
        <h1>Nova Launcher</h1>
        <div className="status">
          <span
            className={`status-indicator ${isOnline ? 'online' : 'offline'}`}
          >
            {isOnline ? '● Online' : '○ Offline'}
          </span>
        </div>
      </header>

      <nav className="main-nav">
        <Link to="/" className="active">
          Home
        </Link>
        <Link to="/library">Library</Link>
        <Link to="/downloads">Downloads</Link>
        <Link to="/settings">Settings</Link>
      </nav>

      <main className="page-content">
        {updates.length > 0 && (
          <section className="updates-section">
            <h2>Updates Available</h2>
            <div className="updates-list">
              {updates.map((update) => (
                <div key={update.gameId} className="update-card">
                  <h3>{update.gameId}</h3>
                  <p>
                    Version {update.currentVersion} → {update.latestVersion}
                  </p>
                  <p className="size">
                    {(update.size / 1024 / 1024).toFixed(1)} MB
                  </p>
                  <button>Update</button>
                </div>
              ))}
            </div>
          </section>
        )}

        <section className="recent-games-section">
          <h2>Recent Games</h2>
          {recentGames.length === 0 ? (
            <p className="empty-state">
              No games played yet. Visit the Library to download games.
            </p>
          ) : (
            <div className="games-grid">
              {recentGames.map((game) => (
                <div key={game.id} className="game-card">
                  <div className="game-thumbnail">
                    {game.thumbnail ? (
                      <img src={game.thumbnail} alt={game.name} />
                    ) : (
                      <div className="placeholder-thumbnail">
                        {game.name[0]}
                      </div>
                    )}
                  </div>
                  <div className="game-info">
                    <h3>{game.name}</h3>
                    <p className="version">v{game.version}</p>
                    {game.lastPlayedAt && (
                      <p className="last-played">
                        Last played: {game.lastPlayedAt.toLocaleDateString()}
                      </p>
                    )}
                  </div>
                  <button onClick={() => playGame(game)}>Play</button>
                </div>
              ))}
            </div>
          )}
        </section>

        <section className="quick-actions">
          <h2>Quick Actions</h2>
          <div className="actions-grid">
            <Link to="/library" className="action-card">
              <h3>Browse Library</h3>
              <p>View all installed games</p>
            </Link>
            <Link to="/downloads" className="action-card">
              <h3>Downloads</h3>
              <p>Manage downloads and updates</p>
            </Link>
            <button
              className="action-card"
              onClick={() => updateManager.checkForUpdates()}
            >
              <h3>Check for Updates</h3>
              <p>Update games and launcher</p>
            </button>
          </div>
        </section>
      </main>
    </div>
  );
}
