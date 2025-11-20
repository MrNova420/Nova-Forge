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
 * Settings Module V2 - Complete Redesign with Image 2 Theme
 *
 * Comprehensive settings management with:
 * - General settings (language, theme, notifications)
 * - Graphics settings (quality, resolution, effects)
 * - Audio settings (volume, spatial audio)
 * - Controls settings (keybinds, sensitivity)
 * - Network settings (region, ping, bandwidth)
 * - Privacy settings (profile visibility, data collection)
 * - Accessibility settings (colorblind mode, font size)
 * - Advanced settings (cache, diagnostics, system info)
 *
 * Theme: Purple/pink/blue space aesthetic from Image 2 mockup
 * NOVA branding throughout
 */

import React, { useState } from 'react';
import './styles/SettingsModuleV2.css';

interface SettingsModuleV2Props {
  platform?: any;
}

export const SettingsModuleV2: React.FC<SettingsModuleV2Props> = () => {
  const [activeCategory, setActiveCategory] = useState('general');
  const [settings, setSettings] = useState({
    // General
    language: 'en',
    theme: 'dark',
    notifications: true,
    autoUpdate: true,

    // Graphics
    quality: 'ultra',
    resolution: '1920x1080',
    vsync: true,
    antialiasing: 'msaa-4x',
    shadows: 'ultra',
    textures: 'ultra',
    effects: 'ultra',
    viewDistance: 100,

    // Audio
    masterVolume: 80,
    musicVolume: 70,
    sfxVolume: 85,
    voiceVolume: 75,
    spatialAudio: true,

    // Controls
    mouseSensitivity: 50,
    invertY: false,
    keybinds: {
      forward: 'W',
      backward: 'S',
      left: 'A',
      right: 'D',
      jump: 'Space',
      crouch: 'C',
      interact: 'E',
    },

    // Network
    region: 'us-east',
    maxPing: 100,
    bandwidthLimit: 'unlimited',

    // Privacy
    profileVisibility: 'friends',
    showOnlineStatus: true,
    dataCollection: true,

    // Accessibility
    colorblindMode: 'none',
    fontSize: 'medium',
    screenShake: true,
    motionBlur: true,
  });

  const categories = [
    { id: 'general', icon: '⚙️', label: 'General' },
    { id: 'graphics', icon: '🎨', label: 'Graphics' },
    { id: 'audio', icon: '🔊', label: 'Audio' },
    { id: 'controls', icon: '🎮', label: 'Controls' },
    { id: 'network', icon: '🌐', label: 'Network' },
    { id: 'privacy', icon: '🔒', label: 'Privacy' },
    { id: 'accessibility', icon: '♿', label: 'Accessibility' },
    { id: 'advanced', icon: '🔧', label: 'Advanced' },
  ];

  const updateSetting = (key: string, value: any) => {
    setSettings({ ...settings, [key]: value });
    // TODO: Backend integration - save settings to database
    // eslint-disable-next-line no-console
    console.log('Settings updated:', { [key]: value });
  };

  const renderGeneralSettings = () => (
    <div className="settings-section">
      <h2>General Settings</h2>

      <div className="setting-group">
        <label>Language</label>
        <select
          value={settings.language}
          onChange={(e) => updateSetting('language', e.target.value)}
        >
          <option value="en">English</option>
          <option value="es">Español</option>
          <option value="fr">Français</option>
          <option value="de">Deutsch</option>
          <option value="ja">日本語</option>
        </select>
      </div>

      <div className="setting-group">
        <label>Theme</label>
        <select
          value={settings.theme}
          onChange={(e) => updateSetting('theme', e.target.value)}
        >
          <option value="dark">Dark (Nova)</option>
          <option value="light">Light</option>
          <option value="auto">Auto</option>
        </select>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.notifications}
            onChange={(e) => updateSetting('notifications', e.target.checked)}
          />
          Enable Notifications
        </label>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.autoUpdate}
            onChange={(e) => updateSetting('autoUpdate', e.target.checked)}
          />
          Auto-Update Games
        </label>
      </div>
    </div>
  );

  const renderGraphicsSettings = () => (
    <div className="settings-section">
      <h2>Graphics Settings</h2>

      <div className="setting-group">
        <label>Quality Preset</label>
        <select
          value={settings.quality}
          onChange={(e) => updateSetting('quality', e.target.value)}
        >
          <option value="low">Low</option>
          <option value="medium">Medium</option>
          <option value="high">High</option>
          <option value="ultra">Ultra</option>
          <option value="custom">Custom</option>
        </select>
      </div>

      <div className="setting-group">
        <label>Resolution</label>
        <select
          value={settings.resolution}
          onChange={(e) => updateSetting('resolution', e.target.value)}
        >
          <option value="1920x1080">1920 x 1080 (1080p)</option>
          <option value="2560x1440">2560 x 1440 (1440p)</option>
          <option value="3840x2160">3840 x 2160 (4K)</option>
        </select>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.vsync}
            onChange={(e) => updateSetting('vsync', e.target.checked)}
          />
          V-Sync
        </label>
      </div>

      <div className="setting-group">
        <label>Anti-Aliasing</label>
        <select
          value={settings.antialiasing}
          onChange={(e) => updateSetting('antialiasing', e.target.value)}
        >
          <option value="off">Off</option>
          <option value="fxaa">FXAA</option>
          <option value="msaa-2x">MSAA 2x</option>
          <option value="msaa-4x">MSAA 4x</option>
          <option value="msaa-8x">MSAA 8x</option>
          <option value="taa">TAA</option>
        </select>
      </div>

      <div className="setting-group">
        <label>Shadows: {settings.shadows}</label>
        <select
          value={settings.shadows}
          onChange={(e) => updateSetting('shadows', e.target.value)}
        >
          <option value="off">Off</option>
          <option value="low">Low</option>
          <option value="medium">Medium</option>
          <option value="high">High</option>
          <option value="ultra">Ultra</option>
        </select>
      </div>

      <div className="setting-group">
        <label>View Distance: {settings.viewDistance}%</label>
        <input
          type="range"
          min="25"
          max="200"
          value={settings.viewDistance}
          onChange={(e) =>
            updateSetting('viewDistance', parseInt(e.target.value))
          }
        />
      </div>
    </div>
  );

  const renderAudioSettings = () => (
    <div className="settings-section">
      <h2>Audio Settings</h2>

      <div className="setting-group">
        <label>Master Volume: {settings.masterVolume}%</label>
        <input
          type="range"
          min="0"
          max="100"
          value={settings.masterVolume}
          onChange={(e) =>
            updateSetting('masterVolume', parseInt(e.target.value))
          }
        />
      </div>

      <div className="setting-group">
        <label>Music Volume: {settings.musicVolume}%</label>
        <input
          type="range"
          min="0"
          max="100"
          value={settings.musicVolume}
          onChange={(e) =>
            updateSetting('musicVolume', parseInt(e.target.value))
          }
        />
      </div>

      <div className="setting-group">
        <label>SFX Volume: {settings.sfxVolume}%</label>
        <input
          type="range"
          min="0"
          max="100"
          value={settings.sfxVolume}
          onChange={(e) => updateSetting('sfxVolume', parseInt(e.target.value))}
        />
      </div>

      <div className="setting-group">
        <label>Voice Volume: {settings.voiceVolume}%</label>
        <input
          type="range"
          min="0"
          max="100"
          value={settings.voiceVolume}
          onChange={(e) =>
            updateSetting('voiceVolume', parseInt(e.target.value))
          }
        />
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.spatialAudio}
            onChange={(e) => updateSetting('spatialAudio', e.target.checked)}
          />
          Enable Spatial Audio (3D Sound)
        </label>
      </div>
    </div>
  );

  const renderControlsSettings = () => (
    <div className="settings-section">
      <h2>Controls Settings</h2>

      <div className="setting-group">
        <label>Mouse Sensitivity: {settings.mouseSensitivity}%</label>
        <input
          type="range"
          min="0"
          max="100"
          value={settings.mouseSensitivity}
          onChange={(e) =>
            updateSetting('mouseSensitivity', parseInt(e.target.value))
          }
        />
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.invertY}
            onChange={(e) => updateSetting('invertY', e.target.checked)}
          />
          Invert Y-Axis
        </label>
      </div>

      <div className="keybinds-section">
        <h3>Keybindings</h3>
        {Object.entries(settings.keybinds).map(([action, key]) => (
          <div key={action} className="keybind-row">
            <span className="keybind-action">
              {action.charAt(0).toUpperCase() + action.slice(1)}
            </span>
            <button className="keybind-button">{key}</button>
          </div>
        ))}
        <p className="note">
          Click a button to rebind (TODO: implement rebind functionality)
        </p>
      </div>
    </div>
  );

  const renderNetworkSettings = () => (
    <div className="settings-section">
      <h2>Network Settings</h2>

      <div className="setting-group">
        <label>Preferred Region</label>
        <select
          value={settings.region}
          onChange={(e) => updateSetting('region', e.target.value)}
        >
          <option value="us-east">US East</option>
          <option value="us-west">US West</option>
          <option value="eu-west">EU West</option>
          <option value="eu-east">EU East</option>
          <option value="asia">Asia</option>
          <option value="oceania">Oceania</option>
        </select>
      </div>

      <div className="setting-group">
        <label>Max Ping: {settings.maxPing}ms</label>
        <input
          type="range"
          min="50"
          max="200"
          step="10"
          value={settings.maxPing}
          onChange={(e) => updateSetting('maxPing', parseInt(e.target.value))}
        />
      </div>

      <div className="setting-group">
        <label>Bandwidth Limit</label>
        <select
          value={settings.bandwidthLimit}
          onChange={(e) => updateSetting('bandwidthLimit', e.target.value)}
        >
          <option value="unlimited">Unlimited</option>
          <option value="10mbps">10 Mbps</option>
          <option value="5mbps">5 Mbps</option>
          <option value="1mbps">1 Mbps</option>
        </select>
      </div>
    </div>
  );

  const renderPrivacySettings = () => (
    <div className="settings-section">
      <h2>Privacy Settings</h2>

      <div className="setting-group">
        <label>Profile Visibility</label>
        <select
          value={settings.profileVisibility}
          onChange={(e) => updateSetting('profileVisibility', e.target.value)}
        >
          <option value="public">Public</option>
          <option value="friends">Friends Only</option>
          <option value="private">Private</option>
        </select>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.showOnlineStatus}
            onChange={(e) =>
              updateSetting('showOnlineStatus', e.target.checked)
            }
          />
          Show Online Status
        </label>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.dataCollection}
            onChange={(e) => updateSetting('dataCollection', e.target.checked)}
          />
          Allow Anonymous Data Collection (helps improve Nova Engine)
        </label>
      </div>
    </div>
  );

  const renderAccessibilitySettings = () => (
    <div className="settings-section">
      <h2>Accessibility Settings</h2>

      <div className="setting-group">
        <label>Colorblind Mode</label>
        <select
          value={settings.colorblindMode}
          onChange={(e) => updateSetting('colorblindMode', e.target.value)}
        >
          <option value="none">None</option>
          <option value="protanopia">Protanopia</option>
          <option value="deuteranopia">Deuteranopia</option>
          <option value="tritanopia">Tritanopia</option>
        </select>
      </div>

      <div className="setting-group">
        <label>Font Size</label>
        <select
          value={settings.fontSize}
          onChange={(e) => updateSetting('fontSize', e.target.value)}
        >
          <option value="small">Small</option>
          <option value="medium">Medium</option>
          <option value="large">Large</option>
          <option value="xlarge">Extra Large</option>
        </select>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.screenShake}
            onChange={(e) => updateSetting('screenShake', e.target.checked)}
          />
          Screen Shake Effects
        </label>
      </div>

      <div className="setting-group checkbox">
        <label>
          <input
            type="checkbox"
            checked={settings.motionBlur}
            onChange={(e) => updateSetting('motionBlur', e.target.checked)}
          />
          Motion Blur
        </label>
      </div>
    </div>
  );

  const renderAdvancedSettings = () => (
    <div className="settings-section">
      <h2>Advanced Settings</h2>

      <div className="advanced-actions">
        <button className="advanced-btn">Clear Cache</button>
        <button className="advanced-btn">Run Diagnostics</button>
        <button className="advanced-btn">Verify Game Files</button>
        <button className="advanced-btn">Export Settings</button>
        <button className="advanced-btn">Import Settings</button>
        <button className="advanced-btn danger">Reset to Defaults</button>
      </div>

      <div className="system-info">
        <h3>System Information</h3>
        <div className="info-grid">
          <div className="info-item">
            <span className="label">OS:</span>
            <span className="value">Windows 11</span>
          </div>
          <div className="info-item">
            <span className="label">CPU:</span>
            <span className="value">AMD Ryzen 9 5900X</span>
          </div>
          <div className="info-item">
            <span className="label">GPU:</span>
            <span className="value">NVIDIA RTX 3080</span>
          </div>
          <div className="info-item">
            <span className="label">RAM:</span>
            <span className="value">32 GB DDR4</span>
          </div>
          <div className="info-item">
            <span className="label">Nova Engine:</span>
            <span className="value">v2.0.0</span>
          </div>
        </div>
      </div>
    </div>
  );

  const renderContent = () => {
    switch (activeCategory) {
      case 'general':
        return renderGeneralSettings();
      case 'graphics':
        return renderGraphicsSettings();
      case 'audio':
        return renderAudioSettings();
      case 'controls':
        return renderControlsSettings();
      case 'network':
        return renderNetworkSettings();
      case 'privacy':
        return renderPrivacySettings();
      case 'accessibility':
        return renderAccessibilitySettings();
      case 'advanced':
        return renderAdvancedSettings();
      default:
        return renderGeneralSettings();
    }
  };

  return (
    <div className="settings-module-v2">
      {/* Header with orbital planet decoration */}
      <div className="settings-header">
        <div className="orbital-planet-decoration"></div>
        <h1 className="settings-title">
          <span className="nova-gradient">NOVA ENGINE</span> Settings
        </h1>
        <p className="settings-subtitle">Customize your experience</p>
      </div>

      <div className="settings-container">
        {/* Sidebar with categories */}
        <div className="settings-sidebar">
          {categories.map((cat) => (
            <button
              key={cat.id}
              className={`settings-category-btn ${activeCategory === cat.id ? 'active' : ''}`}
              onClick={() => setActiveCategory(cat.id)}
            >
              <span className="category-icon">{cat.icon}</span>
              <span className="category-label">{cat.label}</span>
            </button>
          ))}
        </div>

        {/* Main settings content */}
        <div className="settings-content">
          {renderContent()}

          <div className="settings-actions">
            <button className="save-btn">Save Changes</button>
            <button className="cancel-btn">Cancel</button>
          </div>
        </div>
      </div>
    </div>
  );
};
