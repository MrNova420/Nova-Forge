/**
 * Friends Page - Friend list and social features
 */
import React from 'react';
export const FriendsPage: React.FC = () => {
  return (
    <div className="friends-page">
      <h1>Friends</h1>
      <div className="friends-list">
        <div className="friend-card">
          <span>🟢</span> <strong>Player1</strong> - Playing Game X
        </div>
        <div className="friend-card">
          <span>🔴</span> <strong>Player2</strong> - Offline
        </div>
      </div>
      <div className="friend-requests">
        <h2>Friend Requests</h2>
        <p>No pending requests</p>
      </div>
    </div>
  );
};
