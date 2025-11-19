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
 * Project Import Service
 * Allows users to import existing game projects from various sources
 */

import { DatabaseService } from '../database/DatabaseService';
import { ProjectStorageService } from './ProjectStorageService';
import { AssetStorageService } from '../storage/AssetStorageService';
import { VersionControlService } from './VersionControlService';
import * as fs from 'fs/promises';
import * as path from 'path';
// import * as crypto from 'crypto';  // Unused import

/**
 * Helper to extract error message from unknown error type
 */
function getErrorMessage(error: unknown): string {
  if (error instanceof Error) return error.message;
  if (error && typeof error === 'object' && 'message' in error) {
    return String(error.message);
  }
  return String(error);
}

export interface ImportSource {
  type:
    | 'zip'
    | 'git'
    | 'url'
    | 'local'
    | 'unity'
    | 'unreal'
    | 'godot'
    | 'other';
  location: string;
  credentials?: {
    token?: string;
    username?: string;
    password?: string;
  };
}

export interface ImportOptions {
  projectName: string;
  description?: string;
  userId: string;
  source: ImportSource;
  preserveStructure?: boolean;
  convertAssets?: boolean;
  createVersion?: boolean;
}

export interface ImportResult {
  success: boolean;
  projectId?: string;
  assetsImported: number;
  filesImported: number;
  errors: string[];
  warnings: string[];
  duration: number;
}

export interface ImportProgress {
  stage:
    | 'downloading'
    | 'extracting'
    | 'analyzing'
    | 'converting'
    | 'uploading'
    | 'complete';
  progress: number;
  message: string;
  filesProcessed: number;
  totalFiles: number;
}

export class ProjectImportService {
  private db = DatabaseService.getInstance();
  private projectService = new ProjectStorageService();
  private assetService = new AssetStorageService();
  private versionService = new VersionControlService();

  /**
   * Import project from various sources
   */
  async importProject(options: ImportOptions): Promise<ImportResult> {
    const startTime = Date.now();
    const errors: string[] = [];
    const warnings: string[] = [];
    let assetsImported = 0;
    let filesImported = 0;

    try {
      // Create project
      const project = await this.projectService.createProject({
        ownerId: options.userId,
        name: options.projectName,
        description: options.description || 'Imported project',
        visibility: 'private',
      });

      // Download/extract project files based on source type
      const projectFiles = await this.downloadProjectFiles(options.source);

      // Analyze project structure
      const analysis = await this.analyzeProjectStructure(projectFiles);

      // Convert assets if needed
      if (options.convertAssets) {
        await this.convertAssets(analysis, options.source.type);
      }

      // Import assets
      for (const file of analysis.assets) {
        try {
          const buffer = await fs.readFile(file.absolutePath);
          await this.assetService.uploadAsset({
            projectId: project.id,
            path: file.relativePath,
            type: file.mimeType,
            buffer,
          });
          assetsImported++;
        } catch (error: unknown) {
          errors.push(
            `Failed to import asset ${file.relativePath}: ${getErrorMessage(error)}`
          );
        }
      }

      // Import scene files
      for (const file of analysis.scenes) {
        try {
          const buffer = await fs.readFile(file.absolutePath);
          await this.assetService.uploadAsset({
            projectId: project.id,
            path: file.relativePath,
            type: 'application/json',
            buffer,
          });
          filesImported++;
        } catch (error: unknown) {
          errors.push(
            `Failed to import scene ${file.relativePath}: ${getErrorMessage(error)}`
          );
        }
      }

      // Import scripts
      for (const file of analysis.scripts) {
        try {
          const buffer = await fs.readFile(file.absolutePath);
          await this.assetService.uploadAsset({
            projectId: project.id,
            path: file.relativePath,
            type: 'text/javascript',
            buffer,
          });
          filesImported++;
        } catch (error: unknown) {
          errors.push(
            `Failed to import script ${file.relativePath}: ${getErrorMessage(error)}`
          );
        }
      }

      // Create initial version if requested
      if (options.createVersion) {
        await this.versionService.createVersion({
          projectId: project.id,
          commitMessage: `Imported from ${options.source.type}`,
          createdBy: options.userId,
          snapshotData: {
            importSource: options.source.type,
            importDate: new Date().toISOString(),
            originalStructure: analysis.structure,
          },
        });
      }

      const duration = Date.now() - startTime;

      return {
        success: true,
        projectId: project.id,
        assetsImported,
        filesImported,
        errors,
        warnings,
        duration,
      };
    } catch (error: unknown) {
      errors.push(`Import failed: ${getErrorMessage(error)}`);
      return {
        success: false,
        assetsImported,
        filesImported,
        errors,
        warnings,
        duration: Date.now() - startTime,
      };
    }
  }

