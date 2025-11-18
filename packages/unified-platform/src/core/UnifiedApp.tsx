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
import type { PlatformMode } from './UnifiedPlatformCore';
import {
  BrowserRouter,
  Routes,
  Route,
  Navigate,
  useLocation,
  useNavigate,
} from 'react-router-dom';
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
import { LoginPage } from '../pages/LoginPage';
import { RegisterPage } from '../pages/RegisterPage';

// Inner component that has access to router hooks
const UnifiedAppContent: React.FC<{ platform: UnifiedPlatformCore }> = ({
  platform,
}) => {
  const [currentMode, setCurrentMode] = useState<string>('hub');
  const [isLoggedIn, setIsLoggedIn] = useState(false);
  const [currentUser, setCurrentUser] = useState<any>(null);
  const [notifications, setNotifications] = useState<any[]>([]);
  const [isCheckingAuth, setIsCheckingAuth] = useState(true);
  const location = useLocation();
  const navigate = useNavigate();

  // Sync current mode with route
  useEffect(() => {
    const path = location.pathname;
    if (path.startsWith('/hub')) setCurrentMode('hub');
    else if (path.startsWith('/editor')) setCurrentMode('editor');
    else if (path.startsWith('/launcher')) setCurrentMode('launcher');
    else if (path.startsWith('/play')) setCurrentMode('launcher');
    else if (path.startsWith('/multiplayer')) setCurrentMode('multiplayer');
    else if (path.startsWith('/social')) setCurrentMode('social');
    else if (path.startsWith('/settings')) setCurrentMode('settings');
  }, [location.pathname]);

  useEffect(() => {
    // Initialize platform
    platform.initialize();

    // Listen to mode changes from platform and navigate
    platform.on('modeChanged', ({ to }: any) => {
      setCurrentMode(to);
      // Navigate to the corresponding route
      const modeToPath: Record<string, string> = {
        hub: '/hub',
        editor: '/editor',
        launcher: '/launcher',
        multiplayer: '/multiplayer',
        social: '/social',
        settings: '/settings',
      };
      if (modeToPath[to]) {
        navigate(modeToPath[to]);
      }
    });

    // Listen to auth changes
    platform.on('login', (user: any) => {
      setIsLoggedIn(true);
      setCurrentUser(user);
      // Navigate to hub after login
      navigate('/hub');
    });

    platform.on('sessionRestored', (user: any) => {
      setIsLoggedIn(true);
      setCurrentUser(user);
      setIsCheckingAuth(false);
    });

    platform.on('logout', () => {
      setIsLoggedIn(false);
      setCurrentUser(null);
      navigate('/login');
    });

    // Listen to notifications
    platform.on('notification', (notification: any) => {
      setNotifications((prev) => [notification, ...prev]);
    });

    // Check if already logged in
    const token = localStorage.getItem('auth_token');
    if (token) {
      // Restore session asynchronously
      platform
        .restoreSession()
        .then((success) => {
          if (!success) {
            setIsCheckingAuth(false);
          }
        })
        .catch((err) => {
          console.error('Failed to restore session:', err);
          setIsCheckingAuth(false);
        });
    } else {
      setIsCheckingAuth(false);
    }

    return () => {
      platform.removeAllListeners();
    };
  }, [platform, navigate]);

  // Show loading screen while checking authentication
  if (isCheckingAuth) {
    return (
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          minHeight: '100vh',
          background: 'linear-gradient(135deg, #1a1a2e 0%, #16213e 100%)',
          color: 'white',
          fontSize: '24px',
        }}
      >
        Loading Nova Engine...
      </div>
    );
  }

  return (
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
            onModeChange={(mode) => platform.switchMode(mode as PlatformMode)}
          />
        )}

        {/* Main Content Area - Switches based on mode */}
        <main className="unified-main-content">
          <Routes>
            {/* Public routes */}
            <Route path="/login" element={<LoginPage platform={platform} />} />
            <Route
              path="/register"
              element={<RegisterPage platform={platform} />}
            />

            {/* Protected routes - require login */}
            {isLoggedIn ? (
              <>
                {/* Hub - Game Discovery */}
                <Route
                  path="/hub/*"
                  element={<HubModule platform={platform} />}
                />

                {/* Editor - Game Creation */}
                <Route
                  path="/editor/*"
                  element={<EditorModule platform={platform} />}
                />

                {/* Launcher - Game Playing */}
                <Route
                  path="/launcher/*"
                  element={<LauncherModule platform={platform} />}
                />
                <Route
                  path="/play/:gameId"
                  element={<LauncherModule platform={platform} />}
                />

                {/* Multiplayer - Online Features */}
                <Route
                  path="/multiplayer/*"
                  element={<MultiplayerModule platform={platform} />}
                />

                {/* Social - Friends & Achievements */}
                <Route
                  path="/social/*"
                  element={<SocialModule platform={platform} />}
                />

                {/* Settings */}
                <Route
                  path="/settings/*"
                  element={<SettingsModule platform={platform} />}
                />

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
      {notifications.length > 0 && (
        <NotificationCenter
          notifications={notifications}
          onClear={() => setNotifications([])}
        />
      )}
    </div>
  );
};

// Main export wrapper with BrowserRouter
export const UnifiedApp: React.FC = () => {
  const [platform] = useState(() => new UnifiedPlatformCore());

  return (
    <BrowserRouter>
      <UnifiedAppContent platform={platform} />
    </BrowserRouter>
  );
};
