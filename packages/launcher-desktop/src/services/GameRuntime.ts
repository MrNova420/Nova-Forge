/**
 * GameRuntime - Manages game execution in the desktop launcher
 *
 * Features:
 * - Load and execute games using Nova Engine runtime
 * - Manage game lifecycle (start, pause, resume, stop)
 * - Handle game input and rendering
 * - Performance monitoring and optimization
 * - Native rendering integration
 */

import { localGameStorage, LocalGame } from './LocalGameStorage';
import { localSaveData } from './LocalSaveData';

export interface GameInstance {
  id: string;
  gameId: string;
  status: 'loading' | 'running' | 'paused' | 'stopped' | 'error';
  startTime: Date;
  playtime: number; // seconds
  fps: number;
  memory: number; // MB
  error?: string;
}

export interface GameConfig {
  width: number;
  height: number;
  fullscreen: boolean;
  vsync: boolean;
  quality: 'low' | 'medium' | 'high' | 'ultra';
  volume: number; // 0-1
}

class GameRuntimeService {
  private activeGames: Map<string, GameInstance> = new Map();
  private gameWindows: Map<string, any> = new Map(); // Would be Tauri Window
  private defaultConfig: GameConfig = {
    width: 1280,
    height: 720,
    fullscreen: false,
    vsync: true,
    quality: 'high',
    volume: 1.0,
  };

  /**
   * Launch a game
   */
  async launchGame(
    gameId: string,
    config?: Partial<GameConfig>
  ): Promise<GameInstance> {
    try {
      const game = await localGameStorage.getGame(gameId);
      if (!game) {
        throw new Error(`Game ${gameId} not found`);
      }

      // Check if game is already running
      const existing = Array.from(this.activeGames.values()).find(
        (instance) => instance.gameId === gameId
      );
      if (existing) {
        throw new Error(`Game ${gameId} is already running`);
      }

      // Create game instance
      const instanceId = `${gameId}-${Date.now()}`;
      const instance: GameInstance = {
        id: instanceId,
        gameId,
        status: 'loading',
        startTime: new Date(),
        playtime: 0,
        fps: 0,
        memory: 0,
      };

      this.activeGames.set(instanceId, instance);

      // Merge config with defaults
      const finalConfig = { ...this.defaultConfig, ...config };

      // Load game
      await this.loadGameRuntime(game, finalConfig);

      // Start game
      await this.startGameRuntime(instanceId, game, finalConfig);

      // Update instance status
      instance.status = 'running';

      // Update last played timestamp
      await localGameStorage.updateLastPlayed(gameId);

      // Start performance monitoring
      this.startPerformanceMonitoring(instanceId);

      return instance;
    } catch (error) {
      console.error(`Failed to launch game ${gameId}:`, error);

      // Find instance and update with error
      const instance = Array.from(this.activeGames.values()).find(
        (inst) => inst.gameId === gameId
      );
      if (instance) {
        instance.status = 'error';
        instance.error =
          error instanceof Error ? error.message : 'Unknown error';
      }

      throw error;
    }
  }

  /**
   * Load game runtime files
   */
  private async loadGameRuntime(
    game: LocalGame,
    config: GameConfig
  ): Promise<void> {
    // eslint-disable-next-line no-console
    console.log(`Loading game runtime for ${game.name}...`);

    // 1. Load the Nova Engine runtime
    const engineScripts = [
      `${game.installPath}/engine.js`,
      `${game.installPath}/game.js`,
    ];

    for (const script of engineScripts) {
      try {
        // Dynamically load script
        await this.loadScript(script);
      } catch (error) {
        console.warn(`Failed to load script ${script}:`, error);
      }
    }

    // 2. Load game assets manifest
    try {
      const manifestPath = `${game.installPath}/assets/manifest.json`;
      const manifestResponse = await fetch(manifestPath);
      const manifest = await manifestResponse.json();
      // eslint-disable-next-line no-console
      console.log(
        `Loaded ${manifest.assets?.length || 0} assets from manifest`
      );
    } catch (error) {
      console.warn('No asset manifest found, will load assets on-demand');
    }

    // 3. Initialize graphics context
    const canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
    if (canvas) {
      canvas.width = config.width;
      canvas.height = config.height;

      // Set up WebGL context
      const gl = canvas.getContext('webgl2') || canvas.getContext('webgl');
      if (gl) {
        gl.viewport(0, 0, canvas.width, canvas.height);
        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT);
        // eslint-disable-next-line no-console
        console.log('Graphics context initialized');
      }
    }

    // 4. Set up audio system
    try {
      const AudioContext =
        window.AudioContext || (window as any).webkitAudioContext;
      if (AudioContext) {
        const audioContext = new AudioContext();
        audioContext.resume(); // Resume if suspended
        // eslint-disable-next-line no-console
        console.log('Audio system initialized');
      }
    } catch (error) {
      console.warn('Audio system initialization failed:', error);
    }

    // 5. Configure input handling
    this.setupInputHandlers(canvas);

