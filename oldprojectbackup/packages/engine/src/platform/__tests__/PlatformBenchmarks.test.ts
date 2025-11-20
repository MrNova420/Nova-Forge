/**
 * Platform Performance Benchmarks
 * 
 * Comprehensive performance profiling for all platform operations
 * across Web, Desktop, and Mobile implementations.
 */

import { describe, it, expect, beforeAll, afterAll } from '@jest/globals';
import { WebPlatform } from '../WebPlatform';
import { PlatformManager } from '../PlatformManager';
import type { IPlatform, WindowConfig } from '../IPlatform';

interface BenchmarkResult {
  name: string;
  operations: number;
  totalTime: number;
  avgTime: number;
  minTime: number;
  maxTime: number;
  opsPerSecond: number;
  stdDev: number;
}

class PlatformBenchmarker {
  private results: BenchmarkResult[] = [];

  async benchmark(
    name: string,
    operation: () => Promise<void> | void,
    iterations: number = 1000
  ): Promise<BenchmarkResult> {
    const times: number[] = [];
    
    // Warmup
    for (let i = 0; i < 10; i++) {
      await operation();
    }

    // Actual benchmark
    const startTotal = performance.now();
    for (let i = 0; i < iterations; i++) {
      const start = performance.now();
      await operation();
      const end = performance.now();
      times.push(end - start);
    }
    const endTotal = performance.now();

    const totalTime = endTotal - startTotal;
    const avgTime = totalTime / iterations;
    const minTime = Math.min(...times);
    const maxTime = Math.max(...times);
    const opsPerSecond = 1000 / avgTime;

    // Calculate standard deviation
    const variance = times.reduce((sum, time) => {
      return sum + Math.pow(time - avgTime, 2);
    }, 0) / iterations;
    const stdDev = Math.sqrt(variance);

    const result: BenchmarkResult = {
      name,
      operations: iterations,
      totalTime,
      avgTime,
      minTime,
      maxTime,
      opsPerSecond,
      stdDev,
    };

    this.results.push(result);
    return result;
  }

  getResults(): BenchmarkResult[] {
    return this.results;
  }

  printReport(): void {
    console.log('\n=== Platform Performance Benchmark Report ===\n');
    
    for (const result of this.results) {
      console.log(`${result.name}:`);
      console.log(`  Operations: ${result.operations}`);
      console.log(`  Total Time: ${result.totalTime.toFixed(2)}ms`);
      console.log(`  Avg Time: ${result.avgTime.toFixed(4)}ms`);
      console.log(`  Min Time: ${result.minTime.toFixed(4)}ms`);
      console.log(`  Max Time: ${result.maxTime.toFixed(4)}ms`);
      console.log(`  Std Dev: ${result.stdDev.toFixed(4)}ms`);
      console.log(`  Ops/sec: ${result.opsPerSecond.toFixed(2)}`);
      console.log('');
    }
  }
}

