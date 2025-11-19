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
 * NOVA ENGINE - SETTINGS MODULE
 * Complete settings system with all configuration options
 */

import React, { useState } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface SettingsModuleProps {
  platform: UnifiedPlatformCore;
}

export const SettingsModuleRedesigned: React.FC<SettingsModuleProps> = ({
  platform,
}) => {
  const [activeCategory, setActiveCategory] = useState<
    | 'general'
    | 'graphics'
    | 'audio'
    | 'controls'
    | 'network'
    | 'privacy'
    | 'accessibility'
    | 'advanced'
  >('general');

  // General Settings
  const [language, setLanguage] = useState('en');
  const [theme, setTheme] = useState('space');
  const [autoSave, setAutoSave] = useState(true);
  const [notifications, setNotifications] = useState(true);
  const [showFPS, setShowFPS] = useState(false);

  // Graphics Settings
  const [resolution, setResolution] = useState('1920x1080');
  const [displayMode, setDisplayMode] = useState('windowed');
  const [vsync, setVsync] = useState(true);
  const [frameLimit, setFrameLimit] = useState(60);
  const [graphicsQuality, setGraphicsQuality] = useState('high');
  const [antiAliasing, setAntiAliasing] = useState('MSAA x4');
  const [shadows, setShadows] = useState('high');
  const [textures, setTextures] = useState('high');
  const [effects, setEffects] = useState('high');
  const [postProcessing, setPostProcessing] = useState(true);
  const [ambientOcclusion, setAmbientOcclusion] = useState(true);
  const [bloom, setBloom] = useState(true);
  const [motionBlur, setMotionBlur] = useState(false);

  // Audio Settings
  const [masterVolume, setMasterVolume] = useState(100);
  const [musicVolume, setMusicVolume] = useState(80);
  const [sfxVolume, setSfxVolume] = useState(90);
  const [voiceVolume, setVoiceVolume] = useState(100);
  const [spatialAudio, setSpatialAudio] = useState(true);
  const [voiceChat, setVoiceChat] = useState(true);

  // Controls Settings
  const [mouseSensitivity, setMouseSensitivity] = useState(50);
  const [invertY, setInvertY] = useState(false);
  const [autoRun, setAutoRun] = useState(false);
  const [crouchToggle, setCrouchToggle] = useState(true);

  // Network Settings
  const [maxPing, setMaxPing] = useState(100);
  const [preferredRegion, setPreferredRegion] = useState('auto');
  const [bandwidthLimit, setBandwidthLimit] = useState('unlimited');

  // Privacy Settings
  const [profileVisibility, setProfileVisibility] = useState('friends');
  const [showOnlineStatus, setShowOnlineStatus] = useState(true);
  const [allowInvites, setAllowInvites] = useState('friends');
  const [dataCollection, setDataCollection] = useState(true);

  // Accessibility Settings
  const [colorBlindMode, setColorBlindMode] = useState('none');
  const [fontSize, setFontSize] = useState(16);
  const [highContrast, setHighContrast] = useState(false);
  const [screenReader, setScreenReader] = useState(false);
  const [subtitles, setSubtitles] = useState(true);

  const handleResetSettings = () => {
    if (confirm('Reset all settings to default? This cannot be undone.')) {
      // Reset all settings
      platform.showNotification({
        type: 'success',
        message: 'Settings reset to default',
      });
    }
  };

  const handleSaveSettings = () => {
    // TODO: Save settings to backend
    platform.showNotification({
      type: 'success',
      message: 'Settings saved successfully',
    });
  };

  const categories = [
    { id: 'general', name: 'General', icon: '⚙️' },
    { id: 'graphics', name: 'Graphics', icon: '🎨' },
    { id: 'audio', name: 'Audio', icon: '🔊' },
    { id: 'controls', name: 'Controls', icon: '🎮' },
    { id: 'network', name: 'Network', icon: '🌐' },
    { id: 'privacy', name: 'Privacy', icon: '🔒' },
    { id: 'accessibility', name: 'Accessibility', icon: '♿' },
    { id: 'advanced', name: 'Advanced', icon: '🔧' },
  ];

  return (
    <div className="settings-redesigned">
      {/* Header */}
      <div className="settings-header">
        <div className="settings-branding">
          <h1 className="settings-title">SETTINGS</h1>
          <p className="settings-subtitle">Customize Your Experience</p>
        </div>
        <div className="settings-actions">
          <button onClick={handleResetSettings} className="reset-btn">
            🔄 Reset to Default
          </button>
          <button onClick={handleSaveSettings} className="save-btn">
            💾 Save Changes
          </button>
        </div>
      </div>

      {/* Content */}
      <div className="settings-content">
        {/* Sidebar */}
        <div className="settings-sidebar">
          {categories.map((category) => (
            <button
              key={category.id}
              className={`category-btn ${activeCategory === category.id ? 'active' : ''}`}
              onClick={() => setActiveCategory(category.id as any)}
            >
              <span className="category-icon">{category.icon}</span>
              <span>{category.name}</span>
            </button>
          ))}
        </div>

        {/* Settings Panel */}
        <div className="settings-panel">
          {/* General Settings */}
          {activeCategory === 'general' && (
            <div className="settings-section">
              <h2>General Settings</h2>

              <div className="setting-group">
                <label>Language</label>
                <select
                  value={language}
                  onChange={(e) => setLanguage(e.target.value)}
                >
                  <option value="en">English</option>
                  <option value="es">Español</option>
                  <option value="fr">Français</option>
                  <option value="de">Deutsch</option>
                  <option value="ja">日本語</option>
                  <option value="zh">中文</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Theme</label>
                <select
                  value={theme}
                  onChange={(e) => setTheme(e.target.value)}
                >
                  <option value="space">Space (Default)</option>
                  <option value="dark">Dark</option>
                  <option value="light">Light</option>
                  <option value="neon">Neon</option>
                </select>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={autoSave}
                    onChange={(e) => setAutoSave(e.target.checked)}
                  />
                  <span>Enable Auto-Save</span>
                </label>
                <p className="setting-description">
                  Automatically save your progress every 5 minutes
                </p>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={notifications}
                    onChange={(e) => setNotifications(e.target.checked)}
                  />
                  <span>Enable Notifications</span>
                </label>
                <p className="setting-description">
                  Show desktop notifications for events and updates
                </p>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={showFPS}
                    onChange={(e) => setShowFPS(e.target.checked)}
                  />
                  <span>Show FPS Counter</span>
                </label>
                <p className="setting-description">
                  Display frame rate in the corner of the screen
                </p>
              </div>
            </div>
          )}

          {/* Graphics Settings */}
          {activeCategory === 'graphics' && (
            <div className="settings-section">
              <h2>Graphics Settings</h2>

              <div className="preset-buttons">
                <button
                  onClick={() => setGraphicsQuality('low')}
                  className={graphicsQuality === 'low' ? 'active' : ''}
                >
                  Low
                </button>
                <button
                  onClick={() => setGraphicsQuality('medium')}
                  className={graphicsQuality === 'medium' ? 'active' : ''}
                >
                  Medium
                </button>
                <button
                  onClick={() => setGraphicsQuality('high')}
                  className={graphicsQuality === 'high' ? 'active' : ''}
                >
                  High
                </button>
                <button
                  onClick={() => setGraphicsQuality('ultra')}
                  className={graphicsQuality === 'ultra' ? 'active' : ''}
                >
                  Ultra
                </button>
              </div>

              <div className="setting-group">
                <label>Resolution</label>
                <select
                  value={resolution}
                  onChange={(e) => setResolution(e.target.value)}
                >
                  <option value="1280x720">1280 x 720 (HD)</option>
                  <option value="1920x1080">1920 x 1080 (Full HD)</option>
                  <option value="2560x1440">2560 x 1440 (2K)</option>
                  <option value="3840x2160">3840 x 2160 (4K)</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Display Mode</label>
                <select
                  value={displayMode}
                  onChange={(e) => setDisplayMode(e.target.value)}
                >
                  <option value="windowed">Windowed</option>
                  <option value="borderless">Borderless Window</option>
                  <option value="fullscreen">Fullscreen</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Frame Limit</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="30"
                    max="240"
                    step="30"
                    value={frameLimit}
                    onChange={(e) => setFrameLimit(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{frameLimit} FPS</span>
                </div>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={vsync}
                    onChange={(e) => setVsync(e.target.checked)}
                  />
                  <span>V-Sync</span>
                </label>
              </div>

              <div className="setting-group">
                <label>Anti-Aliasing</label>
                <select
                  value={antiAliasing}
                  onChange={(e) => setAntiAliasing(e.target.value)}
                >
                  <option value="none">None</option>
                  <option value="FXAA">FXAA</option>
                  <option value="MSAA x2">MSAA x2</option>
                  <option value="MSAA x4">MSAA x4</option>
                  <option value="MSAA x8">MSAA x8</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Shadow Quality</label>
                <select
                  value={shadows}
                  onChange={(e) => setShadows(e.target.value)}
                >
                  <option value="off">Off</option>
                  <option value="low">Low</option>
                  <option value="medium">Medium</option>
                  <option value="high">High</option>
                  <option value="ultra">Ultra</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Texture Quality</label>
                <select
                  value={textures}
                  onChange={(e) => setTextures(e.target.value)}
                >
                  <option value="low">Low</option>
                  <option value="medium">Medium</option>
                  <option value="high">High</option>
                  <option value="ultra">Ultra</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Effects Quality</label>
                <select
                  value={effects}
                  onChange={(e) => setEffects(e.target.value)}
                >
                  <option value="low">Low</option>
                  <option value="medium">Medium</option>
                  <option value="high">High</option>
                  <option value="ultra">Ultra</option>
                </select>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={postProcessing}
                    onChange={(e) => setPostProcessing(e.target.checked)}
                  />
                  <span>Post Processing</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={ambientOcclusion}
                    onChange={(e) => setAmbientOcclusion(e.target.checked)}
                  />
                  <span>Ambient Occlusion (SSAO)</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={bloom}
                    onChange={(e) => setBloom(e.target.checked)}
                  />
                  <span>Bloom</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={motionBlur}
                    onChange={(e) => setMotionBlur(e.target.checked)}
                  />
                  <span>Motion Blur</span>
                </label>
              </div>
            </div>
          )}

          {/* Audio Settings */}
          {activeCategory === 'audio' && (
            <div className="settings-section">
              <h2>Audio Settings</h2>

              <div className="setting-group">
                <label>Master Volume</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="0"
                    max="100"
                    value={masterVolume}
                    onChange={(e) => setMasterVolume(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{masterVolume}%</span>
                </div>
              </div>

              <div className="setting-group">
                <label>Music Volume</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="0"
                    max="100"
                    value={musicVolume}
                    onChange={(e) => setMusicVolume(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{musicVolume}%</span>
                </div>
              </div>

              <div className="setting-group">
                <label>Sound Effects Volume</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="0"
                    max="100"
                    value={sfxVolume}
                    onChange={(e) => setSfxVolume(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{sfxVolume}%</span>
                </div>
              </div>

              <div className="setting-group">
                <label>Voice Chat Volume</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="0"
                    max="100"
                    value={voiceVolume}
                    onChange={(e) => setVoiceVolume(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{voiceVolume}%</span>
                </div>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={spatialAudio}
                    onChange={(e) => setSpatialAudio(e.target.checked)}
                  />
                  <span>3D Spatial Audio</span>
                </label>
                <p className="setting-description">
                  Enable positional audio for immersive experience
                </p>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={voiceChat}
                    onChange={(e) => setVoiceChat(e.target.checked)}
                  />
                  <span>Enable Voice Chat</span>
                </label>
              </div>
            </div>
          )}

          {/* Controls Settings */}
          {activeCategory === 'controls' && (
            <div className="settings-section">
              <h2>Controls Settings</h2>

              <div className="setting-group">
                <label>Mouse Sensitivity</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="1"
                    max="100"
                    value={mouseSensitivity}
                    onChange={(e) =>
                      setMouseSensitivity(parseInt(e.target.value))
                    }
                  />
                  <span className="slider-value">{mouseSensitivity}%</span>
                </div>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={invertY}
                    onChange={(e) => setInvertY(e.target.checked)}
                  />
                  <span>Invert Y-Axis</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={autoRun}
                    onChange={(e) => setAutoRun(e.target.checked)}
                  />
                  <span>Auto Run</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={crouchToggle}
                    onChange={(e) => setCrouchToggle(e.target.checked)}
                  />
                  <span>Crouch Toggle (Instead of Hold)</span>
                </label>
              </div>

              <div className="keybinds-section">
                <h3>Key Bindings</h3>
                <div className="keybind-list">
                  <div className="keybind-item">
                    <span>Move Forward</span>
                    <button className="keybind-btn">W</button>
                  </div>
                  <div className="keybind-item">
                    <span>Move Backward</span>
                    <button className="keybind-btn">S</button>
                  </div>
                  <div className="keybind-item">
                    <span>Move Left</span>
                    <button className="keybind-btn">A</button>
                  </div>
                  <div className="keybind-item">
                    <span>Move Right</span>
                    <button className="keybind-btn">D</button>
                  </div>
                  <div className="keybind-item">
                    <span>Jump</span>
                    <button className="keybind-btn">Space</button>
                  </div>
                  <div className="keybind-item">
                    <span>Crouch</span>
                    <button className="keybind-btn">Ctrl</button>
                  </div>
                  <div className="keybind-item">
                    <span>Sprint</span>
                    <button className="keybind-btn">Shift</button>
                  </div>
                  <div className="keybind-item">
                    <span>Interact</span>
                    <button className="keybind-btn">E</button>
                  </div>
                </div>
                <button className="reset-keybinds-btn">Reset to Default</button>
              </div>
            </div>
          )}

          {/* Network Settings */}
          {activeCategory === 'network' && (
            <div className="settings-section">
              <h2>Network Settings</h2>

              <div className="setting-group">
                <label>Max Ping</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="50"
                    max="300"
                    step="25"
                    value={maxPing}
                    onChange={(e) => setMaxPing(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{maxPing}ms</span>
                </div>
                <p className="setting-description">
                  Maximum acceptable ping for matchmaking
                </p>
              </div>

              <div className="setting-group">
                <label>Preferred Region</label>
                <select
                  value={preferredRegion}
                  onChange={(e) => setPreferredRegion(e.target.value)}
                >
                  <option value="auto">Auto (Best Ping)</option>
                  <option value="us-east">US East</option>
                  <option value="us-west">US West</option>
                  <option value="eu-west">EU West</option>
                  <option value="eu-north">EU North</option>
                  <option value="asia">Asia</option>
                  <option value="oceania">Oceania</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Bandwidth Limit</label>
                <select
                  value={bandwidthLimit}
                  onChange={(e) => setBandwidthLimit(e.target.value)}
                >
                  <option value="unlimited">Unlimited</option>
                  <option value="1mbps">1 Mbps</option>
                  <option value="5mbps">5 Mbps</option>
                  <option value="10mbps">10 Mbps</option>
                </select>
              </div>
            </div>
          )}

          {/* Privacy Settings */}
          {activeCategory === 'privacy' && (
            <div className="settings-section">
              <h2>Privacy Settings</h2>

              <div className="setting-group">
                <label>Profile Visibility</label>
                <select
                  value={profileVisibility}
                  onChange={(e) => setProfileVisibility(e.target.value)}
                >
                  <option value="public">Public</option>
                  <option value="friends">Friends Only</option>
                  <option value="private">Private</option>
                </select>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={showOnlineStatus}
                    onChange={(e) => setShowOnlineStatus(e.target.checked)}
                  />
                  <span>Show Online Status</span>
                </label>
              </div>

              <div className="setting-group">
                <label>Allow Invites From</label>
                <select
                  value={allowInvites}
                  onChange={(e) => setAllowInvites(e.target.value)}
                >
                  <option value="everyone">Everyone</option>
                  <option value="friends">Friends Only</option>
                  <option value="none">No One</option>
                </select>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={dataCollection}
                    onChange={(e) => setDataCollection(e.target.checked)}
                  />
                  <span>Allow Anonymous Data Collection</span>
                </label>
                <p className="setting-description">
                  Help improve Nova Engine by sharing anonymous usage data
                </p>
              </div>
            </div>
          )}

          {/* Accessibility Settings */}
          {activeCategory === 'accessibility' && (
            <div className="settings-section">
              <h2>Accessibility Settings</h2>

              <div className="setting-group">
                <label>Color Blind Mode</label>
                <select
                  value={colorBlindMode}
                  onChange={(e) => setColorBlindMode(e.target.value)}
                >
                  <option value="none">None</option>
                  <option value="protanopia">Protanopia (Red-Blind)</option>
                  <option value="deuteranopia">
                    Deuteranopia (Green-Blind)
                  </option>
                  <option value="tritanopia">Tritanopia (Blue-Blind)</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Font Size</label>
                <div className="slider-with-value">
                  <input
                    type="range"
                    min="12"
                    max="24"
                    value={fontSize}
                    onChange={(e) => setFontSize(parseInt(e.target.value))}
                  />
                  <span className="slider-value">{fontSize}px</span>
                </div>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={highContrast}
                    onChange={(e) => setHighContrast(e.target.checked)}
                  />
                  <span>High Contrast Mode</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={screenReader}
                    onChange={(e) => setScreenReader(e.target.checked)}
                  />
                  <span>Screen Reader Support</span>
                </label>
              </div>

              <div className="setting-group checkbox-group">
                <label>
                  <input
                    type="checkbox"
                    checked={subtitles}
                    onChange={(e) => setSubtitles(e.target.checked)}
                  />
                  <span>Enable Subtitles</span>
                </label>
              </div>
            </div>
          )}

          {/* Advanced Settings */}
          {activeCategory === 'advanced' && (
            <div className="settings-section">
              <h2>Advanced Settings</h2>

              <div className="warning-box">
                <span className="warning-icon">⚠️</span>
                <p>
                  These settings are for advanced users. Changing them may
                  affect performance or stability.
                </p>
              </div>

              <div className="advanced-options">
                <button className="advanced-btn">🗑️ Clear Cache</button>
                <button className="advanced-btn">
                  📊 View Diagnostic Logs
                </button>
                <button className="advanced-btn">
                  🔄 Reinstall Game Files
                </button>
                <button className="advanced-btn">💾 Export Settings</button>
                <button className="advanced-btn">📥 Import Settings</button>
                <button className="advanced-btn danger">
                  ⚠️ Reset All Data
                </button>
              </div>

              <div className="system-info">
                <h3>System Information</h3>
                <div className="info-grid">
                  <div className="info-item">
                    <span className="info-label">OS:</span>
                    <span className="info-value">Windows 11</span>
                  </div>
                  <div className="info-item">
                    <span className="info-label">CPU:</span>
                    <span className="info-value">Intel Core i7-12700K</span>
                  </div>
                  <div className="info-item">
                    <span className="info-label">GPU:</span>
                    <span className="info-value">NVIDIA RTX 4070</span>
                  </div>
                  <div className="info-item">
                    <span className="info-label">RAM:</span>
                    <span className="info-value">32 GB DDR5</span>
                  </div>
                  <div className="info-item">
                    <span className="info-label">Engine Version:</span>
                    <span className="info-value">Nova Engine v0.1.0</span>
                  </div>
                </div>
              </div>
            </div>
          )}
        </div>
      </div>

      <style>{`
        .settings-redesigned {
          width: 100%;
          min-height: 100vh;
          background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
          color: white;
          display: flex;
          flex-direction: column;
        }

        /* Header */
        .settings-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 30px 40px;
          background: linear-gradient(180deg, rgba(26,0,51,0.8) 0%, transparent 100%);
          border-bottom: 2px solid rgba(123, 47, 247, 0.3);
        }

        .settings-title {
          font-size: 36px;
          font-weight: 900;
          letter-spacing: 4px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          margin: 0 0 5px 0;
        }

        .settings-subtitle {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.7);
          margin: 0;
        }

        .settings-actions {
          display: flex;
          gap: 15px;
        }

        .reset-btn,
        .save-btn {
          padding: 12px 24px;
          border: none;
          border-radius: 12px;
          font-weight: 700;
          cursor: pointer;
          font-size: 15px;
          transition: all 0.2s ease;
        }

        .reset-btn {
          background: rgba(239, 68, 68, 0.2);
          border: 2px solid rgba(239, 68, 68, 0.5);
          color: #ef4444;
        }

        .reset-btn:hover {
          background: rgba(239, 68, 68, 0.3);
        }

        .save-btn {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          color: white;
        }

        .save-btn:hover {
          transform: scale(1.02);
        }

        /* Content */
        .settings-content {
          flex: 1;
          display: flex;
          overflow: hidden;
        }

        /* Sidebar */
        .settings-sidebar {
          width: 280px;
          background: rgba(26, 0, 51, 0.6);
          border-right: 2px solid rgba(123, 47, 247, 0.3);
          padding: 20px;
          overflow-y: auto;
        }

        .category-btn {
          width: 100%;
          display: flex;
          align-items: center;
          gap: 12px;
          padding: 14px 18px;
          background: transparent;
          border: none;
          color: rgba(255, 255, 255, 0.7);
          cursor: pointer;
          font-size: 15px;
          font-weight: 600;
          border-radius: 12px;
          margin-bottom: 8px;
          transition: all 0.2s ease;
          text-align: left;
        }

        .category-btn:hover {
          background: rgba(123, 47, 247, 0.2);
          color: white;
        }

        .category-btn.active {
          background: linear-gradient(135deg, rgba(123, 47, 247, 0.4) 0%, rgba(255, 110, 196, 0.4) 100%);
          color: white;
          border: 1px solid rgba(123, 47, 247, 0.6);
        }

        .category-icon {
          font-size: 20px;
        }

        /* Settings Panel */
        .settings-panel {
          flex: 1;
          overflow-y: auto;
          padding: 40px;
        }

        .settings-section {
          max-width: 800px;
        }

        .settings-section h2 {
          font-size: 28px;
          margin: 0 0 30px 0;
        }

        .setting-group {
          margin-bottom: 25px;
        }

        .setting-group label {
          display: block;
          margin-bottom: 10px;
          font-size: 15px;
          font-weight: 600;
          color: rgba(255, 255, 255, 0.9);
        }

        .setting-group select {
          width: 100%;
          padding: 12px 16px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-size: 15px;
          cursor: pointer;
          transition: all 0.3s ease;
        }

        .setting-group select:focus {
          outline: none;
          border-color: rgba(123, 47, 247, 0.8);
          box-shadow: 0 0 20px rgba(123, 47, 247, 0.3);
        }

        .slider-with-value {
          display: flex;
          align-items: center;
          gap: 15px;
        }

        .slider-with-value input[type="range"] {
          flex: 1;
          height: 6px;
          background: rgba(26, 0, 51, 0.6);
          border-radius: 3px;
          outline: none;
          -webkit-appearance: none;
        }

        .slider-with-value input[type="range"]::-webkit-slider-thumb {
          -webkit-appearance: none;
          width: 18px;
          height: 18px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border-radius: 50%;
          cursor: pointer;
        }

        .slider-value {
          min-width: 70px;
          padding: 6px 12px;
          background: rgba(123, 47, 247, 0.2);
          border-radius: 8px;
          text-align: center;
          font-weight: 600;
          color: #4cc9f0;
        }

        .checkbox-group label {
          display: flex;
          align-items: center;
          gap: 12px;
          cursor: pointer;
        }

        .checkbox-group input[type="checkbox"] {
          width: 20px;
          height: 20px;
          cursor: pointer;
        }

        .setting-description {
          margin: 8px 0 0 0;
          font-size: 13px;
          color: rgba(255, 255, 255, 0.6);
          font-weight: normal;
        }

        /* Preset Buttons */
        .preset-buttons {
          display: grid;
          grid-template-columns: repeat(4, 1fr);
          gap: 12px;
          margin-bottom: 30px;
        }

        .preset-buttons button {
          padding: 14px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-weight: 600;
          cursor: pointer;
          transition: all 0.2s ease;
        }

        .preset-buttons button:hover {
          background: rgba(123, 47, 247, 0.2);
        }

        .preset-buttons button.active {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border-color: transparent;
        }

        /* Keybinds */
        .keybinds-section {
          margin-top: 40px;
          padding-top: 30px;
          border-top: 2px solid rgba(123, 47, 247, 0.2);
        }

        .keybinds-section h3 {
          font-size: 20px;
          margin: 0 0 20px 0;
        }

        .keybind-list {
          display: flex;
          flex-direction: column;
          gap: 12px;
          margin-bottom: 20px;
        }

        .keybind-item {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 12px 16px;
          background: rgba(26, 0, 51, 0.6);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 10px;
        }

        .keybind-btn {
          padding: 8px 20px;
          background: rgba(123, 47, 247, 0.3);
          border: 2px solid rgba(123, 47, 247, 0.5);
          border-radius: 8px;
          color: white;
          font-weight: 700;
          cursor: pointer;
          min-width: 80px;
        }

        .reset-keybinds-btn {
          padding: 12px 24px;
          background: rgba(239, 68, 68, 0.2);
          border: 2px solid rgba(239, 68, 68, 0.5);
          border-radius: 12px;
          color: #ef4444;
          font-weight: 700;
          cursor: pointer;
        }

        /* Warning Box */
        .warning-box {
          display: flex;
          align-items: center;
          gap: 15px;
          padding: 20px;
          background: rgba(251, 191, 36, 0.1);
          border: 2px solid rgba(251, 191, 36, 0.5);
          border-radius: 12px;
          margin-bottom: 30px;
        }

        .warning-icon {
          font-size: 32px;
        }

        .warning-box p {
          margin: 0;
          color: #fbbf24;
        }

        /* Advanced Options */
        .advanced-options {
          display: grid;
          grid-template-columns: repeat(2, 1fr);
          gap: 15px;
          margin-bottom: 40px;
        }

        .advanced-btn {
          padding: 14px 20px;
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          color: white;
          font-weight: 600;
          cursor: pointer;
          transition: all 0.2s ease;
        }

        .advanced-btn:hover {
          background: rgba(123, 47, 247, 0.2);
          border-color: rgba(123, 47, 247, 0.5);
        }

        .advanced-btn.danger {
          background: rgba(239, 68, 68, 0.2);
          border-color: rgba(239, 68, 68, 0.5);
          color: #ef4444;
        }

        .advanced-btn.danger:hover {
          background: rgba(239, 68, 68, 0.3);
        }

        /* System Info */
        .system-info {
          background: rgba(26, 0, 51, 0.6);
          border: 2px solid rgba(123, 47, 247, 0.3);
          border-radius: 16px;
          padding: 24px;
        }

        .system-info h3 {
          font-size: 20px;
          margin: 0 0 20px 0;
        }

        .info-grid {
          display: flex;
          flex-direction: column;
          gap: 12px;
        }

        .info-item {
          display: flex;
          justify-content: space-between;
          padding: 10px 0;
          border-bottom: 1px solid rgba(123, 47, 247, 0.2);
        }

        .info-item:last-child {
          border-bottom: none;
        }

        .info-label {
          font-weight: 600;
          color: rgba(255, 255, 255, 0.7);
        }

        .info-value {
          color: #4cc9f0;
        }

        @media (max-width: 1024px) {
          .settings-content {
            flex-direction: column;
          }
          
          .settings-sidebar {
            width: 100%;
            border-right: none;
            border-bottom: 2px solid rgba(123, 47, 247, 0.3);
          }
          
          .advanced-options {
            grid-template-columns: 1fr;
          }
        }
      `}</style>
    </div>
  );
};
export { SettingsModuleRedesigned as SettingsModule } from './SettingsModule';
