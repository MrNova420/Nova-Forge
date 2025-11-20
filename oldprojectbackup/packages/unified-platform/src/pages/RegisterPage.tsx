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
 * Register Page
 */

import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface RegisterPageProps {
  platform: UnifiedPlatformCore;
}

export const RegisterPage: React.FC<RegisterPageProps> = ({ platform }) => {
  const [email, setEmail] = useState('');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');

    if (password !== confirmPassword) {
      setError('Passwords do not match');
      return;
    }

    setLoading(true);

    try {
      await platform.register(username, email, password);
      navigate('/hub');
    } catch (err: unknown) {
      setError((err as Error).message || 'Registration failed');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="auth-page">
      {/* Animated space background */}
      <div className="space-background">
        <div className="stars"></div>
        <div className="stars2"></div>
        <div className="stars3"></div>
      </div>

      {/* Back to home button */}
      <button className="back-button" onClick={() => navigate('/')}>
        ← Back to Home
      </button>

      {/* Register Form Container */}
      <div className="auth-container">
        <div className="auth-card">
          {/* Nova Engine Branding */}
          <div className="brand-header">
            <h1 className="nova-logo">NOVA</h1>
            <span className="engine-text">ENGINE</span>
          </div>

          <h2 className="auth-title">Join Nova Engine</h2>
          <p className="auth-subtitle">Start creating incredible games today</p>

          <form onSubmit={handleSubmit} className="auth-form">
            <div className="form-group">
              <label htmlFor="email">Email Address</label>
              <input
                id="email"
                type="email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
                placeholder="your@email.com"
              />
            </div>

            <div className="form-group">
              <label htmlFor="username">Username</label>
              <input
                id="username"
                type="text"
                value={username}
                onChange={(e) => setUsername(e.target.value)}
                required
                placeholder="Choose a username"
              />
            </div>

            <div className="form-group">
              <label htmlFor="password">Password</label>
              <input
                id="password"
                type="password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                autoComplete="new-password"
                required
                placeholder="Create a strong password"
              />
            </div>

            <div className="form-group">
              <label htmlFor="confirmPassword">Confirm Password</label>
              <input
                id="confirmPassword"
                type="password"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
                autoComplete="new-password"
                required
                placeholder="Re-enter your password"
              />
            </div>

            {error && (
              <div className="error-message">
                <span>⚠️</span> {error}
              </div>
            )}

            <button type="submit" disabled={loading} className="submit-button">
              {loading ? (
                <>
                  <span className="spinner"></span> Creating Account...
                </>
              ) : (
                'Sign Up'
              )}
            </button>
          </form>

          <div className="auth-footer">
            <p>
              Already have an account? <a href="/login">Log in here</a>
            </p>
          </div>
        </div>
      </div>

      <style>{`
        .auth-page {
          position: fixed;
          top: 0;
          left: 0;
          width: 100vw;
          height: 100vh;
          overflow: hidden;
          display: flex;
          align-items: center;
          justify-content: center;
        }

        .space-background {
          position: absolute;
          top: 0;
          left: 0;
          width: 100%;
          height: 100%;
          background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
          z-index: 0;
        }

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

        .back-button {
          position: fixed;
          top: 30px;
          left: 30px;
          padding: 12px 24px;
          background: rgba(26, 0, 51, 0.8);
          border: 2px solid rgba(123, 47, 247, 0.5);
          border-radius: 25px;
          color: white;
          font-size: 16px;
          font-weight: 600;
          cursor: pointer;
          transition: all 0.3s ease;
          z-index: 100;
          backdrop-filter: blur(10px);
        }

        .back-button:hover {
          background: rgba(123, 47, 247, 0.3);
          border-color: rgba(123, 47, 247, 0.8);
          box-shadow: 0 0 20px rgba(123, 47, 247, 0.5);
          transform: translateX(-5px);
        }

        .auth-container {
          position: relative;
          z-index: 10;
          width: 100%;
          max-width: 480px;
          padding: 20px;
        }

        .auth-card {
          background: linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 24px;
          padding: 50px 40px;
          box-shadow: 
            0 20px 60px rgba(123, 47, 247, 0.3),
            0 0 80px rgba(255, 110, 196, 0.1);
          backdrop-filter: blur(20px);
          animation: slideUp 0.6s ease-out;
        }

        @keyframes slideUp {
          from {
            opacity: 0;
            transform: translateY(30px);
          }
          to {
            opacity: 1;
            transform: translateY(0);
          }
        }

        .brand-header {
          text-align: center;
          margin-bottom: 30px;
        }

        .nova-logo {
          font-size: 56px;
          font-weight: 900;
          letter-spacing: 12px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          background-clip: text;
          margin: 0;
          text-shadow: 0 0 40px rgba(123, 47, 247, 0.5);
          animation: glow 2s ease-in-out infinite alternate;
        }

        @keyframes glow {
          from { filter: drop-shadow(0 0 10px rgba(123, 47, 247, 0.5)); }
          to { filter: drop-shadow(0 0 20px rgba(255, 110, 196, 0.7)); }
        }

        .engine-text {
          display: block;
          font-size: 18px;
          font-weight: 300;
          letter-spacing: 6px;
          color: #a0a0ff;
          margin-top: 5px;
        }

        .auth-title {
          font-size: 32px;
          font-weight: 700;
          color: white;
          text-align: center;
          margin: 0 0 10px 0;
        }

        .auth-subtitle {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.6);
          text-align: center;
          margin: 0 0 40px 0;
        }

        .auth-form {
          display: flex;
          flex-direction: column;
          gap: 20px;
        }

        .form-group {
          display: flex;
          flex-direction: column;
          gap: 8px;
        }

        .form-group label {
          font-size: 14px;
          font-weight: 600;
          color: rgba(255, 255, 255, 0.9);
          letter-spacing: 0.5px;
        }

        .form-group input {
          width: 100%;
          padding: 14px 18px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-size: 16px;
          transition: all 0.3s ease;
          box-sizing: border-box;
        }

        .form-group input::placeholder {
          color: rgba(255, 255, 255, 0.3);
        }

        .form-group input:focus {
          outline: none;
          border-color: rgba(123, 47, 247, 0.8);
          background: rgba(26, 0, 51, 0.8);
          box-shadow: 0 0 20px rgba(123, 47, 247, 0.3);
        }

        .error-message {
          padding: 14px;
          background: rgba(244, 67, 54, 0.15);
          border: 2px solid rgba(244, 67, 54, 0.5);
          border-radius: 12px;
          color: #ff6b6b;
          font-size: 14px;
          display: flex;
          align-items: center;
          gap: 10px;
        }

        .submit-button {
          width: 100%;
          padding: 16px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 12px;
          color: white;
          font-size: 18px;
          font-weight: 700;
          cursor: pointer;
          transition: all 0.3s ease;
          text-transform: uppercase;
          letter-spacing: 2px;
          box-shadow: 0 4px 20px rgba(123, 47, 247, 0.4);
          display: flex;
          align-items: center;
          justify-content: center;
          gap: 10px;
          margin-top: 10px;
        }

        .submit-button:hover:not(:disabled) {
          transform: translateY(-2px);
          box-shadow: 0 6px 30px rgba(123, 47, 247, 0.6);
        }

        .submit-button:disabled {
          opacity: 0.6;
          cursor: not-allowed;
        }

        .spinner {
          width: 18px;
          height: 18px;
          border: 3px solid rgba(255, 255, 255, 0.3);
          border-top-color: white;
          border-radius: 50%;
          animation: spin 0.8s linear infinite;
        }

        @keyframes spin {
          to { transform: rotate(360deg); }
        }

        .auth-footer {
          margin-top: 30px;
          text-align: center;
        }

        .auth-footer p {
          color: rgba(255, 255, 255, 0.6);
          font-size: 15px;
          margin: 0;
        }

        .auth-footer a {
          color: #ff6ec4;
          text-decoration: none;
          font-weight: 600;
          transition: color 0.3s ease;
        }

        .auth-footer a:hover {
          color: #7b2ff7;
          text-decoration: underline;
        }

        @media (max-width: 768px) {
          .auth-card {
            padding: 40px 30px;
          }
          
          .nova-logo {
            font-size: 42px;
            letter-spacing: 8px;
          }
          
          .auth-title {
            font-size: 26px;
          }
        }
      `}</style>
    </div>
  );
};
