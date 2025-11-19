/**
 * NOVA ENGINE - Unified Top Bar V2 (Clean, Working Version)
 * Space-themed navigation bar matching Image 2 mockup
 */

import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import type { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

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
  const navigate = useNavigate();
  const [showNotifications, setShowNotifications] = useState(false);
  const [searchQuery, setSearchQuery] = useState('');

  const handleLogout = () => {
    platform.logout();
  };

  const handleLogoClick = () => {
    navigate('/hub'); // Navigate to hub (main page) when logo is clicked
  };

  const handleHomeClick = () => {
    navigate('/hub'); // Navigate to hub home
  };

  const notifications = [
    {
      id: '1',
      text: 'Friend request from SpeedDemon',
      time: '5m ago',
      type: 'friend',
    },
    {
      id: '2',
      text: 'Achievement unlocked: Master Builder',
      time: '1h ago',
      type: 'achievement',
    },
    {
      id: '3',
      text: 'New game update available',
      time: '2h ago',
      type: 'update',
    },
  ];

  const getModeIcon = (mode: string) => {
    const icons: Record<string, string> = {
      hub: '🏠',
      editor: '✏️',
      launcher: '🎮',
      multiplayer: '🌐',
      social: '👥',
      settings: '⚙️',
    };
    return icons[mode] || '🎮';
  };

  const containerStyle: React.CSSProperties = {
    height: '70px',
    background:
      'linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%)',
    borderBottom: '2px solid rgba(123, 47, 247, 0.3)',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: '0 30px',
    color: 'white',
    position: 'relative',
    zIndex: 100,
  };

  const leftSection: React.CSSProperties = {
    display: 'flex',
    alignItems: 'center',
    gap: '25px',
  };

  const logoStyle: React.CSSProperties = {
    display: 'flex',
    alignItems: 'baseline',
    gap: '8px',
    fontSize: '28px',
    fontWeight: 900,
    letterSpacing: '4px',
    background:
      'linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%)',
    WebkitBackgroundClip: 'text',
    WebkitTextFillColor: 'transparent',
    backgroundClip: 'text',
    cursor: 'pointer',
    userSelect: 'none',
  };

  const modeIndicatorStyle: React.CSSProperties = {
    padding: '8px 16px',
    background: 'rgba(168, 85, 247, 0.2)',
    borderRadius: '8px',
    border: '1px solid rgba(168, 85, 247, 0.4)',
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    fontSize: '14px',
    fontWeight: 600,
  };

  const searchContainerStyle: React.CSSProperties = {
    flex: '0 1 500px',
    position: 'relative',
  };

  const searchInputStyle: React.CSSProperties = {
    width: '100%',
    padding: '12px 40px 12px 16px',
    background: 'rgba(168, 85, 247, 0.1)',
    border: '1px solid rgba(168, 85, 247, 0.3)',
    borderRadius: '8px',
    color: '#fff',
    fontSize: '14px',
    outline: 'none',
  };

  const rightSectionStyle: React.CSSProperties = {
    display: 'flex',
    alignItems: 'center',
    gap: '12px',
  };

  const buttonStyle: React.CSSProperties = {
    padding: '10px 16px',
    background: 'linear-gradient(135deg, #a855f7, #ec4899)',
    border: 'none',
    borderRadius: '8px',
    color: '#fff',
    cursor: 'pointer',
    fontSize: '14px',
    fontWeight: '600',
    transition: 'transform 0.2s',
  };

  const iconButtonStyle: React.CSSProperties = {
    ...buttonStyle,
    background: 'rgba(168, 85, 247, 0.2)',
    padding: '10px',
    position: 'relative',
  };

  return (
    <div style={containerStyle}>
      {/* Left Section */}
      <div style={leftSection}>
        <div style={logoStyle} onClick={handleLogoClick} title="Go to Home">
          <span>NOVA</span>
          <span style={{ fontSize: '16px', fontWeight: 400 }}>ENGINE</span>
        </div>
        <button
          type="button"
          aria-label="Navigate to homepage"
          style={{
            ...buttonStyle,
            background: 'rgba(168, 85, 247, 0.2)',
            padding: '8px 16px',
            fontSize: '13px',
          }}
          onClick={handleHomeClick}
          title="Home"
        >
          🏠 Home
        </button>
        <div style={modeIndicatorStyle}>
          <span>{getModeIcon(currentMode)}</span>
          <span>
            {currentMode.charAt(0).toUpperCase() + currentMode.slice(1)}
          </span>
        </div>
      </div>

      {/* Center Section - Search */}
      <div style={searchContainerStyle}>
        <input
          type="text"
          placeholder="Search games, players, achievements..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
          style={searchInputStyle}
        />
        <span
          style={{
            position: 'absolute',
            right: '12px',
            top: '50%',
            transform: 'translateY(-50%)',
          }}
        >
          🔍
        </span>
      </div>

      {/* Right Section */}
      <div style={rightSectionStyle}>
        {isLoggedIn ? (
          <>
            <button
              style={iconButtonStyle}
              onClick={() => setShowNotifications(!showNotifications)}
            >
              🔔
              <span
                style={{
                  position: 'absolute',
                  top: '4px',
                  right: '4px',
                  background: '#ec4899',
                  borderRadius: '50%',
                  width: '16px',
                  height: '16px',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  fontSize: '10px',
                }}
              >
                3
              </span>
            </button>
            <button
              style={buttonStyle}
              onClick={() => platform.switchMode('social')}
            >
              {currentUser?.username?.[0]?.toUpperCase() || 'U'}
            </button>
            <button
              style={{ ...buttonStyle, background: 'rgba(220, 38, 38, 0.8)' }}
              onClick={handleLogout}
            >
              Logout
            </button>
          </>
        ) : (
          <button
            style={buttonStyle}
            onClick={() => platform.emit('showLogin')}
          >
            Login
          </button>
        )}
      </div>

      {/* Notifications Dropdown */}
      {showNotifications && (
        <div
          style={{
            position: 'absolute',
            top: '80px',
            right: '100px',
            width: '320px',
            background: 'linear-gradient(135deg, #1a0b2e, #2d1b4e)',
            border: '1px solid rgba(168, 85, 247, 0.3)',
            borderRadius: '12px',
            padding: '16px',
            zIndex: 1000,
          }}
        >
          <div
            style={{
              marginBottom: '12px',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
            }}
          >
            <h4 style={{ margin: 0 }}>Notifications</h4>
            <button
              style={{ ...buttonStyle, padding: '4px 8px', fontSize: '12px' }}
            >
              Mark all read
            </button>
          </div>
          {notifications.map((notif) => (
            <div
              key={notif.id}
              style={{
                padding: '12px',
                background: 'rgba(168, 85, 247, 0.1)',
                borderRadius: '8px',
                marginBottom: '8px',
              }}
            >
              <p style={{ margin: '0 0 4px 0', fontSize: '14px' }}>
                {notif.text}
              </p>
              <span style={{ fontSize: '12px', color: '#a855f7' }}>
                {notif.time}
              </span>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

export default UnifiedTopBar;
