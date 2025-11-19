/**
 * NOVA ENGINE - Unified Navigation V2 (Clean, Working Version)
 * Space-themed sidebar navigation matching Image 2 mockup
 * Now with mobile-responsive design
 */

import React, { useState, useEffect } from 'react';

interface UnifiedNavigationProps {
  currentMode: string;
  onModeChange: (mode: string) => void;
  isLoggedIn?: boolean;
  currentUser?: any;
}

export const UnifiedNavigation: React.FC<UnifiedNavigationProps> = ({
  currentMode,
  onModeChange,
  isLoggedIn = false,
  currentUser,
}) => {
  const [isMobile, setIsMobile] = useState(false);
  const [isCollapsed, setIsCollapsed] = useState(false);
  const [hasInitialized, setHasInitialized] = useState(false);

  // Detect mobile and auto-collapse on mobile (only on initial load)
  useEffect(() => {
    const checkMobile = () => {
      const mobile = window.innerWidth <= 768;
      setIsMobile(mobile);
      
      // Only auto-collapse on initial load, not on every resize
      if (!hasInitialized && mobile) {
        setIsCollapsed(true);
        setHasInitialized(true);
      }
    };

    checkMobile();
    window.addEventListener('resize', checkMobile);
    return () => window.removeEventListener('resize', checkMobile);
  }, [hasInitialized]);
  const navItems = [
    { id: 'hub', icon: '🏠', label: 'Hub' },
    { id: 'editor', icon: '✏️', label: 'Editor' },
    { id: 'launcher', icon: '🎮', label: 'Launcher' },
    { id: 'multiplayer', icon: '🌐', label: 'Multiplayer' },
    { id: 'social', icon: '👥', label: 'Social' },
    { id: 'settings', icon: '⚙️', label: 'Settings' },
  ];

  const containerStyle: React.CSSProperties = {
    width: isCollapsed ? '70px' : '240px',
    height: '100vh',
    background:
      'linear-gradient(180deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%)',
    borderRight: '2px solid rgba(123, 47, 247, 0.3)',
    display: 'flex',
    flexDirection: 'column',
    padding: '20px 0',
    color: 'white',
    position: 'fixed',
    left: 0,
    top: isMobile ? 0 : 70, // Full height on mobile
    zIndex: isMobile ? 100 : 90, // Higher z-index on mobile
    transition: 'width 0.3s ease, left 0.3s ease',
    overflow: isCollapsed ? 'hidden' : 'auto',
  };

  const logoSection: React.CSSProperties = {
    padding: '0 20px 20px',
    borderBottom: '1px solid rgba(168, 85, 247, 0.3)',
    marginBottom: '20px',
  };

  const logoStyle: React.CSSProperties = {
    fontSize: '24px',
    fontWeight: 900,
    letterSpacing: '3px',
    background:
      'linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%)',
    WebkitBackgroundClip: 'text',
    WebkitTextFillColor: 'transparent',
    backgroundClip: 'text',
  };

  const navList: React.CSSProperties = {
    flex: 1,
    padding: '0 10px',
  };

  const navItemStyle = (isActive: boolean): React.CSSProperties => ({
    display: 'flex',
    alignItems: 'center',
    justifyContent: isCollapsed ? 'center' : 'flex-start',
    gap: '12px',
    padding: isCollapsed ? '12px' : '12px 16px',
    margin: '4px 0',
    background: isActive
      ? 'linear-gradient(135deg, rgba(168, 85, 247, 0.3), rgba(236, 72, 153, 0.3))'
      : 'transparent',
    border: isActive
      ? '1px solid rgba(168, 85, 247, 0.5)'
      : '1px solid transparent',
    borderRadius: '8px',
    cursor: 'pointer',
    fontSize: '14px',
    fontWeight: 600,
    color: isActive ? '#fff' : '#a0a0a0',
    transition: 'all 0.3s',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
  });

  // Floating toggle button style - always visible
  const permanentToggleStyle: React.CSSProperties = {
    position: 'fixed',
    left: isCollapsed ? '10px' : '250px',
    top: isMobile ? '10px' : '80px',
    background:
      'linear-gradient(135deg, rgba(168, 85, 247, 0.9), rgba(236, 72, 153, 0.9))',
    border: '2px solid rgba(168, 85, 247, 0.8)',
    borderRadius: '50%',
    color: 'white',
    width: '44px',
    height: '44px',
    cursor: 'pointer',
    fontSize: '20px',
    zIndex: 101,
    transition: 'all 0.3s ease',
    boxShadow: '0 4px 16px rgba(123, 47, 247, 0.5)',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
  };

  const userSection: React.CSSProperties = {
    padding: '20px',
    borderTop: '1px solid rgba(168, 85, 247, 0.3)',
  };

  const userCardStyle: React.CSSProperties = {
    display: 'flex',
    alignItems: 'center',
    gap: '12px',
    padding: '12px',
    background: 'rgba(168, 85, 247, 0.1)',
    borderRadius: '8px',
    border: '1px solid rgba(168, 85, 247, 0.3)',
  };

  const avatarStyle: React.CSSProperties = {
    width: '40px',
    height: '40px',
    borderRadius: '50%',
    background: 'linear-gradient(135deg, #a855f7, #ec4899)',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '20px',
    fontWeight: 'bold',
  };

  return (
    <>
      {/* Floating Toggle Button - Always visible for user control */}
      <button
        style={permanentToggleStyle}
        onClick={() => setIsCollapsed(!isCollapsed)}
        onMouseEnter={(e) => {
          e.currentTarget.style.transform = 'scale(1.1)';
          e.currentTarget.style.boxShadow =
            '0 6px 20px rgba(123, 47, 247, 0.7)';
        }}
        onMouseLeave={(e) => {
          e.currentTarget.style.transform = 'scale(1)';
          e.currentTarget.style.boxShadow =
            '0 4px 16px rgba(123, 47, 247, 0.5)';
        }}
        title={isCollapsed ? 'Show Navigation' : 'Hide Navigation'}
      >
        {isCollapsed ? '☰' : '✕'}
      </button>

      {/* Navigation Panel */}
      <div id="navigation-panel" style={containerStyle}>
        {/* Logo */}
        {!isCollapsed && (
          <div style={logoSection}>
            <div style={logoStyle}>NOVA</div>
            <div
              style={{ fontSize: '12px', color: '#a855f7', marginTop: '4px' }}
            >
              ENGINE
            </div>
          </div>
        )}

        {/* Navigation Items */}
        <div style={navList}>
          {navItems.map((item) => (
            <div
              key={item.id}
              style={navItemStyle(currentMode === item.id)}
              onClick={() => {
                onModeChange(item.id);
                if (isMobile) setIsCollapsed(true); // Auto-collapse after selection on mobile
              }}
              onMouseEnter={(e) => {
                if (currentMode !== item.id) {
                  e.currentTarget.style.background = 'rgba(168, 85, 247, 0.1)';
                  e.currentTarget.style.color = '#fff';
                }
              }}
              onMouseLeave={(e) => {
                if (currentMode !== item.id) {
                  e.currentTarget.style.background = 'transparent';
                  e.currentTarget.style.color = '#a0a0a0';
                }
              }}
              title={isCollapsed ? item.label : ''}
            >
              <span style={{ fontSize: '20px' }}>{item.icon}</span>
              {!isCollapsed && <span>{item.label}</span>}
            </div>
          ))}
        </div>

        {/* User Info */}
        {!isCollapsed && isLoggedIn && currentUser && (
          <div style={userSection}>
            <div style={userCardStyle}>
              <div style={avatarStyle}>
                {currentUser?.username?.[0]?.toUpperCase() || 'U'}
              </div>
              <div style={{ flex: 1 }}>
                <div style={{ fontSize: '14px', fontWeight: 600 }}>
                  {currentUser?.username || 'User'}
                </div>
                <div style={{ fontSize: '12px', color: '#a855f7' }}>
                  Level 45
                </div>
              </div>
            </div>
          </div>
        )}
      </div>
    </>
  );
};

export default UnifiedNavigation;
