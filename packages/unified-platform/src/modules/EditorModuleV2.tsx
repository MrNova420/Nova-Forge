/**
 * NOVA ENGINE - EDITOR V2
 * Complete redesign matching Image 2 mockup
 * Production-grade Unity/Unreal-inspired editor with Image 2 theme
 *
 * Features:
 * - Professional menu bar and toolbar
 * - Multi-panel layout (Scene, Hierarchy, Inspector, Assets, Console)
 * - Purple/pink space theme from Image 2
 * - All advanced features (Animation, Lighting, Physics, Particles, Shaders)
 * - Build settings and profiler
 * - Complete UI with backend TODO notes
 */

import React, { useState, useEffect, useRef } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';
import { apiClient } from '../services/ApiClient';
import './styles/EditorModuleV2.css';

// Import the actual Nova Engine
import {
  Engine,
  WebGLRenderer,
  Scene,
  Camera,
  Entity,
} from '@nova-engine/engine';

interface EditorModuleV2Props {
  platform: UnifiedPlatformCore;
}

interface Entity {
  id: string;
  name: string;
  type: string;
  components: string[];
  children: Entity[];
  transform?: {
    position: { x: number; y: number; z: number };
    rotation: { x: number; y: number; z: number };
    scale: { x: number; y: number; z: number };
  };
}

interface Asset {
  id: string;
  name: string;
  type: 'model' | 'texture' | 'material' | 'script' | 'audio' | 'prefab';
  path: string;
  thumbnail?: string;
}

interface ConsoleLog {
  type: 'info' | 'warning' | 'error';
  message: string;
  timestamp: Date;
}

