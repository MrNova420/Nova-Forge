/**
 * User Profile Page - User profiles with game library
 */
import React from 'react';
export const ProfilePage: React.FC = () => {
  return (
    <div className="profile-page">
      <div className="profile-header">
        <h1>Player Profile</h1>
        <div className="avatar">👤</div>
      </div>
      <div className="profile-stats">
        <div className="stat"><span>Games Played:</span> 42</div>
        <div className="stat"><span>Total Hours:</span> 156</div>
        <div className="stat"><span>Achievements:</span> 89</div>
      </div>
      <div className="game-library">
        <h2>My Games</h2>
        <div className="games-grid">{/* Game cards */}</div>
      </div>
    </div>
  );
};
