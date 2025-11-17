/**
 * Unified Top Bar - User profile, notifications, search
 */

import React, { useState } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface UnifiedTopBarProps {
  platform: UnifiedPlatformCore;
  isLoggedIn: boolean;
  currentUser: any;
  currentMode: string;
}

export const UnifiedTopBar: React.FC<UnifiedTopBarProps> = ({
  platform,
  isLoggedIn,
  currentUser,
  currentMode,
}) => {
  const [showNotifications, setShowNotifications] = useState(false);

  const handleLogout = () => {
    platform.logout();
  };

  return (
    <div className="unified-topbar">
      <div className="topbar-left">
        <h1 className="logo">Nova Engine</h1>
        <span className="mode-indicator">
          {currentMode.charAt(0).toUpperCase() + currentMode.slice(1)}
        </span>
      </div>

      <div className="topbar-center">
        <div className="search-bar">
          <input type="text" placeholder="Search..." />
          <button>🔍</button>
        </div>
      </div>

      <div className="topbar-right">
        {isLoggedIn ? (
          <>
            <button
              className="notification-btn"
              onClick={() => setShowNotifications(!showNotifications)}
            >
              🔔
            </button>
            <div className="user-menu">
              <span>{currentUser?.username || 'User'}</span>
              <button onClick={handleLogout}>Logout</button>
            </div>
          </>
        ) : (
          <button onClick={() => platform.emit('showLogin')}>Login</button>
        )}
      </div>

      <style>{`
        .unified-topbar {
          height: 60px;
          background: #2a2a2a;
          border-bottom: 1px solid #3a3a3a;
          display: flex;
          align-items: center;
          justify-content: space-between;
          padding: 0 20px;
          color: white;
        }

        .topbar-left {
          display: flex;
          align-items: center;
          gap: 20px;
        }

        .logo {
          margin: 0;
          font-size: 1.5em;
          background: linear-gradient(45deg, #00d4ff, #7b2ff7);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
        }

        .mode-indicator {
          padding: 4px 12px;
          background: #7b2ff7;
          border-radius: 12px;
          font-size: 0.9em;
        }

        .search-bar {
          display: flex;
          align-items: center;
          background: #1a1a1a;
          border-radius: 20px;
          padding: 8px 16px;
        }

        .search-bar input {
          background: transparent;
          border: none;
          color: white;
          outline: none;
          width: 300px;
        }

        .search-bar button {
          background: transparent;
          border: none;
          color: white;
          cursor: pointer;
          font-size: 1.2em;
        }

        .topbar-right {
          display: flex;
          align-items: center;
          gap: 15px;
        }

        .notification-btn {
          background: transparent;
          border: none;
          font-size: 1.5em;
          cursor: pointer;
        }

        .user-menu {
          display: flex;
          align-items: center;
          gap: 10px;
        }

        .user-menu button {
          padding: 6px 12px;
          background: #7b2ff7;
          border: none;
          border-radius: 4px;
          color: white;
          cursor: pointer;
        }
      `}</style>
    </div>
  );
};
