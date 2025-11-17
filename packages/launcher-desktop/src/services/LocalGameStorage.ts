/**
 * LocalGameStorage - Manages downloaded games on local filesystem
 *
 * Features:
 * - Download games from Nova Hub
 * - Store games locally for offline play
 * - Manage game versions and updates
 * - Track download progress
 * - Handle game installation
 */

import {
  BaseDirectory,
  exists,
  mkdir,
  remove,
  writeFile,
  readFile,
} from '@tauri-apps/plugin-fs';

// Helper function to convert string to Uint8Array
function stringToUint8Array(str: string): Uint8Array {
  const encoder = new TextEncoder();
  return encoder.encode(str);
}

export interface LocalGame {
  id: string;
  name: string;
  version: string;
  size: number;
  downloadedAt: Date;
  lastPlayedAt?: Date;
  installPath: string;
  thumbnail?: string;
  metadata: GameMetadata;
}

export interface GameMetadata {
  title: string;
  description: string;
  developer: string;
  category: string;
  tags: string[];
  rating?: number;
}

export interface DownloadProgress {
  gameId: string;
  downloaded: number;
  total: number;
  speed: number;
  status: 'downloading' | 'installing' | 'completed' | 'error';
  error?: string;
}

class LocalGameStorageService {
  private gamesDir = 'nova-games';
  private metadataFile = 'games-metadata.json';
  private games: Map<string, LocalGame> = new Map();
  private downloadProgress: Map<string, DownloadProgress> = new Map();

  /**
   * Initialize the local game storage
   */
  async initialize(): Promise<void> {
    try {
      // Create games directory if it doesn't exist
      const gamesPath = `${this.gamesDir}`;
      const dirExists = await exists(gamesPath, {
        baseDir: BaseDirectory.AppData,
      });

      if (!dirExists) {
        await mkdir(gamesPath, {
          baseDir: BaseDirectory.AppData,
          recursive: true,
        });
      }

      // Load existing games metadata
      await this.loadMetadata();
    } catch (error) {
      console.error('Failed to initialize LocalGameStorage:', error);
      throw error;
    }
  }

  /**
   * Download a game from Nova Hub
   */
  async downloadGame(
    gameId: string,
    downloadUrl: string,
    metadata: GameMetadata,
    onProgress?: (progress: DownloadProgress) => void
  ): Promise<LocalGame> {
    try {
      // Create game directory
      const gamePath = `${this.gamesDir}/${gameId}`;
      await mkdir(gamePath, {
        baseDir: BaseDirectory.AppData,
        recursive: true,
      });

      // Initialize download progress
      const progress: DownloadProgress = {
        gameId,
        downloaded: 0,
        total: 0,
        speed: 0,
        status: 'downloading',
      };
      this.downloadProgress.set(gameId, progress);
      onProgress?.(progress);

      // Download game files using fetch API with progress tracking
      const gameFilePath = `${gamePath}/game.zip`;

      const downloadResponse = await fetch(downloadUrl);
      if (!downloadResponse.ok) {
        throw new Error(`Download failed: ${downloadResponse.statusText}`);
      }

      const contentLength = parseInt(
        downloadResponse.headers.get('content-length') || '0'
      );
      progress.total = contentLength;

      // Stream download with progress tracking
      const reader = downloadResponse.body?.getReader();
      const chunks: Uint8Array[] = [];
      let receivedLength = 0;

      if (reader) {
        while (true) {
          const { done, value } = await reader.read();
          if (done) break;

          chunks.push(value);
          receivedLength += value.length;
          progress.downloaded = receivedLength;
          // Calculate speed (simplified - could be improved with time tracking)
          progress.speed = receivedLength / 1024; // KB/s (simplified)
          onProgress?.(progress);
        }
      }

      // Combine chunks and save file
      const gameData = new Uint8Array(receivedLength);
      let position = 0;
      for (const chunk of chunks) {
        gameData.set(chunk, position);
        position += chunk.length;
      }

      // Write to file system
      await writeFile(gameFilePath, gameData, {
        baseDir: BaseDirectory.AppData,
      });

      // Update progress to installing
      progress.status = 'installing';
      onProgress?.(progress);

      // Extract and install game
      await this.installGame(gameId, gameFilePath, gameData);

      // Create local game record
      const localGame: LocalGame = {
        id: gameId,
        name: metadata.title,
        version: '1.0.0',
        size: progress.total,
        downloadedAt: new Date(),
        installPath: gamePath,
        metadata,
      };

      // Save to games map
      this.games.set(gameId, localGame);
      await this.saveMetadata();

      // Update progress to completed
      progress.status = 'completed';
      onProgress?.(progress);
      this.downloadProgress.delete(gameId);

      return localGame;
    } catch (error) {
      const progress = this.downloadProgress.get(gameId);
      if (progress) {
        progress.status = 'error';
        progress.error =
          error instanceof Error ? error.message : 'Unknown error';
        onProgress?.(progress);
      }
      throw error;
    }
  }

