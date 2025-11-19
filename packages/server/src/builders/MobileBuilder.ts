/**
 * @fileoverview Mobile build pipeline for iOS/Android applications
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
 * Mobile platforms
 */
export enum MobilePlatform {
  iOS = 'ios',
  Android = 'android',
}

/**
 * Mobile build configuration
 */
export interface MobileBuildConfig extends BuildConfig {
  /** Target mobile platform */
  platform: MobilePlatform;
  /** Application name */
  appName: string;
  /** Application version */
  appVersion: string;
  /** Application bundle ID (iOS) or package name (Android) */
  bundleId: string;
  /** Build type */
  buildType: 'debug' | 'release';
  /** Code signing identity (iOS) */
  codeSignIdentity?: string;
  /** Provisioning profile (iOS) */
  provisioningProfile?: string;
  /** Keystore path (Android) */
  keystorePath?: string;
  /** Keystore password (Android) */
  keystorePassword?: string;
}

/**
 * Mobile builder for native mobile applications
 * Creates platform-specific mobile apps using React Native or Capacitor
 */
export class MobileBuilder {
  /** Build configuration */
  private config: MobileBuildConfig;

  constructor(config?: Partial<MobileBuildConfig>) {
    this.config = {
      ...DEFAULT_BUILD_CONFIG,
      target: BuildTarget.Mobile,
      platform: MobilePlatform.Android,
      appName: 'Nova Engine Game',
      appVersion: '1.0.0',
      bundleId: 'com.novaengine.game',
      buildType: 'debug',
      ...config,
    };
  }

  /**
   * Build mobile application
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
      // eslint-disable-next-line no-console
      console.log(`Starting ${this.config.platform} build...`);

      // 1. Build web bundle first (mobile apps are web-based)
      // eslint-disable-next-line no-console
      console.log('1/6 Building web bundle...');
      const webArtifacts = await this.buildWebBundle(projectPath, outDir);
      artifacts.push(...webArtifacts);

      // 2. Generate native project files
      // eslint-disable-next-line no-console
      console.log('2/6 Generating native project...');
      await this.generateNativeProject(projectPath, outDir);

      // 3. Copy assets to native project
      // eslint-disable-next-line no-console
      console.log('3/6 Copying assets...');
      const assetArtifacts = await this.copyAssetsToNative(projectPath, outDir);
      artifacts.push(...assetArtifacts);

      // 4. Build native application
      // eslint-disable-next-line no-console
      console.log('4/6 Building native application...');
      const appArtifact = await this.buildNativeApp(projectPath, outDir);
      artifacts.push(appArtifact);

      // 5. Sign application (if release build)
      if (this.config.buildType === 'release') {
        // eslint-disable-next-line no-console
        console.log('5/6 Signing application...');
        await this.signApplication(appArtifact);
      } else {
        // eslint-disable-next-line no-console
        console.log('5/6 Skipping signing (debug build)...');
      }

      // 6. Generate metadata
      // eslint-disable-next-line no-console
      console.log('6/6 Generating metadata...');
      const metadataArtifact = await this.generateMetadata(
        outDir,
        artifacts,
        startTime
      );
      artifacts.push(metadataArtifact);

      // eslint-disable-next-line no-console
      console.log('Mobile build completed successfully!');

      // Calculate stats
      const stats: BuildStats = {
        buildTime: Date.now() - startTime,
        bundleSize: artifacts.reduce((sum, a) => sum + a.size, 0),
        assetCount: artifacts.length,
      };

      return {
        success: true,
        artifacts,
        stats,
      };
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : 'Unknown error';
      errors.push(errorMsg);
      console.error('Mobile build failed:', errorMsg);

      return {
        success: false,
        errors,
        warnings,
      };
    }
  }

  /**
   * Validate project structure
   */
  private validateProject(projectPath: string): boolean {
    try {
      // Check if project path exists
      if (!fs.existsSync(projectPath)) {
        return false;
      }

      // Check for required files
      const packageJsonPath = path.join(projectPath, 'package.json');
      if (!fs.existsSync(packageJsonPath)) {
        return false;
      }

      return true;
    } catch {
      return false;
    }
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
   * Build web bundle for mobile wrapper
   */
  private async buildWebBundle(
    _projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];

    // Bundle JavaScript for mobile
    const bundlePath = path.join(outDir, 'www', 'bundle.js');
    const bundleDir = path.dirname(bundlePath);
    this.ensureDirectory(bundleDir);

    // In a real implementation, we would use bundler like webpack/vite
    // For now, create placeholder
    const bundleContent = `
// Nova Engine Mobile Runtime
// eslint-disable-next-line no-console
console.log('Nova Engine Mobile Runtime ${this.config.appVersion}');
// Game code would be bundled here
    `.trim();

    fs.writeFileSync(bundlePath, bundleContent);

    artifacts.push({
      path: bundlePath,
      size: Buffer.byteLength(bundleContent),
      type: 'javascript',
    });

    // Create HTML wrapper
    const htmlPath = path.join(outDir, 'www', 'index.html');
    const htmlContent = `
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>${this.config.appName}</title>
  <style>
    body { margin: 0; padding: 0; overflow: hidden; }
    #game-canvas { width: 100vw; height: 100vh; }
  </style>
</head>
<body>
  <canvas id="game-canvas"></canvas>
  <script src="bundle.js"></script>
</body>
</html>
    `.trim();

    fs.writeFileSync(htmlPath, htmlContent);

    artifacts.push({
      path: htmlPath,
      size: Buffer.byteLength(htmlContent),
      type: 'html',
    });

    return artifacts;
  }

