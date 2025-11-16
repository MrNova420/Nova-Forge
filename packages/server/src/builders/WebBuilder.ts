/**
 * @fileoverview Web build pipeline for HTML5/WebGL games
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
 * Web builder for HTML5/WebGL games
 * Creates optimized web builds with all necessary assets
 */
export class WebBuilder {
  /** Build configuration */
  private config: BuildConfig;

  constructor(config?: Partial<BuildConfig>) {
    this.config = {
      ...DEFAULT_BUILD_CONFIG,
      target: BuildTarget.Web,
      ...config,
    };
  }

  /**
   * Build web application
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

      // Build steps
      console.log('Starting web build...');

      // 1. Bundle JavaScript/TypeScript
      console.log('1/6 Bundling scripts...');
      const scriptArtifacts = await this.bundleScripts(projectPath, outDir);
      artifacts.push(...scriptArtifacts);

      // 2. Process assets
      console.log('2/6 Processing assets...');
      const assetArtifacts = await this.processAssets(projectPath, outDir);
      artifacts.push(...assetArtifacts);

      // 3. Generate HTML
      console.log('3/6 Generating HTML...');
      const htmlArtifact = await this.generateHTML(outDir, scriptArtifacts);
      artifacts.push(htmlArtifact);

      // 4. Generate service worker (if enabled)
      if (this.config.serviceWorker) {
        console.log('4/6 Generating service worker...');
        const swArtifact = await this.generateServiceWorker(outDir, artifacts);
        artifacts.push(swArtifact);
      } else {
        console.log('4/6 Skipping service worker...');
      }

      // 5. Compress assets (if enabled)
      if (this.config.compressAssets) {
        console.log('5/6 Compressing assets...');
        await this.compressAssets(artifacts);
      } else {
        console.log('5/6 Skipping compression...');
      }

      // 6. Generate manifest
      console.log('6/6 Generating manifest...');
      const manifestArtifact = await this.generateManifest(outDir);
      artifacts.push(manifestArtifact);

      // Calculate stats
      const duration = Date.now() - startTime;
      const stats: BuildStats = {
        duration,
        totalSize: artifacts.reduce((sum, a) => sum + a.size, 0),
        artifactCount: artifacts.length,
      };

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
    // Check if project directory exists
    if (!fs.existsSync(projectPath)) {
      return false;
    }

    // Check for required files (package.json, entry point, etc.)
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
   * Bundle scripts
   */
  private async bundleScripts(
    _projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    // In a real implementation, this would use Vite/Rollup/Webpack
    // For now, we'll create placeholder artifacts
    const artifacts: BuildArtifact[] = [];

    // Main bundle
    const mainBundle: BuildArtifact = {
      type: 'js',
      path: path.join(outDir, 'main.js'),
      size: 0,
      gzipSize: 0,
    };

    // Create placeholder file
    const content = `
// Nova Engine - Web Build
console.log('Nova Engine initialized');
    `.trim();

    fs.writeFileSync(mainBundle.path, content);
    mainBundle.size = fs.statSync(mainBundle.path).size;
    artifacts.push(mainBundle);

    return artifacts;
  }

  /**
   * Process assets
   */
  private async processAssets(
    projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];
    const assetsDir = path.join(projectPath, 'assets');

    // Check if assets directory exists
    if (!fs.existsSync(assetsDir)) {
      return artifacts;
    }

    // Copy assets to output directory
    const outAssetsDir = path.join(outDir, 'assets');
    this.ensureDirectory(outAssetsDir);

    // In a real implementation, this would:
    // - Optimize images
    // - Convert audio formats
    // - Compress models
    // - Generate atlases

    return artifacts;
  }

  /**
   * Generate HTML entry point
   */
  private async generateHTML(
    outDir: string,
    scripts: BuildArtifact[]
  ): Promise<BuildArtifact> {
    const html = `
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Nova Engine Game</title>
  <link rel="manifest" href="/manifest.json">
  <meta name="theme-color" content="#3b82f6">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      width: 100vw;
      height: 100vh;
      overflow: hidden;
      background: #000;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    canvas {
      display: block;
      max-width: 100%;
      max-height: 100%;
    }
    #loading {
      color: #fff;
      font-family: system-ui;
      text-align: center;
    }
  </style>
</head>
<body>
  <div id="loading">Loading...</div>
  <canvas id="game-canvas" style="display: none;"></canvas>
  ${scripts.map((s) => `<script src="${path.basename(s.path)}"></script>`).join('\\n  ')}
  <script>
    // Register service worker
    if ('serviceWorker' in navigator) {
      navigator.serviceWorker.register('/service-worker.js')
        .then(reg => console.log('Service worker registered'))
        .catch(err => console.error('Service worker registration failed:', err));
    }
  </script>
</body>
</html>
    `.trim();

    const htmlPath = path.join(outDir, 'index.html');
    fs.writeFileSync(htmlPath, html);

    return {
      type: 'html',
      path: htmlPath,
      size: fs.statSync(htmlPath).size,
    };
  }

  /**
   * Generate service worker
   */
  private async generateServiceWorker(
    outDir: string,
    artifacts: BuildArtifact[]
  ): Promise<BuildArtifact> {
    const cacheAssets = artifacts
      .filter((a) => a.type === 'js' || a.type === 'css' || a.type === 'html')
      .map((a) => '/' + path.basename(a.path));

    const sw = `
const CACHE_NAME = 'nova-engine-v1';
const ASSETS_TO_CACHE = ${JSON.stringify(cacheAssets, null, 2)};

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME)
      .then((cache) => cache.addAll(ASSETS_TO_CACHE))
  );
});

self.addEventListener('fetch', (event) => {
  event.respondWith(
    caches.match(event.request)
      .then((response) => response || fetch(event.request))
  );
});
    `.trim();

    const swPath = path.join(outDir, 'service-worker.js');
    fs.writeFileSync(swPath, sw);

    return {
      type: 'js',
      path: swPath,
      size: fs.statSync(swPath).size,
    };
  }

  /**
   * Compress assets
   */
  private async compressAssets(artifacts: BuildArtifact[]): Promise<void> {
    // In a real implementation, this would use gzip/brotli compression
    // For now, we'll just simulate by calculating gzip size estimates
    for (const artifact of artifacts) {
      if (artifact.type === 'js' || artifact.type === 'html') {
        // Rough estimate: 30% of original size
        artifact.gzipSize = Math.floor(artifact.size * 0.3);
      }
    }
  }

  /**
   * Generate web app manifest
   */
  private async generateManifest(outDir: string): Promise<BuildArtifact> {
    const manifest = {
      name: 'Nova Engine Game',
      short_name: 'Nova Game',
      start_url: '/',
      display: 'fullscreen',
      orientation: 'landscape',
      background_color: '#000000',
      theme_color: '#3b82f6',
      icons: [
        {
          src: '/icon-192.png',
          sizes: '192x192',
          type: 'image/png',
        },
        {
          src: '/icon-512.png',
          sizes: '512x512',
          type: 'image/png',
        },
      ],
    };

    const manifestPath = path.join(outDir, 'manifest.json');
    fs.writeFileSync(manifestPath, JSON.stringify(manifest, null, 2));

    return {
      type: 'asset',
      path: manifestPath,
      size: fs.statSync(manifestPath).size,
    };
  }
}
