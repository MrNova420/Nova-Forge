import React from 'react';
import { Game } from '../store/gameStore';
import GameCard from './GameCard';

interface GameGridProps {
  games: Game[];
  isLoading?: boolean;
}

const GameGrid: React.FC<GameGridProps> = ({ games, isLoading }) => {
  if (isLoading) {
    return (
      <div className="grid grid-cols-3 gap-6">
        {[...Array(6)].map((_, i) => (
          <div key={i} className="card animate-pulse">
            <div className="aspect-video bg-bg-tertiary"></div>
            <div className="p-4 space-y-3">
              <div className="h-6 bg-bg-tertiary rounded w-3/4"></div>
              <div className="h-4 bg-bg-tertiary rounded w-1/2"></div>
              <div className="h-12 bg-bg-tertiary rounded"></div>
            </div>
          </div>
        ))}
      </div>
    );
  }

  if (games.length === 0) {
    return (
      <div className="text-center py-12 text-text-secondary">
        <p className="text-xl">No games found</p>
        <p className="mt-2">Try adjusting your filters or search query</p>
      </div>
    );
  }

  return (
    <div className="grid grid-cols-3 gap-6">
      {games.map((game) => (
        <GameCard key={game.id} game={game} />
      ))}
    </div>
  );
};

export default GameGrid;
