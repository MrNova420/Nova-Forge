import React from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { useQuery } from '@tanstack/react-query';
import { apiService } from '../services/apiService';
import GamePlayer from '../components/GamePlayer';

const GamePlayerPage: React.FC = () => {
  const { gameId } = useParams<{ gameId: string }>();
  const navigate = useNavigate();

  const { data: game, isLoading } = useQuery({
    queryKey: ['game', gameId],
    queryFn: async () => {
      // Increment play count
      await apiService.playGame(gameId!);
      return apiService.getGameById(gameId!);
    },
    enabled: !!gameId,
  });

  if (isLoading || !game) {
    return (
      <div className="fixed inset-0 bg-black flex items-center justify-center">
        <div className="text-center">
          <div className="animate-spin text-6xl mb-4">⚙️</div>
          <p className="text-xl">Loading game...</p>
        </div>
      </div>
    );
  }

  return (
    <GamePlayer
      game={game}
      onClose={() => navigate(`/games/${gameId}`)}
    />
  );
};

export default GamePlayerPage;
