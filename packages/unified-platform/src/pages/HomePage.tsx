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
  // These are invisible overlays that become visible on hover
  const planetHotspots = [
    {
      id: 'editor',
      name: 'Editor',
      description: 'Create and design your games',
      path: '/editor',
      // Top-right planet with ring (as you specified)
      position: { top: '8%', right: '8%', width: '180px', height: '180px' },
    },
    {
      id: 'hub',
      name: 'Game Hub',
      description: 'Browse and discover games',
      path: '/hub',
      // Bottom-left large planet with ring (as you specified)
      position: { bottom: '15%', left: '8%', width: '200px', height: '200px' },
    },
    {
      id: 'multiplayer',
      name: 'Multiplayer',
      description: 'Play online with friends',
      path: '/multiplayer',
      // Top-left smaller planet
      position: { top: '18%', left: '12%', width: '120px', height: '120px' },
    },
    {
      id: 'social',
      name: 'Social',
      description: 'Connect with the community',
      path: '/social',
      // Right side planet
      position: { top: '38%', right: '12%', width: '130px', height: '130px' },
    },
    {
      id: 'settings',
      name: 'Settings',
      description: 'Configure your experience',
      path: '/settings',
      // Bottom right planet
      position: {
        bottom: '28%',
        right: '15%',
        width: '110px',
        height: '110px',
      },
    },
    {
      id: 'launcher',
      name: 'My Games',
      description: 'Your game library',
      path: '/launcher',
      // Bottom center-right planet
      position: {
        bottom: '20%',
        right: '35%',
        width: '100px',
        height: '100px',
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
      {/* Top Bar */}
      <div className="home-topbar">
        <div className="home-logo">
          <span className="logo-nova">NOVA</span>
          <span className="logo-engine">ENGINE</span>
        </div>
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

      {/* Invisible clickable planet hotspots over the real background image */}
      <div className="planet-hotspots-container">
        {planetHotspots.map((planet) => (
          <div
            key={planet.id}
            className={`planet-hotspot ${hoveredPlanet === planet.id ? 'hovered' : ''} ${!isLoggedIn ? 'locked' : ''}`}
            style={planet.position}
            onClick={() => handlePlanetClick(planet.path)}
            onMouseEnter={() => setHoveredPlanet(planet.id)}
            onMouseLeave={() => setHoveredPlanet(null)}
          >
            {/* Tooltip that appears on hover */}
            <div className="planet-tooltip">
              <h3>{planet.name}</h3>
              <p>{planet.description}</p>
              {!isLoggedIn && (
                <span className="lock-badge">🔒 Login Required</span>
              )}
            </div>

            {/* Visual indicator circle (subtle, only visible on hover) */}
            <div className="hotspot-circle"></div>
          </div>
        ))}
      </div>

      {/* Login prompt for unauthenticated users */}
      {!isLoggedIn && (
        <div className="login-reminder">
          <p>👆 Login or Sign Up to explore all features</p>
        </div>
      )}

      <style>{`
        .homepage-with-real-bg {
          width: 100%;
          min-height: 100vh;
          /* USE LOCAL PLANET BACKGROUND IMAGE */
          background-image: url('/planet-background.svg');
          background-size: cover;
          background-position: center;
          background-repeat: no-repeat;
          background-attachment: fixed;
          position: relative;
          overflow: hidden;
        }

        /* Top Bar */
        .home-topbar {
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          height: 80px;
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 0 40px;
          background: linear-gradient(180deg, rgba(0,0,0,0.6) 0%, transparent 100%);
          z-index: 100;
        }

        .home-logo {
          display: flex;
          align-items: baseline;
          gap: 10px;
        }

        .logo-nova {
          font-size: 36px;
          font-weight: 900;
          letter-spacing: 6px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          text-shadow: 0 0 20px rgba(123, 47, 247, 0.5);
        }

        .logo-engine {
          font-size: 14px;
          font-weight: 300;
          letter-spacing: 3px;
          color: #a0a0ff;
        }

        .home-auth-btns {
          display: flex;
          gap: 15px;
        }

        .home-login-btn,
        .home-signup-btn {
          padding: 12px 30px;
          border-radius: 25px;
          font-weight: 700;
          font-size: 15px;
          cursor: pointer;
          transition: all 0.3s ease;
          border: none;
        }

        .home-login-btn {
          background: rgba(123, 47, 247, 0.3);
          border: 2px solid rgba(123, 47, 247, 0.6);
          color: white;
        }

        .home-login-btn:hover {
          background: rgba(123, 47, 247, 0.5);
          transform: translateY(-2px);
        }

        .home-signup-btn {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          color: white;
        }

        .home-signup-btn:hover {
          transform: translateY(-2px);
          box-shadow: 0 8px 25px rgba(123, 47, 247, 0.4);
        }

        /* Planet Hotspots Container */
        .planet-hotspots-container {
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
        }

        /* Individual Planet Hotspot (invisible clickable area) */
        .planet-hotspot {
          position: absolute;
          cursor: pointer;
          border-radius: 50%;
          transition: all 0.3s ease;
          z-index: 10;
        }

        .planet-hotspot.locked {
          cursor: not-allowed;
        }

        /* Subtle circle indicator (only visible on hover) */
        .hotspot-circle {
          width: 100%;
          height: 100%;
          border-radius: 50%;
          border: 3px solid transparent;
          transition: all 0.3s ease;
          position: absolute;
          top: 0;
          left: 0;
        }

        .planet-hotspot:hover .hotspot-circle {
          border-color: rgba(123, 47, 247, 0.6);
          box-shadow: 0 0 30px rgba(123, 47, 247, 0.8), inset 0 0 20px rgba(123, 47, 247, 0.3);
          animation: pulse-ring 1.5s ease-in-out infinite;
        }

        @keyframes pulse-ring {
          0%, 100% {
            transform: scale(1);
            opacity: 1;
          }
          50% {
            transform: scale(1.05);
            opacity: 0.8;
          }
        }

        /* Tooltip that appears on hover */
        .planet-tooltip {
          position: absolute;
          bottom: calc(100% + 15px);
          left: 50%;
          transform: translateX(-50%) translateY(10px);
          background: linear-gradient(135deg, rgba(26,0,51,0.98) 0%, rgba(58,12,88,0.98) 100%);
          border: 2px solid rgba(123, 47, 247, 0.6);
          border-radius: 16px;
          padding: 20px 25px;
          min-width: 250px;
          text-align: center;
          opacity: 0;
          visibility: hidden;
          transition: all 0.3s ease;
          backdrop-filter: blur(10px);
          box-shadow: 0 10px 40px rgba(0, 0, 0, 0.5);
          pointer-events: none;
          z-index: 100;
        }

        .planet-hotspot:hover .planet-tooltip {
          opacity: 1;
          visibility: visible;
          transform: translateX(-50%) translateY(0);
        }

        .planet-tooltip h3 {
          margin: 0 0 8px 0;
          font-size: 20px;
          font-weight: 700;
          color: white;
          background: linear-gradient(135deg, #ff6ec4 0%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
        }

        .planet-tooltip p {
          margin: 0 0 10px 0;
          font-size: 14px;
          color: rgba(255, 255, 255, 0.8);
        }

        .lock-badge {
          display: inline-block;
          padding: 6px 14px;
          background: rgba(239, 68, 68, 0.3);
          border: 1px solid rgba(239, 68, 68, 0.6);
          border-radius: 20px;
          font-size: 13px;
          color: #ef4444;
          font-weight: 600;
        }



        /* Login Reminder */
        .login-reminder {
          position: absolute;
          bottom: 40px;
          left: 50%;
          transform: translateX(-50%);
          background: linear-gradient(135deg, rgba(123, 47, 247, 0.95) 0%, rgba(255, 110, 196, 0.95) 100%);
          border: 2px solid rgba(255, 255, 255, 0.3);
          border-radius: 30px;
          padding: 15px 40px;
          backdrop-filter: blur(10px);
          box-shadow: 0 10px 40px rgba(123, 47, 247, 0.5);
          animation: float 3s ease-in-out infinite;
        }

        @keyframes float {
          0%, 100% { transform: translateX(-50%) translateY(0); }
          50% { transform: translateX(-50%) translateY(-10px); }
        }

        .login-reminder p {
          margin: 0;
          color: white;
          font-size: 16px;
          font-weight: 600;
          text-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);
        }

        @media (max-width: 768px) {
          .home-topbar {
            padding: 0 20px;
          }

          .logo-nova {
            font-size: 28px;
          }

          .planet-tooltip {
            min-width: 200px;
            padding: 15px 20px;
          }

          .planet-tooltip h3 {
            font-size: 18px;
          }

          .planet-tooltip p {
            font-size: 13px;
          }
        }
      `}</style>
    </div>
  );
};
