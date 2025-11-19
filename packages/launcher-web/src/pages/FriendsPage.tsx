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
