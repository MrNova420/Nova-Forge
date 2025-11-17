/**
 * Nova Engine Unified Application
 * 
 * Complete all-in-one platform combining:
 * - Hub (discover & browse games)
 * - Editor (create games)  
 * - Launcher (play games)
 * - Multiplayer (online features)
 * - Social (friends, achievements)
 * - Settings (account, preferences)
 * 
 * ONE application, seamless experience, fully integrated
 */

import React, { useState, useEffect } from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { UnifiedPlatformCore } from './UnifiedPlatformCore';
import { UnifiedNavigation } from '../ui/UnifiedNavigation';
import { UnifiedTopBar } from '../ui/UnifiedTopBar';
import { NotificationCenter } from '../ui/NotificationCenter';

// Import all integrated modules
import { HubModule } from '../modules/HubModule';
import { EditorModule } from '../modules/EditorModule';
import { LauncherModule } from '../modules/LauncherModule';
import { MultiplayerModule } from '../modules/MultiplayerModule';
import { SocialModule } from '../modules/SocialModule';
import { SettingsModule } from '../modules/SettingsModule';

export const UnifiedApp: React.FC = () => {
  const [platform] = useState(() => new UnifiedPlatformCore());
  const [currentMode, setCurrentMode] = useState<string>('hub');
  const [isLoggedIn, setIsLoggedIn] = useState(false);
  const [currentUser, setCurrentUser] = useState<any>(null);
  const [notifications, setNotifications] = useState<any[]>([]);

  useEffect(() => {
    // Initialize platform
    platform.initialize();

    // Listen to mode changes
    platform.on('modeChanged', ({ to }: any) => {
      setCurrentMode(to);
    });

    // Listen to auth changes
    platform.on('login', (user: any) => {
      setIsLoggedIn(true);
      setCurrentUser(user);
    });

    platform.on('logout', () => {
      setIsLoggedIn(false);
      setCurrentUser(null);
    });

    // Listen to notifications
    platform.on('notification', (notification: any) => {
      setNotifications(prev => [notification, ...prev]);
    });

    // Check if already logged in
    const token = localStorage.getItem('auth_token');
    if (token) {
      platform.restoreSession();
    }

    return () => {
      platform.removeAllListeners();
    };
  }, [platform]);

  return (
    <BrowserRouter>
      <div className="unified-app">
        {/* Top Bar - Always visible */}
        <UnifiedTopBar
          platform={platform}
          isLoggedIn={isLoggedIn}
          currentUser={currentUser}
          currentMode={currentMode}
        />

        <div className="unified-app-content">
          {/* Side Navigation - Always visible when logged in */}
          {isLoggedIn && (
            <UnifiedNavigation
              platform={platform}
              currentMode={currentMode}
              onModeChange={(mode) => platform.switchMode(mode)}
            />
          )}

          {/* Main Content Area - Switches based on mode */}
          <main className="unified-main-content">
            <Routes>
              {/* Public routes */}
              <Route path="/login" element={<LoginPage platform={platform} />} />
              <Route path="/register" element={<RegisterPage platform={platform} />} />

              {/* Protected routes - require login */}
              {isLoggedIn ? (
                <>
                  {/* Hub - Game Discovery */}
                  <Route path="/hub/*" element={<HubModule platform={platform} />} />
                  
                  {/* Editor - Game Creation */}
                  <Route path="/editor/*" element={<EditorModule platform={platform} />} />
                  
                  {/* Launcher - Game Playing */}
                  <Route path="/launcher/*" element={<LauncherModule platform={platform} />} />
                  <Route path="/play/:gameId" element={<LauncherModule platform={platform} />} />
                  
                  {/* Multiplayer - Online Features */}
                  <Route path="/multiplayer/*" element={<MultiplayerModule platform={platform} />} />
                  
                  {/* Social - Friends & Achievements */}
                  <Route path="/social/*" element={<SocialModule platform={platform} />} />
                  
                  {/* Settings */}
                  <Route path="/settings/*" element={<SettingsModule platform={platform} />} />
                  
                  {/* Default redirect to hub */}
                  <Route path="/" element={<Navigate to="/hub" replace />} />
                </>
              ) : (
                <Route path="*" element={<Navigate to="/login" replace />} />
              )}
            </Routes>
          </main>
        </div>

        {/* Notification Center - Always available */}
        <NotificationCenter
          notifications={notifications}
          onDismiss={(id) => {
            setNotifications(prev => prev.filter(n => n.id !== id));
          }}
        />

        {/* Global Modals */}
        <GlobalModals platform={platform} />
      </div>
    </BrowserRouter>
  );
};

