/**
 * UpdateManager - Manages game and launcher updates
 *
 * Features:
 * - Check for game updates
 * - Download and install game updates
 * - Launcher auto-update
 * - Update notifications
 * - Rollback support
 */

import { check } from '@tauri-apps/plugin-updater';
import { relaunch } from '@tauri-apps/plugin-process';
import { localGameStorage, LocalGame } from './LocalGameStorage';

export interface GameUpdate {
  gameId: string;
  currentVersion: string;
  latestVersion: string;
  size: number;
  changelog: string;
  releaseDate: Date;
  required: boolean; // If true, game won't run without update
}

export interface LauncherUpdate {
  currentVersion: string;
  latestVersion: string;
  size: number;
  releaseNotes: string;
  downloadUrl: string;
}

export interface UpdateProgress {
  id: string;
  type: 'game' | 'launcher';
  downloaded: number;
  total: number;
  status: 'checking' | 'downloading' | 'installing' | 'completed' | 'error';
  error?: string;
}

class UpdateManagerService {
  private updateCheckInterval: number | null = null;
  private gameUpdates: Map<string, GameUpdate> = new Map();
  private updateProgress: Map<string, UpdateProgress> = new Map();
  private launcherVersion: string = '0.1.0';

  /**
   * Initialize the update manager
   */
  async initialize(): Promise<void> {
    // Start periodic update checks (every 6 hours)
    this.startUpdateChecks(6 * 60 * 60 * 1000);

    // Check for updates immediately
    await this.checkForUpdates();
  }

  /**
   * Start periodic update checks
   */
  startUpdateChecks(intervalMs: number): void {
    if (this.updateCheckInterval) {
      clearInterval(this.updateCheckInterval);
    }

    this.updateCheckInterval = window.setInterval(() => {
      this.checkForUpdates().catch(console.error);
    }, intervalMs);
  }

  /**
   * Stop periodic update checks
   */
  stopUpdateChecks(): void {
    if (this.updateCheckInterval) {
      clearInterval(this.updateCheckInterval);
      this.updateCheckInterval = null;
    }
  }

  /**
   * Check for all updates (games and launcher)
   */
  async checkForUpdates(): Promise<void> {
    try {
      // Check for game updates
      await this.checkGameUpdates();

      // Check for launcher updates
      await this.checkLauncherUpdate();
    } catch (error) {
      console.error('Failed to check for updates:', error);
    }
  }

  /**
   * Check for game updates
   */
  async checkGameUpdates(): Promise<GameUpdate[]> {
    try {
      const games = await localGameStorage.getInstalledGames();
      const updates: GameUpdate[] = [];

      for (const game of games) {
        const update = await this.checkGameUpdate(game);
        if (update) {
          updates.push(update);
          this.gameUpdates.set(game.id, update);
        }
      }

      return updates;
    } catch (error) {
      console.error('Failed to check game updates:', error);
      return [];
    }
  }

  /**
   * Check for update for a specific game
   */
  private async checkGameUpdate(game: LocalGame): Promise<GameUpdate | null> {
    try {
      // Implementation would:
      // 1. Call Nova Hub API to get latest version
      // 2. Compare with installed version
      // 3. Return update info if newer version exists

      // Placeholder implementation
      const latestVersion = game.version; // Would be fetched from API

      if (latestVersion !== game.version) {
        return {
          gameId: game.id,
          currentVersion: game.version,
          latestVersion,
          size: 50 * 1024 * 1024, // 50MB placeholder
          changelog: 'Bug fixes and improvements',
          releaseDate: new Date(),
          required: false,
        };
      }

      return null;
    } catch (error) {
      console.error(`Failed to check update for game ${game.id}:`, error);
      return null;
    }
  }

  /**
   * Check for launcher update
   */
  async checkLauncherUpdate(): Promise<LauncherUpdate | null> {
    try {
      // Use Tauri updater plugin
      const update = await check();

      if (update?.available) {
        return {
          currentVersion: this.launcherVersion,
          latestVersion: update.version,
          size: 0, // Size not provided by Tauri
          releaseNotes: update.body || 'Update available',
          downloadUrl: '', // URL handled internally by Tauri
        };
      }

      return null;
    } catch (error) {
      console.error('Failed to check launcher update:', error);
      return null;
    }
  }

  /**
   * Update a game to the latest version
   */
  async updateGame(
    gameId: string,
    onProgress?: (progress: UpdateProgress) => void
  ): Promise<void> {
    const update = this.gameUpdates.get(gameId);
    if (!update) {
      throw new Error(`No update available for game ${gameId}`);
    }

    try {
      // Initialize progress
      const progress: UpdateProgress = {
        id: gameId,
        type: 'game',
        downloaded: 0,
        total: update.size,
        status: 'downloading',
      };
      this.updateProgress.set(gameId, progress);
      onProgress?.(progress);

      // Download update
      // Implementation would download the update package

      // Update progress
      progress.status = 'installing';
      progress.downloaded = progress.total;
      onProgress?.(progress);

      // Install update
      // Implementation would install the update

      // Complete
      progress.status = 'completed';
      onProgress?.(progress);
      this.updateProgress.delete(gameId);
      this.gameUpdates.delete(gameId);

      // Update game metadata
      const game = await localGameStorage.getGame(gameId);
      if (game) {
        game.version = update.latestVersion;
      }
    } catch (error) {
      const progress = this.updateProgress.get(gameId);
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
   * Update the launcher
   */
  async updateLauncher(): Promise<void> {
    try {
      const update = await check();

      if (update?.available) {
        // eslint-disable-next-line no-console
        console.log('Installing launcher update...');

        // Download and install the update
        await update.downloadAndInstall();

        // Relaunch the application
        await relaunch();
      } else {
        // eslint-disable-next-line no-console
        console.log('No launcher update available');
      }
    } catch (error) {
      console.error('Failed to update launcher:', error);
      throw error;
    }
  }

  /**
   * Get available updates
   */
  getAvailableUpdates(): GameUpdate[] {
    return Array.from(this.gameUpdates.values());
  }

  /**
   * Get update for specific game
   */
  getGameUpdate(gameId: string): GameUpdate | null {
    return this.gameUpdates.get(gameId) || null;
  }

  /**
   * Get update progress
   */
  getUpdateProgress(id: string): UpdateProgress | null {
    return this.updateProgress.get(id) || null;
  }

  /**
   * Check if game has update available
   */
  hasGameUpdate(gameId: string): boolean {
    return this.gameUpdates.has(gameId);
  }

  /**
   * Update all games
   */
  async updateAllGames(
    onProgress?: (gameId: string, progress: UpdateProgress) => void
  ): Promise<void> {
    const updates = this.getAvailableUpdates();

    for (const update of updates) {
      try {
        await this.updateGame(update.gameId, (progress) => {
          onProgress?.(update.gameId, progress);
        });
      } catch (error) {
        console.error(`Failed to update game ${update.gameId}:`, error);
      }
    }
  }

  /**
   * Get launcher version
   */
  getLauncherVersion(): string {
    return this.launcherVersion;
  }
}

export const updateManager = new UpdateManagerService();
