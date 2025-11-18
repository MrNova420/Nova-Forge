/**
 * Unified Navigation - Sidebar with mode switching
 */

import React from 'react';
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

  const handleNavigation = (item: (typeof navItems)[0]) => {
    // Navigate using React Router
    navigate(item.path);
    // Also update platform state
    onModeChange(item.id);
  };

  // Determine active item from current route
  const activeItem =
    navItems.find((item) => location.pathname.startsWith(item.path))?.id ||
    'hub';

  return (
    <nav className="unified-navigation">
      {navItems.map((item) => (
        <button
          key={item.id}
          className={`nav-item ${activeItem === item.id ? 'active' : ''}`}
          onClick={() => handleNavigation(item)}
          title={item.label}
        >
          <span className="icon">{item.icon}</span>
          <span className="label">{item.label}</span>
        </button>
      ))}

      <style>{`
        .unified-navigation {
          width: 200px;
          background: #2a2a2a;
          border-right: 1px solid #3a3a3a;
          display: flex;
          flex-direction: column;
          padding: 20px 10px;
          gap: 5px;
        }

        .nav-item {
          display: flex;
          align-items: center;
          gap: 12px;
          padding: 12px 16px;
          background: transparent;
          border: none;
          border-radius: 8px;
          color: #ffffff;
          cursor: pointer;
          transition: all 0.3s;
          text-align: left;
        }

        .nav-item:hover {
          background: #3a3a3a;
        }

        .nav-item.active {
          background: #7b2ff7;
        }

        .icon {
          font-size: 1.4em;
        }

        .label {
          font-size: 1em;
        }
      `}</style>
    </nav>
  );
};