// Login Page Component
const LoginPage: React.FC<{ platform: UnifiedPlatformCore }> = ({ platform }) => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');
    setLoading(true);

    try {
      const success = await platform.login(email, password);
      if (success) {
        window.location.href = '/hub';
      } else {
        setError('Invalid credentials');
      }
    } catch (err) {
      setError('Login failed. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="login-page">
      <div className="login-container">
        <div className="login-logo">
          <h1>Nova Engine</h1>
          <p>Create. Play. Share.</p>
        </div>
        
        <form onSubmit={handleLogin} className="login-form">
          <h2>Sign In</h2>
          
          {error && <div className="error-message">{error}</div>}
          
          <div className="form-group">
            <label>Email</label>
            <input
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              required
              autoFocus
            />
          </div>
          
          <div className="form-group">
            <label>Password</label>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              required
            />
          </div>
          
          <button type="submit" disabled={loading} className="btn-primary">
            {loading ? 'Signing in...' : 'Sign In'}
          </button>
          
          <div className="login-links">
            <a href="/register">Create Account</a>
            <a href="/forgot-password">Forgot Password?</a>
          </div>
        </form>
      </div>
    </div>
  );
};

// Register Page Component
const RegisterPage: React.FC<{ platform: UnifiedPlatformCore }> = ({ platform }) => {
  const [formData, setFormData] = useState({
    username: '',
    email: '',
    password: '',
    confirmPassword: '',
  });
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');

    if (formData.password !== formData.confirmPassword) {
      setError('Passwords do not match');
      return;
    }

    setLoading(true);

    try {
      const success = await platform.register(
        formData.username,
        formData.email,
        formData.password
      );
      
      if (success) {
        window.location.href = '/hub';
      } else {
        setError('Registration failed');
      }
    } catch (err) {
      setError('Registration failed. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="register-page">
      <div className="register-container">
        <div className="register-logo">
          <h1>Join Nova Engine</h1>
          <p>Start creating amazing games today</p>
        </div>
        
        <form onSubmit={handleRegister} className="register-form">
          <h2>Create Account</h2>
          
          {error && <div className="error-message">{error}</div>}
          
          <div className="form-group">
            <label>Username</label>
            <input
              type="text"
              value={formData.username}
              onChange={(e) => setFormData({ ...formData, username: e.target.value })}
              required
            />
          </div>
          
          <div className="form-group">
            <label>Email</label>
            <input
              type="email"
              value={formData.email}
              onChange={(e) => setFormData({ ...formData, email: e.target.value })}
              required
            />
          </div>
          
          <div className="form-group">
            <label>Password</label>
            <input
              type="password"
              value={formData.password}
              onChange={(e) => setFormData({ ...formData, password: e.target.value })}
              required
              minLength={8}
            />
          </div>
          
          <div className="form-group">
            <label>Confirm Password</label>
            <input
              type="password"
              value={formData.confirmPassword}
              onChange={(e) => setFormData({ ...formData, confirmPassword: e.target.value })}
              required
            />
          </div>
          
          <button type="submit" disabled={loading} className="btn-primary">
            {loading ? 'Creating Account...' : 'Create Account'}
          </button>
          
          <div className="register-links">
            <a href="/login">Already have an account? Sign in</a>
          </div>
        </form>
      </div>
    </div>
  );
};

// Global Modals (friend invites, achievements, etc.)
const GlobalModals: React.FC<{ platform: UnifiedPlatformCore }> = ({ platform }) => {
  return null; // Implement modal system as needed
};
