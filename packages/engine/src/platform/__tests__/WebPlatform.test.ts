/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

import { describe, it, expect, beforeEach, jest } from '@jest/globals';
import { WebPlatform } from '../WebPlatform';
import { PlatformType, OSType } from '../IPlatform';

// Mock browser globals for Node.js test environment
const mockIndexedDB = (() => {
  const stores = new Map<string, Map<string, any>>();

  return {
    open: jest.fn((name: string, version: number) => {
      const request: any = {
        result: null,
        error: null,
        onsuccess: null,
        onerror: null,
        onupgradeneeded: null,
      };

      setTimeout(() => {
        // Trigger upgrade if needed
        if (request.onupgradeneeded) {
          const db: any = {
            objectStoreNames: {
              contains: (storeName: string) => stores.has(storeName),
            },
            createObjectStore: (storeName: string) => {
              stores.set(storeName, new Map());
              return {};
            },
          };
          request.onupgradeneeded({ target: { result: db } });
        }

        // Create mock database
        const mockDB: any = {
          transaction: (storeNames: string[], mode: string) => {
            const storeName = storeNames[0];
            if (!stores.has(storeName)) {
              stores.set(storeName, new Map());
            }
            const store = stores.get(storeName)!;

            return {
              objectStore: () => ({
                get: (key: string) => {
                  const getRequest: any = {
                    result: store.get(key),
                    onsuccess: null,
                    onerror: null,
                  };
                  setTimeout(() => getRequest.onsuccess?.(), 0);
                  return getRequest;
                },
                put: (value: any, key: string) => {
                  const putRequest: any = {
                    onsuccess: null,
                    onerror: null,
                  };
                  store.set(key, value);
                  setTimeout(() => putRequest.onsuccess?.(), 0);
                  return putRequest;
                },
                delete: (key: string) => {
                  const deleteRequest: any = {
                    onsuccess: null,
                    onerror: null,
                  };
                  store.delete(key);
                  setTimeout(() => deleteRequest.onsuccess?.(), 0);
                  return deleteRequest;
                },
                getAllKeys: () => {
                  const keysRequest: any = {
                    result: Array.from(store.keys()),
                    onsuccess: null,
                    onerror: null,
                  };
                  setTimeout(() => keysRequest.onsuccess?.(), 0);
                  return keysRequest;
                },
              }),
            };
          },
        };

        request.result = mockDB;
        if (request.onsuccess) {
          request.onsuccess();
        }
      }, 0);

      return request;
    }),
  };
})();

describe('WebPlatform', () => {
  let platform: WebPlatform;

  beforeEach(() => {
    // Setup browser-like globals
    (global as any).indexedDB = mockIndexedDB;
    (global as any).navigator = {
      hardwareConcurrency: 4,
      onLine: true,
      userAgent: 'Mozilla/5.0 (Test)',
    };
    (global as any).performance = {
      now: () => Date.now(),
    };

    platform = new WebPlatform();
  });

  describe('Platform Properties', () => {
    it('should return correct platform type', () => {
      expect(platform.getType()).toBe(PlatformType.WEB);
    });

    it('should return an OS type', () => {
      const osType = platform.getOS();
      expect(Object.values(OSType)).toContain(osType);
    });

    it('should return a version string', () => {
      const version = platform.getVersion();
      expect(version).toContain('Web/');
    });
  });

  describe('Initialization', () => {
    it('should initialize successfully', async () => {
      await expect(platform.initialize()).resolves.not.toThrow();
    });

    it('should shutdown successfully', async () => {
      await platform.initialize();
      await expect(platform.shutdown()).resolves.not.toThrow();
    });
  });

  describe('File System', () => {
    beforeEach(async () => {
      await platform.initialize();
    });

    it('should get platform paths', async () => {
      const paths = await Promise.all([
        platform.fileSystem.getPath('user_data' as any),
        platform.fileSystem.getPath('app_data' as any),
        platform.fileSystem.getPath('cache' as any),
      ]);

      expect(paths[0]).toBe('/user_data');
      expect(paths[1]).toBe('/app_data');
      expect(paths[2]).toBe('/cache');
    });

    it('should write and read text files', async () => {
      const testPath = '/test.txt';
      const testContent = 'Hello, Nova Engine!';

      await platform.fileSystem.writeText(testPath, testContent);
      const exists = await platform.fileSystem.exists(testPath);
      expect(exists).toBe(true);

      const content = await platform.fileSystem.readText(testPath);
      expect(content).toBe(testContent);
    });

    it('should write and read binary files', async () => {
      const testPath = '/test.bin';
      const testData = new Uint8Array([1, 2, 3, 4, 5]);

      await platform.fileSystem.writeBinary(testPath, testData.buffer);
      const data = await platform.fileSystem.readBinary(testPath);

      const readArray = new Uint8Array(data);
      expect(readArray).toEqual(testData);
    });

    it('should delete files', async () => {
      const testPath = '/test-delete.txt';
      await platform.fileSystem.writeText(testPath, 'delete me');

      let exists = await platform.fileSystem.exists(testPath);
      expect(exists).toBe(true);

      await platform.fileSystem.delete(testPath);
      exists = await platform.fileSystem.exists(testPath);
      expect(exists).toBe(false);
    });

    it('should get file size', async () => {
      const testPath = '/test-size.txt';
      const testContent = 'test';
      await platform.fileSystem.writeText(testPath, testContent);

      const size = await platform.fileSystem.getFileSize(testPath);
      expect(size).toBeGreaterThan(0);
    });

    it('should get modified time', async () => {
      const testPath = '/test-time.txt';
      await platform.fileSystem.writeText(testPath, 'test');

      const modTime = await platform.fileSystem.getModifiedTime(testPath);
      expect(modTime).toBeInstanceOf(Date);
    });
  });

  describe('Threading', () => {
    it('should get core count', () => {
      const coreCount = platform.threading.getCoreCount();
      expect(coreCount).toBe(4);
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
        { execute: async () => count++ },
        { execute: async () => count++ },
        { execute: async () => count++ },
      ];

      await platform.threading.scheduleJobs(jobs);
      expect(count).toBe(3);
    });
  });

  describe('Network', () => {
    it('should check online status', () => {
      const isOnline = platform.network.isOnline();
      expect(isOnline).toBe(true);
    });

    it('should create WebSocket', () => {
      const ws = platform.network.createWebSocket();
      expect(ws).toBeDefined();
      expect(ws.isConnected()).toBe(false);
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

    it('should sleep', async () => {
      const start = Date.now();
      await platform.timing.sleep(50);
      const elapsed = Date.now() - start;
      expect(elapsed).toBeGreaterThanOrEqual(45); // Allow some margin
    });

    it('should set and clear interval', (done) => {
      let count = 0;
      const handle = platform.timing.setInterval(() => {
        count++;
        if (count === 2) {
          platform.timing.clearInterval(handle);
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
});
