/**
 * Leaderboards Page - Global and game-specific leaderboards
 */
import React from 'react';
export const LeaderboardsPage: React.FC = () => {
  return (
    <div className="leaderboards-page">
      <h1>Leaderboards</h1>
      <div className="leaderboard-tabs">
        <button>Global</button>
        <button>Friends</button>
        <button>Weekly</button>
      </div>
      <div className="leaderboard-list">
        <div className="leaderboard-entry rank-1">
          <span className="rank">1</span>
          <span className="player">TopPlayer</span>
          <span className="score">9,999</span>
        </div>
        <div className="leaderboard-entry rank-2">
          <span className="rank">2</span>
          <span className="player">SecondBest</span>
          <span className="score">8,500</span>
        </div>
      </div>
    </div>
  );
};
