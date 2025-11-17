/**
 * Settings Module - Account & Preferences
 */

import React, { useState } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface SettingsModuleProps {
  platform: UnifiedPlatformCore;
}

export const SettingsModule: React.FC<SettingsModuleProps> = ({
  platform: _platform,
}) => {
  const [activeSection, setActiveSection] = useState('account');

  return (
    <div className="settings-module">
      <div className="settings-sidebar">
        <button
          className={activeSection === 'account' ? 'active' : ''}
          onClick={() => setActiveSection('account')}
        >
          Account
        </button>
        <button
          className={activeSection === 'preferences' ? 'active' : ''}
          onClick={() => setActiveSection('preferences')}
        >
          Preferences
        </button>
        <button
          className={activeSection === 'privacy' ? 'active' : ''}
          onClick={() => setActiveSection('privacy')}
        >
          Privacy
        </button>
      </div>

      <div className="settings-content">
        {activeSection === 'account' && (
          <div className="section">
            <h2>Account Settings</h2>
            <div className="setting-group">
              <label>Email</label>
              <input type="email" placeholder="user@example.com" />
            </div>
            <div className="setting-group">
              <label>Username</label>
              <input type="text" placeholder="username" />
            </div>
            <button className="save-btn">Save Changes</button>
          </div>
        )}

        {activeSection === 'preferences' && (
          <div className="section">
            <h2>Preferences</h2>
            <div className="setting-group">
              <label>Theme</label>
              <select>
                <option>Dark</option>
                <option>Light</option>
              </select>
            </div>
            <div className="setting-group">
              <label>Language</label>
              <select>
                <option>English</option>
                <option>Spanish</option>
              </select>
            </div>
          </div>
        )}

        {activeSection === 'privacy' && (
          <div className="section">
            <h2>Privacy Settings</h2>
            <div className="setting-group">
              <label>
                <input type="checkbox" />
                Show online status
              </label>
            </div>
            <div className="setting-group">
              <label>
                <input type="checkbox" />
                Allow friend requests
              </label>
            </div>
          </div>
        )}
      </div>

      <style>{`
        .settings-module {
          display: flex;
          height: 100%;
          background: #1a1a1a;
          color: white;
        }
        .settings-sidebar {
          width: 200px;
          background: #2a2a2a;
          padding: 20px;
          display: flex;
          flex-direction: column;
          gap: 5px;
        }
        .settings-sidebar button {
          padding: 10px;
          background: transparent;
          border: none;
          color: white;
          text-align: left;
          cursor: pointer;
          border-radius: 4px;
        }
        .settings-sidebar button.active {
          background: #7b2ff7;
        }
        .settings-content {
          flex: 1;
          padding: 40px;
          overflow-y: auto;
        }
        .section h2 {
          margin-bottom: 30px;
        }
        .setting-group {
          margin-bottom: 20px;
        }
        .setting-group label {
          display: block;
          margin-bottom: 8px;
          color: #aaa;
        }
        .setting-group input, .setting-group select {
          width: 100%;
          max-width: 400px;
          padding: 10px;
          background: #2a2a2a;
          border: 1px solid #3a3a3a;
          border-radius: 4px;
          color: white;
        }
        .save-btn {
          padding: 12px 24px;
          background: #7b2ff7;
          border: none;
          border-radius: 4px;
          color: white;
          cursor: pointer;
          margin-top: 20px;
        }
      `}</style>
    </div>
  );
};
