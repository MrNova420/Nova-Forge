/**
 * Nova Engine - Space-Themed Homepage
 *
 * Interactive planet-based navigation system
 * - Central large planet (reserved for future feature)
 * - Smaller orbiting planets as navigation buttons
 * - Top-right planet with ring → Editor
 * - Bottom-left large planet with ring → Hub/Game Library
 * - Other planets for additional features
 */

import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface HomePageProps {
  platform?: UnifiedPlatformCore;
  isLoggedIn: boolean;
  onNavigate: (path: string) => void;
}

interface PlanetButton {
  id: string;
  label: string;
  path: string;
  position: { x: string; y: string };
  size: string;
  hasRing?: boolean;
  ringRotation?: string;
  description: string;
}

export const HomePage: React.FC<HomePageProps> = ({
  isLoggedIn,
  onNavigate,
}) => {
  const navigate = useNavigate();
  const [hoveredPlanet, setHoveredPlanet] = useState<string | null>(null);
  const [showLoginPrompt, setShowLoginPrompt] = useState(false);

  // Define planet button positions matching the background image
  const planets: PlanetButton[] = [
    {
      id: 'editor',
      label: 'Editor',
      path: '/editor',
      position: { x: '75%', y: '15%' },
      size: '140px',
      hasRing: true,
      ringRotation: '25deg',
      description: 'Create and design your games',
    },
    {
      id: 'hub',
      label: 'Game Hub',
      path: '/hub',
      position: { x: '18%', y: '70%' },
      size: '160px',
      hasRing: true,
      ringRotation: '-15deg',
      description: 'Browse and discover games',
    },
    {
      id: 'multiplayer',
      label: 'Multiplayer',
      path: '/multiplayer',
      position: { x: '15%', y: '12%' },
      size: '90px',
      description: 'Play online with friends',
    },
    {
      id: 'social',
      label: 'Social',
      path: '/social',
      position: { x: '85%', y: '55%' },
      size: '100px',
      description: 'Connect with the community',
    },
    {
      id: 'settings',
      label: 'Settings',
      path: '/settings',
      position: { x: '82%', y: '82%' },
      size: '95px',
      description: 'Configure your experience',
    },
    {
      id: 'launcher',
      label: 'My Games',
      path: '/launcher',
      position: { x: '45%', y: '85%' },
      size: '85px',
      description: 'Your game library',
    },
  ];

  const handlePlanetClick = (planet: PlanetButton) => {
    if (!isLoggedIn) {
      setShowLoginPrompt(true);
      setTimeout(() => setShowLoginPrompt(false), 3000);
      return;
    }

    navigate(planet.path);
    onNavigate(planet.path);
  };

  const handleAuthClick = (type: 'login' | 'register') => {
    navigate(`/${type}`);
  };

  return (
    <div className="homepage-container">
      {/* Header with Auth Buttons */}
      <header className="homepage-header">
        <div className="logo-section">
          <h1 className="nova-logo">NOVA</h1>
          <span className="tagline">ENGINE</span>
        </div>

        {!isLoggedIn && (
          <div className="auth-buttons">
            <button
              className="btn-login"
              onClick={() => handleAuthClick('login')}
            >
              Login
            </button>
            <button
              className="btn-signup"
              onClick={() => handleAuthClick('register')}
            >
              Sign Up
            </button>
          </div>
        )}
      </header>

      {/* Space Background */}
      <div className="space-background">
        {/* Animated stars */}
        <div className="stars"></div>
        <div className="stars2"></div>
        <div className="stars3"></div>

        {/* Central large planet (reserved for future feature) */}
        <div className="central-planet">
          <div className="planet-glow"></div>
          <div className="planet-surface"></div>
          <div className="planet-rings"></div>
          <div className="planet-label">Coming Soon</div>
        </div>

        {/* Interactive planet buttons */}
        {planets.map((planet) => (
          <div
            key={planet.id}
            className={`planet-button ${hoveredPlanet === planet.id ? 'hovered' : ''}`}
            style={{
              left: planet.position.x,
              top: planet.position.y,
              width: planet.size,
              height: planet.size,
            }}
            onMouseEnter={() => setHoveredPlanet(planet.id)}
            onMouseLeave={() => setHoveredPlanet(null)}
            onClick={() => handlePlanetClick(planet)}
          >
            <div className="planet-orb">
              {planet.hasRing && (
                <div
                  className="planet-ring"
                  style={{
                    transform: `rotateX(75deg) rotateZ(${planet.ringRotation})`,
                  }}
                ></div>
              )}
            </div>
            <div className="planet-info">
              <div className="planet-name">{planet.label}</div>
              <div className="planet-description">{planet.description}</div>
            </div>
            {!isLoggedIn && <div className="lock-indicator">🔒</div>}
          </div>
        ))}

        {/* Login prompt overlay */}
        {showLoginPrompt && (
          <div className="login-prompt">
            <div className="prompt-content">
              <h3>🔒 Authentication Required</h3>
              <p>Please log in or sign up to access this feature</p>
              <div className="prompt-buttons">
                <button onClick={() => handleAuthClick('login')}>Login</button>
                <button onClick={() => handleAuthClick('register')}>
                  Sign Up
                </button>
              </div>
            </div>
          </div>
        )}
      </div>

      <style>{`
        .homepage-container {
          position: fixed;
          top: 0;
          left: 0;
          width: 100vw;
          height: 100vh;
          overflow: hidden;
          background: #000;
        }

        .homepage-header {
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 30px 50px;
          z-index: 100;
          background: linear-gradient(180deg, rgba(0,0,0,0.8) 0%, rgba(0,0,0,0) 100%);
        }

        .logo-section {
          display: flex;
          align-items: baseline;
          gap: 10px;
        }

        .nova-logo {
          font-size: 48px;
          font-weight: 900;
          letter-spacing: 8px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          background-clip: text;
          margin: 0;
          text-shadow: 0 0 30px rgba(123, 47, 247, 0.5);
        }

        .tagline {
          font-size: 20px;
          font-weight: 300;
          letter-spacing: 4px;
          color: #a0a0ff;
        }

        .auth-buttons {
          display: flex;
          gap: 15px;
        }

        .btn-login,
        .btn-signup {
          padding: 12px 30px;
          font-size: 16px;
          font-weight: 600;
          border: none;
          border-radius: 25px;
          cursor: pointer;
          transition: all 0.3s ease;
          text-transform: uppercase;
          letter-spacing: 1px;
        }

        .btn-login {
          background: transparent;
          color: white;
          border: 2px solid rgba(123, 47, 247, 0.8);
        }

        .btn-login:hover {
          background: rgba(123, 47, 247, 0.2);
          box-shadow: 0 0 20px rgba(123, 47, 247, 0.5);
        }

        .btn-signup {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          color: white;
          box-shadow: 0 4px 15px rgba(123, 47, 247, 0.4);
        }

        .btn-signup:hover {
          transform: translateY(-2px);
          box-shadow: 0 6px 25px rgba(123, 47, 247, 0.6);
        }

        /* Space Background */
        .space-background {
          position: relative;
          width: 100%;
          height: 100%;
          background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
        }

        /* Animated stars */
        .stars,
        .stars2,
        .stars3 {
          position: absolute;
          top: 0;
          left: 0;
          width: 100%;
          height: 100%;
          background: transparent;
        }

        .stars {
          background-image: 
            radial-gradient(2px 2px at 20px 30px, white, transparent),
            radial-gradient(2px 2px at 60px 70px, white, transparent),
            radial-gradient(1px 1px at 50px 50px, white, transparent),
            radial-gradient(1px 1px at 130px 80px, white, transparent),
            radial-gradient(2px 2px at 90px 10px, white, transparent);
          background-size: 200px 200px;
          animation: twinkle 3s infinite;
        }

        .stars2 {
          background-image: 
            radial-gradient(1px 1px at 100px 120px, rgba(255,255,255,0.5), transparent),
            radial-gradient(1px 1px at 150px 90px, rgba(255,255,255,0.5), transparent);
          background-size: 250px 250px;
          animation: twinkle 4s infinite;
        }

        .stars3 {
          background-image: 
            radial-gradient(1px 1px at 75px 200px, rgba(255,255,255,0.3), transparent),
            radial-gradient(1px 1px at 180px 150px, rgba(255,255,255,0.3), transparent);
          background-size: 300px 300px;
          animation: twinkle 5s infinite;
        }

        @keyframes twinkle {
          0%, 100% { opacity: 1; }
          50% { opacity: 0.3; }
        }

        /* Central Planet */
        .central-planet {
          position: absolute;
          left: 50%;
          top: 50%;
          transform: translate(-50%, -50%);
          width: 350px;
          height: 350px;
          z-index: 1;
        }

        .planet-glow {
          position: absolute;
          width: 100%;
          height: 100%;
          border-radius: 50%;
          background: radial-gradient(circle, rgba(123,47,247,0.3) 0%, transparent 70%);
          animation: pulse 4s ease-in-out infinite;
        }

        .planet-surface {
          position: absolute;
          width: 100%;
          height: 100%;
          border-radius: 50%;
          background: 
            radial-gradient(circle at 30% 30%, #ff6ec4 0%, #7b2ff7 40%, #4a148c 100%);
          box-shadow: 
            inset -30px -30px 60px rgba(0,0,0,0.5),
            0 0 80px rgba(123,47,247,0.6);
          animation: rotate 60s linear infinite;
        }

        .planet-rings {
          position: absolute;
          width: 500px;
          height: 500px;
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%) rotateX(75deg);
          border: 3px solid rgba(76, 201, 240, 0.3);
          border-radius: 50%;
          box-shadow: 
            0 0 20px rgba(76, 201, 240, 0.3),
            inset 0 0 20px rgba(76, 201, 240, 0.2);
        }

        .planet-label {
          position: absolute;
          bottom: -50px;
          left: 50%;
          transform: translateX(-50%);
          color: rgba(255, 255, 255, 0.7);
          font-size: 18px;
          font-weight: 600;
          letter-spacing: 2px;
          text-align: center;
        }

        @keyframes pulse {
          0%, 100% { transform: scale(1); opacity: 0.3; }
          50% { transform: scale(1.1); opacity: 0.5; }
        }

        @keyframes rotate {
          from { transform: rotate(0deg); }
          to { transform: rotate(360deg); }
        }

        /* Planet Buttons */
        .planet-button {
          position: absolute;
          transform: translate(-50%, -50%);
          cursor: pointer;
          transition: all 0.4s ease;
          z-index: 10;
        }

        .planet-button:hover {
          transform: translate(-50%, -50%) scale(1.15);
          z-index: 20;
        }

        .planet-orb {
          position: relative;
          width: 100%;
          height: 100%;
          border-radius: 50%;
          background: radial-gradient(circle at 35% 35%, #d946ef 0%, #7b2ff7 50%, #3730a3 100%);
          box-shadow: 
            inset -10px -10px 30px rgba(0,0,0,0.4),
            0 0 40px rgba(123,47,247,0.4);
          animation: float 6s ease-in-out infinite;
        }

        .planet-button:nth-child(1) .planet-orb { animation-delay: 0s; }
        .planet-button:nth-child(2) .planet-orb { animation-delay: 1s; }
        .planet-button:nth-child(3) .planet-orb { animation-delay: 2s; }
        .planet-button:nth-child(4) .planet-orb { animation-delay: 3s; }
        .planet-button:nth-child(5) .planet-orb { animation-delay: 4s; }
        .planet-button:nth-child(6) .planet-orb { animation-delay: 5s; }

        @keyframes float {
          0%, 100% { transform: translateY(0px); }
          50% { transform: translateY(-20px); }
        }

        .planet-ring {
          position: absolute;
          width: 150%;
          height: 150%;
          top: 50%;
          left: 50%;
          transform-origin: center;
          border: 2px solid rgba(76, 201, 240, 0.4);
          border-radius: 50%;
          box-shadow: 
            0 0 15px rgba(76, 201, 240, 0.3),
            inset 0 0 15px rgba(76, 201, 240, 0.2);
        }

        .planet-info {
          position: absolute;
          bottom: -80px;
          left: 50%;
          transform: translateX(-50%);
          text-align: center;
          opacity: 0;
          transition: opacity 0.3s ease;
          pointer-events: none;
          width: 200px;
        }

        .planet-button.hovered .planet-info {
          opacity: 1;
        }

        .planet-name {
          font-size: 22px;
          font-weight: 700;
          color: #fff;
          margin-bottom: 8px;
          text-transform: uppercase;
          letter-spacing: 2px;
          text-shadow: 0 0 10px rgba(123,47,247,0.8);
        }

        .planet-description {
          font-size: 14px;
          color: rgba(255, 255, 255, 0.7);
          line-height: 1.4;
        }

        .lock-indicator {
          position: absolute;
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          font-size: 40px;
          pointer-events: none;
          opacity: 0;
          transition: opacity 0.3s ease;
        }

        .planet-button:hover .lock-indicator {
          opacity: 0.9;
        }

        /* Login Prompt */
        .login-prompt {
          position: fixed;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          display: flex;
          align-items: center;
          justify-content: center;
          background: rgba(0, 0, 0, 0.85);
          z-index: 1000;
          animation: fadeIn 0.3s ease;
        }

        @keyframes fadeIn {
          from { opacity: 0; }
          to { opacity: 1; }
        }

        .prompt-content {
          background: linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%);
          padding: 50px;
          border-radius: 20px;
          text-align: center;
          border: 2px solid rgba(123,47,247,0.5);
          box-shadow: 0 0 50px rgba(123,47,247,0.5);
          max-width: 500px;
        }

        .prompt-content h3 {
          font-size: 32px;
          margin: 0 0 20px 0;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          background-clip: text;
        }

        .prompt-content p {
          font-size: 18px;
          color: rgba(255, 255, 255, 0.8);
          margin: 0 0 30px 0;
        }

        .prompt-buttons {
          display: flex;
          gap: 20px;
          justify-content: center;
        }

        .prompt-buttons button {
          padding: 15px 40px;
          font-size: 16px;
          font-weight: 600;
          border: none;
          border-radius: 25px;
          cursor: pointer;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          color: white;
          transition: all 0.3s ease;
          text-transform: uppercase;
          letter-spacing: 1px;
        }

        .prompt-buttons button:hover {
          transform: translateY(-2px);
          box-shadow: 0 6px 25px rgba(123, 47, 247, 0.6);
        }

        /* Responsive Design */
        @media (max-width: 768px) {
          .nova-logo { font-size: 36px; }
          .tagline { font-size: 16px; }
          .homepage-header { padding: 20px 30px; }
          .central-planet { width: 250px; height: 250px; }
          .planet-button { font-size: 12px; }
        }
      `}</style>
    </div>
  );
};
