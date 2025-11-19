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
 * SettingsPage - Launcher settings and preferences
 */

import { useState } from 'react';
import { Link } from 'react-router-dom';
import { updateManager } from '../services/UpdateManager';

export default function SettingsPage() {
  const [autoUpdate, setAutoUpdate] = useState(true);
  const [startMinimized, setStartMinimized] = useState(false);

  async function checkForLauncherUpdate() {
    try {
      const update = await updateManager.checkLauncherUpdate();
      if (update) {
        if (
          confirm(`Update available: v${update.latestVersion}. Install now?`)
        ) {
          await updateManager.updateLauncher();
        }
      } else {
        alert('You have the latest version');
      }
    } catch (error) {
      console.error('Failed to check for updates:', error);
      alert('Failed to check for updates');
    }
  }

  return (
    <div className="page settings-page">
      <header className="page-header">
        <h1>Settings</h1>
        <nav className="main-nav">
          <Link to="/">Home</Link>
          <Link to="/library">Library</Link>
          <Link to="/downloads">Downloads</Link>
          <Link to="/settings" className="active">
            Settings
          </Link>
        </nav>
      </header>

      <main className="page-content">
        <section className="settings-section">
          <h2>General</h2>
          <div className="setting-item">
            <label>
              <input
                type="checkbox"
                checked={autoUpdate}
                onChange={(e) => setAutoUpdate(e.target.checked)}
              />
              Auto-update games
            </label>
          </div>
          <div className="setting-item">
            <label>
              <input
                type="checkbox"
                checked={startMinimized}
                onChange={(e) => setStartMinimized(e.target.checked)}
              />
              Start minimized to system tray
            </label>
          </div>
        </section>

        <section className="settings-section">
          <h2>Launcher</h2>
          <div className="setting-item">
            <p>Version: {updateManager.getLauncherVersion()}</p>
            <button onClick={checkForLauncherUpdate}>Check for Updates</button>
          </div>
        </section>

        <section className="settings-section">
          <h2>About</h2>
          <p>Nova Launcher Desktop</p>
          <p>
            A native desktop application for playing Nova Engine games offline
          </p>
          <p>© 2024 Nova Engine Team</p>
        </section>
      </main>
    </div>
  );
}