    // eslint-disable-next-line no-console
    console.log(`Game runtime loaded for ${game.name}`);
  }

  /**
   * Load a script dynamically
   */
  private loadScript(src: string): Promise<void> {
    return new Promise((resolve, reject) => {
      const script = document.createElement('script');
      script.src = src;
      script.onload = () => resolve();
      script.onerror = () => reject(new Error(`Failed to load script: ${src}`));
      document.head.appendChild(script);
    });
  }

  /**
   * Setup input handlers for the game
   */
  private setupInputHandlers(canvas: HTMLCanvasElement | null): void {
    if (!canvas) return;

    // Keyboard input
    window.addEventListener('keydown', (e) => {
      // Forward to game engine
      (window as any).NovaEngine?.input?.handleKeyDown(e.code);
    });

    window.addEventListener('keyup', (e) => {
      (window as any).NovaEngine?.input?.handleKeyUp(e.code);
    });

    // Mouse input
    canvas.addEventListener('mousemove', (e) => {
      const rect = canvas.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const y = e.clientY - rect.top;
      (window as any).NovaEngine?.input?.handleMouseMove(x, y);
    });

    canvas.addEventListener('mousedown', (e) => {
      (window as any).NovaEngine?.input?.handleMouseDown(e.button);
    });

    canvas.addEventListener('mouseup', (e) => {
      (window as any).NovaEngine?.input?.handleMouseUp(e.button);
    });

    // Touch input (for touch-enabled desktops)
    canvas.addEventListener('touchstart', (e) => {
      e.preventDefault();
      const touch = e.touches[0];
      const rect = canvas.getBoundingClientRect();
      const x = touch.clientX - rect.left;
      const y = touch.clientY - rect.top;
      (window as any).NovaEngine?.input?.handleTouchStart(x, y);
    });

    canvas.addEventListener('touchmove', (e) => {
      e.preventDefault();
      const touch = e.touches[0];
      const rect = canvas.getBoundingClientRect();
      const x = touch.clientX - rect.left;
      const y = touch.clientY - rect.top;
      (window as any).NovaEngine?.input?.handleTouchMove(x, y);
    });

    canvas.addEventListener('touchend', (e) => {
      e.preventDefault();
      (window as any).NovaEngine?.input?.handleTouchEnd();
    });

    // eslint-disable-next-line no-console
    console.log('Input handlers configured');
  }

  /**
   * Start game runtime
   */
  private async startGameRuntime(
    instanceId: string,
    game: LocalGame,
    config: GameConfig
  ): Promise<void> {
    // eslint-disable-next-line no-console
    console.log(`Starting game runtime for ${game.name}...`);

    // 1. Create or get game canvas
    let canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
    if (!canvas) {
      canvas = document.createElement('canvas');
      canvas.id = 'game-canvas';
      canvas.width = config.width;
      canvas.height = config.height;
      document.body.appendChild(canvas);
    }

    // 2. Initialize the Nova Engine runtime
    try {
      // Call the global NovaEngine initialization if available
      if ((window as any).NovaEngine) {
        await (window as any).NovaEngine.init({
          canvas: canvas,
          width: config.width,
          height: config.height,
          quality: config.quality,
          vsync: config.vsync,
        });
        // eslint-disable-next-line no-console
        console.log('Nova Engine runtime initialized');
      }
    } catch (error) {
      console.error('Failed to initialize Nova Engine:', error);
      throw error;
    }

    // 3. Load the game scene
    try {
      const scenePath = `${game.installPath}/scenes/main.json`;
      const sceneResponse = await fetch(scenePath);
      const sceneData = await sceneResponse.json();

      if ((window as any).NovaEngine) {
        await (window as any).NovaEngine.loadScene(sceneData);
        // eslint-disable-next-line no-console
        console.log('Game scene loaded');
      }
    } catch (error) {
      console.warn('Failed to load game scene:', error);
      // Continue anyway - game might handle its own scene loading
    }

    // 4. Start the game loop
    if ((window as any).NovaEngine) {
      (window as any).NovaEngine.start();
      // eslint-disable-next-line no-console
      console.log('Game loop started');
    }

    // 5. Store window reference for management
    this.gameWindows.set(instanceId, {
      title: game.name,
      width: config.width,
      height: config.height,
      fullscreen: config.fullscreen,
      canvas: canvas,
      engineInstance: (window as any).NovaEngine,
    });

    // eslint-disable-next-line no-console
    console.log(`Game ${game.name} is now running`);
  }

  /**
   * Pause a running game
   */
  async pauseGame(instanceId: string): Promise<void> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) {
      throw new Error(`Game instance ${instanceId} not found`);
    }

    if (instance.status !== 'running') {
      throw new Error(`Game is not running (status: ${instance.status})`);
    }

    // Pause the Nova Engine runtime
    const window = this.gameWindows.get(instanceId);
    if (window && window.engineInstance) {
      window.engineInstance.pause();
      // eslint-disable-next-line no-console
      console.log('Engine paused');
    }

    instance.status = 'paused';
    // eslint-disable-next-line no-console
    console.log(`Game ${instance.gameId} paused`);
  }

  /**
   * Resume a paused game
   */
  async resumeGame(instanceId: string): Promise<void> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) {
      throw new Error(`Game instance ${instanceId} not found`);
    }

    if (instance.status !== 'paused') {
      throw new Error(`Game is not paused (status: ${instance.status})`);
    }

    // Resume the Nova Engine runtime
    const window = this.gameWindows.get(instanceId);
    if (window && window.engineInstance) {
      window.engineInstance.resume();
      // eslint-disable-next-line no-console
      console.log('Engine resumed');
    }

    instance.status = 'running';
    // eslint-disable-next-line no-console
    console.log(`Game ${instance.gameId} resumed`);
  }

  /**
   * Stop a running game
   */
  async stopGame(instanceId: string): Promise<void> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) {
      throw new Error(`Game instance ${instanceId} not found`);
    }

    try {
      // Auto-save before stopping
      if (instance.status === 'running' || instance.status === 'paused') {
        await this.autoSaveGame(instanceId);
      }

      // Stop game loop and cleanup
      // In real implementation: stop Nova Engine runtime, cleanup resources
      instance.status = 'stopped';

      // Close game window
      const window = this.gameWindows.get(instanceId);
      if (window) {
        // In real implementation: close Tauri window
        this.gameWindows.delete(instanceId);
      }

      // Remove from active games
      this.activeGames.delete(instanceId);

      // eslint-disable-next-line no-console
      console.log(`Game ${instance.gameId} stopped`);
    } catch (error) {
      console.error(`Error stopping game ${instanceId}:`, error);
      throw error;
    }
  }

  /**
   * Auto-save game state
   */
  private async autoSaveGame(instanceId: string): Promise<void> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) return;

    try {
      // In real implementation: get game state from runtime
      const gameState = {
        timestamp: new Date().toISOString(),
        playtime: instance.playtime,
      };

      await localSaveData.autoSave(
        instance.gameId,
        gameState,
        instance.playtime,
        0 // progress would come from game
      );

      // eslint-disable-next-line no-console
      console.log(`Auto-saved game ${instance.gameId}`);
    } catch (error) {
      console.error(`Failed to auto-save game ${instanceId}:`, error);
    }
  }

  /**
   * Start performance monitoring
   */
  private startPerformanceMonitoring(instanceId: string): void {
    const updateInterval = setInterval(() => {
      const instance = this.activeGames.get(instanceId);
      if (!instance || instance.status === 'stopped') {
        clearInterval(updateInterval);
        return;
      }

      if (instance.status === 'running') {
        // Update playtime
        instance.playtime += 1;

        // In real implementation: get actual FPS and memory from runtime
        instance.fps = 60; // Placeholder
        instance.memory = 150; // Placeholder MB
      }
    }, 1000);
  }

  /**
   * Get active game instances
   */
  getActiveGames(): GameInstance[] {
    return Array.from(this.activeGames.values());
  }

  /**
   * Get specific game instance
   */
  getGameInstance(instanceId: string): GameInstance | null {
    return this.activeGames.get(instanceId) || null;
  }

  /**
   * Check if a game is running
   */
  isGameRunning(gameId: string): boolean {
    return Array.from(this.activeGames.values()).some(
      (instance) =>
        instance.gameId === gameId &&
        (instance.status === 'running' || instance.status === 'paused')
    );
  }

  /**
   * Get total playtime across all sessions
   */
  async getTotalPlaytime(_gameId: string): Promise<number> {
    // In real implementation: aggregate from save data
    return 0;
  }

  /**
   * Update game configuration
   */
  async updateGameConfig(
    instanceId: string,
    config: Partial<GameConfig>
  ): Promise<void> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) {
      throw new Error(`Game instance ${instanceId} not found`);
    }

    // Apply config changes
    // In real implementation: update Nova Engine runtime settings
    // eslint-disable-next-line no-console
    console.log(`Updated config for game ${instanceId}:`, config);
  }

  /**
   * Take screenshot
   */
  async takeScreenshot(instanceId: string): Promise<string> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) {
      throw new Error(`Game instance ${instanceId} not found`);
    }

    // In real implementation: capture frame from renderer
    // Return base64 image or file path
    return 'data:image/png;base64,placeholder';
  }

  /**
   * Toggle fullscreen
   */
  async toggleFullscreen(instanceId: string): Promise<void> {
    const instance = this.activeGames.get(instanceId);
    if (!instance) {
      throw new Error(`Game instance ${instanceId} not found`);
    }

    const window = this.gameWindows.get(instanceId);
    if (window) {
      window.fullscreen = !window.fullscreen;
      // In real implementation: update Tauri window
      // eslint-disable-next-line no-console
      console.log(`Toggled fullscreen for game ${instanceId}`);
    }
  }

  /**
   * Cleanup all games
   */
  async cleanup(): Promise<void> {
    const instances = Array.from(this.activeGames.keys());
    for (const instanceId of instances) {
      try {
        await this.stopGame(instanceId);
      } catch (error) {
        console.error(`Failed to stop game ${instanceId}:`, error);
      }
    }
  }
}

export const gameRuntime = new GameRuntimeService();