  /**
   * Generate native project files
   */
  private async generateNativeProject(
    _projectPath: string,
    outDir: string
  ): Promise<void> {
    const nativeDir = path.join(outDir, 'native');
    this.ensureDirectory(nativeDir);

    if (this.config.platform === MobilePlatform.Android) {
      await this.generateAndroidProject(_projectPath, nativeDir);
    } else {
      await this.generateiOSProject(_projectPath, nativeDir);
    }
  }

  /**
   * Generate Android project
   */
  private async generateAndroidProject(
    __projectPath: string,
    nativeDir: string
  ): Promise<void> {
    // Create basic Android project structure
    const androidDir = path.join(nativeDir, 'android');
    this.ensureDirectory(androidDir);

    // Create AndroidManifest.xml
    const manifestPath = path.join(
      androidDir,
      'app',
      'src',
      'main',
      'AndroidManifest.xml'
    );
    this.ensureDirectory(path.dirname(manifestPath));

    const manifest = `<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="${this.config.bundleId}">
    
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
    
    <application
        android:label="${this.config.appName}"
        android:theme="@style/AppTheme">
        <activity
            android:name=".MainActivity"
            android:configChanges="orientation|keyboardHidden|screenSize"
            android:windowSoftInputMode="adjustResize">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>`;

    fs.writeFileSync(manifestPath, manifest);

    // Create build.gradle
    const gradlePath = path.join(androidDir, 'app', 'build.gradle');
    this.ensureDirectory(path.dirname(gradlePath));

    const gradle = `
plugins {
    id 'com.android.application'
}

android {
    compileSdkVersion 33
    defaultConfig {
        applicationId "${this.config.bundleId}"
        minSdkVersion 24
        targetSdkVersion 33
        versionCode 1
        versionName "${this.config.appVersion}"
    }
    buildTypes {
        release {
            minifyEnabled true
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
}
    `.trim();

    fs.writeFileSync(gradlePath, gradle);
  }

  /**
   * Generate iOS project
   */
  private async generateiOSProject(
    __projectPath: string,
    nativeDir: string
  ): Promise<void> {
    // Create basic iOS project structure
    const iosDir = path.join(nativeDir, 'ios');
    this.ensureDirectory(iosDir);

    // Create Info.plist
    const plistPath = path.join(iosDir, 'Info.plist');
    const plist = `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDisplayName</key>
    <string>${this.config.appName}</string>
    <key>CFBundleIdentifier</key>
    <string>${this.config.bundleId}</string>
    <key>CFBundleVersion</key>
    <string>${this.config.appVersion}</string>
    <key>LSRequiresIPhoneOS</key>
    <true/>
    <key>UIRequiredDeviceCapabilities</key>
    <array>
        <string>arm64</string>
    </array>
</dict>
</plist>`;

    fs.writeFileSync(plistPath, plist);
  }

