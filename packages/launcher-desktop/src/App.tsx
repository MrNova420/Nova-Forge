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

import { useEffect, useState } from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { initializeServices, cleanupServices } from './services';
import './App.css';

// Placeholder pages (to be implemented)
import HomePage from './pages/HomePage';
import LibraryPage from './pages/LibraryPage';
import DownloadsPage from './pages/DownloadsPage';
import SettingsPage from './pages/SettingsPage';

function App() {
  const [isInitialized, setIsInitialized] = useState(false);
  const [initError, setInitError] = useState<string | null>(null);

  useEffect(() => {
    // Initialize services on app start
    initializeServices()
      .then(() => {
        // eslint-disable-next-line no-console
        console.log('Nova Launcher Desktop initialized');
        setIsInitialized(true);
      })
      .catch((error) => {
        console.error('Failed to initialize:', error);
        setInitError(error.message);
      });

    // Cleanup on unmount
    return () => {
      cleanupServices();
    };
  }, []);

  if (initError) {
    return (
      <div className="init-error">
        <h1>Initialization Error</h1>
        <p>{initError}</p>
        <button onClick={() => window.location.reload()}>Retry</button>
      </div>
    );
  }

  if (!isInitialized) {
    return (
      <div className="loading">
        <h1>Nova Launcher</h1>
        <p>Initializing...</p>
      </div>
    );
  }

  return (
    <BrowserRouter>
      <div className="app">
        <Routes>
          <Route path="/" element={<HomePage />} />
          <Route path="/library" element={<LibraryPage />} />
          <Route path="/downloads" element={<DownloadsPage />} />
          <Route path="/settings" element={<SettingsPage />} />
          <Route path="*" element={<Navigate to="/" replace />} />
        </Routes>
      </div>
    </BrowserRouter>
  );
}

export default App;
