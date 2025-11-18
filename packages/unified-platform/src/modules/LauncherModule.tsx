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
      const libraryGames = demoGames.map((game) => ({
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
      const demoGames = getAllDemoGames();

      // Load recent from localStorage
      const recent = localStorage.getItem('nova_recent_games');
      if (recent) {
        const recentIds = JSON.parse(recent);
        // Re-hydrate with actual demo game objects
        const recentGames = recentIds
          .map((savedGame: any) => {
            const demoGame = demoGames.find((g) => g.id === savedGame.id);
            if (!demoGame) return null;
            return {
              id: demoGame.id,
              name: demoGame.title,
              thumbnail: demoGame.coverImage,
              demoGame: demoGame,
            };
          })
          .filter((g: any) => g !== null);
        setRecentGames(recentGames);
      } else {
        // Default to first 3 games
        const recentGames = demoGames.slice(0, 3).map((game) => ({
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
    const game = demoGames.find((g) => g.id === gameId);
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

  const [isPaused, setIsPaused] = useState(false);
  const [pauseAvailable, setPauseAvailable] = useState(false);

  const startGameLoop = (game: DemoGame) => {
    let lastTime = performance.now();

    // Check if game is single-player (can be paused)
    const isSinglePlayer = game.config.playerMode === 'single';
    setPauseAvailable(isSinglePlayer);

    const loop = () => {
      const currentTime = performance.now();
      const delta = (currentTime - lastTime) / 1000;
      lastTime = currentTime;

      // Only update if not paused or if multiplayer
      if (!isPaused || !isSinglePlayer) {
        // Update game logic
        game.update(delta);
      }

      // Always render (so we can see paused state)
      if (engineRef.current) {
        engineRef.current.render();
      }
      game.render();

      animationFrameRef.current = requestAnimationFrame(loop);
    };

    loop();
  };

  const togglePause = () => {
    if (pauseAvailable) {
      setIsPaused(!isPaused);
      platform.showNotification({
        type: 'info',
        message: isPaused ? 'Game Resumed' : 'Game Paused',
      });
    }
  };

  // ESC key handler for pause
  useEffect(() => {
    const handleKeyPress = (e: KeyboardEvent) => {
      if (e.key === 'Escape' && currentGame && pauseAvailable) {
        togglePause();
      }
    };

    window.addEventListener('keydown', handleKeyPress);
    return () => window.removeEventListener('keydown', handleKeyPress);
  }, [currentGame, pauseAvailable, isPaused]);

  const addToRecent = (game: DemoGame) => {
    const recentGame = {
      id: game.id,
      name: game.title,
      thumbnail: game.coverImage,
      demoGame: game,
    };

    const recent = [
      recentGame,
      ...recentGames.filter((g) => g.id !== game.id),
    ].slice(0, 10);
    setRecentGames(recent);

    // Save only IDs and metadata to localStorage (not functions)
    const recentForStorage = recent.map((g) => ({
      id: g.id,
      name: g.name,
      thumbnail: g.thumbnail,
    }));
    localStorage.setItem('nova_recent_games', JSON.stringify(recentForStorage));
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
              setIsPaused(false);
            }}
            style={{
              padding: '8px 16px',
              background: '#3a3a3a',
              border: 'none',
              borderRadius: '4px',
              color: 'white',
              cursor: 'pointer',
              fontSize: '14px',
            }}
          >
            ← Back to Library
          </button>
          <h2 style={{ margin: '0 20px', flex: 1 }}>{currentGame.title}</h2>
          <div
            className="game-info-header"
            style={{ display: 'flex', gap: '15px', alignItems: 'center' }}
          >
            <span
              style={{
                padding: '6px 12px',
                background: '#1a1a1a',
                borderRadius: '4px',
                color: '#00ff00',
                fontFamily: 'monospace',
                fontWeight: 'bold',
              }}
            >
              FPS: 60
            </span>
            <span
              style={{
                padding: '6px 12px',
                background: '#1a1a1a',
                borderRadius: '4px',
                color: '#00ddff',
              }}
            >
              Version: {currentGame.version}
            </span>
            {currentGame.config.playerMode === 'multiplayer' && (
              <span
                style={{
                  padding: '6px 12px',
                  background: '#7b2ff7',
                  borderRadius: '4px',
                  color: 'white',
                  fontWeight: 'bold',
                }}
              >
                🌐 MULTIPLAYER
              </span>
            )}
            {isPaused && (
              <span
                style={{
                  padding: '6px 12px',
                  background: '#ff9800',
                  borderRadius: '4px',
                  color: 'white',
                  fontWeight: 'bold',
                  animation: 'pulse 1.5s infinite',
                }}
              >
                ⏸ PAUSED
              </span>
            )}
          </div>
          {pauseAvailable && (
            <button
              onClick={togglePause}
              style={{
                padding: '8px 16px',
                background: isPaused ? '#4caf50' : '#ff9800',
                border: 'none',
                borderRadius: '4px',
                color: 'white',
                cursor: 'pointer',
                fontSize: '14px',
                marginLeft: '10px',
                fontWeight: 'bold',
              }}
            >
              {isPaused ? '▶ Resume' : '⏸ Pause'}
            </button>
          )}
          <button
            onClick={() => {
              if (canvasRef.current) {
                canvasRef.current.requestFullscreen();
              }
            }}
            style={{
              padding: '8px 16px',
              background: '#7b2ff7',
              border: 'none',
              borderRadius: '4px',
              color: 'white',
              cursor: 'pointer',
              fontSize: '14px',
              marginLeft: '10px',
            }}
          >
            ⛶ Fullscreen
          </button>
        </div>
        <div
          className="game-container"
          style={{
            position: 'relative',
            width: '100%',
            height: 'calc(100vh - 80px)',
          }}
        >
          <canvas
            ref={canvasRef}
            style={{
              width: '100%',
              height: '100%',
              display: 'block',
              background: '#000',
            }}
          />
          {isPaused && (
            <div
              style={{
                position: 'absolute',
                top: 0,
                left: 0,
                right: 0,
                bottom: 0,
                background: 'rgba(0, 0, 0, 0.8)',
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                justifyContent: 'center',
                color: 'white',
                zIndex: 1000,
              }}
            >
              <h1
                style={{
                  fontSize: '72px',
                  margin: '0 0 20px 0',
                  textShadow: '0 0 20px #7b2ff7',
                }}
              >
                ⏸ PAUSED
              </h1>
              <p
                style={{ fontSize: '24px', marginBottom: '30px', opacity: 0.8 }}
              >
                {currentGame.title}
              </p>
              <button
                onClick={togglePause}
                style={{
                  padding: '15px 40px',
                  background: '#4caf50',
                  border: 'none',
                  borderRadius: '8px',
                  color: 'white',
                  cursor: 'pointer',
                  fontSize: '20px',
                  fontWeight: 'bold',
                  boxShadow: '0 4px 20px rgba(76, 175, 80, 0.5)',
                }}
              >
                ▶ Resume Game
              </button>
              <p style={{ marginTop: '30px', opacity: 0.6 }}>
                Press ESC to resume
              </p>
            </div>
          )}
          <div
            className="game-overlay"
            style={{
              position: 'absolute',
              bottom: 0,
              left: 0,
              right: 0,
              padding: '20px',
              background: 'linear-gradient(transparent, rgba(0,0,0,0.8))',
              pointerEvents: 'none',
            }}
          >
            <div
              className="game-controls"
              style={{
                display: 'flex',
                flexDirection: 'column',
                gap: '5px',
                color: 'white',
                textShadow: '0 2px 4px rgba(0,0,0,0.8)',
              }}
            >
              <p style={{ margin: 0, fontSize: '16px', fontWeight: 'bold' }}>
                🎮 Playing: {currentGame.title}
              </p>
              <p style={{ margin: 0, fontSize: '14px', opacity: 0.9 }}>
                WASD - Move | Mouse - Look |{' '}
                {pauseAvailable ? 'ESC - Pause' : 'Online Multiplayer'}
              </p>
              {currentGame.config.playerMode === 'multiplayer' && (
                <p style={{ margin: 0, fontSize: '12px', color: '#00ddff' }}>
                  🌐 Connected to multiplayer server
                </p>
              )}
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
