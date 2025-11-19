/**
 * NOVA ENGINE - Homepage
 * Uses the ACTUAL planet background image provided by user
 * Invisible clickable hotspots positioned over the real planets
 */

import React, { useState } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface HomePageProps {
  platform: UnifiedPlatformCore;
  isLoggedIn: boolean;
  onNavigate: (path: string) => void;
}

export const HomePage: React.FC<HomePageProps> = ({
  platform: _platform,
  isLoggedIn,
  onNavigate,
}) => {
  const [hoveredPlanet, setHoveredPlanet] = useState<string | null>(null);

  // Clickable hotspot areas positioned over the actual planets in the background image
  // These are invisible overlays with proper touch targets
  const planetHotspots = [
    {
      id: 'editor',
      name: 'Editor',
      description: 'Create and design your games',
      path: '/editor',
      // Top-right planet with ring - adjusted for better hit detection
      position: { top: '10%', right: '10%', width: '200px', height: '200px' },
    },
    {
      id: 'hub',
      name: 'Game Hub',
      description: 'Browse and discover games',
      path: '/hub',
      // Bottom-left large planet with ring - adjusted
      position: { bottom: '12%', left: '10%', width: '220px', height: '220px' },
    },
    {
      id: 'multiplayer',
      name: 'Multiplayer',
      description: 'Play online with friends',
      path: '/multiplayer',
      // Top-left smaller planet - adjusted
      position: { top: '15%', left: '15%', width: '140px', height: '140px' },
    },
    {
      id: 'social',
      name: 'Social',
      description: 'Connect with the community',
      path: '/social',
      // Right side planet - adjusted
      position: { top: '35%', right: '15%', width: '150px', height: '150px' },
    },
    {
      id: 'settings',
      name: 'Settings',
      description: 'Configure your experience',
      path: '/settings',
      // Bottom right planet - adjusted
      position: {
        bottom: '25%',
        right: '18%',
        width: '130px',
        height: '130px',
      },
    },
    {
      id: 'launcher',
      name: 'My Games',
      description: 'Your game library',
      path: '/launcher',
      // Bottom center-right planet - adjusted
      position: {
        bottom: '18%',
        right: '38%',
        width: '120px',
        height: '120px',
      },
    },
  ];

  const handlePlanetClick = (path: string) => {
    if (!isLoggedIn) {
      alert('Please Login or Sign Up to access features');
      return;
    }
    onNavigate(path);
  };

  return (
    <div className="homepage-with-real-bg">
      {/* Top Bar with Auth Buttons */}
      <div className="home-topbar">
        {!isLoggedIn && (
          <div className="home-auth-btns">
            <button
              className="home-login-btn"
              onClick={() => onNavigate('/login')}
            >
              Login
            </button>
            <button
              className="home-signup-btn"
              onClick={() => onNavigate('/register')}
            >
              Sign Up
            </button>
          </div>
        )}
      </div>

      {/* Main Planet Center Title */}
      <div className="center-title">
        <div className="home-logo">
          <span className="logo-nova">NOVA</span>
          <span className="logo-engine">ENGINE</span>
        </div>
      </div>

      {/* Invisible clickable planet hotspots over the real background image */}
      <div className="planet-hotspots-container">
        {planetHotspots.map((planet) => (
          <div
            key={planet.id}
            className="planet-wrapper"
            style={planet.position}
          >
            {/* Simple text label above planet - always visible */}
            <div className="planet-label">
              {planet.name}
              {!isLoggedIn && <span className="lock-icon">🔒</span>}
            </div>

            {/* Invisible clickable hotspot */}
            <div
              className={`planet-hotspot ${hoveredPlanet === planet.id ? 'hovered' : ''} ${!isLoggedIn ? 'locked' : ''}`}
              onClick={() => handlePlanetClick(planet.path)}
              onMouseEnter={() => setHoveredPlanet(planet.id)}
              onMouseLeave={() => setHoveredPlanet(null)}
            >
              {/* Subtle hover indicator */}
              <div className="hotspot-indicator"></div>
            </div>
          </div>
        ))}
      </div>

      {/* Login prompt for unauthenticated users */}
      {!isLoggedIn && (
        <div className="login-reminder">
          <p>Hover over planets to explore • Login to access features</p>
        </div>
      )}

      <style>{`
        .homepage-with-real-bg {
          width: 100%;
          min-height: 100vh;
          /* USE ACTUAL PLANET BACKGROUND IMAGE */
          background-image: url('/homepagebackground.png');
          background-size: cover;
          background-position: center;
          background-repeat: no-repeat;
          background-attachment: fixed;
          position: relative;
          overflow: hidden;
        }

        /* Top Bar - Minimal, just auth buttons */
        .home-topbar {
          position: absolute;
          top: 30px;
          right: 40px;
          z-index: 100;
        }

        .home-auth-btns {
          display: flex;
          gap: 15px;
        }

        .home-login-btn,
        .home-signup-btn {
          padding: 10px 28px;
          border-radius: 6px;
          font-weight: 600;
          font-size: 14px;
          cursor: pointer;
          transition: all 0.2s ease;
          border: none;
          text-transform: uppercase;
          letter-spacing: 1px;
        }

        .home-login-btn {
          background: transparent;
          border: 1px solid rgba(255, 255, 255, 0.3);
          color: white;
        }

        .home-login-btn:hover {
          background: rgba(255, 255, 255, 0.1);
          border-color: rgba(255, 255, 255, 0.5);
        }

        .home-signup-btn {
          background: rgba(255, 255, 255, 0.15);
          color: white;
          border: 1px solid rgba(255, 255, 255, 0.3);
        }

        .home-signup-btn:hover {
          background: rgba(255, 255, 255, 0.25);
        }

        /* Center Title under main planet */
        .center-title {
          position: absolute;
          top: 52%;
          left: 50%;
          transform: translate(-50%, -50%);
          z-index: 50;
        }

        .home-logo {
          display: flex;
          flex-direction: column;
          align-items: center;
          text-align: center;
        }

        .logo-nova {
          font-size: 72px;
          font-weight: 900;
          letter-spacing: 12px;
          color: white;
          text-shadow: 
            0 0 20px rgba(255, 255, 255, 0.5),
            0 0 40px rgba(123, 47, 247, 0.3),
            0 2px 8px rgba(0, 0, 0, 0.8);
        }

        .logo-engine {
          font-size: 18px;
          font-weight: 300;
          letter-spacing: 8px;
          color: rgba(255, 255, 255, 0.8);
          margin-top: 10px;
          text-shadow: 0 2px 8px rgba(0, 0, 0, 0.8);
        }

        /* Planet Hotspots Container */
        .planet-hotspots-container {
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
        }

        /* Planet Wrapper */
        .planet-wrapper {
          position: absolute;
          display: flex;
          flex-direction: column;
          align-items: center;
        }

        /* Simple text label above planet - always visible, no box */
        .planet-label {
          position: absolute;
          top: -35px;
          left: 50%;
          transform: translateX(-50%);
          font-size: 16px;
          font-weight: 600;
          color: white;
          text-shadow: 
            0 2px 8px rgba(0, 0, 0, 0.9),
            0 0 20px rgba(0, 0, 0, 0.5);
          letter-spacing: 1px;
          white-space: nowrap;
          z-index: 20;
          text-transform: uppercase;
        }

        .lock-icon {
          margin-left: 6px;
          font-size: 14px;
          opacity: 0.8;
        }

        /* Clickable hotspot - properly sized for touch */
        .planet-hotspot {
          width: 100%;
          height: 100%;
          cursor: pointer;
          border-radius: 50%;
          position: relative;
          z-index: 10;
          /* Ensure proper hit detection */
          touch-action: manipulation;
          -webkit-tap-highlight-color: transparent;
          /* Minimum touch target size for mobile */
          min-width: 44px;
          min-height: 44px;
        }

        .planet-hotspot.locked {
          cursor: not-allowed;
        }

        /* Subtle hover/active indicator */
        .hotspot-indicator {
          width: 100%;
          height: 100%;
          border-radius: 50%;
          border: 3px solid transparent;
          transition: all 0.3s ease;
          position: absolute;
          top: 0;
          left: 0;
          pointer-events: none;
          /* Add subtle background for better visibility */
          background: rgba(168, 85, 247, 0.05);
        }

        .planet-hotspot:hover .hotspot-indicator {
          border-color: rgba(255, 255, 255, 0.5);
          box-shadow: 0 0 25px rgba(255, 255, 255, 0.4);
          background: rgba(168, 85, 247, 0.15);
        }

        .planet-hotspot:active .hotspot-indicator {
          border-color: rgba(255, 255, 255, 0.7);
          box-shadow: 0 0 30px rgba(255, 255, 255, 0.6);
          background: rgba(168, 85, 247, 0.25);
          transform: scale(0.95);
        }

        /* Login Reminder - cleaner design */
        .login-reminder {
          position: absolute;
          bottom: 50px;
          left: 50%;
          transform: translateX(-50%);
          background: rgba(0, 0, 0, 0.6);
          border: 1px solid rgba(255, 255, 255, 0.2);
          border-radius: 8px;
          padding: 12px 30px;
          backdrop-filter: blur(10px);
          z-index: 50;
        }

        .login-reminder p {
          margin: 0;
          color: rgba(255, 255, 255, 0.9);
          font-size: 14px;
          font-weight: 400;
          letter-spacing: 0.5px;
        }

        @media (max-width: 768px) {
          .home-topbar {
            top: 20px;
            right: 20px;
          }

          .logo-nova {
            font-size: 48px;
            letter-spacing: 8px;
          }

          .logo-engine {
            font-size: 14px;
            letter-spacing: 6px;
          }

          .planet-label {
            font-size: 14px;
            top: -30px;
          }

          .home-login-btn,
          .home-signup-btn {
            padding: 8px 20px;
            font-size: 12px;
          }
        }
      `}</style>
    </div>
  );
};
