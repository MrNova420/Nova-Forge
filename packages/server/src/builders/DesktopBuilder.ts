/**
 * @fileoverview Desktop build pipeline for Electron/Tauri applications
 */

import * as fs from 'fs';
import * as path from 'path';
import {
  BuildConfig,
  BuildResult,
  BuildArtifact,
  BuildStats,
  BuildTarget,
  DEFAULT_BUILD_CONFIG,
} from './BuildConfig';

/**
 * Desktop platforms
 */
export enum DesktopPlatform {
  Windows = 'windows',
  MacOS = 'macos',
  Linux = 'linux',
}

/**
 * Desktop build configuration
 */
export interface DesktopBuildConfig extends BuildConfig {
  /** Target desktop platform */
  platform: DesktopPlatform;
  /** Application name */
  appName: string;
  /** Application version */
  appVersion: string;
  /** Build with Electron or Tauri */
  framework: 'electron' | 'tauri';
}

/**
 * Desktop builder for native applications
 * Creates platform-specific executables using Electron or Tauri
 */
export class DesktopBuilder {
  /** Build configuration */
  private config: DesktopBuildConfig;

  constructor(config?: Partial<DesktopBuildConfig>) {
    this.config = {
      ...DEFAULT_BUILD_CONFIG,
      target: BuildTarget.Desktop,
      platform: DesktopPlatform.Windows,
      appName: 'Nova Engine Game',
      appVersion: '1.0.0',
      framework: 'tauri',
      ...config,
    };
  }

  /**
   * Build desktop application
   */
  public async build(projectPath: string): Promise<BuildResult> {
    const startTime = Date.now();
    const errors: string[] = [];
    const warnings: string[] = [];
    const artifacts: BuildArtifact[] = [];

    try {
      // Validate project
      if (!this.validateProject(projectPath)) {
        errors.push('Invalid project structure');
        return {
          success: false,
          errors,
        };
      }

      // Create output directory
      const outDir = path.join(projectPath, this.config.outDir);
      this.ensureDirectory(outDir);

      // eslint-disable-next-line no-console
      console.log(`Starting desktop build for ${this.config.platform}...`);

      // Build based on framework
      if (this.config.framework === 'electron') {
        await this.buildWithElectron(projectPath, outDir, artifacts);
      } else {
        await this.buildWithTauri(projectPath, outDir, artifacts);
      }

      // Calculate stats
      const duration = Date.now() - startTime;
      const stats: BuildStats = {
        duration,
        totalSize: artifacts.reduce((sum, a) => sum + a.size, 0),
        artifactCount: artifacts.length,
      };

      // eslint-disable-next-line no-console
      console.log(`Build completed in ${duration}ms`);

      return {
        success: true,
        outputPath: outDir,
        artifacts,
        warnings,
        stats,
      };
    } catch (error) {
      errors.push(error instanceof Error ? error.message : String(error));
      return {
        success: false,
        errors,
      };
    }
  }

  /**
   * Validate project structure
   */
  private validateProject(projectPath: string): boolean {
    if (!fs.existsSync(projectPath)) {
      return false;
    }

    const requiredFiles = ['package.json'];
    return requiredFiles.every((file) =>
      fs.existsSync(path.join(projectPath, file))
    );
  }

  /**
   * Ensure directory exists
   */
  private ensureDirectory(dir: string): void {
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir, { recursive: true });
    }
  }

  /**
   * Build with Electron
   */
  private async buildWithElectron(
    _projectPath: string,
    outDir: string,
    artifacts: BuildArtifact[]
  ): Promise<void> {
    // eslint-disable-next-line no-console
    console.log('Building with Electron...');

    // 1. Create Electron main process file
    const mainContent = this.generateElectronMain();
    const mainPath = path.join(outDir, 'main.js');
    fs.writeFileSync(mainPath, mainContent);
    artifacts.push({
      type: 'js',
      path: mainPath,
      size: fs.statSync(mainPath).size,
    });

    // 2. Create package.json for Electron
    const packageJson = this.generateElectronPackageJson();
    const packagePath = path.join(outDir, 'package.json');
    fs.writeFileSync(packagePath, JSON.stringify(packageJson, null, 2));

    // 3. In a real implementation:
    // - Bundle renderer process with webpack/vite
    // - Package with electron-builder
    // - Sign the application
    // - Create installers

    // eslint-disable-next-line no-console
    console.log('Electron build completed');
  }

  /**
   * Build with Tauri
   */
  private async buildWithTauri(
    _projectPath: string,
    outDir: string,
    _artifacts: BuildArtifact[]
  ): Promise<void> {
    // eslint-disable-next-line no-console
    console.log('Building with Tauri...');

    // 1. Create Tauri config
    const tauriConfig = this.generateTauriConfig();
    const configPath = path.join(outDir, 'tauri.conf.json');
    fs.writeFileSync(configPath, JSON.stringify(tauriConfig, null, 2));

    // 2. In a real implementation:
    // - Build frontend with vite
    // - Compile Rust backend
    // - Package with tauri-cli
    // - Sign the application
    // - Create installers

    // eslint-disable-next-line no-console
    console.log('Tauri build completed');
  }

  /**
   * Generate Electron main process code
   */
  private generateElectronMain(): string {
    return `
const { app, BrowserWindow } = require('electron');
const path = require('path');

function createWindow() {
  const win = new BrowserWindow({
    width: 1280,
    height: 720,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js')
    }
  });

  win.loadFile('index.html');
  
  // Open DevTools in development
  if (process.env.NODE_ENV === 'development') {
    win.webContents.openDevTools();
  }
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
    `.trim();
  }

  /**
   * Generate Electron package.json
   */
  private generateElectronPackageJson() {
    return {
      name: this.config.appName.toLowerCase().replace(/\s+/g, '-'),
      version: this.config.appVersion,
      main: 'main.js',
      scripts: {
        start: 'electron .',
      },
      dependencies: {
        electron: '^27.0.0',
      },
    };
  }

  /**
   * Generate Tauri configuration
   */
  private generateTauriConfig() {
    return {
      build: {
        distDir: '../dist',
        devPath: 'http://localhost:3000',
        beforeDevCommand: 'npm run dev',
        beforeBuildCommand: 'npm run build',
      },
      package: {
        productName: this.config.appName,
        version: this.config.appVersion,
      },
      tauri: {
        allowlist: {
          all: false,
          window: {
            all: true,
          },
          fs: {
            readFile: true,
            writeFile: true,
          },
        },
        bundle: {
          active: true,
          targets: this.getTauriTargets(),
          identifier: `com.novaengine.${this.config.appName.toLowerCase().replace(/\s+/g, '')}`,
        },
        windows: [
          {
            title: this.config.appName,
            width: 1280,
            height: 720,
            resizable: true,
            fullscreen: false,
          },
        ],
      },
    };
  }

  /**
   * Get Tauri build targets for platform
   */
  private getTauriTargets(): string[] {
    switch (this.config.platform) {
      case DesktopPlatform.Windows:
        return ['msi', 'nsis'];
      case DesktopPlatform.MacOS:
        return ['dmg', 'app'];
      case DesktopPlatform.Linux:
        return ['deb', 'appimage'];
      default:
        return ['all'];
    }
  }
}