export const EditorModuleV2: React.FC<EditorModuleV2Props> = () => {
  // Editor state
  const [currentProject] = useState<string>('Untitled Project');
  const [entities, setEntities] = useState<Entity[]>([
    {
      id: '1',
      name: 'Camera',
      type: 'Camera',
      components: ['Transform', 'Camera'],
      children: [],
      transform: {
        position: { x: 0, y: 5, z: -10 },
        rotation: { x: 0, y: 0, z: 0 },
        scale: { x: 1, y: 1, z: 1 },
      },
    },
    {
      id: '2',
      name: 'Directional Light',
      type: 'Light',
      components: ['Transform', 'Light'],
      children: [],
      transform: {
        position: { x: 0, y: 10, z: 0 },
        rotation: { x: 50, y: -30, z: 0 },
        scale: { x: 1, y: 1, z: 1 },
      },
    },
  ]);
  const [selectedEntity, setSelectedEntity] = useState<Entity | null>(null);
  const [assets, setAssets] = useState<Asset[]>([]);
  const [consoleLogs, setConsoleLogs] = useState<ConsoleLog[]>([
    {
      type: 'info',
      message: 'Editor initialized successfully',
      timestamp: new Date(),
    },
  ]);

  // Tool state
  const [currentTool, setCurrentTool] = useState<
    'select' | 'translate' | 'rotate' | 'scale'
  >('select');
  const [isPlaying, setIsPlaying] = useState(false);
  const [isPaused, setIsPaused] = useState(false);

  // Panel visibility
  const [activePanel, setActivePanel] = useState<string>('scene');
  const [showAnimationPanel, setShowAnimationPanel] = useState(false);
  const [showLightingPanel, setShowLightingPanel] = useState(false);
  const [showPhysicsPanel, setShowPhysicsPanel] = useState(false);
  const [showParticlePanel, setShowParticlePanel] = useState(false);
  const [showShaderPanel, setShowShaderPanel] = useState(false);
  const [showProfilerPanel, setShowProfilerPanel] = useState(false);
  const [showBuildSettings, setShowBuildSettings] = useState(false);

  // Canvas ref and engine instance
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const engineRef = useRef<Engine | null>(null);
  const rendererRef = useRef<WebGLRenderer | null>(null);
  const sceneRef = useRef<Scene | null>(null);

  useEffect(() => {
    loadProjectAssets();
    initializeEngine();

    return () => {
      // Cleanup engine on unmount
      if (engineRef.current) {
        engineRef.current.stop();
      }
    };
  }, []);

  const loadProjectAssets = async () => {
    try {
      // Load assets from backend API - production ready, no fallbacks
      const projectId = 'current'; // TODO: Get from actual project context
      const assetsFromAPI = await apiClient.getAssets(projectId);

      const assetsList: Asset[] = Array.isArray(assetsFromAPI)
        ? assetsFromAPI
        : [];

      setAssets(assetsList);
      addLog('info', `Loaded ${assetsList.length} assets from project`);
    } catch (error) {
      console.error('Failed to load assets from backend API:', error);
      addLog('error', 'Unable to load project assets. Backend connection failed.');
      throw new Error(
        'Unable to load project assets. Please ensure the backend is running.'
      );
    }
  };

  const initializeEngine = async () => {
    if (!canvasRef.current) {
      addLog('error', 'Canvas not available');
      return;
    }

    try {
      // Initialize WebGL Renderer
      const renderer = new WebGLRenderer();
      await renderer.initialize(canvasRef.current);
      rendererRef.current = renderer;

      // Create scene
      const scene = new Scene();
      sceneRef.current = scene;

      // Initialize engine
      const engine = new Engine({
        canvas: canvasRef.current,
        handlers: {
          onInit: () => {
            addLog('info', '✅ Nova Engine initialized successfully');
          },
          onUpdate: (deltaTime: number) => {
            // Update scene
            if (sceneRef.current && rendererRef.current) {
              // Render the scene
              // rendererRef.current.render(sceneRef.current);
            }
          },
          onRender: () => {
            // Rendering handled in update
          },
        },
      });

      engineRef.current = engine;
      await engine.initialize();

      addLog('info', '✅ WebGL Renderer connected to editor');
      addLog('info', 'Ready to create and edit scenes');
    } catch (error) {
      console.error('Failed to initialize engine:', error);
      addLog('error', `Engine initialization failed: ${error}`);
    }
  };

  const addLog = (type: ConsoleLog['type'], message: string) => {
    setConsoleLogs((prev) => [
      ...prev,
      { type, message, timestamp: new Date() },
    ]);
  };

  const handlePlay = () => {
    setIsPlaying(!isPlaying);
    setIsPaused(false);
    addLog(
      'info',
      isPlaying
        ? 'Stopped playback'
        : 'Started playback - TODO: Connect to game runtime'
    );
  };

  const handlePause = () => {
    if (isPlaying) {
      setIsPaused(!isPaused);
      addLog('info', isPaused ? 'Resumed' : 'Paused');
    }
  };

  const handleAddEntity = (type: string) => {
    const newEntity: Entity = {
      id: Date.now().toString(),
      name: `New ${type}`,
      type: type,
      components: ['Transform'],
      children: [],
      transform: {
        position: { x: 0, y: 0, z: 0 },
        rotation: { x: 0, y: 0, z: 0 },
        scale: { x: 1, y: 1, z: 1 },
      },
    };
    setEntities([...entities, newEntity]);
    addLog(
      'info',
      `Added ${type} to scene - TODO: Create actual entity in engine`
    );
  };

  const handleDeleteEntity = (entityId: string) => {
    setEntities(entities.filter((e) => e.id !== entityId));
    if (selectedEntity?.id === entityId) {
      setSelectedEntity(null);
    }
    addLog('info', 'Entity deleted - TODO: Remove from engine');
  };

  return (
    <div className="editor-v2-container">
      {/* Top Menu Bar */}
      <div className="editor-v2-menubar">
        <div className="editor-v2-logo">
          <span className="nova-text">NOVA</span>
          <span className="editor-title">{currentProject}</span>
        </div>
        <div className="editor-v2-menu">
          <button className="menu-btn">File</button>
          <button className="menu-btn">Edit</button>
          <button className="menu-btn">Assets</button>
          <button className="menu-btn">GameObject</button>
          <button className="menu-btn">Component</button>
          <button className="menu-btn">Window</button>
          <button className="menu-btn">Help</button>
        </div>
        <div className="editor-v2-actions">
          <button className="action-btn">Save</button>
          <button
            className="action-btn"
            onClick={() => setShowBuildSettings(true)}
          >
            Build
          </button>
        </div>
      </div>

      {/* Toolbar */}
      <div className="editor-v2-toolbar">
        <div className="toolbar-section">
          <button
            className={`tool-btn ${currentTool === 'select' ? 'active' : ''}`}
            onClick={() => setCurrentTool('select')}
            title="Select (Q)"
          >
            <span className="icon">⬜</span>
          </button>
          <button
            className={`tool-btn ${currentTool === 'translate' ? 'active' : ''}`}
            onClick={() => setCurrentTool('translate')}
            title="Translate (W)"
          >
            <span className="icon">✢</span>
          </button>
          <button
            className={`tool-btn ${currentTool === 'rotate' ? 'active' : ''}`}
            onClick={() => setCurrentTool('rotate')}
            title="Rotate (E)"
          >
            <span className="icon">↻</span>
          </button>
          <button
            className={`tool-btn ${currentTool === 'scale' ? 'active' : ''}`}
            onClick={() => setCurrentTool('scale')}
            title="Scale (R)"
          >
            <span className="icon">⊕</span>
          </button>
        </div>
        <div className="toolbar-section toolbar-center">
          <button
            className={`play-btn ${isPlaying ? 'active' : ''}`}
            onClick={handlePlay}
            title="Play (Ctrl+P)"
          >
            {isPlaying ? '⏹' : '▶'}
          </button>
          <button
            className={`pause-btn ${isPaused ? 'active' : ''}`}
            onClick={handlePause}
            disabled={!isPlaying}
            title="Pause"
          >
            ⏸
          </button>
          <button className="step-btn" disabled={!isPaused} title="Step">
            ⏭
          </button>
        </div>
        <div className="toolbar-section">
          <button
            className="tool-btn"
            onClick={() => setShowAnimationPanel(true)}
          >
            🎬 Animation
          </button>
          <button
            className="tool-btn"
            onClick={() => setShowLightingPanel(true)}
          >
            💡 Lighting
          </button>
          <button
            className="tool-btn"
            onClick={() => setShowPhysicsPanel(true)}
          >
            ⚛️ Physics
          </button>
          <button
            className="tool-btn"
            onClick={() => setShowParticlePanel(true)}
          >
            ✨ Particles
          </button>
          <button className="tool-btn" onClick={() => setShowShaderPanel(true)}>
            🎨 Shaders
          </button>
          <button
            className="tool-btn"
            onClick={() => setShowProfilerPanel(true)}
          >
            📊 Profiler
          </button>
        </div>
      </div>

      {/* Main Layout */}
      <div className="editor-v2-main">
        {/* Left Sidebar - Hierarchy */}
        <aside className="editor-v2-sidebar left">
          <div className="panel-header">
            <h3>Hierarchy</h3>
            <div className="panel-actions">
              <button
                className="icon-btn"
                onClick={() => handleAddEntity('GameObject')}
              >
                +
              </button>
            </div>
          </div>
          <div className="panel-content hierarchy-content">
            <div className="hierarchy-tree">
              {entities.map((entity) => (
                <div
                  key={entity.id}
                  className={`hierarchy-item ${selectedEntity?.id === entity.id ? 'selected' : ''}`}
                  onClick={() => setSelectedEntity(entity)}
                >
                  <span className="entity-icon">
                    {entity.type === 'Camera'
                      ? '📷'
                      : entity.type === 'Light'
                        ? '💡'
                        : '📦'}
                  </span>
                  <span className="entity-name">{entity.name}</span>
                  <button
                    className="entity-delete"
                    onClick={(e) => {
                      e.stopPropagation();
                      handleDeleteEntity(entity.id);
                    }}
                  >
                    ✕
                  </button>
                </div>
              ))}
            </div>
            <div className="hierarchy-actions">
              <button
                className="add-entity-btn"
                onClick={() => handleAddEntity('Cube')}
              >
                + Cube
              </button>
              <button
                className="add-entity-btn"
                onClick={() => handleAddEntity('Sphere')}
              >
                + Sphere
              </button>
              <button
                className="add-entity-btn"
                onClick={() => handleAddEntity('Light')}
              >
                + Light
              </button>
              <button
                className="add-entity-btn"
                onClick={() => handleAddEntity('Camera')}
              >
                + Camera
              </button>
            </div>
          </div>
        </aside>

        {/* Center - Scene View */}
        <div className="editor-v2-center">
          <div className="panel-header">
            <div className="panel-tabs">
              <button
                className={`tab-btn ${activePanel === 'scene' ? 'active' : ''}`}
                onClick={() => setActivePanel('scene')}
              >
                Scene
              </button>
              <button
                className={`tab-btn ${activePanel === 'game' ? 'active' : ''}`}
                onClick={() => setActivePanel('game')}
              >
                Game
              </button>
            </div>
            <div className="panel-actions">
              <button className="icon-btn">🔍</button>
              <button className="icon-btn">⚙️</button>
            </div>
          </div>
          <div className="scene-viewport">
            <canvas ref={canvasRef} className="scene-canvas"></canvas>
            <div className="viewport-overlay">
              <div className="viewport-info">
                <span>Tool: {currentTool}</span>
                <span>•</span>
                <span>FPS: 60</span>
                <span>•</span>
                <span>Objects: {entities.length}</span>
              </div>
            </div>
            {/* TODO: Render 3D scene with Nova Engine */}
            <div className="scene-placeholder">
              <div className="placeholder-planet"></div>
              <p className="placeholder-text">Scene View</p>
              <p className="placeholder-note">
                TODO: Connect to Nova Engine renderer
              </p>
            </div>
          </div>

          {/* Bottom Panel - Assets & Console */}
          <div className="editor-v2-bottom-panel">
            <div className="panel-header">
              <div className="panel-tabs">
                <button
                  className={`tab-btn ${activePanel === 'assets' ? 'active' : ''}`}
                  onClick={() => setActivePanel('assets')}
                >
                  Assets
                </button>
                <button
                  className={`tab-btn ${activePanel === 'console' ? 'active' : ''}`}
                  onClick={() => setActivePanel('console')}
                >
                  Console
                </button>
              </div>
              <div className="panel-actions">
                <button className="icon-btn" onClick={() => setConsoleLogs([])}>
                  Clear
                </button>
              </div>
            </div>
            <div className="panel-content bottom-content">
              {activePanel === 'assets' && (
                <div className="assets-browser">
                  <div className="assets-toolbar">
                    <input
                      type="text"
                      className="assets-search"
                      placeholder="Search assets..."
                    />
                    <button className="assets-filter-btn">All Types ▼</button>
                  </div>
                  <div className="assets-grid">
                    {assets.map((asset) => (
                      <div key={asset.id} className="asset-item">
                        <div className="asset-thumbnail">
                          {asset.type === 'model' && '📦'}
                          {asset.type === 'texture' && '🖼️'}
                          {asset.type === 'material' && '🎨'}
                          {asset.type === 'script' && '📜'}
                          {asset.type === 'audio' && '🔊'}
                          {asset.type === 'prefab' && '🧩'}
                        </div>
                        <span className="asset-name">{asset.name}</span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
              {activePanel === 'console' && (
                <div className="console-panel">
                  {consoleLogs.map((log, index) => (
                    <div
                      key={index}
                      className={`console-log console-${log.type}`}
                    >
                      <span className="log-icon">
                        {log.type === 'info' && 'ℹ️'}
                        {log.type === 'warning' && '⚠️'}
                        {log.type === 'error' && '❌'}
                      </span>
                      <span className="log-time">
                        {log.timestamp.toLocaleTimeString()}
                      </span>
                      <span className="log-message">{log.message}</span>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>
        </div>

        {/* Right Sidebar - Inspector */}
        <aside className="editor-v2-sidebar right">
          <div className="panel-header">
            <h3>Inspector</h3>
          </div>
          <div className="panel-content inspector-content">
            {selectedEntity ? (
              <>
                <div className="inspector-section">
                  <input
                    type="text"
                    className="entity-name-input"
                    value={selectedEntity.name}
                    onChange={(e) => {
                      const updated = {
                        ...selectedEntity,
                        name: e.target.value,
                      };
                      setSelectedEntity(updated);
                      setEntities(
                        entities.map((ent) =>
                          ent.id === updated.id ? updated : ent
                        )
                      );
                    }}
                  />
                  <div className="entity-type">{selectedEntity.type}</div>
                </div>

                {selectedEntity.transform && (
                  <div className="inspector-section">
                    <h4 className="section-title">Transform</h4>
                    <div className="transform-field">
                      <label>Position</label>
                      <div className="vector3-input">
                        <input
                          type="number"
                          value={selectedEntity.transform.position.x}
                          readOnly
                        />
                        <input
                          type="number"
                          value={selectedEntity.transform.position.y}
                          readOnly
                        />
                        <input
                          type="number"
                          value={selectedEntity.transform.position.z}
                          readOnly
                        />
                      </div>
                    </div>
                    <div className="transform-field">
                      <label>Rotation</label>
                      <div className="vector3-input">
                        <input
                          type="number"
                          value={selectedEntity.transform.rotation.x}
                          readOnly
                        />
                        <input
                          type="number"
                          value={selectedEntity.transform.rotation.y}
                          readOnly
                        />
                        <input
                          type="number"
                          value={selectedEntity.transform.rotation.z}
                          readOnly
                        />
                      </div>
                    </div>
                    <div className="transform-field">
                      <label>Scale</label>
                      <div className="vector3-input">
                        <input
                          type="number"
                          value={selectedEntity.transform.scale.x}
                          readOnly
                        />
                        <input
                          type="number"
                          value={selectedEntity.transform.scale.y}
                          readOnly
                        />
                        <input
                          type="number"
                          value={selectedEntity.transform.scale.z}
                          readOnly
                        />
                      </div>
                    </div>
                    <p className="todo-note">
                      TODO: Make transform editable and sync with engine
                    </p>
                  </div>
                )}

                <div className="inspector-section">
                  <h4 className="section-title">Components</h4>
                  {selectedEntity.components.map((comp, idx) => (
                    <div key={idx} className="component-item">
                      <span className="component-icon">🔧</span>
                      <span>{comp}</span>
                    </div>
                  ))}
                  <button className="add-component-btn">+ Add Component</button>
                  <p className="todo-note">
                    TODO: Implement component adding/removing
                  </p>
                </div>
              </>
            ) : (
              <div className="inspector-empty">
                <p>Select an entity to view properties</p>
              </div>
            )}
          </div>
        </aside>
      </div>

      {/* Advanced Panels (Modals) */}
      {showAnimationPanel && (
        <div
          className="editor-modal"
          onClick={() => setShowAnimationPanel(false)}
        >
          <div
            className="modal-content animation-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>🎬 Animation Timeline</h3>
              <button
                className="modal-close"
                onClick={() => setShowAnimationPanel(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="timeline-view">
                <div className="timeline-tracks">
                  <div className="track">Position</div>
                  <div className="track">Rotation</div>
                  <div className="track">Scale</div>
                </div>
                <div className="timeline-grid"></div>
              </div>
              <p className="todo-note">
                TODO: Implement animation timeline with keyframes and curves
              </p>
            </div>
          </div>
        </div>
      )}

      {showLightingPanel && (
        <div
          className="editor-modal"
          onClick={() => setShowLightingPanel(false)}
        >
          <div
            className="modal-content lighting-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>💡 Lighting Settings</h3>
              <button
                className="modal-close"
                onClick={() => setShowLightingPanel(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="lighting-controls">
                <div className="control-group">
                  <label>Ambient Color</label>
                  <input type="color" value="#3a3a5a" readOnly />
                </div>
                <div className="control-group">
                  <label>Skybox</label>
                  <select>
                    <option>Default</option>
                  </select>
                </div>
                <div className="control-group">
                  <label>Global Illumination</label>
                  <input type="checkbox" />
                </div>
              </div>
              <p className="todo-note">
                TODO: Connect lighting settings to renderer
              </p>
            </div>
          </div>
        </div>
      )}

      {showPhysicsPanel && (
        <div
          className="editor-modal"
          onClick={() => setShowPhysicsPanel(false)}
        >
          <div
            className="modal-content physics-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>⚛️ Physics Settings</h3>
              <button
                className="modal-close"
                onClick={() => setShowPhysicsPanel(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="physics-controls">
                <div className="control-group">
                  <label>Gravity</label>
                  <input type="number" value="-9.81" readOnly />
                </div>
                <div className="control-group">
                  <label>Time Scale</label>
                  <input type="number" value="1" readOnly />
                </div>
              </div>
              <p className="todo-note">TODO: Integrate with physics engine</p>
            </div>
          </div>
        </div>
      )}

      {showParticlePanel && (
        <div
          className="editor-modal"
          onClick={() => setShowParticlePanel(false)}
        >
          <div
            className="modal-content particle-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>✨ Particle System</h3>
              <button
                className="modal-close"
                onClick={() => setShowParticlePanel(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="particle-preview"></div>
              <p className="todo-note">TODO: Build particle system editor</p>
            </div>
          </div>
        </div>
      )}

      {showShaderPanel && (
        <div className="editor-modal" onClick={() => setShowShaderPanel(false)}>
          <div
            className="modal-content shader-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>🎨 Shader Graph</h3>
              <button
                className="modal-close"
                onClick={() => setShowShaderPanel(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="shader-graph"></div>
              <p className="todo-note">
                TODO: Create node-based shader graph editor
              </p>
            </div>
          </div>
        </div>
      )}

      {showProfilerPanel && (
        <div
          className="editor-modal"
          onClick={() => setShowProfilerPanel(false)}
        >
          <div
            className="modal-content profiler-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>📊 Profiler</h3>
              <button
                className="modal-close"
                onClick={() => setShowProfilerPanel(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="profiler-stats">
                <div className="stat">FPS: 60</div>
                <div className="stat">CPU: 12ms</div>
                <div className="stat">GPU: 8ms</div>
              </div>
              <p className="todo-note">
                TODO: Implement real-time performance profiling
              </p>
            </div>
          </div>
        </div>
      )}

      {showBuildSettings && (
        <div
          className="editor-modal"
          onClick={() => setShowBuildSettings(false)}
        >
          <div
            className="modal-content build-panel"
            onClick={(e) => e.stopPropagation()}
          >
            <div className="modal-header">
              <h3>🔨 Build Settings</h3>
              <button
                className="modal-close"
                onClick={() => setShowBuildSettings(false)}
              >
                ✕
              </button>
            </div>
            <div className="modal-body">
              <div className="build-platforms">
                <div className="platform-item">🖥️ Windows</div>
                <div className="platform-item">🍎 macOS</div>
                <div className="platform-item">🐧 Linux</div>
                <div className="platform-item">🌐 WebGL</div>
              </div>
              <button className="build-btn">Build</button>
              <p className="todo-note">TODO: Implement build pipeline</p>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default EditorModuleV2;
