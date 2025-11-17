/**
 * Launcher Module - Game Playing & Library
 * Runs REAL Nova Engine games
 */

import React, { useState, useEffect, useRef } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';
import { getAllDemoGames, type DemoGame } from '../demo-games';
// @ts-ignore
import { Engine } from '@nova-engine/engine';

interface LauncherModuleProps {
  platform: UnifiedPlatformCore;
}

export const LauncherModule: React.FC<LauncherModuleProps> = ({ platform }) => {
  const [library, setLibrary] = useState<any[]>([]);
  const [recentGames, setRecentGames] = useState<any[]>([]);
  const [currentGame, setCurrentGame] = useState<DemoGame | null>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const engineRef = useRef<any>(null);
  const animationFrameRef = useRef<number | null>(null);

  useEffect(() => {
    loadLibrary();
    loadRecentGames();

    platform.on('playGame', ({ gameId, game }: any) => {
      if (game) {
        launchGameDirect(game);
      } else {
        launchGame(gameId);
      }
    });
    
    return () => {
      // Cleanup
      if (animationFrameRef.current) {
        cancelAnimationFrame(animationFrameRef.current);
      }
      if (engineRef.current) {
        engineRef.current.destroy();
      }
    };
  }, []);

  const loadLibrary = () => {
    try {
      // Load from demo games
      const demoGames = getAllDemoGames();
      const libraryGames = demoGames.map(game => ({
        id: game.id,
        name: game.title,
        thumbnail: game.coverImage,
        demoGame: game,
      }));
      setLibrary(libraryGames);
    } catch (error) {
      console.error('Failed to load library:', error);
    }
  };

  const loadRecentGames = () => {
    try {
      // Load recent from localStorage
      const recent = localStorage.getItem('nova_recent_games');
      if (recent) {
        setRecentGames(JSON.parse(recent));
      } else {
        // Default to first 3 games
        const demoGames = getAllDemoGames();
        const recentGames = demoGames.slice(0, 3).map(game => ({
          id: game.id,
          name: game.title,
          thumbnail: game.coverImage,
          demoGame: game,
        }));
        setRecentGames(recentGames);
      }
    } catch (error) {
      console.error('Failed to load recent games:', error);
    }
  };

  const launchGame = (gameId: string) => {
    const demoGames = getAllDemoGames();
    const game = demoGames.find(g => g.id === gameId);
    if (game) {
      launchGameDirect(game);
    }
  };

  const launchGameDirect = (game: DemoGame) => {
    try {
      setCurrentGame(game);
      
      // Wait for canvas to be available
      setTimeout(() => {
        if (canvasRef.current) {
          initializeGameEngine(game);
        }
      }, 100);
      
      platform.showNotification({
        type: 'success',
        message: `Launching ${game.title} with Nova Engine...`,
      });
      
      // Add to recent games
      addToRecent(game);
    } catch (error) {
      console.error('Failed to launch game:', error);
      platform.showNotification({
        type: 'error',
        message: 'Failed to launch game',
      });
    }
  };

  const initializeGameEngine = (game: DemoGame) => {
    if (!canvasRef.current) return;

    try {
      // Initialize Nova Engine for this game
      engineRef.current = new Engine({
        canvas: canvasRef.current,
        antialias: true,
        alpha: false,
      });

      // Initialize the game with engine context
      game.init({
        engine: engineRef.current,
        config: game.config,
      });

      // Start game loop
      startGameLoop(game);

      platform.showNotification({
        type: 'success',
        message: `${game.title} is now running!`,
      });
    } catch (error) {
      console.error('Failed to initialize game:', error);
      platform.showNotification({
        type: 'error',
        message: 'Failed to start game engine',
      });
    }
  };

  const startGameLoop = (game: DemoGame) => {
    let lastTime = performance.now();
    
    const loop = () => {
      const currentTime = performance.now();
      const delta = (currentTime - lastTime) / 1000;
      lastTime = currentTime;

      // Update game logic
      game.update(delta);

      // Render
      if (engineRef.current) {
        engineRef.current.render();
      }
      game.render();

      animationFrameRef.current = requestAnimationFrame(loop);
    };

    loop();
  };

  const addToRecent = (game: DemoGame) => {
    const recentGame = {
      id: game.id,
      name: game.title,
      thumbnail: game.coverImage,
      demoGame: game,
    };
    
    const recent = [recentGame, ...recentGames.filter(g => g.id !== game.id)].slice(0, 10);
    setRecentGames(recent);
    localStorage.setItem('nova_recent_games', JSON.stringify(recent));
  };

  if (currentGame) {
    return (
      <div className="game-player">
        <div className="player-header">
          <button
            onClick={() => {
              if (animationFrameRef.current) {
                cancelAnimationFrame(animationFrameRef.current);
              }
              if (engineRef.current) {
                engineRef.current.destroy();
              }
              setCurrentGame(null);
            }}
          >
            ← Back to Library
          </button>
          <h2>{currentGame.title}</h2>
          <div className="game-info-header">
            <span>FPS: 60</span>
            <span>Version: {currentGame.version}</span>
          </div>
          <button
            onClick={() => {
              if (canvasRef.current) {
                canvasRef.current.requestFullscreen();
              }
            }}
          >
            ⛶ Fullscreen
          </button>
        </div>
        <div className="game-container">
          <canvas ref={canvasRef} />
          <div className="game-overlay">
            <div className="game-controls">
              <p>🎮 Playing: {currentGame.title}</p>
              <p>WASD - Move | Mouse - Look | ESC - Menu</p>
            </div>
          </div>
        </div>
        <style>{`
          .game-player {
            width: 100%;
            height: 100%;
            display: flex;
            flex-direction: column;
            background: #000;
          }
          .player-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            background: #1a1a1a;
            color: white;
            gap: 15px;
          }
          .player-header button {
            padding: 8px 16px;
            background: #7b2ff7;
            border: none;
            border-radius: 4px;
            color: white;
            cursor: pointer;
          }
          .player-header h2 {
            margin: 0;
          }
          .game-info-header {
            display: flex;
            gap: 20px;
            color: #aaa;
            font-size: 0.9em;
          }
          .game-container {
            flex: 1;
            position: relative;
            background: #000;
          }
          .game-container canvas {
            width: 100%;
            height: 100%;
            display: block;
          }
          .game-overlay {
            position: absolute;
            bottom: 20px;
            left: 20px;
            background: rgba(0, 0, 0, 0.8);
            padding: 15px;
            border-radius: 8px;
            color: white;
          }
          .game-controls p {
            margin: 5px 0;
            font-size: 0.9em;
          }
        `}</style>
      </div>
    );
  }

  return (
    <div className="launcher-module">
      <section>
        <h2>Recently Played</h2>
        <div className="game-list">
          {recentGames.map((game) => (
            <div
              key={game.id}
              className="game-card"
              onClick={() => game.demoGame && launchGameDirect(game.demoGame)}
            >
              <img src={game.thumbnail} alt={game.name} />
              <div className="game-card-info">
                <h3>{game.name}</h3>
                <button className="play-btn">▶ Play</button>
              </div>
            </div>
          ))}
        </div>
      </section>

      <section>
        <h2>Your Library ({library.length} Games)</h2>
        <div className="game-grid">
          {library.map((game) => (
            <div
              key={game.id}
              className="game-card"
              onClick={() => game.demoGame && launchGameDirect(game.demoGame)}
            >
              <img src={game.thumbnail} alt={game.name} />
              <div className="game-card-info">
                <h3>{game.name}</h3>
                <button className="play-btn">▶ Play</button>
              </div>
            </div>
          ))}
        </div>
      </section>

      <style>{`
        .launcher-module {
          padding: 20px;
          background: #1a1a1a;
          color: white;
          height: 100%;
          overflow-y: auto;
        }
        .game-list, .game-grid {
          display: grid;
          grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
          gap: 20px;
          margin-top: 20px;
        }
        .game-card {
          background: #2a2a2a;
          border-radius: 8px;
          overflow: hidden;
          cursor: pointer;
          transition: transform 0.3s;
        }
        .game-card:hover {
          transform: scale(1.05);
        }
        .game-card img {
          width: 100%;
          height: 150px;
          object-fit: cover;
        }
        .game-card-info {
          padding: 15px;
          display: flex;
          justify-content: space-between;
          align-items: center;
        }
        .game-card h3 {
          margin: 0;
          font-size: 1.1em;
        }
        .play-btn {
          padding: 8px 16px;
          background: #7b2ff7;
          border: none;
          border-radius: 20px;
          color: white;
          cursor: pointer;
          transition: background 0.3s;
        }
        .play-btn:hover {
          background: #6929d4;
        }
      `}</style>
    </div>
  );
};