  /**
   * Download project files from source
   */
  private async downloadProjectFiles(source: ImportSource): Promise<string> {
    const tempDir = path.join('/tmp', `import-${Date.now()}`);
    await fs.mkdir(tempDir, { recursive: true });

    switch (source.type) {
      case 'zip':
        return this.downloadFromZip(source.location, tempDir);

      case 'git':
        return this.downloadFromGit(
          source.location,
          tempDir,
          source.credentials
        );

      case 'url':
        return this.downloadFromUrl(source.location, tempDir);

      case 'local':
        return source.location;

      case 'unity':
        return this.importFromUnity(source.location, tempDir);

      case 'unreal':
        return this.importFromUnreal(source.location, tempDir);

      case 'godot':
        return this.importFromGodot(source.location, tempDir);

      default:
        return this.downloadFromUrl(source.location, tempDir);
    }
  }

  /**
   * Download from ZIP file
   */
  private async downloadFromZip(
    zipPath: string,
    targetDir: string
  ): Promise<string> {
    // In production, would use a proper zip library
    // For now, assume files are extracted
    return targetDir;
  }

  /**
   * Download from Git repository
   */
  private async downloadFromGit(
    repoUrl: string,
    targetDir: string,
    _credentials?: { token?: string; username?: string; password?: string }
  ): Promise<string> {
    // In production, would use git CLI or library
    // git clone with authentication
    return targetDir;
  }

  /**
   * Download from URL
   */
  private async downloadFromUrl(
    url: string,
    targetDir: string
  ): Promise<string> {
    // Download file from URL and extract if needed
    return targetDir;
  }

  /**
   * Import from Unity project
   */
  private async importFromUnity(
    unityProjectPath: string,
    targetDir: string
  ): Promise<string> {
    // Convert Unity project structure to Nova Engine format
    // - Assets/ folder becomes assets/
    // - Scenes become .scene.json files
    // - Scripts become TypeScript (manual conversion needed)
    // - Materials become PBR materials
    return targetDir;
  }

  /**
   * Import from Unreal project
   */
  private async importFromUnreal(
    unrealProjectPath: string,
    targetDir: string
  ): Promise<string> {
    // Convert Unreal project structure to Nova Engine format
    // - Content/ folder becomes assets/
    // - Maps become scenes
    // - Blueprints need manual conversion
    // - Materials convert to PBR
    return targetDir;
  }

  /**
   * Import from Godot project
   */
  private async importFromGodot(
    godotProjectPath: string,
    targetDir: string
  ): Promise<string> {
    // Convert Godot project structure to Nova Engine format
    // - Similar structure, easier conversion
    // - .tscn files convert to .scene.json
    // - GDScript needs manual conversion to TypeScript
    return targetDir;
  }

