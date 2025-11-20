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

// Import all integrated modules - USING V2 REDESIGNED VERSIONS
import { HubModuleV2 as HubModule } from '../modules/HubModuleV2';
import { EditorModuleV2 as EditorModule } from '../modules/EditorModuleV2';
import { LauncherModuleV2 as LauncherModule } from '../modules/LauncherModuleV2';
import { MultiplayerModuleV2 as MultiplayerModule } from '../modules/MultiplayerModuleV2';
import { SocialModuleV2 as SocialModule } from '../modules/SocialModuleV2';
import { SettingsModuleV2 as SettingsModule } from '../modules/SettingsModuleV2';
import { LoginPage } from '../pages/LoginPage';
import { RegisterPage } from '../pages/RegisterPage';
import { HomePage } from '../pages/HomePage';

// Inner component that has access to router hooks
const UnifiedAppContent: React.FC<{ platform: UnifiedPlatformCore }> = ({
  platform,
}) => {
  const [currentMode, setCurrentMode] = useState<string>('hub');
  const [isLoggedIn, setIsLoggedIn] = useState(false);
  const [currentUser, setCurrentUser] = useState<any>(null);
  const [notifications, setNotifications] = useState<any[]>([]);
  const [isCheckingAuth, setIsCheckingAuth] = useState(true);
  const [isNavOpen, setIsNavOpen] = useState(true);
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
    let mounted = true;

    // Initialize platform once
    platform.initialize();

    // Mode change handler
    const handleModeChange = ({ to }: any) => {
      if (!mounted) return;
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
    };

    // Login handler
    const handleLogin = (user: any) => {
      if (!mounted) return;
      setIsLoggedIn(true);
      setCurrentUser(user);
      // Only navigate to hub after login if on auth pages
      const currentPath = window.location.pathname;
      if (
        currentPath === '/login' ||
        currentPath === '/register' ||
        currentPath === '/'
      ) {
        navigate('/hub');
      }
    };

    // Session restore handler
    const handleSessionRestored = (user: any) => {
      if (!mounted) return;
      setIsLoggedIn(true);
      setCurrentUser(user);
      setIsCheckingAuth(false);
      // Don't navigate on session restore - stay on current page
    };

    // Logout handler
    const handleLogout = () => {
      if (!mounted) return;
      setIsLoggedIn(false);
      setCurrentUser(null);
      navigate('/login');
    };

    // Notification handler
    const handleNotification = (notification: any) => {
      if (!mounted) return;
      setNotifications((prev) => [notification, ...prev]);
    };

    // Register event handlers
    platform.on('modeChanged', handleModeChange);
    platform.on('login', handleLogin);
    platform.on('sessionRestored', handleSessionRestored);
    platform.on('logout', handleLogout);
    platform.on('notification', handleNotification);

    // Check if already logged in
    const token = localStorage.getItem('auth_token');
    if (token) {
      // Restore session asynchronously
      platform
        .restoreSession()
        .then((success) => {
          if (!success && mounted) {
            setIsCheckingAuth(false);
          }
        })
        .catch((err) => {
          console.error('Failed to restore session:', err);
          if (mounted) {
            setIsCheckingAuth(false);
          }
        });
    } else {
      setIsCheckingAuth(false);
    }

    return () => {
      mounted = false;
      // Remove specific handlers
      platform.off('modeChanged', handleModeChange);
      platform.off('login', handleLogin);
      platform.off('sessionRestored', handleSessionRestored);
      platform.off('logout', handleLogout);
      platform.off('notification', handleNotification);
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

  // Check if we're on a standalone page (homepage, login, register)
  // const isStandalonePage = ['/', '/login', '/register'].includes(
  //   location.pathname
  // );

  return (
    <div className="unified-app">
      <Routes>
        {/* Standalone Public Routes - NO TopBar/Navigation */}
        <Route
          path="/"
          element={
            <HomePage
              platform={platform}
              isLoggedIn={isLoggedIn}
              onNavigate={(path) => navigate(path)}
            />
          }
        />
        <Route path="/login" element={<LoginPage platform={platform} />} />
        <Route
          path="/register"
          element={<RegisterPage platform={platform} />}
        />

        {/* Protected Routes - WITH TopBar/Navigation */}
        {isLoggedIn ? (
          <Route
            path="*"
            element={
              <>
                {/* Top Bar - Only for logged-in app pages */}
                <UnifiedTopBar
                  platform={platform}
                  isLoggedIn={isLoggedIn}
                  currentUser={currentUser}
                  currentMode={currentMode}
                  isNavOpen={isNavOpen}
                  onToggleNav={() => setIsNavOpen(!isNavOpen)}
                />

                <div className="unified-app-content">
                  {/* Side Navigation */}
                  <UnifiedNavigation
                    currentMode={currentMode}
                    onModeChange={(mode) =>
                      platform.switchMode(mode as PlatformMode)
                    }
                    isLoggedIn={isLoggedIn}
                    currentUser={currentUser}
                    isOpen={isNavOpen}
                    onNavigateHome={() => navigate('/')}
                  />

                  {/* Main Content Area */}
                  <main className="unified-main-content">
                    <Routes>
                      {/* Hub - Game Discovery - V2 REDESIGNED */}
                      <Route
                        path="/hub/*"
                        element={<HubModule platform={platform} />}
                      />

                      {/* Editor - Game Creation - V2 REDESIGNED */}
                      <Route
                        path="/editor/*"
                        element={<EditorModule platform={platform} />}
                      />

                      {/* Launcher - Game Playing - V2 REDESIGNED */}
                      <Route
                        path="/launcher/*"
                        element={<LauncherModule platform={platform} />}
                      />
                      <Route
                        path="/play/:gameId"
                        element={<LauncherModule platform={platform} />}
                      />

                      {/* Multiplayer - Online Features - V2 REDESIGNED */}
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
                    </Routes>
                  </main>
                </div>
              </>
            }
          />
        ) : (
          <Route path="*" element={<Navigate to="/" replace />} />
        )}
      </Routes>

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
    <BrowserRouter
      future={{
        v7_startTransition: true,
        v7_relativeSplatPath: true,
      }}
    >
      <UnifiedAppContent platform={platform} />
    </BrowserRouter>
  );
};
