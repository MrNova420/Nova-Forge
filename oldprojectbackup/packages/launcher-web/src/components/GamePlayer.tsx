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

import React, { useEffect, useRef, useState } from 'react';
import { Game } from '../store/gameStore';

interface GamePlayerProps {
  game: Game;
  onClose: () => void;
}

const GamePlayer: React.FC<GamePlayerProps> = ({ game, onClose }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [isFullscreen, setIsFullscreen] = useState(false);
  const [volume, setVolume] = useState(100);
  const [isMuted, setIsMuted] = useState(false);

  useEffect(() => {
    // Initialize Nova Runtime
    const initializeGame = async () => {
      try {
        setIsLoading(true);
        // TODO: Load game from Hub CDN
        // const gameData = await loadGameData(game.id);
        // const runtime = new NovaRuntime(canvasRef.current!);
        // await runtime.load(gameData);
        // runtime.start();
        
        // Simulate loading
        await new Promise((resolve) => setTimeout(resolve, 2000));
        setIsLoading(false);
      } catch (error) {
        console.error('Failed to load game:', error);
      }
    };

    initializeGame();

    return () => {
      // Cleanup runtime
    };
  }, [game.id]);

  const toggleFullscreen = async () => {
    if (!containerRef.current) return;

    try {
      if (!isFullscreen) {
        await containerRef.current.requestFullscreen();
        setIsFullscreen(true);
      } else {
        await document.exitFullscreen();
        setIsFullscreen(false);
      }
    } catch (error) {
      console.error('Fullscreen error:', error);
    }
  };

  return (
    <div ref={containerRef} className="fixed inset-0 bg-black z-50 flex flex-col">
      {/* Controls Bar */}
      <div className="bg-bg-secondary border-b border-border p-4 flex items-center justify-between">
        <div className="flex items-center gap-4">
          <button onClick={onClose} className="btn btn-secondary">
            ← Back
          </button>
          <h2 className="font-bold text-lg">{game.title}</h2>
        </div>
        
        <div className="flex items-center gap-4">
          {/* Volume Control */}
          <div className="flex items-center gap-2">
            <button
              onClick={() => setIsMuted(!isMuted)}
              className="text-2xl"
            >
              {isMuted ? '🔇' : '🔊'}
            </button>
            <input
              type="range"
              min="0"
              max="100"
              value={volume}
              onChange={(e) => setVolume(Number(e.target.value))}
              className="w-24"
            />
          </div>
          
          {/* Fullscreen */}
          <button onClick={toggleFullscreen} className="text-2xl">
            {isFullscreen ? '🗗' : '⛶'}
          </button>
        </div>
      </div>

      {/* Game Canvas */}
      <div className="flex-1 relative bg-black">
        {isLoading && (
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="text-center">
              <div className="animate-spin text-6xl mb-4">⚙️</div>
              <p className="text-xl">Loading {game.title}...</p>
              <p className="text-text-secondary mt-2">Initializing Nova Runtime</p>
            </div>
          </div>
        )}
        
        <canvas
          ref={canvasRef}
          className="w-full h-full"
          style={{ display: isLoading ? 'none' : 'block' }}
        />
      </div>

      {/* Status Bar */}
      <div className="bg-bg-secondary border-t border-border px-4 py-2 flex items-center justify-between text-sm text-text-secondary">
        <div>FPS: 60 | Draw Calls: 42 | Entities: 156</div>
        <div>Version: {game.version}</div>
      </div>
    </div>
  );
};

export default GamePlayer;