  /**
   * Install game from downloaded archive
   */
  private async installGame(
    gameId: string,
    archivePath: string,
    gameData: Uint8Array
  ): Promise<void> {
    console.log(`Installing game ${gameId} from ${archivePath}`);

    // Extract game files from archive
    // In production, this would use proper ZIP extraction
    // For now, we'll create the essential game structure

    const gameDir = archivePath.replace('/game.zip', '');

    // Create game directory structure
    const directories = ['assets', 'scenes', 'scripts', 'saves'];
    for (const dir of directories) {
      await mkdir(`${gameDir}/${dir}`, {
        recursive: true,
        baseDir: BaseDirectory.AppData,
      });
    }

    // Create manifest file
    const manifest = {
      id: gameId,
      version: '1.0.0',
      files: {
        'engine.js': true,
        'game.js': true,
        'assets/': true,
        'scenes/main.json': true,
      },
      installed: new Date().toISOString(),
      size: gameData.length,
    };

    await writeFile(
      `${gameDir}/manifest.json`,
      stringToUint8Array(JSON.stringify(manifest, null, 2)),
      { baseDir: BaseDirectory.AppData }
    );

    // Create placeholder engine and game files
    // In production, these would be extracted from the archive
    const engineStub = `
// Nova Engine Runtime - ${gameId}
console.log('Nova Engine initializing...');

window.NovaEngine = {
  init: async function(config) {
    console.log('Engine initialized with config:', config);
    this.config = config;
    return true;
  },
  loadScene: async function(sceneData) {
    console.log('Loading scene:', sceneData);
    return true;
  },
  start: function() {
    console.log('Game loop started');
  },
  pause: function() {
    console.log('Game paused');
  },
  resume: function() {
    console.log('Game resumed');
  },
  stop: function() {
    console.log('Game stopped');
  },
  cleanup: function() {
    console.log('Resources cleaned up');
  }
};
    `.trim();

    await writeFile(`${gameDir}/engine.js`, stringToUint8Array(engineStub), {
      baseDir: BaseDirectory.AppData,
    });

    const gameStub = `
// Game Logic - ${gameId}
console.log('Game script loaded');
    `.trim();

    await writeFile(`${gameDir}/game.js`, stringToUint8Array(gameStub), {
      baseDir: BaseDirectory.AppData,
    });

    // Create default scene
    const defaultScene = {
      name: 'Main Scene',
      entities: [],
      settings: {
        ambientLight: { r: 0.2, g: 0.2, b: 0.2 },
        gravity: { x: 0, y: -9.81, z: 0 },
      },
    };

    await writeFile(
      `${gameDir}/scenes/main.json`,
      stringToUint8Array(JSON.stringify(defaultScene, null, 2)),
      { baseDir: BaseDirectory.AppData }
    );

    console.log(`Game ${gameId} installation complete`);
  }

  /**
   * Get all locally installed games
   */
  async getInstalledGames(): Promise<LocalGame[]> {
    return Array.from(this.games.values());
  }

  /**
   * Get a specific game by ID
   */
  async getGame(gameId: string): Promise<LocalGame | null> {
    return this.games.get(gameId) || null;
  }

  /**
   * Check if a game is installed
   */
  async isGameInstalled(gameId: string): Promise<boolean> {
    return this.games.has(gameId);
  }

  /**
   * Uninstall a game
   */
  async uninstallGame(gameId: string): Promise<void> {
    const game = this.games.get(gameId);
    if (!game) {
      throw new Error(`Game ${gameId} not found`);
    }

    try {
      // Delete game directory
      const gamePath = `${this.gamesDir}/${gameId}`;
      await remove(gamePath, {
        baseDir: BaseDirectory.AppData,
        recursive: true,
      });

      // Remove from games map
      this.games.delete(gameId);
      await this.saveMetadata();
    } catch (error) {
      console.error(`Failed to uninstall game ${gameId}:`, error);
      throw error;
    }
  }

  /**
   * Update game last played timestamp
   */
  async updateLastPlayed(gameId: string): Promise<void> {
    const game = this.games.get(gameId);
    if (game) {
      game.lastPlayedAt = new Date();
      await this.saveMetadata();
    }
  }

  /**
   * Get download progress for a game
   */
  getDownloadProgress(gameId: string): DownloadProgress | null {
    return this.downloadProgress.get(gameId) || null;
  }

  /**
   * Get total size of all installed games
   */
  async getTotalSize(): Promise<number> {
    return Array.from(this.games.values()).reduce(
      (total, game) => total + game.size,
      0
    );
  }

  /**
   * Load games metadata from file
   */
  private async loadMetadata(): Promise<void> {
    try {
      const metadataPath = `${this.gamesDir}/${this.metadataFile}`;
      const fileExists = await exists(metadataPath, {
        baseDir: BaseDirectory.AppData,
      });

      if (fileExists) {
        const content = await readFile(metadataPath, {
          baseDir: BaseDirectory.AppData,
        });
        const text = new TextDecoder().decode(content);
        const gamesArray: LocalGame[] = JSON.parse(text);

        // Convert dates from strings
        gamesArray.forEach((game) => {
          game.downloadedAt = new Date(game.downloadedAt);
          if (game.lastPlayedAt) {
            game.lastPlayedAt = new Date(game.lastPlayedAt);
          }
          this.games.set(game.id, game);
        });
      }
    } catch (error) {
      console.error('Failed to load games metadata:', error);
    }
  }

  /**
   * Save games metadata to file
   */
  private async saveMetadata(): Promise<void> {
    try {
      const gamesArray = Array.from(this.games.values());
      const json = JSON.stringify(gamesArray, null, 2);
      const encoder = new TextEncoder();
      const data = encoder.encode(json);

      const metadataPath = `${this.gamesDir}/${this.metadataFile}`;
      await writeFile(metadataPath, data, { baseDir: BaseDirectory.AppData });
    } catch (error) {
      console.error('Failed to save games metadata:', error);
      throw error;
    }
  }
}

export const localGameStorage = new LocalGameStorageService();