  /**
   * Analyze project structure
   */
  private async analyzeProjectStructure(projectPath: string): Promise<{
    structure: string;
    assets: Array<{
      relativePath: string;
      absolutePath: string;
      mimeType: string;
    }>;
    scenes: Array<{ relativePath: string; absolutePath: string }>;
    scripts: Array<{ relativePath: string; absolutePath: string }>;
    config: any;
  }> {
    const assets: Array<{
      relativePath: string;
      absolutePath: string;
      mimeType: string;
    }> = [];
    const scenes: Array<{ relativePath: string; absolutePath: string }> = [];
    const scripts: Array<{ relativePath: string; absolutePath: string }> = [];

    await this.scanDirectory(projectPath, projectPath, assets, scenes, scripts);

    return {
      structure: 'nova-engine', // or detected structure type
      assets,
      scenes,
      scripts,
      config: {},
    };
  }

  /**
   * Scan directory recursively
   */
  private async scanDirectory(
    baseDir: string,
    currentDir: string,
    assets: Array<{
      relativePath: string;
      absolutePath: string;
      mimeType: string;
    }>,
    scenes: Array<{ relativePath: string; absolutePath: string }>,
    scripts: Array<{ relativePath: string; absolutePath: string }>
  ): Promise<void> {
    const entries = await fs.readdir(currentDir, { withFileTypes: true });

    for (const entry of entries) {
      const absolutePath = path.join(currentDir, entry.name);
      const relativePath = path.relative(baseDir, absolutePath);

      if (entry.isDirectory()) {
        await this.scanDirectory(
          baseDir,
          absolutePath,
          assets,
          scenes,
          scripts
        );
      } else {
        const ext = path.extname(entry.name).toLowerCase();

        // Categorize files
        if (['.png', '.jpg', '.jpeg', '.gif', '.webp', '.svg'].includes(ext)) {
          assets.push({
            relativePath,
            absolutePath,
            mimeType: `image/${ext.substring(1)}`,
          });
        } else if (['.gltf', '.glb', '.obj', '.fbx'].includes(ext)) {
          assets.push({
            relativePath,
            absolutePath,
            mimeType: 'model/gltf-binary',
          });
        } else if (['.mp3', '.wav', '.ogg', '.m4a'].includes(ext)) {
          assets.push({
            relativePath,
            absolutePath,
            mimeType: `audio/${ext.substring(1)}`,
          });
        } else if (['.scene', '.scene.json'].includes(ext)) {
          scenes.push({ relativePath, absolutePath });
        } else if (['.ts', '.js', '.tsx', '.jsx'].includes(ext)) {
          scripts.push({ relativePath, absolutePath });
        }
      }
    }
  }

  /**
   * Convert assets to Nova Engine format
   */
  private async convertAssets(
    _analysis: any,
    _sourceType: string
  ): Promise<void> {
    // Convert materials, shaders, etc. based on source engine
    // Unity materials -> PBR materials
    // Unreal materials -> PBR materials
    // etc.
  }

