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
 * LocalSaveData - Manages game save data locally
 *
 * Features:
 * - Save/load game progress
 * - Multiple save slots per game
 * - Auto-save support
 * - Save data backup
 * - Cross-device sync preparation
 */

import {
  BaseDirectory,
  exists,
  mkdir,
  readFile,
  writeFile,
  readDir,
  remove,
} from '@tauri-apps/plugin-fs';

export interface SaveSlot {
  id: string;
  gameId: string;
  name: string;
  data: any;
  timestamp: Date;
  playtime: number; // seconds
  progress: number; // percentage 0-100
  screenshot?: string; // base64 or path
}

export interface SaveMetadata {
  gameId: string;
  slots: SaveSlot[];
  autoSaveEnabled: boolean;
  lastAutoSave?: Date;
}

class LocalSaveDataService {
  private savesDir = 'nova-saves';
  private saveCache: Map<string, SaveMetadata> = new Map();

  /**
   * Initialize save data service
   */
  async initialize(): Promise<void> {
    try {
      // Create saves directory if it doesn't exist
      const dirExists = await exists(this.savesDir, {
        baseDir: BaseDirectory.AppData,
      });

      if (!dirExists) {
        await mkdir(this.savesDir, {
          baseDir: BaseDirectory.AppData,
          recursive: true,
        });
      }

      // Load existing save metadata
      await this.loadAllSaveMetadata();
    } catch (error) {
      console.error('Failed to initialize LocalSaveData:', error);
      throw error;
    }
  }

  /**
   * Save game data to a specific slot
   */
  async saveGame(
    gameId: string,
    slotId: string,
    data: any,
    metadata?: {
      name?: string;
      playtime?: number;
      progress?: number;
      screenshot?: string;
    }
  ): Promise<SaveSlot> {
    try {
      // Create game saves directory
      const gameDir = `${this.savesDir}/${gameId}`;
      const dirExists = await exists(gameDir, {
        baseDir: BaseDirectory.AppData,
      });

      if (!dirExists) {
        await mkdir(gameDir, {
          baseDir: BaseDirectory.AppData,
          recursive: true,
        });
      }

      // Create save slot
      const saveSlot: SaveSlot = {
        id: slotId,
        gameId,
        name: metadata?.name || `Save ${slotId}`,
        data,
        timestamp: new Date(),
        playtime: metadata?.playtime || 0,
        progress: metadata?.progress || 0,
        screenshot: metadata?.screenshot,
      };

      // Save to file
      const savePath = `${gameDir}/${slotId}.json`;
      const json = JSON.stringify(saveSlot, null, 2);
      const encoder = new TextEncoder();
      const fileData = encoder.encode(json);

      await writeFile(savePath, fileData, { baseDir: BaseDirectory.AppData });

      // Update cache
      await this.updateSaveMetadata(gameId, saveSlot);

      return saveSlot;
    } catch (error) {
      console.error(`Failed to save game ${gameId} slot ${slotId}:`, error);
      throw error;
    }
  }

  /**
   * Load game data from a specific slot
   */
  async loadGame(gameId: string, slotId: string): Promise<SaveSlot | null> {
    try {
      const savePath = `${this.savesDir}/${gameId}/${slotId}.json`;
      const fileExists = await exists(savePath, {
        baseDir: BaseDirectory.AppData,
      });

      if (!fileExists) {
        return null;
      }

      const content = await readFile(savePath, {
        baseDir: BaseDirectory.AppData,
      });
      const text = new TextDecoder().decode(content);
      const saveSlot: SaveSlot = JSON.parse(text);

      // Convert timestamp from string to Date
      saveSlot.timestamp = new Date(saveSlot.timestamp);

      return saveSlot;
    } catch (error) {
      console.error(`Failed to load game ${gameId} slot ${slotId}:`, error);
      return null;
    }
  }

  /**
   * Get all save slots for a game
   */
  async getSaveSlots(gameId: string): Promise<SaveSlot[]> {
    const metadata = this.saveCache.get(gameId);
    if (metadata) {
      return metadata.slots;
    }

    try {
      const gameDir = `${this.savesDir}/${gameId}`;
      const dirExists = await exists(gameDir, {
        baseDir: BaseDirectory.AppData,
      });

      if (!dirExists) {
        return [];
      }

      const entries = await readDir(gameDir, {
        baseDir: BaseDirectory.AppData,
      });
      const slots: SaveSlot[] = [];

      for (const entry of entries) {
        if (entry.name?.endsWith('.json') && !entry.name.includes('metadata')) {
          const slotId = entry.name.replace('.json', '');
          const slot = await this.loadGame(gameId, slotId);
          if (slot) {
            slots.push(slot);
          }
        }
      }

      // Sort by timestamp (newest first)
      slots.sort((a, b) => b.timestamp.getTime() - a.timestamp.getTime());

      return slots;
    } catch (error) {
      console.error(`Failed to get save slots for game ${gameId}:`, error);
      return [];
    }
  }

  /**
   * Delete a save slot
   */
  async deleteSave(gameId: string, slotId: string): Promise<void> {
    try {
      const savePath = `${this.savesDir}/${gameId}/${slotId}.json`;
      await remove(savePath, { baseDir: BaseDirectory.AppData });

      // Update cache
      const metadata = this.saveCache.get(gameId);
      if (metadata) {
        metadata.slots = metadata.slots.filter((slot) => slot.id !== slotId);
        await this.saveSaveMetadata(gameId, metadata);
      }
    } catch (error) {
      console.error(`Failed to delete save ${gameId}/${slotId}:`, error);
      throw error;
    }
  }

