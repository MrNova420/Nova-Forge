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

import React from 'react';
import { Routes, Route } from 'react-router-dom';
import Layout from './components/Layout';
import HomePage from './pages/HomePage';
import GameBrowserPage from './pages/GameBrowserPage';
import GameDetailPage from './pages/GameDetailPage';
import GamePlayerPage from './pages/GamePlayerPage';
import { ProfilePage } from './pages/ProfilePage';
import LibraryPage from './pages/LibraryPage';
import { useAuthStore } from './store/authStore';

const App: React.FC = () => {
  const { user, initialize } = useAuthStore();

  React.useEffect(() => {
    initialize();
  }, [initialize]);

  return (
    <Routes>
      <Route path="/" element={<Layout />}>
        <Route index element={<HomePage />} />
        <Route path="games" element={<GameBrowserPage />} />
        <Route path="games/:gameId" element={<GameDetailPage />} />
        <Route path="play/:gameId" element={<GamePlayerPage />} />
        <Route path="profile" element={user ? <ProfilePage /> : <HomePage />} />
        <Route path="library" element={user ? <LibraryPage /> : <HomePage />} />
      </Route>
    </Routes>
  );
};

export default App;