  /**
   * Import from existing Nova Engine project (for continuing work)
   */
  async importExistingNovaProject(
    userId: string,
    projectPath: string,
    options: {
      projectName?: string;
      syncAssets?: boolean;
    } = {}
  ): Promise<ImportResult> {
    const startTime = Date.now();
    const errors: string[] = [];
    const warnings: string[] = [];

    try {
      // Read project metadata
      const metadataPath = path.join(projectPath, 'project.json');
      let metadata: any = {};

      try {
        const metadataContent = await fs.readFile(metadataPath, 'utf-8');
        metadata = JSON.parse(metadataContent);
        // eslint-disable-next-line @typescript-eslint/no-unused-vars
      } catch (_error) {
        warnings.push('No project.json found, creating new project metadata');
      }

      // Create or link project
      const project = await this.projectService.createProject({
        ownerId: userId,
        name: options.projectName || metadata.name || 'Imported Nova Project',
        description: metadata.description || 'Imported Nova Engine project',
        visibility: 'private',
      });

      // Import all assets
      const analysis = await this.analyzeProjectStructure(projectPath);
      let assetsImported = 0;

      for (const asset of analysis.assets) {
        try {
          const buffer = await fs.readFile(asset.absolutePath);
          await this.assetService.uploadAsset({
            projectId: project.id,
            path: asset.relativePath,
            type: asset.mimeType,
            buffer,
          });
          assetsImported++;
        } catch (error: unknown) {
          errors.push(
            `Failed to import ${asset.relativePath}: ${getErrorMessage(error)}`
          );
        }
      }

      // Import scenes
      for (const scene of analysis.scenes) {
        try {
          const buffer = await fs.readFile(scene.absolutePath);
          await this.assetService.uploadAsset({
            projectId: project.id,
            path: scene.relativePath,
            type: 'application/json',
            buffer,
          });
        } catch (error: unknown) {
          errors.push(
            `Failed to import scene ${scene.relativePath}: ${getErrorMessage(error)}`
          );
        }
      }

      // Import scripts
      for (const script of analysis.scripts) {
        try {
          const buffer = await fs.readFile(script.absolutePath);
          await this.assetService.uploadAsset({
            projectId: project.id,
            path: script.relativePath,
            type: 'text/javascript',
            buffer,
          });
        } catch (error: unknown) {
          errors.push(
            `Failed to import script ${script.relativePath}: ${getErrorMessage(error)}`
          );
        }
      }

      // Create initial version
      await this.versionService.createVersion({
        projectId: project.id,
        commitMessage: 'Imported existing Nova Engine project',
        createdBy: userId,
        snapshotData: metadata,
      });

      return {
        success: true,
        projectId: project.id,
        assetsImported,
        filesImported: analysis.scenes.length + analysis.scripts.length,
        errors,
        warnings,
        duration: Date.now() - startTime,
      };
    } catch (error: unknown) {
      errors.push(`Import failed: ${getErrorMessage(error)}`);
      return {
        success: false,
        assetsImported: 0,
        filesImported: 0,
        errors,
        warnings,
        duration: Date.now() - startTime,
      };
    }
  }

  /**
   * Get import progress
   */
  async getImportProgress(_importId: string): Promise<ImportProgress | null> {
    // In production, this would track active import operations
    return null;
  }

  /**
   * Validate import source
   */
  async validateImportSource(source: ImportSource): Promise<{
    valid: boolean;
    errors: string[];
    estimatedSize?: number;
    fileCount?: number;
  }> {
    const errors: string[] = [];

    // Validate based on source type
    switch (source.type) {
      case 'git':
        // Check if Git repo is accessible
        if (!source.location.match(/^https?:\/\/.+\.git$/)) {
          errors.push('Invalid Git repository URL');
        }
        break;

      case 'url':
        // Check if URL is valid
        try {
          new URL(source.location);
        } catch {
          errors.push('Invalid URL');
        }
        break;

      case 'local':
        // Check if path exists
        try {
          await fs.access(source.location);
        } catch {
          errors.push('Local path does not exist');
        }
        break;
    }

    return {
      valid: errors.length === 0,
      errors,
    };
  }

  /**
   * List supported import formats
   */
  getSupportedFormats(): Array<{
    type: string;
    name: string;
    description: string;
    extensions: string[];
  }> {
    return [
      {
        type: 'zip',
        name: 'ZIP Archive',
        description: 'Import from a ZIP file containing your game project',
        extensions: ['.zip'],
      },
      {
        type: 'git',
        name: 'Git Repository',
        description: 'Clone and import from a Git repository',
        extensions: [],
      },
      {
        type: 'local',
        name: 'Local Folder',
        description: 'Import from a folder on your computer',
        extensions: [],
      },
      {
        type: 'unity',
        name: 'Unity Project',
        description:
          'Import and convert Unity project assets (manual script conversion required)',
        extensions: [],
      },
      {
        type: 'unreal',
        name: 'Unreal Engine Project',
        description: 'Import and convert Unreal Engine project assets',
        extensions: [],
      },
      {
        type: 'godot',
        name: 'Godot Project',
        description: 'Import and convert Godot project',
        extensions: [],
      },
      {
        type: 'other',
        name: 'Other Engine',
        description: 'Import generic game assets and resources',
        extensions: [],
      },
    ];
  }
}
