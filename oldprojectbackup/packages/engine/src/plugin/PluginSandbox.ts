/**
 * @file PluginSandbox.ts
 * @description Plugin security and sandboxing system
 * Provides isolated execution environments with resource limits
 */

import { IPlugin, PluginContext } from './IPlugin';
import { IPlatform } from '../platform/IPlatform';

/**
 * Resource limits for plugins
 */
export interface ResourceLimits {
  maxMemoryMB: number;
  maxCPUPercent: number;
  maxFileSize: number;
  maxNetworkBandwidthKBps: number;
  maxConcurrentRequests: number;
  timeoutMs: number;
}

/**
 * Sandbox configuration
 */
export interface SandboxConfig {
  limits: ResourceLimits;
  allowedAPIs: string[];
  allowedFilePatterns: RegExp[];
  allowedNetworkHosts: string[];
  enableLogging: boolean;
}

/**
 * Sandbox violation
 */
export interface SandboxViolation {
  type: 'memory' | 'cpu' | 'timeout' | 'file' | 'network' | 'api';
  message: string;
  timestamp: number;
  pluginName: string;
}

/**
 * Plugin sandbox - executes plugins in isolated environment
 */
export class PluginSandbox {
  private violations: Map<string, SandboxViolation[]> = new Map();
  private resourceUsage: Map<string, ResourceUsage> = new Map();

  constructor(private config: SandboxConfig) {}

  /**
   * Execute plugin in sandbox
   */
  async executeInSandbox(
    plugin: IPlugin,
    context: PluginContext,
    operation: () => Promise<void>
  ): Promise<void> {
    const startTime = Date.now();
    const startMemory = this.getMemoryUsage();

    try {
      // Create limited context
      const limitedContext = this.createLimitedContext(plugin, context);

      // Execute with timeout
      await this.executeWithTimeout(
        () => operation(),
        this.config.limits.timeoutMs,
        plugin.name
      );

      // Check resource usage
      const endTime = Date.now();
      const endMemory = this.getMemoryUsage();

      this.updateResourceUsage(plugin.name, {
        memoryUsed: endMemory - startMemory,
        cpuTime: endTime - startTime,
        fileOps: 0,
        networkOps: 0,
      });

      this.enforceResourceLimits(plugin);
    } catch (error) {
      this.recordViolation(plugin.name, {
        type: 'timeout',
        message: `Plugin execution failed: ${error}`,
        timestamp: Date.now(),
        pluginName: plugin.name,
      });
      throw error;
    }
  }

  /**
   * Create limited context for plugin
   */
  private createLimitedContext(
    plugin: IPlugin,
    context: PluginContext
  ): PluginContext {
    return {
      platform: this.createLimitedPlatform(plugin, context.platform),
      logger: context.logger,
      config: context.config,
      extensions: context.extensions,
      events: context.events,
    };
  }

  /**
   * Create limited platform access
   */
  private createLimitedPlatform(plugin: IPlugin, platform: IPlatform): IPlatform {
    const sandbox = this;

    return {
      ...platform,

      // Override filesystem with restrictions
      getFileSystem: () => {
        const fs = platform.getFileSystem();
        return {
          ...fs,
          readFile: async (path: string) => {
            if (!sandbox.isFileAccessAllowed(plugin.name, path)) {
              throw new Error(`File access denied: ${path}`);
            }
            const size = await sandbox.getFileSize(path);
            if (size > sandbox.config.limits.maxFileSize) {
              throw new Error(`File too large: ${path}`);
            }
            return fs.readFile(path);
          },
          writeFile: async (path: string, data: any) => {
            if (!sandbox.isFileAccessAllowed(plugin.name, path)) {
              throw new Error(`File access denied: ${path}`);
            }
            if (data.length > sandbox.config.limits.maxFileSize) {
              throw new Error(`Data too large for file: ${path}`);
            }
            return fs.writeFile(path, data);
          },
          // Other methods similarly restricted...
        } as any;
      },

      // Override network with restrictions
      fetch: async (url: string, options?: any) => {
        if (!sandbox.isNetworkAccessAllowed(plugin.name, url)) {
          throw new Error(`Network access denied: ${url}`);
        }
        sandbox.trackNetworkOp(plugin.name);
        return platform.fetch(url, options);
      },
    };
  }

  /**
   * Execute with timeout
   */
  private async executeWithTimeout<T>(
    fn: () => Promise<T>,
    timeoutMs: number,
    pluginName: string
  ): Promise<T> {
    return Promise.race([
      fn(),
      new Promise<T>((_, reject) => {
        setTimeout(() => {
          reject(new Error(`Plugin ${pluginName} execution timeout`));
        }, timeoutMs);
      }),
    ]);
  }

  /**
   * Check if file access is allowed
   */
  private isFileAccessAllowed(pluginName: string, path: string): boolean {
    for (const pattern of this.config.allowedFilePatterns) {
      if (pattern.test(path)) {
        return true;
      }
    }

    this.recordViolation(pluginName, {
      type: 'file',
      message: `Unauthorized file access: ${path}`,
      timestamp: Date.now(),
      pluginName,
    });

    return false;
  }

