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

import React from 'react';
import { Link } from 'react-router-dom';
import { Game } from '../store/gameStore';

interface GameCardProps {
  game: Game;
}

const GameCard: React.FC<GameCardProps> = ({ game }) => {
  return (
    <Link to={`/games/${game.id}`} className="card block">
      <div className="aspect-video bg-bg-tertiary relative overflow-hidden">
        {game.thumbnail ? (
          <img
            src={game.thumbnail}
            alt={game.title}
            className="w-full h-full object-cover"
          />
        ) : (
          <div className="w-full h-full flex items-center justify-center text-text-secondary">
            No Image
          </div>
        )}
        <div className="absolute top-2 right-2 bg-accent px-2 py-1 rounded text-sm font-semibold">
          ★ {game.rating.toFixed(1)}
        </div>
      </div>
      
      <div className="p-4">
        <h3 className="font-bold text-lg mb-1 truncate">{game.title}</h3>
        <p className="text-text-secondary text-sm mb-2">by {game.author}</p>
        <p className="text-text-secondary text-sm line-clamp-2 mb-3">
          {game.description}
        </p>
        
        <div className="flex items-center gap-4 text-xs text-text-secondary">
          <span>👁️ {game.playCount.toLocaleString()} plays</span>
          <span>⬇️ {game.downloadCount.toLocaleString()}</span>
        </div>
        
        <div className="flex gap-1 mt-3 flex-wrap">
          {game.tags.slice(0, 3).map((tag) => (
            <span
              key={tag}
              className="px-2 py-1 bg-bg-tertiary rounded text-xs"
            >
              {tag}
            </span>
          ))}
        </div>
      </div>
    </Link>
  );
};

export default GameCard;
