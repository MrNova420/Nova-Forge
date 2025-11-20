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
 * LibraryPage - Shows all installed games
 */

import { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import { localGameStorage, LocalGame } from '../services/LocalGameStorage';

export default function LibraryPage() {
  const [games, setGames] = useState<LocalGame[]>([]);
  const [sortBy, setSortBy] = useState<'name' | 'date'>('name');

  useEffect(() => {
    loadGames();
  }, [sortBy]);

  async function loadGames() {
    try {
      const allGames = await localGameStorage.getInstalledGames();

      // Sort games
      if (sortBy === 'name') {
        allGames.sort((a, b) => a.name.localeCompare(b.name));
      } else {
        allGames.sort(
          (a, b) => b.downloadedAt.getTime() - a.downloadedAt.getTime()
        );
      }

      setGames(allGames);
    } catch (error) {
      console.error('Failed to load games:', error);
    }
  }

  async function uninstallGame(gameId: string) {
    if (confirm('Are you sure you want to uninstall this game?')) {
      try {
        await localGameStorage.uninstallGame(gameId);
        await loadGames();
      } catch (error) {
        console.error('Failed to uninstall game:', error);
      }
    }
  }

  return (
    <div className="page library-page">
      <header className="page-header">
        <h1>Game Library</h1>
        <nav className="main-nav">
          <Link to="/">Home</Link>
          <Link to="/library" className="active">
            Library
          </Link>
          <Link to="/downloads">Downloads</Link>
          <Link to="/settings">Settings</Link>
        </nav>
      </header>

      <div className="library-controls">
        <div className="sort-controls">
          <label>Sort by:</label>
          <select
            value={sortBy}
            onChange={(e) => setSortBy(e.target.value as 'name' | 'date')}
          >
            <option value="name">Name</option>
            <option value="date">Date Added</option>
          </select>
        </div>
        <div className="stats">
          {games.length} game{games.length !== 1 ? 's' : ''} installed
        </div>
      </div>

      <main className="page-content">
        {games.length === 0 ? (
          <div className="empty-state">
            <h2>No games installed</h2>
            <p>Download games from Nova Hub to play offline</p>
          </div>
        ) : (
          <div className="games-list">
            {games.map((game) => (
              <div key={game.id} className="game-item">
                <div className="game-thumbnail">
                  {game.thumbnail ? (
                    <img src={game.thumbnail} alt={game.name} />
                  ) : (
                    <div className="placeholder-thumbnail">{game.name[0]}</div>
                  )}
                </div>
                <div className="game-details">
                  <h3>{game.name}</h3>
                  <p className="metadata">
                    {game.metadata.category} • v{game.version}
                  </p>
                  <p className="developer">by {game.metadata.developer}</p>
                  <p className="description">{game.metadata.description}</p>
                  <p className="size">
                    Size: {(game.size / 1024 / 1024).toFixed(1)} MB
                  </p>
                  {game.lastPlayedAt && (
                    <p className="last-played">
                      Last played: {game.lastPlayedAt.toLocaleDateString()}
                    </p>
                  )}
                </div>
                <div className="game-actions">
                  <button className="btn-primary">Play</button>
                  <button
                    className="btn-secondary"
                    onClick={() => uninstallGame(game.id)}
                  >
                    Uninstall
                  </button>
                </div>
              </div>
            ))}
          </div>
        )}
      </main>
    </div>
  );
}
