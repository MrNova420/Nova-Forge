/**
 * Platform Integration Tests
 * 
 * Cross-platform integration testing to ensure consistent behavior
 * across Web, Desktop, and Mobile implementations.
 */

import { describe, it, expect, beforeEach, afterEach } from '@jest/globals';
import { WebPlatform } from '../WebPlatform';
import { PlatformManager } from '../PlatformManager';
import type { IPlatform, IWindow } from '../IPlatform';

describe('Platform Integration Tests', () => {
  let platform: IPlatform;

  beforeEach(async () => {
    platform = new WebPlatform();
    await platform.initialize();
  });

  afterEach(async () => {
    await platform.shutdown();
  });

  describe('Multi-Window Management', () => {
    it('should manage multiple windows independently', async () => {
      const window1 = platform.createWindow({
        title: 'Window 1',
        width: 800,
        height: 600,
      });

      const window2 = platform.createWindow({
        title: 'Window 2',
        width: 1024,
        height: 768,
      });

      const window3 = platform.createWindow({
        title: 'Window 3',
        width: 640,
        height: 480,
      });

      // Verify all windows exist
      const windows = platform.getWindows();
      expect(windows).toHaveLength(3);
      expect(windows).toContain(window1);
      expect(windows).toContain(window2);
      expect(windows).toContain(window3);

      // Modify one window shouldn't affect others
      window1.setSize(1920, 1080);
      expect(window1.getSize()).toEqual({ width: 1920, height: 1080 });
      expect(window2.getSize()).toEqual({ width: 1024, height: 768 });
      expect(window3.getSize()).toEqual({ width: 640, height: 480 });

      // Destroy one window shouldn't affect others
      window2.destroy();
      expect(platform.getWindows()).toHaveLength(2);
      expect(platform.getWindows()).toContain(window1);
      expect(platform.getWindows()).toContain(window3);

      // Cleanup
      window1.destroy();
      window3.destroy();
    });

    it('should maintain main window correctly', () => {
      expect(platform.getMainWindow()).toBeNull();

      const window1 = platform.createWindow({ title: 'First' });
      expect(platform.getMainWindow()).toBe(window1);

      const window2 = platform.createWindow({ title: 'Second' });
      expect(platform.getMainWindow()).toBe(window1); // First window is still main

      window1.destroy();
      expect(platform.getMainWindow()).toBe(window2); // Second becomes main

      window2.destroy();
      expect(platform.getMainWindow()).toBeNull();
    });
  });

  describe('File System Integration', () => {
    it('should handle concurrent file operations', async () => {
      const fs = platform.getFileSystem();
      const encoder = new TextEncoder();

      // Write multiple files concurrently
      const writePromises = [];
      for (let i = 0; i < 10; i++) {
        const data = encoder.encode(`File ${i} content`);
        writePromises.push(fs.writeFile(`/concurrent/file_${i}.txt`, data));
      }
      await Promise.all(writePromises);

      // Read all files concurrently
      const readPromises = [];
      for (let i = 0; i < 10; i++) {
        readPromises.push(fs.readFile(`/concurrent/file_${i}.txt`));
      }
      const results = await Promise.all(readPromises);

      // Verify all files
      const decoder = new TextDecoder();
      for (let i = 0; i < 10; i++) {
        expect(decoder.decode(results[i])).toBe(`File ${i} content`);
      }
    });

    it('should handle nested directory operations', async () => {
      const fs = platform.getFileSystem();
      const encoder = new TextEncoder();

      // Create nested structure
      await fs.writeFile('/nested/level1/level2/file.txt', encoder.encode('deep file'));
      await fs.writeFile('/nested/level1/file.txt', encoder.encode('level 1 file'));
      await fs.writeFile('/nested/file.txt', encoder.encode('root level file'));

      // Verify all exist
      expect(await fs.exists('/nested/level1/level2/file.txt')).toBe(true);
      expect(await fs.exists('/nested/level1/file.txt')).toBe(true);
      expect(await fs.exists('/nested/file.txt')).toBe(true);

      // Read nested file
      const data = await fs.readFile('/nested/level1/level2/file.txt');
      expect(new TextDecoder().decode(data)).toBe('deep file');
    });

    it('should handle file operations with large data', async () => {
      const fs = platform.getFileSystem();
      
      // Create 5MB file
      const largeData = new Uint8Array(5 * 1024 * 1024);
      for (let i = 0; i < largeData.length; i++) {
        largeData[i] = i % 256;
      }

      await fs.writeFile('/large/bigfile.bin', largeData);

      const readData = await fs.readFile('/large/bigfile.bin');
      expect(readData.length).toBe(largeData.length);

      // Verify data integrity
      let matches = true;
      for (let i = 0; i < 1000; i++) { // Sample check
        if (readData[i] !== largeData[i]) {
          matches = false;
          break;
        }
      }
      expect(matches).toBe(true);
    });
  });

  describe('Worker Integration', () => {
    it('should handle worker communication patterns', async () => {
      const worker = platform.createWorker(`
        let counter = 0;
        self.onmessage = (e) => {
          if (e.data.command === 'increment') {
            counter++;
            self.postMessage({ counter });
          } else if (e.data.command === 'get') {
            self.postMessage({ counter });
          }
        };
      `);

      // Send multiple increments
      for (let i = 0; i < 5; i++) {
        worker.postMessage({ command: 'increment' });
      }

      // Wait a bit for processing
      await platform.sleep(100);

      // Get final count
      const resultPromise = new Promise<number>((resolve) => {
        worker.onMessage((data: any) => {
          resolve(data.counter);
        });
        worker.postMessage({ command: 'get' });
      });

      const finalCount = await resultPromise;
      expect(finalCount).toBe(5);

      worker.terminate();
    });

    it('should handle multiple workers concurrently', async () => {
      const workerCount = 4;
      const workers = [];

      for (let i = 0; i < workerCount; i++) {
        workers.push(platform.createWorker(`
          self.onmessage = (e) => {
            const { id, value } = e.data;
            let result = 0;
            for (let i = 0; i < value; i++) {
              result += i;
            }
            self.postMessage({ id, result });
          };
        `));
      }

      // Send work to all workers
      const promises = workers.map((worker, idx) => {
        return new Promise<number>((resolve) => {
          worker.onMessage((data: any) => {
            resolve(data.result);
          });
          worker.postMessage({ id: idx, value: 100000 });
        });
      });

      const results = await Promise.all(promises);
      expect(results).toHaveLength(workerCount);

      // All should have same result (sum of 0 to 99999)
      const expected = (100000 * 99999) / 2;
      results.forEach(result => {
        expect(result).toBe(expected);
      });

      // Cleanup
      workers.forEach(w => w.terminate());
    });
  });

  describe('Memory and Resource Management', () => {
    it('should clean up resources on shutdown', async () => {
      // Create various resources
      const window1 = platform.createWindow({ title: 'Test 1' });
      const window2 = platform.createWindow({ title: 'Test 2' });
      
      const fs = platform.getFileSystem();
      await fs.writeFile('/test/file1.txt', new TextEncoder().encode('data'));
      await fs.writeFile('/test/file2.txt', new TextEncoder().encode('data'));

      const worker1 = platform.createWorker('console.log("test")');
      const worker2 = platform.createWorker('console.log("test")');

      // Verify resources exist
      expect(platform.getWindows()).toHaveLength(2);

      // Shutdown
      await platform.shutdown();

      // Resources should be cleaned up
      expect(platform.getWindows()).toHaveLength(0);
      expect(platform.getMainWindow()).toBeNull();
    });

    it('should handle resource leak scenarios', async () => {
      const windows = [];
      
      // Create many windows
      for (let i = 0; i < 20; i++) {
        windows.push(platform.createWindow({
          title: `Window ${i}`,
          width: 100,
          height: 100,
        }));
      }

      expect(platform.getWindows()).toHaveLength(20);

      // Destroy half
      for (let i = 0; i < 10; i++) {
        windows[i].destroy();
      }

      expect(platform.getWindows()).toHaveLength(10);

      // Verify remaining windows are still functional
      const remaining = platform.getWindows();
      remaining[0].setSize(200, 200);
      expect(remaining[0].getSize()).toEqual({ width: 200, height: 200 });

      // Cleanup
      remaining.forEach(w => w.destroy());
    });
  });

  describe('Error Recovery', () => {
    it('should recover from file system errors', async () => {
      const fs = platform.getFileSystem();

      // Try to read non-existent file
      await expect(fs.readFile('/nonexistent/file.txt')).rejects.toThrow();

      // Platform should still be functional
      const data = new TextEncoder().encode('recovery test');
      await fs.writeFile('/recovery/test.txt', data);
      
      const readData = await fs.readFile('/recovery/test.txt');
      expect(new TextDecoder().decode(readData)).toBe('recovery test');
    });

    it('should handle worker errors gracefully', async () => {
      const worker = platform.createWorker(`
        self.onmessage = (e) => {
          if (e.data.crash) {
            throw new Error('Intentional crash');
          } else {
            self.postMessage({ ok: true });
          }
        };
      `);

      // Send crashing message
      worker.postMessage({ crash: true });

      // Wait a bit
      await platform.sleep(100);

      // Platform should still be functional
      const newWorker = platform.createWorker(`
        self.onmessage = (e) => {
          self.postMessage({ result: e.data.value * 2 });
        };
      `);

      const result = await new Promise<number>((resolve) => {
        newWorker.onMessage((data: any) => {
          resolve(data.result);
        });
        newWorker.postMessage({ value: 21 });
      });

      expect(result).toBe(42);

      worker.terminate();
      newWorker.terminate();
    });
  });

  describe('Platform Capabilities', () => {
    it('should correctly report capabilities', () => {
      const info = platform.getInfo();
      const capabilities = platform.getCapabilities();

      expect(info.name).toBeDefined();
      expect(info.version).toBeDefined();
      expect(info.os).toBeDefined();

      expect(typeof capabilities.supportsWebGL).toBe('boolean');
      expect(typeof capabilities.supportsWebGPU).toBe('boolean');
      expect(typeof capabilities.supportsWebAudio).toBe('boolean');
      expect(typeof capabilities.supportsTouch).toBe('boolean');
      expect(typeof capabilities.supportsGamepad).toBe('boolean');
    });

    it('should respect hardware concurrency', () => {
      const concurrency = platform.getHardwareConcurrency();
      expect(concurrency).toBeGreaterThan(0);
      expect(concurrency).toBeLessThanOrEqual(navigator.hardwareConcurrency || 8);
    });
  });

  describe('Event Handling', () => {
    it('should handle platform-level events', async () => {
      const events: string[] = [];

      platform.addEventListener('test-event', (data: any) => {
        events.push(data.message);
      });

      platform.dispatchEvent('test-event', { message: 'event 1' });
      platform.dispatchEvent('test-event', { message: 'event 2' });
      platform.dispatchEvent('test-event', { message: 'event 3' });

      // Events should be captured
      await platform.sleep(50);
      expect(events).toEqual(['event 1', 'event 2', 'event 3']);

      platform.removeEventListener('test-event');
    });

    it('should handle window-level events', () => {
      const window = platform.createWindow({ title: 'Event Test' });
      const events: string[] = [];

      window.addEventListener('resize', () => {
        events.push('resize');
      });

      window.setSize(1024, 768);
      window.setSize(800, 600);

      expect(events.length).toBeGreaterThanOrEqual(1);

      window.destroy();
    });
  });
});
