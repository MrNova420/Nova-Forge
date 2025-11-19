/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

import { describe, it, expect, beforeEach, afterEach } from '@jest/globals';
import { DesktopPlatform } from '../DesktopPlatform';
import { PlatformType, OSType } from '../IPlatform';
import * as fs from 'fs/promises';
import * as path from 'path';
import * as os from 'os';

describe('DesktopPlatform', () => {
  let platform: DesktopPlatform;
  let testDir: string;

  beforeEach(async () => {
    platform = new DesktopPlatform();
    await platform.initialize();

    // Create a temporary test directory
    testDir = path.join(os.tmpdir(), 'nova-engine-test-' + Date.now());
    await fs.mkdir(testDir, { recursive: true });
  });

  afterEach(async () => {
    await platform.shutdown();

    // Clean up test directory
    try {
      await fs.rm(testDir, { recursive: true, force: true });
    } catch {
      // Ignore cleanup errors
    }
  });

  describe('Platform Properties', () => {
    it('should return correct platform type', () => {
      expect(platform.getType()).toBe(PlatformType.DESKTOP);
    });

    it('should return a valid OS type', () => {
      const osType = platform.getOS();
      expect([OSType.WINDOWS, OSType.MACOS, OSType.LINUX]).toContain(osType);
    });

    it('should return a version string', () => {
      const version = platform.getVersion();
      expect(version).toContain('Desktop/');
      expect(version).toContain('Node/');
    });
  });

  describe('Initialization', () => {
    it('should initialize successfully', async () => {
      const newPlatform = new DesktopPlatform();
      await expect(newPlatform.initialize()).resolves.not.toThrow();
      await newPlatform.shutdown();
    });

    it('should shutdown successfully', async () => {
      await expect(platform.shutdown()).resolves.not.toThrow();
    });
  });

  describe('File System', () => {
    it('should get platform paths', async () => {
      const userDataPath = await platform.fileSystem.getPath(
        'user_data' as any
      );
      expect(userDataPath).toContain('.nova-engine');

      const tempPath = await platform.fileSystem.getPath('temp' as any);
      expect(tempPath).toBeTruthy();
    });

    it('should write and read text files', async () => {
      const testPath = path.join(testDir, 'test.txt');
      const testContent = 'Hello, Desktop Platform!';

      await platform.fileSystem.writeText(testPath, testContent);

      const exists = await platform.fileSystem.exists(testPath);
      expect(exists).toBe(true);

      const content = await platform.fileSystem.readText(testPath);
      expect(content).toBe(testContent);
    });

    it('should write and read binary files', async () => {
      const testPath = path.join(testDir, 'test.bin');
      const testData = new Uint8Array([10, 20, 30, 40, 50]);

      await platform.fileSystem.writeBinary(testPath, testData.buffer);

      const data = await platform.fileSystem.readBinary(testPath);
      const readArray = new Uint8Array(data);

      expect(readArray).toEqual(testData);
    });

    it('should delete files', async () => {
      const testPath = path.join(testDir, 'test-delete.txt');
      await platform.fileSystem.writeText(testPath, 'delete me');

      let exists = await platform.fileSystem.exists(testPath);
      expect(exists).toBe(true);

      await platform.fileSystem.delete(testPath);

      exists = await platform.fileSystem.exists(testPath);
      expect(exists).toBe(false);
    });

    it('should list files in directory', async () => {
      // Create test files
      await platform.fileSystem.writeText(
        path.join(testDir, 'file1.txt'),
        'test1'
      );
      await platform.fileSystem.writeText(
        path.join(testDir, 'file2.txt'),
        'test2'
      );

      const files = await platform.fileSystem.listFiles(testDir);
      expect(files.length).toBe(2);
      expect(files.some((f) => f.includes('file1.txt'))).toBe(true);
      expect(files.some((f) => f.includes('file2.txt'))).toBe(true);
    });

    it('should create directories', async () => {
      const newDir = path.join(testDir, 'subdir', 'nested');
      await platform.fileSystem.createDirectory(newDir);

      const exists = await platform.fileSystem.exists(newDir);
      expect(exists).toBe(true);
    });

    it('should get file size', async () => {
      const testPath = path.join(testDir, 'test-size.txt');
      const testContent = 'test content';
      await platform.fileSystem.writeText(testPath, testContent);

      const size = await platform.fileSystem.getFileSize(testPath);
      expect(size).toBeGreaterThan(0);
      expect(size).toBe(Buffer.from(testContent).length);
    });

    it('should get modified time', async () => {
      const testPath = path.join(testDir, 'test-time.txt');
      await platform.fileSystem.writeText(testPath, 'test');

      const modTime = await platform.fileSystem.getModifiedTime(testPath);
      expect(typeof modTime.getTime).toBe('function');
      expect(modTime.getTime()).toBeLessThanOrEqual(Date.now());
    });

    it('should handle non-existent files correctly', async () => {
      const nonExistent = path.join(testDir, 'does-not-exist.txt');
      const exists = await platform.fileSystem.exists(nonExistent);
      expect(exists).toBe(false);

      await expect(platform.fileSystem.readText(nonExistent)).rejects.toThrow();
    });
  });

  describe('Threading', () => {
    it('should get core count', () => {
      const coreCount = platform.threading.getCoreCount();
      expect(coreCount).toBeGreaterThan(0);
      expect(coreCount).toBe(os.cpus().length);
    });

    it('should check if threading is supported', () => {
      const supported = platform.threading.isSupported();
      expect(typeof supported).toBe('boolean');
    });

    it('should schedule a job', async () => {
      let executed = false;
      const job = {
        execute: async () => {
          executed = true;
        },
        name: 'test-job',
      };

      await platform.threading.scheduleJob(job);
      expect(executed).toBe(true);
    });

    it('should schedule multiple jobs', async () => {
      let count = 0;
      const jobs = [
        {
          execute: async () => {
            count++;
          },
        },
        {
          execute: async () => {
            count++;
          },
        },
        {
          execute: async () => {
            count++;
          },
        },
      ];

      await platform.threading.scheduleJobs(jobs);
      expect(count).toBe(3);
    });

    it('should handle job errors gracefully', async () => {
      const job = {
        execute: async () => {
          throw new Error('Job failed');
        },
        name: 'failing-job',
      };

      await expect(platform.threading.scheduleJob(job)).rejects.toThrow(
        'Job failed'
      );
    });
  });

  describe('Network', () => {
    it('should check online status', () => {
      const isOnline = platform.network.isOnline();
      expect(typeof isOnline).toBe('boolean');
    });

    it('should create WebSocket', () => {
      const ws = platform.network.createWebSocket();
      expect(ws).toBeDefined();
      expect(ws.isConnected()).toBe(false);
    });
  });

  describe('Window', () => {
    it('should create window with options', async () => {
      await platform.window.create({
        title: 'Test Window',
        width: 800,
        height: 600,
        fullscreen: false,
        resizable: true,
      });

      expect(platform.window.getTitle()).toBe('Test Window');
      expect(platform.window.getSize()).toEqual({ width: 800, height: 600 });
    });

    it('should set and get window size', () => {
      platform.window.setSize(1024, 768);
      expect(platform.window.getSize()).toEqual({ width: 1024, height: 768 });
    });

    it('should set and get window title', () => {
      platform.window.setTitle('New Title');
      expect(platform.window.getTitle()).toBe('New Title');
    });

    it('should get pixel ratio', () => {
      const ratio = platform.window.getPixelRatio();
      expect(ratio).toBeGreaterThan(0);
    });

    it('should set callbacks', () => {
      const callbacks = {
        onResize: (width: number, height: number) => {
          expect(width).toBe(1920);
          expect(height).toBe(1080);
        },
      };

      platform.window.setCallbacks(callbacks);
      platform.window.setSize(1920, 1080);
    });
  });

  describe('Timing', () => {
    it('should get current time', () => {
      const now = platform.timing.now();
      expect(now).toBeGreaterThan(0);
    });

    it('should get performance time', () => {
      const perfTime = platform.timing.performance();
      expect(perfTime).toBeGreaterThan(0);
    });

    it('should measure time accurately', () => {
      const start = platform.timing.performance();
      // Do some work
      for (let i = 0; i < 1000; i++) {
        Math.sqrt(i);
      }
      const end = platform.timing.performance();
      expect(end).toBeGreaterThan(start);
    });

    it('should sleep for specified time', async () => {
      const start = Date.now();
      await platform.timing.sleep(100);
      const elapsed = Date.now() - start;
      expect(elapsed).toBeGreaterThanOrEqual(95); // Allow some margin
    });

    it('should set and clear interval', (done) => {
      let count = 0;
      const handle = platform.timing.setInterval(() => {
        count++;
        if (count === 3) {
          platform.timing.clearInterval(handle);
          expect(count).toBe(3);
          done();
        }
      }, 10);
    });

    it('should set and clear timeout', (done) => {
      let executed = false;
      const handle = platform.timing.setTimeout(() => {
        executed = true;
        expect(executed).toBe(true);
        done();
      }, 10);

      expect(handle).toBeDefined();
    });
  });

  describe('Integration', () => {
    it('should support full file read/write workflow', async () => {
      const testPath = path.join(testDir, 'workflow-test.json');
      const testData = { name: 'Nova Engine', version: '1.0', active: true };

      // Write JSON data
      await platform.fileSystem.writeText(testPath, JSON.stringify(testData));

      // Verify exists
      expect(await platform.fileSystem.exists(testPath)).toBe(true);

      // Read and parse
      const content = await platform.fileSystem.readText(testPath);
      const parsed = JSON.parse(content);

      expect(parsed).toEqual(testData);

      // Get metadata
      const size = await platform.fileSystem.getFileSize(testPath);
      expect(size).toBeGreaterThan(0);

      const modTime = await platform.fileSystem.getModifiedTime(testPath);
      expect(typeof modTime.getTime).toBe('function');
      expect(modTime.getTime()).toBeGreaterThan(0);

      // Clean up
      await platform.fileSystem.delete(testPath);
      expect(await platform.fileSystem.exists(testPath)).toBe(false);
    });
  });
});
