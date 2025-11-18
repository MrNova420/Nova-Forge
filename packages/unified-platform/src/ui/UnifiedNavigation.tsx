/**
 * Unified Navigation - Sidebar with mode switching
 */

import React, { useCallback } from 'react';
import { useNavigate, useLocation } from 'react-router-dom';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface UnifiedNavigationProps {
  platform: UnifiedPlatformCore;
  currentMode: string;
  onModeChange: (mode: string) => void;
}

export const UnifiedNavigation: React.FC<UnifiedNavigationProps> = ({
  platform: _platform,
  currentMode: _currentMode,
  onModeChange,
}) => {
  const navigate = useNavigate();
  const location = useLocation();

  const navItems = [
    { id: 'hub', label: 'Hub', icon: '🏠', path: '/hub' },
    { id: 'editor', label: 'Editor', icon: '🛠️', path: '/editor' },
    { id: 'launcher', label: 'Launcher', icon: '🎮', path: '/launcher' },
    {
      id: 'multiplayer',
      label: 'Multiplayer',
      icon: '🌐',
      path: '/multiplayer',
    },
    { id: 'social', label: 'Social', icon: '👥', path: '/social' },
    { id: 'settings', label: 'Settings', icon: '⚙️', path: '/settings' },
  ];

  const handleNavigation = useCallback(
    (item: (typeof navItems)[0]) => {
      // Navigate using React Router
      navigate(item.path);
      // Also update platform state
      onModeChange(item.id);
    },
    [navigate, onModeChange]
  );

  // Determine active item from current route
  const activeItem =
    navItems.find((item) => location.pathname.startsWith(item.path))?.id ||
    'hub';

  return (
    <nav
      className="unified-navigation"
      style={{
        width: '200px',
        background: '#2a2a2a',
        borderRight: '1px solid #3a3a3a',
        display: 'flex',
        flexDirection: 'column',
        padding: '20px 10px',
        gap: '5px',
      }}
    >
      {navItems.map((item) => (
        <button
          key={item.id}
          className={`nav-item ${activeItem === item.id ? 'active' : ''}`}
          onClick={(e) => {
            e.preventDefault();
            e.stopPropagation();
            handleNavigation(item);
          }}
          title={item.label}
          type="button"
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '12px',
            padding: '12px 16px',
            background: activeItem === item.id ? '#7b2ff7' : 'transparent',
            border: 'none',
            borderRadius: '8px',
            color: '#ffffff',
            cursor: 'pointer',
            transition: 'all 0.3s',
            textAlign: 'left',
            fontSize: '1em',
            width: '100%',
          }}
          onMouseEnter={(e) => {
            if (activeItem !== item.id) {
              e.currentTarget.style.background = '#3a3a3a';
            }
          }}
          onMouseLeave={(e) => {
            if (activeItem !== item.id) {
              e.currentTarget.style.background = 'transparent';
            }
          }}
        >
          <span style={{ fontSize: '1.4em' }}>{item.icon}</span>
          <span style={{ fontSize: '1em' }}>{item.label}</span>
        </button>
      ))}
    </nav>
  );
};