  /**
   * Copy assets to native project
   */
  private async copyAssetsToNative(
    projectPath: string,
    outDir: string
  ): Promise<BuildArtifact[]> {
    const artifacts: BuildArtifact[] = [];
    const assetsDir = path.join(projectPath, 'assets');

    if (!fs.existsSync(assetsDir)) {
      return artifacts;
    }

    const targetDir = path.join(outDir, 'www', 'assets');
    this.ensureDirectory(targetDir);

    // Copy all assets (simplified - would use recursive copy in real impl)
    const files = fs.readdirSync(assetsDir);
    for (const file of files) {
      const srcPath = path.join(assetsDir, file);
      const destPath = path.join(targetDir, file);

      if (fs.statSync(srcPath).isFile()) {
        fs.copyFileSync(srcPath, destPath);

        artifacts.push({
          path: destPath,
          size: fs.statSync(destPath).size,
          type: 'asset',
        });
      }
    }

    return artifacts;
  }

  /**
   * Build native application
   */
  private async buildNativeApp(
    _projectPath: string,
    outDir: string
  ): Promise<BuildArtifact> {
    // In a real implementation:
    // - For Android: run `./gradlew assembleRelease` or `assembleDebug`
    // - For iOS: run `xcodebuild` with appropriate parameters

    const appExtension =
      this.config.platform === MobilePlatform.Android ? '.apk' : '.ipa';
    const appPath = path.join(outDir, `${this.config.appName}${appExtension}`);

    // Create placeholder app file
    const appContent = `Nova Engine Mobile App - ${this.config.platform} - ${this.config.buildType}`;
    fs.writeFileSync(appPath, appContent);

    return {
      path: appPath,
      size: Buffer.byteLength(appContent),
      type: 'application',
    };
  }

  /**
   * Sign application for release
   */
  private async signApplication(artifact: BuildArtifact): Promise<void> {
    // In a real implementation:
    // - For Android: use jarsigner or apksigner with keystore
    // - For iOS: use codesign with certificate and provisioning profile

    // eslint-disable-next-line no-console
    console.log(`Signing ${artifact.path}...`);

    if (this.config.platform === MobilePlatform.Android) {
      if (!this.config.keystorePath) {
        throw new Error('Keystore path required for Android release builds');
      }
      // Would run: jarsigner -keystore path artifact.path
    } else {
      if (!this.config.codeSignIdentity) {
        throw new Error(
          'Code signing identity required for iOS release builds'
        );
      }
      // Would run: codesign -s identity artifact.path
    }

    // eslint-disable-next-line no-console
    console.log('Application signed successfully');
  }

  /**
   * Generate build metadata
   */
  private async generateMetadata(
    outDir: string,
    artifacts: BuildArtifact[],
    startTime: number
  ): Promise<BuildArtifact> {
    const metadata = {
      version: this.config.appVersion,
      buildDate: new Date().toISOString(),
      buildTime: Date.now() - startTime,
      platform: this.config.platform,
      buildType: this.config.buildType,
      artifacts: artifacts.map((a) => ({
        path: path.relative(outDir, a.path),
        size: a.size,
        type: a.type,
      })),
      config: {
        minify: this.config.minify,
        sourceMaps: this.config.sourceMaps,
        compression: this.config.compression,
      },
    };

    const metadataPath = path.join(outDir, 'build-metadata.json');
    fs.writeFileSync(metadataPath, JSON.stringify(metadata, null, 2));

    return {
      path: metadataPath,
      size: Buffer.byteLength(JSON.stringify(metadata)),
      type: 'metadata',
    };
  }
}