describe('Platform Performance Benchmarks', () => {
  let platform: IPlatform;
  let benchmarker: PlatformBenchmarker;

  beforeAll(async () => {
    platform = new WebPlatform();
    await platform.initialize();
    benchmarker = new PlatformBenchmarker();
  });

  afterAll(async () => {
    await platform.shutdown();
    benchmarker.printReport();
  });

  describe('Window Operations', () => {
    it('should benchmark window creation', async () => {
      const config: WindowConfig = {
        title: 'Benchmark Window',
        width: 800,
        height: 600,
      };

      const result = await benchmarker.benchmark(
        'Window Creation',
        () => {
          const window = platform.createWindow(config);
          window.destroy();
        },
        100
      );

      expect(result.avgTime).toBeLessThan(10); // Should be < 10ms avg
      expect(result.opsPerSecond).toBeGreaterThan(100); // Should be > 100 ops/sec
    });

    it('should benchmark window resize', async () => {
      const window = platform.createWindow({
        title: 'Resize Test',
        width: 800,
        height: 600,
      });

      const result = await benchmarker.benchmark(
        'Window Resize',
        () => {
          window.setSize(1024, 768);
          window.setSize(800, 600);
        },
        1000
      );

      expect(result.avgTime).toBeLessThan(1); // Should be < 1ms avg
      expect(result.opsPerSecond).toBeGreaterThan(1000);

      window.destroy();
    });

    it('should benchmark window fullscreen toggle', async () => {
      const window = platform.createWindow({
        title: 'Fullscreen Test',
        width: 800,
        height: 600,
      });

      const result = await benchmarker.benchmark(
        'Window Fullscreen Toggle',
        () => {
          window.setFullscreen(true);
          window.setFullscreen(false);
        },
        100
      );

      expect(result.avgTime).toBeLessThan(5); // Should be < 5ms avg

      window.destroy();
    });
  });

  describe('File System Operations', () => {
    it('should benchmark file write', async () => {
      const fs = platform.getFileSystem();
      const data = new TextEncoder().encode('Benchmark data');

      const result = await benchmarker.benchmark(
        'File Write (small)',
        async () => {
          await fs.writeFile('/bench/write_test.txt', data);
        },
        100
      );

      expect(result.avgTime).toBeLessThan(20); // Should be < 20ms avg
    });

    it('should benchmark file read', async () => {
      const fs = platform.getFileSystem();
      const data = new TextEncoder().encode('Benchmark data');
      await fs.writeFile('/bench/read_test.txt', data);

      const result = await benchmarker.benchmark(
        'File Read (small)',
        async () => {
          await fs.readFile('/bench/read_test.txt');
        },
        100
      );

      expect(result.avgTime).toBeLessThan(20); // Should be < 20ms avg
    });

    it('should benchmark file existence check', async () => {
      const fs = platform.getFileSystem();

      const result = await benchmarker.benchmark(
        'File Exists Check',
        async () => {
          await fs.exists('/bench/read_test.txt');
        },
        1000
      );

      expect(result.avgTime).toBeLessThan(5); // Should be < 5ms avg
      expect(result.opsPerSecond).toBeGreaterThan(200);
    });

    it('should benchmark large file operations', async () => {
      const fs = platform.getFileSystem();
      const largeData = new Uint8Array(1024 * 1024); // 1MB
      for (let i = 0; i < largeData.length; i++) {
        largeData[i] = i % 256;
      }

      const writeResult = await benchmarker.benchmark(
        'File Write (1MB)',
        async () => {
          await fs.writeFile('/bench/large_file.bin', largeData);
        },
        10
      );

      expect(writeResult.avgTime).toBeLessThan(500); // Should be < 500ms for 1MB

      const readResult = await benchmarker.benchmark(
        'File Read (1MB)',
        async () => {
          await fs.readFile('/bench/large_file.bin');
        },
        10
      );

      expect(readResult.avgTime).toBeLessThan(500);
    });
  });

  describe('Worker Operations', () => {
    it('should benchmark worker creation', async () => {
      const result = await benchmarker.benchmark(
        'Worker Creation',
        () => {
          const worker = platform.createWorker('console.log("test")');
          worker.terminate();
        },
        50
      );

      expect(result.avgTime).toBeLessThan(50); // Should be < 50ms avg
    });

    it('should benchmark worker message passing', async () => {
      const worker = platform.createWorker(`
        self.onmessage = (e) => {
          self.postMessage({ result: e.data.value * 2 });
        };
      `);

      const result = await benchmarker.benchmark(
        'Worker Message Passing',
        () => {
          return new Promise<void>((resolve) => {
            worker.onMessage((data) => {
              resolve();
            });
            worker.postMessage({ value: 42 });
          });
        },
        100
      );

      expect(result.avgTime).toBeLessThan(10); // Should be < 10ms avg

      worker.terminate();
    });
  });

  describe('Time Operations', () => {
    it('should benchmark high-resolution timer', async () => {
      const result = await benchmarker.benchmark(
        'High-Resolution Timer (now)',
        () => {
          platform.now();
        },
        10000
      );

      expect(result.avgTime).toBeLessThan(0.01); // Should be < 0.01ms (microseconds)
      expect(result.opsPerSecond).toBeGreaterThan(100000);
    });

    it('should benchmark sleep accuracy', async () => {
      const sleepDuration = 10; // 10ms
      const measurements: number[] = [];

      for (let i = 0; i < 20; i++) {
        const start = platform.now();
        await platform.sleep(sleepDuration);
        const end = platform.now();
        measurements.push(end - start);
      }

      const avgActual = measurements.reduce((a, b) => a + b) / measurements.length;
      const error = Math.abs(avgActual - sleepDuration);
      const errorPercent = (error / sleepDuration) * 100;

      console.log(`  Sleep Accuracy: target=${sleepDuration}ms, actual=${avgActual.toFixed(2)}ms, error=${errorPercent.toFixed(2)}%`);

      expect(errorPercent).toBeLessThan(20); // Within 20% error
    });
  });

  describe('Network Operations', () => {
    it('should benchmark fetch operation', async () => {
      // Mock fetch for testing
      global.fetch = async () => {
        return new Response('test data', { status: 200 });
      };

      const result = await benchmarker.benchmark(
        'Fetch Request',
        async () => {
          await platform.fetch('https://example.com/test');
        },
        50
      );

      expect(result.avgTime).toBeLessThan(100); // Mock should be fast
    });
  });

  describe('Clipboard Operations', () => {
    it('should benchmark clipboard read/write', async () => {
      const testData = 'Benchmark clipboard data';

      const writeResult = await benchmarker.benchmark(
        'Clipboard Write',
        async () => {
          await platform.writeClipboard(testData);
        },
        50
      );

      expect(writeResult.avgTime).toBeLessThan(10);

      const readResult = await benchmarker.benchmark(
        'Clipboard Read',
        async () => {
          await platform.readClipboard();
        },
        50
      );

      expect(readResult.avgTime).toBeLessThan(10);
    });
  });

  describe('Stress Tests', () => {
    it('should handle 50 concurrent windows', async () => {
      const windows = [];
      const start = platform.now();

      for (let i = 0; i < 50; i++) {
        windows.push(platform.createWindow({
          title: `Window ${i}`,
          width: 320,
          height: 240,
        }));
      }

      const creationTime = platform.now() - start;
      console.log(`  Created 50 windows in ${creationTime.toFixed(2)}ms`);

      expect(creationTime).toBeLessThan(1000); // Should create 50 windows in < 1s

      // Cleanup
      for (const window of windows) {
        window.destroy();
      }
    });

    it('should handle 1000 file operations', async () => {
      const fs = platform.getFileSystem();
      const data = new TextEncoder().encode('Test data');
      
      const start = platform.now();

      for (let i = 0; i < 1000; i++) {
        await fs.writeFile(`/stress/file_${i}.txt`, data);
      }

      const writeTime = platform.now() - start;
      console.log(`  Wrote 1000 files in ${writeTime.toFixed(2)}ms (${(writeTime / 1000).toFixed(2)}ms avg)`);

      expect(writeTime).toBeLessThan(30000); // Should complete in < 30s
    });

    it('should handle 8 concurrent workers', async () => {
      const workers = [];
      const start = platform.now();

      for (let i = 0; i < 8; i++) {
        workers.push(platform.createWorker(`
          self.onmessage = (e) => {
            let sum = 0;
            for (let i = 0; i < 1000000; i++) {
              sum += i;
            }
            self.postMessage({ sum, id: e.data.id });
          };
        `));
      }

      const creationTime = platform.now() - start;
      console.log(`  Created 8 workers in ${creationTime.toFixed(2)}ms`);

      // Send work to all workers
      const workStart = platform.now();
      const promises = workers.map((worker, idx) => {
        return new Promise<void>((resolve) => {
          worker.onMessage(() => resolve());
          worker.postMessage({ id: idx });
        });
      });

      await Promise.all(promises);
      const workTime = platform.now() - workStart;
      console.log(`  Completed parallel work in ${workTime.toFixed(2)}ms`);

      // Cleanup
      workers.forEach(w => w.terminate());

      expect(creationTime).toBeLessThan(500);
      expect(workTime).toBeLessThan(5000);
    });
  });
});
