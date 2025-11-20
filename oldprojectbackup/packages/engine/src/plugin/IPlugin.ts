/**
 * @file IPlugin.ts
 * @description Plugin system interfaces and types
 */

import { IPlatform } from '../platform/IPlatform';

/**
 * Plugin dependency specification
 */
export interface PluginDependency {
  name: string;
  version: string; // Semver format
  optional?: boolean;
}

/**
 * Plugin metadata
 */
export interface PluginMetadata {
  name: string;
  version: string;
  description: string;
  author: string;
  license?: string;
  homepage?: string;
  repository?: string;
  keywords?: string[];
  path: string;
}

/**
 * Plugin information (before loading)
 */
export interface PluginInfo {
  name: string;
  version: string;
  description: string;
  author: string;
  dependencies: PluginDependency[];
  path: string;
  enabled: boolean;
}

/**
 * Plugin context provided to plugins
 */
export interface PluginContext {
  platform: IPlatform;
  logger: PluginLogger;
  config: Record<string, any>;
  extensions: ExtensionRegistry;
  events: EventBus;
}

/**
 * Plugin logger interface
 */
export interface PluginLogger {
  debug(...args: any[]): void;
  info(...args: any[]): void;
  warn(...args: any[]): void;
  error(...args: any[]): void;
}

/**
 * Extension registry interface
 */
export interface ExtensionRegistry {
  register<T>(pointId: string, extension: T): void;
  get<T>(pointId: string): T[];
  unregister<T>(pointId: string, extension: T): void;
}

/**
 * Event bus interface
 */
export interface EventBus {
  on(event: string, callback: (data: any) => void): void;
  off(event: string, callback: (data: any) => void): void;
  emit(event: string, data: any): void;
}

/**
 * Main plugin interface
 */
export interface IPlugin {
  readonly name: string;
  readonly version: string;
  readonly dependencies: PluginDependency[];
  readonly metadata: PluginMetadata;

  /**
   * Initialize plugin - called once when plugin is loaded
   */
  initialize(context: PluginContext): Promise<void>;

  /**
   * Activate plugin - called to enable plugin functionality
   */
  activate(): Promise<void>;

  /**
   * Deactivate plugin - called to disable plugin functionality
   */
  deactivate(): Promise<void>;

  /**
   * Shutdown plugin - called once when plugin is unloaded
   */
  shutdown(): Promise<void>;

  /**
   * Hot reload hook - called when plugin is hot reloaded (optional)
   */
  onHotReload?(): Promise<void>;
}

/**
 * Base plugin class with common functionality
 */
export abstract class BasePlugin implements IPlugin {
  abstract readonly name: string;
  abstract readonly version: string;
  abstract readonly metadata: PluginMetadata;
  
  readonly dependencies: PluginDependency[] = [];

  protected context?: PluginContext;
  protected isActive: boolean = false;

  async initialize(context: PluginContext): Promise<void> {
    this.context = context;
  }

  async activate(): Promise<void> {
    this.isActive = true;
  }

  async deactivate(): Promise<void> {
    this.isActive = false;
  }

  async shutdown(): Promise<void> {
    this.context = undefined;
  }

  protected log(...args: any[]): void {
    if (this.context) {
      this.context.logger.info(...args);
    }
  }

  protected logError(...args: any[]): void {
    if (this.context) {
      this.context.logger.error(...args);
    }
  }
}