  /**
   * Check if network access is allowed
   */
  private isNetworkAccessAllowed(pluginName: string, url: string): boolean {
    const urlObj = new URL(url);
    const host = urlObj.hostname;

    if (this.config.allowedNetworkHosts.includes('*')) {
      return true;
    }

    if (this.config.allowedNetworkHosts.includes(host)) {
      return true;
    }

    // Check for wildcard patterns
    for (const allowed of this.config.allowedNetworkHosts) {
      if (allowed.startsWith('*.')) {
        const domain = allowed.slice(2);
        if (host.endsWith(domain)) {
          return true;
        }
      }
    }

    this.recordViolation(pluginName, {
      type: 'network',
      message: `Unauthorized network access: ${url}`,
      timestamp: Date.now(),
      pluginName,
    });

    return false;
  }

  /**
   * Track network operation
   */
  private trackNetworkOp(pluginName: string): void {
    const usage = this.resourceUsage.get(pluginName);
    if (usage) {
      usage.networkOps++;
    }
  }

  /**
   * Enforce resource limits
   */
  private enforceResourceLimits(plugin: IPlugin): void {
    const usage = this.resourceUsage.get(plugin.name);
    if (!usage) return;

    // Check memory limit
    if (usage.memoryUsed > this.config.limits.maxMemoryMB * 1024 * 1024) {
      this.recordViolation(plugin.name, {
        type: 'memory',
        message: `Memory limit exceeded: ${usage.memoryUsed} bytes`,
        timestamp: Date.now(),
        pluginName: plugin.name,
      });
      throw new Error(`Plugin ${plugin.name} exceeded memory limit`);
    }

    // Check network operations
    if (usage.networkOps > this.config.limits.maxConcurrentRequests) {
      this.recordViolation(plugin.name, {
        type: 'network',
        message: `Too many concurrent network requests: ${usage.networkOps}`,
        timestamp: Date.now(),
        pluginName: plugin.name,
      });
      throw new Error(`Plugin ${plugin.name} exceeded network request limit`);
    }
  }

  /**
   * Update resource usage
   */
  private updateResourceUsage(pluginName: string, usage: Partial<ResourceUsage>): void {
    if (!this.resourceUsage.has(pluginName)) {
      this.resourceUsage.set(pluginName, {
        memoryUsed: 0,
        cpuTime: 0,
        fileOps: 0,
        networkOps: 0,
      });
    }

    const current = this.resourceUsage.get(pluginName)!;
    Object.assign(current, usage);
  }

  /**
   * Record sandbox violation
   */
  private recordViolation(pluginName: string, violation: SandboxViolation): void {
    if (!this.violations.has(pluginName)) {
      this.violations.set(pluginName, []);
    }

    this.violations.get(pluginName)!.push(violation);

    if (this.config.enableLogging) {
      console.warn(`[Sandbox Violation] ${violation.message}`);
    }
  }

  /**
   * Get violations for plugin
   */
  getViolations(pluginName: string): SandboxViolation[] {
    return this.violations.get(pluginName) || [];
  }

  /**
   * Get all violations
   */
  getAllViolations(): Map<string, SandboxViolation[]> {
    return new Map(this.violations);
  }

  /**
   * Clear violations
   */
  clearViolations(pluginName?: string): void {
    if (pluginName) {
      this.violations.delete(pluginName);
    } else {
      this.violations.clear();
    }
  }

  /**
   * Get resource usage for plugin
   */
  getResourceUsage(pluginName: string): ResourceUsage | null {
    return this.resourceUsage.get(pluginName) || null;
  }

  /**
   * Get memory usage (platform-specific)
   */
  private getMemoryUsage(): number {
    // Platform-specific implementation
    // For Node.js: process.memoryUsage().heapUsed
    // For browser: performance.memory?.usedJSHeapSize || 0
    return 0; // Placeholder
  }

  /**
   * Get file size
   */
  private async getFileSize(path: string): Promise<number> {
    // Platform-specific implementation
    return 0; // Placeholder
  }

  /**
   * Get statistics
   */
  getStats(): {
    totalViolations: number;
    violationsByType: Record<string, number>;
    averageMemoryUsage: number;
    pluginsTracked: number;
  } {
    let totalViolations = 0;
    const violationsByType: Record<string, number> = {};
    let totalMemory = 0;

    for (const violations of this.violations.values()) {
      totalViolations += violations.length;
      for (const v of violations) {
        violationsByType[v.type] = (violationsByType[v.type] || 0) + 1;
      }
    }

    for (const usage of this.resourceUsage.values()) {
      totalMemory += usage.memoryUsed;
    }

    return {
      totalViolations,
      violationsByType,
      averageMemoryUsage:
        this.resourceUsage.size > 0 ? totalMemory / this.resourceUsage.size : 0,
      pluginsTracked: this.resourceUsage.size,
    };
  }
}

/**
 * Resource usage tracking
 */
interface ResourceUsage {
  memoryUsed: number;
  cpuTime: number;
  fileOps: number;
  networkOps: number;
}

/**
 * Default sandbox configuration
 */
export const DEFAULT_SANDBOX_CONFIG: SandboxConfig = {
  limits: {
    maxMemoryMB: 100,
    maxCPUPercent: 25,
    maxFileSize: 10 * 1024 * 1024, // 10MB
    maxNetworkBandwidthKBps: 1024, // 1MB/s
    maxConcurrentRequests: 5,
    timeoutMs: 30000, // 30 seconds
  },
  allowedAPIs: ['console', 'setTimeout', 'setInterval', 'Promise', 'fetch'],
  allowedFilePatterns: [
    /^\/plugins\/.*/,
    /^\/data\/.*/,
    /^\/config\/.*/,
  ],
  allowedNetworkHosts: ['api.example.com', '*.cdn.example.com'],
  enableLogging: true,
};
