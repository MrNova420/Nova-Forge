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
   * Bundle scripts using a build system approach
   */
  private async bundleScripts(
    projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    // Find entry point
    const entryPoint = this.findEntryPoint(projectPath);
    if (!entryPoint) {
      throw new Error('No entry point found (index.ts/js, main.ts/js, or src/index.ts/js)');
    }

    // Read and bundle entry point
    const content = fs.readFileSync(entryPoint, 'utf-8');
    
    // Simple bundling: wrap in IIFE with imports resolved
    const bundledContent = this.createBundle(content, projectPath, entryPoint);
    
    // Main bundle
    const mainBundle: BuildArtifact = {
      type: 'js',
      path: path.join(outDir, 'main.js'),
      size: 0,
      gzipSize: 0,
    };

    fs.writeFileSync(mainBundle.path, bundledContent);
    mainBundle.size = fs.statSync(mainBundle.path).size;
    mainBundle.gzipSize = Math.floor(mainBundle.size * 0.3); // Estimate
    artifacts.push(mainBundle);

    // Vendor bundle for Nova Engine runtime
    const vendorBundle = this.createVendorBundle(outDir);
    artifacts.push(vendorBundle);

    return artifacts;
  }

  /**
   * Find entry point file
   */
  private findEntryPoint(projectPath: string): string | null {
    const candidates = [
      'index.ts',
      'index.js',
      'main.ts',
      'main.js',
      'src/index.ts',
      'src/index.js',
      'src/main.ts',
      'src/main.js',
    ];

    for (const candidate of candidates) {
      const fullPath = path.join(projectPath, candidate);
      if (fs.existsSync(fullPath)) {
        return fullPath;
      }
    }

    return null;
  }

  /**
   * Create bundled JavaScript
   */
  private createBundle(content: string, projectPath: string, entryPath: string): string {
    // Strip TypeScript types (basic approach)
    let processed = content
      .replace(/: \w+(\[\])?/g, '') // Remove type annotations
      .replace(/interface \w+ \{[^}]+\}/g, '') // Remove interfaces
      .replace(/type \w+ = [^;]+;/g, ''); // Remove type aliases

    // Wrap in IIFE
    return `
(function() {
  'use strict';
  
  // Nova Engine Game Bundle
  // Built: ${new Date().toISOString()}
  
  ${processed}
  
})();
    `.trim();
  }

  /**
   * Create vendor bundle with Nova Engine runtime
   */
  private createVendorBundle(outDir: string): BuildArtifact {
    const vendorContent = `
// Nova Engine Runtime - Vendor Bundle
(function(window) {
  'use strict';
  
  // Engine runtime stubs
  window.NovaEngine = {
    init: function(canvas) {
      console.log('Nova Engine initialized');
      return {
        start: function() { console.log('Game started'); },
        stop: function() { console.log('Game stopped'); },
        update: function(dt) { /* update loop */ },
        render: function() { /* render loop */ }
      };
    },
    VERSION: '1.0.0'
  };
  
})(window);
    `.trim();

    const vendorPath = path.join(outDir, 'vendor.js');
    fs.writeFileSync(vendorPath, vendorContent);

    return {
      type: 'js',
      path: vendorPath,
      size: fs.statSync(vendorPath).size,
      gzipSize: Math.floor(fs.statSync(vendorPath).size * 0.3),
    };
  }

  /**
   * Process and optimize assets
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

    // Process each asset type
    const files = this.getAllFiles(assetsDir);
    
    for (const file of files) {
      const relativePath = path.relative(assetsDir, file);
      const outPath = path.join(outAssetsDir, relativePath);
      
      // Ensure output directory exists
      this.ensureDirectory(path.dirname(outPath));
      
      // Copy file (in real implementation, would optimize based on type)
      fs.copyFileSync(file, outPath);
      
      artifacts.push({
        type: 'asset',
        path: outPath,
        size: fs.statSync(outPath).size,
      });
    }

    return artifacts;
  }

  /**
   * Get all files in directory recursively
   */
  private getAllFiles(dir: string): string[] {
    const files: string[] = [];
    
    const items = fs.readdirSync(dir);
    for (const item of items) {
      const fullPath = path.join(dir, item);
      const stat = fs.statSync(fullPath);
      
      if (stat.isDirectory()) {
        files.push(...this.getAllFiles(fullPath));
      } else {
        files.push(fullPath);
      }
    }
    
    return files;
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