  /**
   * Auto-save game data
   */
  async autoSave(
    gameId: string,
    data: any,
    playtime: number,
    progress: number
  ): Promise<SaveSlot> {
    const metadata = this.saveCache.get(gameId) || {
      gameId,
      slots: [],
      autoSaveEnabled: true,
    };

    if (!metadata.autoSaveEnabled) {
      throw new Error('Auto-save is disabled for this game');
    }

    const autoSaveSlot = await this.saveGame(gameId, 'autosave', data, {
      name: 'Auto Save',
      playtime,
      progress,
    });

    metadata.lastAutoSave = new Date();
    await this.saveSaveMetadata(gameId, metadata);

    return autoSaveSlot;
  }

  /**
   * Enable/disable auto-save for a game
   */
  async setAutoSaveEnabled(gameId: string, enabled: boolean): Promise<void> {
    let metadata = this.saveCache.get(gameId);
    if (!metadata) {
      metadata = {
        gameId,
        slots: [],
        autoSaveEnabled: enabled,
      };
    } else {
      metadata.autoSaveEnabled = enabled;
    }

    await this.saveSaveMetadata(gameId, metadata);
  }

  /**
   * Get total size of all save data
   */
  async getTotalSaveSize(): Promise<number> {
    // Implementation would calculate actual file sizes
    // For now, return estimated size based on slot count
    let totalSlots = 0;
    for (const metadata of this.saveCache.values()) {
      totalSlots += metadata.slots.length;
    }
    return totalSlots * 1024 * 100; // Estimate 100KB per save
  }

  /**
   * Export save data (for backup or transfer)
   */
  async exportSaveData(gameId: string, slotId: string): Promise<Blob> {
    const saveSlot = await this.loadGame(gameId, slotId);
    if (!saveSlot) {
      throw new Error('Save slot not found');
    }

    const json = JSON.stringify(saveSlot, null, 2);
    return new Blob([json], { type: 'application/json' });
  }

  /**
   * Import save data (from backup or transfer)
   */
  async importSaveData(gameId: string, saveData: string): Promise<SaveSlot> {
    try {
      const saveSlot: SaveSlot = JSON.parse(saveData);
      saveSlot.timestamp = new Date(saveSlot.timestamp);

      // Generate new ID if needed
      if (!saveSlot.id) {
        saveSlot.id = `import-${Date.now()}`;
      }

      // Save the imported data
      return await this.saveGame(gameId, saveSlot.id, saveSlot.data, {
        name: saveSlot.name,
        playtime: saveSlot.playtime,
        progress: saveSlot.progress,
        screenshot: saveSlot.screenshot,
      });
    } catch (error) {
      console.error('Failed to import save data:', error);
      throw error;
    }
  }

  /**
   * Load all save metadata from disk
   */
  private async loadAllSaveMetadata(): Promise<void> {
    try {
      const dirExists = await exists(this.savesDir, {
        baseDir: BaseDirectory.AppData,
      });
      if (!dirExists) {
        return;
      }

      const entries = await readDir(this.savesDir, {
        baseDir: BaseDirectory.AppData,
      });

      for (const entry of entries) {
        if (entry.isDirectory && entry.name) {
          const gameId = entry.name;
          const slots = await this.getSaveSlots(gameId);
          this.saveCache.set(gameId, {
            gameId,
            slots,
            autoSaveEnabled: true,
          });
        }
      }
    } catch (error) {
      console.error('Failed to load save metadata:', error);
    }
  }

  /**
   * Update save metadata for a game
   */
  private async updateSaveMetadata(
    gameId: string,
    newSlot: SaveSlot
  ): Promise<void> {
    let metadata = this.saveCache.get(gameId);
    if (!metadata) {
      metadata = {
        gameId,
        slots: [],
        autoSaveEnabled: true,
      };
    }

    // Update or add slot
    const existingIndex = metadata.slots.findIndex(
      (slot) => slot.id === newSlot.id
    );
    if (existingIndex >= 0) {
      metadata.slots[existingIndex] = newSlot;
    } else {
      metadata.slots.push(newSlot);
    }

    // Sort by timestamp
    metadata.slots.sort(
      (a, b) => b.timestamp.getTime() - a.timestamp.getTime()
    );

    this.saveCache.set(gameId, metadata);
    await this.saveSaveMetadata(gameId, metadata);
  }

  /**
   * Save metadata to disk
   */
  private async saveSaveMetadata(
    gameId: string,
    metadata: SaveMetadata
  ): Promise<void> {
    try {
      const metadataPath = `${this.savesDir}/${gameId}/metadata.json`;
      const json = JSON.stringify(metadata, null, 2);
      const encoder = new TextEncoder();
      const data = encoder.encode(json);

      await writeFile(metadataPath, data, { baseDir: BaseDirectory.AppData });
      this.saveCache.set(gameId, metadata);
    } catch (error) {
      console.error(`Failed to save metadata for game ${gameId}:`, error);
    }
  }
}

export const localSaveData = new LocalSaveDataService();
