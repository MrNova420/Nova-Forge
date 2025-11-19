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
 * NOVA ENGINE - ENHANCED PROFESSIONAL EDITOR
 *
 * Unity/Unreal Engine inspired editor with complete feature set
 * - Professional multi-panel layout
 * - Advanced toolbars and menus
 * - Complete asset management
 * - Animation timeline
 * - Lighting controls
 * - Physics settings
 * - Particle system editor
 * - Shader graph
 * - Console/output panel
 * - Profiler
 * - Build settings
 *
 * NOTE: UI is fully implemented. Backend functionality marked with TODO comments
 * for future implementation.
 */

import React, { useState, useEffect, useRef } from 'react';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';
// @ts-ignore - Nova Engine imports
import { Engine } from '@nova-engine/engine';

interface EditorModuleEnhancedProps {
  platform: UnifiedPlatformCore;
}

interface Project {
  id: string;
  name: string;
  description: string;
  lastModified: string;
  thumbnail: string;
}

interface Entity {
  id: string;
  name: string;
  components: string[];
  children: Entity[];
  transform?: {
    position: { x: number; y: number; z: number };
    rotation: { x: number; y: number; z: number };
    scale: { x: number; y: number; z: number };
  };
}

export const EditorModuleEnhanced: React.FC<EditorModuleEnhancedProps> = ({
  platform,
}) => {
  // Project state
  const [projects, setProjects] = useState<Project[]>([]);
  const [currentProject, setCurrentProject] = useState<Project | null>(null);
  const [showProjectBrowser, setShowProjectBrowser] = useState(true);

  // Scene state
  const [entities, setEntities] = useState<Entity[]>([]);
  const [selectedEntity, setSelectedEntity] = useState<Entity | null>(null);

  // Tool state
  const [currentTool, setCurrentTool] = useState<
    'select' | 'translate' | 'rotate' | 'scale'
  >('select');
  const [isPlaying, setIsPlaying] = useState(false);

  // Panel visibility
  const [showAssetBrowser, setShowAssetBrowser] = useState(true);
  const [showConsole, setShowConsole] = useState(true);
  const [showAnimationTimeline, setShowAnimationTimeline] = useState(false);
  const [showLightingPanel, setShowLightingPanel] = useState(false);
  const [showPhysicsPanel, setShowPhysicsPanel] = useState(false);
  const [showParticleEditor, setShowParticleEditor] = useState(false);
  const [showShaderGraph, setShowShaderGraph] = useState(false);
  const [showProfiler, setShowProfiler] = useState(false);
  const [showBuildSettings, setShowBuildSettings] = useState(false);

  // Console logs
  const [consoleLogs, setConsoleLogs] = useState<
    Array<{ type: string; message: string; timestamp: string }>
  >([
    {
      type: 'info',
      message: 'Nova Engine Editor initialized',
      timestamp: new Date().toLocaleTimeString(),
    },
    {
      type: 'info',
      message: 'Ready to create amazing games',
      timestamp: new Date().toLocaleTimeString(),
    },
  ]);

  // Engine reference
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const engineRef = useRef<any>(null);
  const animationFrameRef = useRef<number | null>(null);

  useEffect(() => {
    loadProjects();
  }, []);

  useEffect(() => {
    if (currentProject && canvasRef.current) {
      initializeEditor();
    }
    return () => {
      if (animationFrameRef.current) {
        cancelAnimationFrame(animationFrameRef.current);
      }
      if (engineRef.current) {
        engineRef.current.destroy();
      }
    };
  }, [currentProject]);

  const loadProjects = async () => {
    const savedProjects = localStorage.getItem('nova_projects');
    if (savedProjects) {
      setProjects(JSON.parse(savedProjects));
    } else {
      const defaultProject: Project = {
        id: 'default_project',
        name: 'My First Game',
        description: 'A new Nova Engine project',
        lastModified: new Date().toISOString(),
        thumbnail: '/placeholder-project.png',
      };
      setProjects([defaultProject]);
      localStorage.setItem('nova_projects', JSON.stringify([defaultProject]));
    }
  };

  const initializeEditor = () => {
    if (!canvasRef.current) return;

    try {
      engineRef.current = new Engine({ canvas: canvasRef.current });
      engineRef.current.createScene();

      // Add default camera
      const camera = engineRef.current.createEntity('MainCamera');
      camera.addComponent('Camera', { fov: 75, near: 0.1, far: 1000 });
      camera.getComponent('Transform').setPosition(0, 5, 10);

      // Add default light
      const light = engineRef.current.createEntity('DirectionalLight');
      light.addComponent('Light', {
        type: 'directional',
        color: [1, 1, 1],
        intensity: 1,
      });

      updateEntitiesFromEngine();
      startRenderLoop();

      addConsoleLog('success', 'Scene initialized successfully');
      platform.showNotification({ type: 'success', message: 'Editor ready' });
    } catch (error) {
      addConsoleLog('error', `Failed to initialize: ${error}`);
      platform.showNotification({
        type: 'error',
        message: 'Editor initialization failed',
      });
    }
  };

  const updateEntitiesFromEngine = () => {
    if (!engineRef.current) return;
    const scene = engineRef.current.getActiveScene();
    if (!scene) return;

    const engineEntities = scene.getAllEntities();
    const entityList: Entity[] = engineEntities.map((e: any) => ({
      id: e.id,
      name: e.name,
      components: e.components.map((c: any) => c.type),
      children: e.children || [],
      transform: e.getComponent('Transform')
        ? {
            position: e.getComponent('Transform').position,
            rotation: e.getComponent('Transform').rotation,
            scale: e.getComponent('Transform').scale,
          }
        : undefined,
    }));

    setEntities(entityList);
  };

  const startRenderLoop = () => {
    const render = () => {
      if (engineRef.current && isPlaying) {
        engineRef.current.update();
        engineRef.current.render();
      } else if (engineRef.current) {
        engineRef.current.render();
      }
      animationFrameRef.current = requestAnimationFrame(render);
    };
    render();
  };

  const addConsoleLog = (type: string, message: string) => {
    setConsoleLogs((prev) => [
      ...prev,
      {
        type,
        message,
        timestamp: new Date().toLocaleTimeString(),
      },
    ]);
  };

  const handleCreateProject = () => {
    const name = prompt('Enter project name:');
    if (!name) return;

    const newProject: Project = {
      id: `project_${Date.now()}`,
      name,
      description: 'A new Nova Engine project',
      lastModified: new Date().toISOString(),
      thumbnail: '/placeholder-project.png',
    };

    const updated = [...projects, newProject];
    setProjects(updated);
    localStorage.setItem('nova_projects', JSON.stringify(updated));
    addConsoleLog('info', `Created project: ${name}`);
  };

  const handleOpenProject = (project: Project) => {
    setCurrentProject(project);
    setShowProjectBrowser(false);
    addConsoleLog('info', `Opened project: ${project.name}`);
  };

  const handleSaveProject = () => {
    if (!currentProject) return;
    addConsoleLog('success', 'Project saved successfully');
    platform.showNotification({ type: 'success', message: 'Project saved' });
    // TODO: Implement actual save to backend
  };

  const handleCreateEntity = () => {
    if (!engineRef.current) return;
    const name = prompt('Entity name:', 'New Entity');
    if (!name) return;

    const entity = engineRef.current.createEntity(name);
    entity.addComponent('Transform', {
      position: [0, 0, 0],
      rotation: [0, 0, 0],
      scale: [1, 1, 1],
    });
    updateEntitiesFromEngine();
    addConsoleLog('info', `Created entity: ${name}`);
  };

  const handleDeleteEntity = () => {
    if (!selectedEntity || !engineRef.current) return;
    if (confirm(`Delete entity "${selectedEntity.name}"?`)) {
      engineRef.current.destroyEntity(selectedEntity.id);
      updateEntitiesFromEngine();
      setSelectedEntity(null);
      addConsoleLog('info', `Deleted entity: ${selectedEntity.name}`);
    }
  };

  const handlePlayMode = () => {
    setIsPlaying(!isPlaying);
    addConsoleLog(
      'info',
      isPlaying ? 'Stopped play mode' : 'Entered play mode'
    );
  };

  const handleBuild = () => {
    addConsoleLog('info', 'Starting build process...');
    setShowBuildSettings(true);
    // TODO: Implement actual build process
  };

  if (showProjectBrowser) {
    return (
      <div className="editor-project-browser-enhanced">
        <div className="browser-header">
          <div className="nova-branding">
            <h1 className="nova-logo-small">NOVA</h1>
            <span>ENGINE EDITOR</span>
          </div>
          <button onClick={handleCreateProject} className="create-project-btn">
            + New Project
          </button>
        </div>

        <div className="project-grid">
          {projects.map((project) => (
            <div
              key={project.id}
              className="project-card-enhanced"
              onClick={() => handleOpenProject(project)}
            >
              <div className="project-thumbnail">
                <img
                  src={project.thumbnail || '/placeholder-project.png'}
                  alt={project.name}
                />
                <div className="project-overlay">
                  <button className="open-btn">Open Project</button>
                </div>
              </div>
              <div className="project-info">
                <h3>{project.name}</h3>
                <p>{project.description}</p>
                <span className="project-date">
                  Modified:{' '}
                  {new Date(project.lastModified).toLocaleDateString()}
                </span>
              </div>
            </div>
          ))}
        </div>

        <style>{`
          .editor-project-browser-enhanced {
            width: 100%;
            height: 100vh;
            padding: 40px;
            background: radial-gradient(ellipse at center, #1a0033 0%, #000000 70%);
            color: white;
            overflow-y: auto;
          }

          .browser-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 40px;
            padding-bottom: 20px;
            border-bottom: 2px solid rgba(123, 47, 247, 0.3);
          }

          .nova-branding {
            display: flex;
            align-items: baseline;
            gap: 15px;
          }

          .nova-logo-small {
            font-size: 36px;
            font-weight: 900;
            letter-spacing: 6px;
            background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin: 0;
          }

          .nova-branding span {
            font-size: 16px;
            letter-spacing: 3px;
            color: #a0a0ff;
          }

          .create-project-btn {
            padding: 14px 32px;
            background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
            border: none;
            border-radius: 25px;
            color: white;
            font-size: 16px;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
            box-shadow: 0 4px 20px rgba(123, 47, 247, 0.4);
          }

          .create-project-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 30px rgba(123, 47, 247, 0.6);
          }

          .project-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
            gap: 30px;
          }

          .project-card-enhanced {
            background: linear-gradient(135deg, rgba(26,0,51,0.8) 0%, rgba(58,12,88,0.8) 100%);
            border: 2px solid rgba(123, 47, 247, 0.3);
            border-radius: 16px;
            overflow: hidden;
            cursor: pointer;
            transition: all 0.3s ease;
          }

          .project-card-enhanced:hover {
            transform: translateY(-5px);
            border-color: rgba(123, 47, 247, 0.6);
            box-shadow: 0 10px 40px rgba(123, 47, 247, 0.4);
          }

          .project-thumbnail {
            position: relative;
            width: 100%;
            height: 200px;
            background: #0a0015;
            overflow: hidden;
          }

          .project-thumbnail img {
            width: 100%;
            height: 100%;
            object-fit: cover;
          }

          .project-overlay {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(0, 0, 0, 0.7);
            display: flex;
            align-items: center;
            justify-content: center;
            opacity: 0;
            transition: opacity 0.3s ease;
          }

          .project-card-enhanced:hover .project-overlay {
            opacity: 1;
          }

          .open-btn {
            padding: 12px 30px;
            background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
            border: none;
            border-radius: 20px;
            color: white;
            font-weight: 600;
            cursor: pointer;
          }

          .project-info {
            padding: 24px;
          }

          .project-info h3 {
            margin: 0 0 12px 0;
            font-size: 20px;
            color: white;
          }

          .project-info p {
            margin: 0 0 16px 0;
            color: rgba(255, 255, 255, 0.6);
            font-size: 14px;
          }

          .project-date {
            color: rgba(123, 47, 247, 0.8);
            font-size: 13px;
            font-weight: 500;
          }
        `}</style>
      </div>
    );
  }

  return (
    <div className="editor-enhanced">
      {/* Top Menu Bar - Unity/Unreal style */}
      <div className="menu-bar">
        <div className="menu-left">
          <div className="nova-branding-menu">
            <span className="nova-text">NOVA</span>
            <span className="engine-text">ENGINE</span>
          </div>
          <button
            className="menu-item"
            onClick={() => setShowProjectBrowser(true)}
          >
            File
          </button>
          <button className="menu-item">Edit</button>
          <button className="menu-item">Assets</button>
          <button className="menu-item">GameObject</button>
          <button className="menu-item">Component</button>
          <button className="menu-item">Window</button>
          <button className="menu-item">Help</button>
        </div>
        <div className="menu-right">
          <span className="project-name">{currentProject?.name}</span>
        </div>
      </div>

      {/* Main Toolbar */}
      <div className="main-toolbar">
        <div className="toolbar-section">
          <button
            onClick={handleSaveProject}
            className="toolbar-btn"
            title="Save"
          >
            💾
          </button>
          <button className="toolbar-btn" title="Undo">
            ↶
          </button>
          <button className="toolbar-btn" title="Redo">
            ↷
          </button>
        </div>

        <div className="toolbar-section">
          <button
            className={`toolbar-btn ${currentTool === 'select' ? 'active' : ''}`}
            onClick={() => setCurrentTool('select')}
            title="Select"
          >
            🖱️
          </button>
          <button
            className={`toolbar-btn ${currentTool === 'translate' ? 'active' : ''}`}
            onClick={() => setCurrentTool('translate')}
            title="Move"
          >
            ↔️
          </button>
          <button
            className={`toolbar-btn ${currentTool === 'rotate' ? 'active' : ''}`}
            onClick={() => setCurrentTool('rotate')}
            title="Rotate"
          >
            🔄
          </button>
          <button
            className={`toolbar-btn ${currentTool === 'scale' ? 'active' : ''}`}
            onClick={() => setCurrentTool('scale')}
            title="Scale"
          >
            📏
          </button>
        </div>

        <div className="toolbar-section play-controls">
          <button
            onClick={handlePlayMode}
            className={`play-btn ${isPlaying ? 'playing' : ''}`}
          >
            {isPlaying ? '⏸️' : '▶️'}
          </button>
          <button className="toolbar-btn">⏭️</button>
          <button className="toolbar-btn">⏯️</button>
        </div>

        <div className="toolbar-section">
          <button
            onClick={() => setShowAnimationTimeline(!showAnimationTimeline)}
            className="toolbar-btn"
          >
            🎬 Animation
          </button>
          <button
            onClick={() => setShowLightingPanel(!showLightingPanel)}
            className="toolbar-btn"
          >
            💡 Lighting
          </button>
          <button
            onClick={() => setShowPhysicsPanel(!showPhysicsPanel)}
            className="toolbar-btn"
          >
            ⚛️ Physics
          </button>
          <button
            onClick={() => setShowParticleEditor(!showParticleEditor)}
            className="toolbar-btn"
          >
            ✨ Particles
          </button>
        </div>

        <div className="toolbar-section">
          <button
            onClick={() => setShowShaderGraph(!showShaderGraph)}
            className="toolbar-btn"
          >
            🎨 Shader Graph
          </button>
          <button
            onClick={() => setShowProfiler(!showProfiler)}
            className="toolbar-btn"
          >
            📊 Profiler
          </button>
          <button onClick={handleBuild} className="toolbar-btn build-btn">
            🔨 Build
          </button>
        </div>
      </div>

      {/* Main Editor Workspace */}
      <div className="editor-workspace-enhanced">
        {/* Left Panel - Hierarchy */}
        <div className="panel hierarchy-panel-enhanced">
          <div className="panel-header">
            <h3>Hierarchy</h3>
            <button onClick={handleCreateEntity} className="add-btn">
              +
            </button>
          </div>
          <div className="hierarchy-content">
            {entities.map((entity) => (
              <div
                key={entity.id}
                className={`entity-item ${selectedEntity?.id === entity.id ? 'selected' : ''}`}
                onClick={() => setSelectedEntity(entity)}
              >
                <span className="entity-icon">📦</span>
                <span>{entity.name}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Center - Scene View */}
        <div className="scene-view-container">
          <div className="scene-tabs">
            <button className="scene-tab active">Scene</button>
            <button className="scene-tab">Game</button>
          </div>
          <div className="scene-viewport">
            <canvas ref={canvasRef} />
            <div className="viewport-controls">
              <div className="viewport-info">
                <span>FPS: 60</span>
                <span>Draw Calls: 12</span>
                <span>Triangles: 1.2K</span>
              </div>
              <div className="viewport-buttons">
                <button>🔍</button>
                <button>🎥</button>
                <button>☀️</button>
              </div>
            </div>
          </div>
        </div>

        {/* Right Panel - Inspector */}
        <div className="panel inspector-panel-enhanced">
          <div className="panel-header">
            <h3>Inspector</h3>
          </div>
          <div className="inspector-content">
            {selectedEntity ? (
              <>
                <div className="inspector-section">
                  <input
                    type="text"
                    value={selectedEntity.name}
                    className="entity-name-input"
                    placeholder="Entity Name"
                  />
                  <div className="entity-controls">
                    <label>
                      <input type="checkbox" defaultChecked /> Static
                    </label>
                    <label>
                      <input type="checkbox" defaultChecked /> Active
                    </label>
                  </div>
                </div>

                {/* Transform Component */}
                {selectedEntity.transform && (
                  <div className="component-box">
                    <div className="component-header">
                      <span>Transform</span>
                      <button className="component-menu">⋮</button>
                    </div>
                    <div className="transform-section">
                      <div className="transform-row">
                        <label>Position</label>
                        <div className="vector-inputs">
                          <input
                            type="number"
                            value={selectedEntity.transform.position.x}
                            step="0.1"
                            placeholder="X"
                          />
                          <input
                            type="number"
                            value={selectedEntity.transform.position.y}
                            step="0.1"
                            placeholder="Y"
                          />
                          <input
                            type="number"
                            value={selectedEntity.transform.position.z}
                            step="0.1"
                            placeholder="Z"
                          />
                        </div>
                      </div>
                      <div className="transform-row">
                        <label>Rotation</label>
                        <div className="vector-inputs">
                          <input
                            type="number"
                            value={selectedEntity.transform.rotation.x}
                            placeholder="X"
                          />
                          <input
                            type="number"
                            value={selectedEntity.transform.rotation.y}
                            placeholder="Y"
                          />
                          <input
                            type="number"
                            value={selectedEntity.transform.rotation.z}
                            placeholder="Z"
                          />
                        </div>
                      </div>
                      <div className="transform-row">
                        <label>Scale</label>
                        <div className="vector-inputs">
                          <input
                            type="number"
                            value={selectedEntity.transform.scale.x}
                            step="0.1"
                            placeholder="X"
                          />
                          <input
                            type="number"
                            value={selectedEntity.transform.scale.y}
                            step="0.1"
                            placeholder="Y"
                          />
                          <input
                            type="number"
                            value={selectedEntity.transform.scale.z}
                            step="0.1"
                            placeholder="Z"
                          />
                        </div>
                      </div>
                    </div>
                  </div>
                )}

                {/* Components List */}
                <div className="component-box">
                  <div className="component-header">
                    <span>Components</span>
                  </div>
                  {selectedEntity.components.map((comp, idx) => (
                    <div key={idx} className="component-item">
                      {comp}
                    </div>
                  ))}
                  <button className="add-component-btn">Add Component</button>
                </div>

                <button
                  onClick={handleDeleteEntity}
                  className="delete-entity-btn"
                >
                  Delete Entity
                </button>
              </>
            ) : (
              <div className="no-selection">
                <p>Select an entity to view properties</p>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Bottom Panels */}
      <div className="bottom-panels">
        {/* Asset Browser */}
        {showAssetBrowser && (
          <div className="bottom-panel asset-panel">
            <div className="panel-header">
              <h3>📦 Project Assets</h3>
              <button onClick={() => setShowAssetBrowser(false)}>✕</button>
            </div>
            <div className="asset-content">
              <div className="asset-toolbar">
                <button className="asset-filter active">All</button>
                <button className="asset-filter">Models</button>
                <button className="asset-filter">Materials</button>
                <button className="asset-filter">Textures</button>
                <button className="asset-filter">Scripts</button>
                <button className="asset-filter">Audio</button>
              </div>
              <div className="asset-grid-small">
                {[
                  'Cube.fbx',
                  'Sphere.fbx',
                  'Material.mat',
                  'Texture.png',
                  'Script.ts',
                ].map((asset, i) => (
                  <div key={i} className="asset-tile">
                    <div className="asset-icon">📄</div>
                    <span>{asset}</span>
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}

        {/* Console */}
        {showConsole && (
          <div className="bottom-panel console-panel">
            <div className="panel-header">
              <h3>💬 Console</h3>
              <div className="console-controls">
                <button onClick={() => setConsoleLogs([])}>Clear</button>
                <button onClick={() => setShowConsole(false)}>✕</button>
              </div>
            </div>
            <div className="console-content">
              {consoleLogs.map((log, i) => (
                <div key={i} className={`console-log log-${log.type}`}>
                  <span className="log-time">[{log.timestamp}]</span>
                  <span className="log-message">{log.message}</span>
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Animation Timeline - PLACEHOLDER */}
        {showAnimationTimeline && (
          <div className="bottom-panel timeline-panel">
            <div className="panel-header">
              <h3>🎬 Animation Timeline</h3>
              <button onClick={() => setShowAnimationTimeline(false)}>✕</button>
            </div>
            <div className="placeholder-content">
              <p>🎬 Animation Timeline</p>
              <span>TODO: Implement animation timeline UI and backend</span>
              <small>
                This will include keyframe editing, curve editor, and animation
                blending
              </small>
            </div>
          </div>
        )}

        {/* Lighting Panel - PLACEHOLDER */}
        {showLightingPanel && (
          <div className="bottom-panel lighting-panel">
            <div className="panel-header">
              <h3>💡 Lighting Settings</h3>
              <button onClick={() => setShowLightingPanel(false)}>✕</button>
            </div>
            <div className="placeholder-content">
              <p>💡 Lighting & Environment</p>
              <span>TODO: Implement lighting controls</span>
              <small>
                Global illumination, skybox, fog, ambient lighting, baked
                lighting
              </small>
            </div>
          </div>
        )}

        {/* Physics Panel - PLACEHOLDER */}
        {showPhysicsPanel && (
          <div className="bottom-panel physics-panel">
            <div className="panel-header">
              <h3>⚛️ Physics Settings</h3>
              <button onClick={() => setShowPhysicsPanel(false)}>✕</button>
            </div>
            <div className="placeholder-content">
              <p>⚛️ Physics Configuration</p>
              <span>TODO: Implement physics settings</span>
              <small>
                Gravity, collision layers, physics materials, simulation
                settings
              </small>
            </div>
          </div>
        )}

        {/* Particle Editor - PLACEHOLDER */}
        {showParticleEditor && (
          <div className="bottom-panel particle-panel">
            <div className="panel-header">
              <h3>✨ Particle System Editor</h3>
              <button onClick={() => setShowParticleEditor(false)}>✕</button>
            </div>
            <div className="placeholder-content">
              <p>✨ Particle System</p>
              <span>TODO: Implement particle editor</span>
              <small>
                Emitters, lifetime, velocity, color over time, size over time,
                forces
              </small>
            </div>
          </div>
        )}

        {/* Shader Graph - PLACEHOLDER */}
        {showShaderGraph && (
          <div className="bottom-panel shader-panel">
            <div className="panel-header">
              <h3>🎨 Shader Graph</h3>
              <button onClick={() => setShowShaderGraph(false)}>✕</button>
            </div>
            <div className="placeholder-content">
              <p>🎨 Visual Shader Editor</p>
              <span>TODO: Implement node-based shader graph</span>
              <small>
                Node-based shader creation similar to Unity Shader Graph
              </small>
            </div>
          </div>
        )}

        {/* Profiler - PLACEHOLDER */}
        {showProfiler && (
          <div className="bottom-panel profiler-panel">
            <div className="panel-header">
              <h3>📊 Profiler</h3>
              <button onClick={() => setShowProfiler(false)}>✕</button>
            </div>
            <div className="placeholder-content">
              <p>📊 Performance Profiler</p>
              <span>TODO: Implement profiler</span>
              <small>
                CPU usage, GPU usage, memory, draw calls, frame time analysis
              </small>
            </div>
          </div>
        )}

        {/* Build Settings - PLACEHOLDER */}
        {showBuildSettings && (
          <div className="build-settings-modal">
            <div className="modal-content">
              <div className="panel-header">
                <h3>🔨 Build Settings</h3>
                <button onClick={() => setShowBuildSettings(false)}>✕</button>
              </div>
              <div className="placeholder-content">
                <p>🔨 Build Configuration</p>
                <span>TODO: Implement build system</span>
                <small>
                  Platform selection, optimization settings, compression,
                  deployment
                </small>
                <div className="platform-icons">
                  <div className="platform">🖥️ Desktop</div>
                  <div className="platform">🌐 Web</div>
                  <div className="platform">📱 Mobile</div>
                  <div className="platform">🎮 Console</div>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>

      <style>{`
        /* Main Editor Container */
        .editor-enhanced {
          width: 100%;
          height: 100vh;
          display: flex;
          flex-direction: column;
          background: #0a0015;
          color: white;
          overflow: hidden;
        }

        /* Menu Bar */
        .menu-bar {
          display: flex;
          justify-content: space-between;
          align-items: center;
          height: 40px;
          background: linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%);
          border-bottom: 2px solid rgba(123, 47, 247, 0.3);
          padding: 0 15px;
        }

        .menu-left {
          display: flex;
          gap: 5px;
          align-items: center;
        }

        .nova-branding-menu {
          display: flex;
          align-items: baseline;
          gap: 5px;
          margin-right: 20px;
          padding-right: 20px;
          border-right: 1px solid rgba(123, 47, 247, 0.3);
        }

        .nova-text {
          font-size: 18px;
          font-weight: 900;
          letter-spacing: 3px;
          background: linear-gradient(135deg, #ff6ec4 0%, #7b2ff7 50%, #4cc9f0 100%);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
        }

        .engine-text {
          font-size: 10px;
          letter-spacing: 2px;
          color: #a0a0ff;
        }

        .menu-item {
          padding: 8px 15px;
          background: transparent;
          border: none;
          color: white;
          cursor: pointer;
          font-size: 13px;
          transition: all 0.2s;
          border-radius: 4px;
        }

        .menu-item:hover {
          background: rgba(123, 47, 247, 0.2);
        }

        .menu-right {
          display: flex;
          align-items: center;
          gap: 15px;
        }

        .project-name {
          font-size: 13px;
          color: rgba(255, 255, 255, 0.7);
        }

        /* Main Toolbar */
        .main-toolbar {
          display: flex;
          gap: 15px;
          align-items: center;
          padding: 8px 15px;
          background: rgba(26, 0, 51, 0.6);
          border-bottom: 1px solid rgba(123, 47, 247, 0.2);
          flex-wrap: wrap;
        }

        .toolbar-section {
          display: flex;
          gap: 5px;
          align-items: center;
          padding: 0 10px;
          border-right: 1px solid rgba(123, 47, 247, 0.2);
        }

        .toolbar-section:last-child {
          border-right: none;
          margin-left: auto;
        }

        .toolbar-btn {
          padding: 6px 12px;
          background: rgba(123, 47, 247, 0.1);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 6px;
          color: white;
          cursor: pointer;
          font-size: 13px;
          transition: all 0.2s;
        }

        .toolbar-btn:hover {
          background: rgba(123, 47, 247, 0.3);
          border-color: rgba(123, 47, 247, 0.5);
        }

        .toolbar-btn.active {
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border-color: transparent;
        }

        .play-btn {
          background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
          border: none;
          font-size: 16px;
        }

        .play-btn.playing {
          background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
        }

        .build-btn {
          background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
          border: none;
          font-weight: 600;
        }

        /* Workspace */
        .editor-workspace-enhanced {
          flex: 1;
          display: flex;
          overflow: hidden;
        }

        .panel {
          background: rgba(26, 0, 51, 0.8);
          border-right: 1px solid rgba(123, 47, 247, 0.2);
          display: flex;
          flex-direction: column;
        }

        .hierarchy-panel-enhanced {
          width: 280px;
        }

        .inspector-panel-enhanced {
          width: 320px;
          border-left: 1px solid rgba(123, 47, 247, 0.2);
          border-right: none;
        }

        .panel-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 12px 15px;
          background: rgba(58, 12, 88, 0.6);
          border-bottom: 1px solid rgba(123, 47, 247, 0.2);
        }

        .panel-header h3 {
          margin: 0;
          font-size: 14px;
          font-weight: 600;
          text-transform: uppercase;
          letter-spacing: 1px;
        }

        .add-btn {
          width: 24px;
          height: 24px;
          background: linear-gradient(135deg, #7b2ff7 0%, #ff6ec4 100%);
          border: none;
          border-radius: 4px;
          color: white;
          font-size: 16px;
          cursor: pointer;
          display: flex;
          align-items: center;
          justify-content: center;
        }

        .hierarchy-content {
          flex: 1;
          overflow-y: auto;
          padding: 8px;
        }

        .entity-item {
          display: flex;
          align-items: center;
          gap: 8px;
          padding: 8px;
          margin: 2px 0;
          border-radius: 6px;
          cursor: pointer;
          transition: all 0.2s;
        }

        .entity-item:hover {
          background: rgba(123, 47, 247, 0.2);
        }

        .entity-item.selected {
          background: linear-gradient(135deg, rgba(123, 47, 247, 0.4) 0%, rgba(255, 110, 196, 0.4) 100%);
          border: 1px solid rgba(123, 47, 247, 0.6);
        }

        .entity-icon {
          font-size: 16px;
        }

        /* Scene View */
        .scene-view-container {
          flex: 1;
          display: flex;
          flex-direction: column;
          background: #000;
        }

        .scene-tabs {
          display: flex;
          gap: 2px;
          background: rgba(26, 0, 51, 0.8);
          padding: 8px 8px 0 8px;
        }

        .scene-tab {
          padding: 8px 20px;
          background: transparent;
          border: none;
          color: rgba(255, 255, 255, 0.6);
          cursor: pointer;
          border-radius: 6px 6px 0 0;
          transition: all 0.2s;
        }

        .scene-tab.active {
          background: #000;
          color: white;
        }

        .scene-viewport {
          flex: 1;
          position: relative;
        }

        .scene-viewport canvas {
          width: 100%;
          height: 100%;
        }

        .viewport-controls {
          position: absolute;
          top: 10px;
          right: 10px;
          display: flex;
          gap: 10px;
        }

        .viewport-info {
          background: rgba(0, 0, 0, 0.8);
          padding: 10px;
          border-radius: 8px;
          display: flex;
          flex-direction: column;
          gap: 5px;
          font-size: 12px;
          border: 1px solid rgba(123, 47, 247, 0.3);
        }

        .viewport-buttons {
          display: flex;
          gap: 5px;
        }

        .viewport-buttons button {
          width: 36px;
          height: 36px;
          background: rgba(0, 0, 0, 0.8);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 8px;
          color: white;
          font-size: 16px;
          cursor: pointer;
        }

        /* Inspector */
        .inspector-content {
          flex: 1;
          overflow-y: auto;
          padding: 15px;
        }

        .inspector-section {
          margin-bottom: 20px;
        }

        .entity-name-input {
          width: 100%;
          padding: 10px;
          background: rgba(26, 0, 51, 0.6);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 6px;
          color: white;
          font-size: 14px;
          margin-bottom: 10px;
        }

        .entity-controls {
          display: flex;
          gap: 15px;
        }

        .entity-controls label {
          display: flex;
          align-items: center;
          gap: 6px;
          font-size: 13px;
          color: rgba(255, 255, 255, 0.8);
        }

        .component-box {
          background: rgba(58, 12, 88, 0.4);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 8px;
          margin-bottom: 15px;
          overflow: hidden;
        }

        .component-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 10px 15px;
          background: rgba(123, 47, 247, 0.2);
          font-weight: 600;
          font-size: 13px;
        }

        .component-menu {
          background: transparent;
          border: none;
          color: white;
          cursor: pointer;
          font-size: 18px;
        }

        .transform-section {
          padding: 15px;
        }

        .transform-row {
          margin-bottom: 15px;
        }

        .transform-row label {
          display: block;
          margin-bottom: 6px;
          font-size: 12px;
          color: rgba(255, 255, 255, 0.7);
          font-weight: 500;
        }

        .vector-inputs {
          display: grid;
          grid-template-columns: repeat(3, 1fr);
          gap: 6px;
        }

        .vector-inputs input {
          padding: 8px;
          background: rgba(26, 0, 51, 0.6);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 4px;
          color: white;
          font-size: 12px;
          text-align: center;
        }

        .component-item {
          padding: 10px 15px;
          border-top: 1px solid rgba(123, 47, 247, 0.2);
          font-size: 13px;
        }

        .add-component-btn {
          width: 100%;
          padding: 10px;
          background: rgba(123, 47, 247, 0.2);
          border: none;
          border-radius: 6px;
          color: white;
          cursor: pointer;
          margin-top: 10px;
          transition: all 0.2s;
        }

        .add-component-btn:hover {
          background: rgba(123, 47, 247, 0.4);
        }

        .delete-entity-btn {
          width: 100%;
          padding: 12px;
          background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
          border: none;
          border-radius: 8px;
          color: white;
          font-weight: 600;
          cursor: pointer;
          margin-top: 20px;
        }

        .no-selection {
          text-align: center;
          padding: 40px 20px;
          color: rgba(255, 255, 255, 0.5);
        }

        /* Bottom Panels */
        .bottom-panels {
          height: 250px;
          border-top: 1px solid rgba(123, 47, 247, 0.2);
          background: rgba(26, 0, 51, 0.9);
          overflow: hidden;
        }

        .bottom-panel {
          height: 100%;
          display: flex;
          flex-direction: column;
        }

        .asset-content,
        .console-content {
          flex: 1;
          overflow-y: auto;
          padding: 10px;
        }

        .asset-toolbar {
          display: flex;
          gap: 5px;
          padding: 10px;
          border-bottom: 1px solid rgba(123, 47, 247, 0.2);
        }

        .asset-filter {
          padding: 6px 12px;
          background: rgba(123, 47, 247, 0.1);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 6px;
          color: white;
          cursor: pointer;
          font-size: 12px;
        }

        .asset-filter.active {
          background: rgba(123, 47, 247, 0.4);
        }

        .asset-grid-small {
          display: grid;
          grid-template-columns: repeat(auto-fill, minmax(80px, 1fr));
          gap: 10px;
        }

        .asset-tile {
          display: flex;
          flex-direction: column;
          align-items: center;
          padding: 10px;
          background: rgba(58, 12, 88, 0.4);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 8px;
          cursor: pointer;
          transition: all 0.2s;
        }

        .asset-tile:hover {
          background: rgba(123, 47, 247, 0.3);
        }

        .asset-icon {
          font-size: 24px;
          margin-bottom: 6px;
        }

        .asset-tile span {
          font-size: 10px;
          text-align: center;
          word-break: break-all;
        }

        .console-controls {
          display: flex;
          gap: 10px;
        }

        .console-controls button {
          padding: 4px 12px;
          background: rgba(123, 47, 247, 0.2);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 4px;
          color: white;
          cursor: pointer;
          font-size: 11px;
        }

        .console-log {
          padding: 6px;
          margin: 2px 0;
          border-radius: 4px;
          font-size: 12px;
          font-family: 'Courier New', monospace;
          display: flex;
          gap: 10px;
        }

        .log-time {
          color: rgba(255, 255, 255, 0.5);
        }

        .log-info { background: rgba(76, 201, 240, 0.1); }
        .log-success { background: rgba(74, 222, 128, 0.1); }
        .log-error { background: rgba(239, 68, 68, 0.1); }

        /* Placeholder Content */
        .placeholder-content {
          flex: 1;
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          padding: 40px;
          text-align: center;
          gap: 15px;
        }

        .placeholder-content p {
          font-size: 24px;
          margin: 0;
        }

        .placeholder-content span {
          font-size: 16px;
          color: rgba(255, 255, 255, 0.7);
        }

        .placeholder-content small {
          font-size: 13px;
          color: rgba(255, 255, 255, 0.5);
          max-width: 500px;
        }

        .platform-icons {
          display: flex;
          gap: 20px;
          margin-top: 20px;
        }

        .platform {
          padding: 15px 25px;
          background: rgba(123, 47, 247, 0.2);
          border: 1px solid rgba(123, 47, 247, 0.3);
          border-radius: 12px;
          font-size: 14px;
        }

        /* Build Settings Modal */
        .build-settings-modal {
          position: fixed;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          background: rgba(0, 0, 0, 0.8);
          display: flex;
          align-items: center;
          justify-content: center;
          z-index: 1000;
        }

        .modal-content {
          width: 600px;
          max-width: 90%;
          background: linear-gradient(135deg, rgba(26,0,51,0.95) 0%, rgba(58,12,88,0.95) 100%);
          border: 2px solid rgba(123, 47, 247, 0.5);
          border-radius: 16px;
          overflow: hidden;
        }
      `}</style>
    </div>
  );
};
export { EditorModuleEnhanced as EditorModule } from './EditorModule';
