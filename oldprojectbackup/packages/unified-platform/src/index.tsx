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
 * Nova Engine - Unified Web Application Entry Point
 *
 * Single web app that includes EVERYTHING:
 * - Hub (game browser)
 * - Editor (game creation)
 * - Launcher (game player)
 * - Multiplayer (online features)
 * - Social (friends, achievements)
 * - Settings (account, preferences)
 *
 * Just run: npm start
 * Access: http://localhost:3000
 */

import React from 'react';
import ReactDOM from 'react-dom/client';
import { UnifiedApp } from './core/UnifiedApp';
import './styles/index.css';

// Initialize service worker for PWA
if ('serviceWorker' in navigator) {
  window.addEventListener('load', () => {
    navigator.serviceWorker
      .register('/service-worker.js')
      .then((registration) => {
        // eslint-disable-next-line no-console
        console.log('✅ Service Worker registered:', registration);
      })
      .catch((error) => {
        // eslint-disable-next-line no-console
        console.log('Service Worker registration failed:', error);
      });
  });
}

// Render the unified web application
const root = ReactDOM.createRoot(
  document.getElementById('root') as HTMLElement
);

root.render(
  <React.StrictMode>
    <UnifiedApp />
  </React.StrictMode>
);

// Log startup info
// eslint-disable-next-line no-console
console.log(`
╔═══════════════════════════════════════════════════════════╗
║                                                             ║
║              🎮 NOVA ENGINE - UNIFIED PLATFORM 🎮          ║
║                                                             ║
║           Create ◆ Play ◆ Share ◆ One Web App             ║
║                                                             ║
╚═══════════════════════════════════════════════════════════╝

✅ All features loaded
✅ Hub - Browse games
✅ Editor - Create games  
✅ Launcher - Play games
✅ Multiplayer - Online play
✅ Social - Friends & achievements
✅ Settings - Account management

Running on: ${window.location.origin}
Mode: ${process.env.NODE_ENV}
`);

// Global error handler
window.addEventListener('error', (event) => {
  console.error('Global error:', event.error);
});

// Global unhandled rejection handler
window.addEventListener('unhandledrejection', (event) => {
  console.error('Unhandled rejection:', event.reason);
});

// Expose Nova Engine API globally
declare global {
  interface Window {
    NovaEngine: any;
  }
}

// Make platform accessible from console
import { unifiedPlatform } from './core/UnifiedPlatformCore';
window.NovaEngine = unifiedPlatform;

// eslint-disable-next-line no-console
console.log('Access the platform: window.NovaEngine');
