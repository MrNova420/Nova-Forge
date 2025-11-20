/**
 * @file PluginManager.ts
 * @description Production-grade plugin management system
 * Handles plugin discovery, loading, dependency resolution, and lifecycle
 */

import { IPlugin, PluginDependency, PluginMetadata, PluginContext, PluginInfo } from './IPlugin';
import { IPlatform } from '../platform/IPlatform';

/**
 * Plugin manager - handles all plugin operations
 */
export class PluginManager {
  private plugins: Map<string, IPlugin> = new Map();
  private pluginContexts: Map<string, PluginContext> = new Map();
  private dependencyGraph: Map<string, string[]> = new Map();
  private loadOrder: string[] = [];
  private searchPaths: string[] = [];

  constructor(private platform: IPlatform) {}

  /**
   * Add search path for plugin discovery
   */
  addSearchPath(path: string): void {
    if (!this.searchPaths.includes(path)) {
      this.searchPaths.push(path);
    }
  }

  /**
   * Discover plugins from all search paths
   */
  async discoverPlugins(additionalPaths?: string[]): Promise<PluginInfo[]> {
    const paths = [...this.searchPaths, ...(additionalPaths || [])];
    const discovered: PluginInfo[] = [];

    for (const path of paths) {
      try {
        const files = await this.platform.getFileSystem().readdir(path);
        
        for (const file of files) {
          if (this.isPluginFile(file)) {
            const fullPath = `${path}/${file}`;
            const info = await this.loadPluginInfo(fullPath);
            if (info) {
              discovered.push(info);
            }
          }
        }
      } catch (error) {
        console.warn(`Failed to discover plugins in ${path}:`, error);
      }
    }

    return discovered;
  }

  /**
   * Load plugin info without initializing
   */
  private async loadPluginInfo(pluginPath: string): Promise<PluginInfo | null> {
    try {
      const content = await this.platform.getFileSystem().readFile(pluginPath);
      const data = JSON.parse(content.toString());

      return {
        name: data.name,
        version: data.version,
        description: data.description || '',
        author: data.author || '',
        dependencies: data.dependencies || [],
        path: pluginPath,
        enabled: true,
      };
    } catch (error) {
      console.warn(`Failed to load plugin info from ${pluginPath}:`, error);
      return null;
    }
  }

  /**
   * Load plugin from path
   */
  async loadPlugin(pluginPath: string): Promise<IPlugin> {
    try {
      // Load plugin module
      const pluginModule = await this.loadPluginModule(pluginPath);
      
      // Create plugin instance
      const plugin: IPlugin = new pluginModule.default();

      // Validate plugin
      this.validatePlugin(plugin);

      // Store plugin
      this.plugins.set(plugin.name, plugin);

      // Build dependency graph
      this.buildDependencyGraph(plugin);

      return plugin;
    } catch (error) {
      throw new Error(`Failed to load plugin from ${pluginPath}: ${error}`);
    }
  }

  /**
   * Load multiple plugins
   */
  async loadPlugins(pluginPaths: string[]): Promise<IPlugin[]> {
    const plugins: IPlugin[] = [];

    for (const path of pluginPaths) {
      try {
        const plugin = await this.loadPlugin(path);
        plugins.push(plugin);
      } catch (error) {
        console.error(`Failed to load plugin from ${path}:`, error);
      }
    }

    return plugins;
  }

  /**
   * Unload plugin
   */
  async unloadPlugin(pluginName: string): Promise<void> {
    const plugin = this.plugins.get(pluginName);
    if (!plugin) {
      throw new Error(`Plugin not found: ${pluginName}`);
    }

    // Check if other plugins depend on this one
    const dependents = this.findDependentPlugins(pluginName);
    if (dependents.length > 0) {
      throw new Error(
        `Cannot unload ${pluginName}: depended on by ${dependents.join(', ')}`
      );
    }

    // Deactivate and shutdown
    try {
      await plugin.deactivate();
      await plugin.shutdown();
    } catch (error) {
      console.error(`Error during plugin shutdown:`, error);
    }

    // Remove from collections
    this.plugins.delete(pluginName);
    this.pluginContexts.delete(pluginName);
    this.dependencyGraph.delete(pluginName);
    
    const index = this.loadOrder.indexOf(pluginName);
    if (index !== -1) {
      this.loadOrder.splice(index, 1);
    }
  }

  /**
   * Resolve plugin dependencies and determine load order
   */
  resolveDependencies(plugins: IPlugin[]): IPlugin[] {
    const sorted: IPlugin[] = [];
    const visited = new Set<string>();
    const visiting = new Set<string>();

    const visit = (plugin: IPlugin) => {
      if (visited.has(plugin.name)) {
        return;
      }

      if (visiting.has(plugin.name)) {
        throw new Error(`Circular dependency detected: ${plugin.name}`);
      }

      visiting.add(plugin.name);

      // Visit dependencies first
      for (const dep of plugin.dependencies) {
        const depPlugin = plugins.find(p => p.name === dep.name);
        
        if (!depPlugin) {
          if (!dep.optional) {
            throw new Error(
              `Missing dependency: ${plugin.name} requires ${dep.name}`
            );
          }
          continue;
        }

        // Check version compatibility
        if (!this.isVersionCompatible(depPlugin.version, dep.version)) {
          throw new Error(
            `Version mismatch: ${plugin.name} requires ${dep.name}@${dep.version}, found ${depPlugin.version}`
          );
        }

        visit(depPlugin);
      }

      visiting.delete(plugin.name);
      visited.add(plugin.name);
      sorted.push(plugin);
    };

    for (const plugin of plugins) {
      visit(plugin);
    }

    return sorted;
  }

  /**
   * Initialize all plugins in dependency order
   */
  async initializeAll(): Promise<void> {
    const plugins = Array.from(this.plugins.values());
    const orderedPlugins = this.resolveDependencies(plugins);

    for (const plugin of orderedPlugins) {
      try {
        const context = this.createContext(plugin);
        this.pluginContexts.set(plugin.name, context);

        await plugin.initialize(context);
        this.loadOrder.push(plugin.name);
      } catch (error) {
        throw new Error(`Failed to initialize plugin ${plugin.name}: ${error}`);
      }
    }
  }

  /**
   * Activate all plugins
   */
  async activateAll(): Promise<void> {
    for (const pluginName of this.loadOrder) {
      const plugin = this.plugins.get(pluginName);
      if (plugin) {
        try {
          await plugin.activate();
        } catch (error) {
          console.error(`Failed to activate plugin ${pluginName}:`, error);
        }
      }
    }
  }

  /**
   * Deactivate all plugins
   */
  async deactivateAll(): Promise<void> {
    // Deactivate in reverse order
    for (let i = this.loadOrder.length - 1; i >= 0; i--) {
      const pluginName = this.loadOrder[i];
      const plugin = this.plugins.get(pluginName);
      if (plugin) {
        try {
          await plugin.deactivate();
        } catch (error) {
          console.error(`Failed to deactivate plugin ${pluginName}:`, error);
        }
      }
    }
  }

  /**
   * Shutdown all plugins
   */
  async shutdownAll(): Promise<void> {
    // Shutdown in reverse order
    for (let i = this.loadOrder.length - 1; i >= 0; i--) {
      const pluginName = this.loadOrder[i];
      const plugin = this.plugins.get(pluginName);
      if (plugin) {
        try {
          await plugin.shutdown();
        } catch (error) {
          console.error(`Failed to shutdown plugin ${pluginName}:`, error);
        }
      }
    }

    this.plugins.clear();
    this.pluginContexts.clear();
    this.dependencyGraph.clear();
    this.loadOrder = [];
  }

  /**
   * Hot reload a plugin
   */
  async reloadPlugin(pluginName: string): Promise<void> {
    const plugin = this.plugins.get(pluginName);
    if (!plugin) {
      throw new Error(`Plugin not found: ${pluginName}`);
    }

    const metadata = plugin.metadata;

    // Deactivate current plugin
    await plugin.deactivate();

    // Reload module
    const newModule = await this.loadPluginModule(metadata.path);
    const newPlugin: IPlugin = new newModule.default();

    // Validate new plugin
    this.validatePlugin(newPlugin);

    // Call hot reload hook if available
    if (newPlugin.onHotReload) {
      try {
        await newPlugin.onHotReload();
      } catch (error) {
        console.error(`Hot reload hook failed for ${pluginName}:`, error);
      }
    }

    // Initialize new plugin
    const context = this.pluginContexts.get(pluginName);
    if (!context) {
      throw new Error(`Context not found for ${pluginName}`);
    }

    await newPlugin.initialize(context);

    // Activate new plugin
    await newPlugin.activate();

    // Update plugin in map
    this.plugins.set(pluginName, newPlugin);
  }

  /**
   * Get plugin by name
   */
  getPlugin(name: string): IPlugin | undefined {
    return this.plugins.get(name);
  }

  /**
   * Get all loaded plugins
   */
  getAllPlugins(): IPlugin[] {
    return Array.from(this.plugins.values());
  }

  /**
   * Check if plugin is loaded
   */
  hasPlugin(name: string): boolean {
    return this.plugins.has(name);
  }

  /**
   * Enable plugin
   */
  async enablePlugin(name: string): Promise<void> {
    const plugin = this.plugins.get(name);
    if (!plugin) {
      throw new Error(`Plugin not found: ${name}`);
    }

    await plugin.activate();
  }

  /**
   * Disable plugin
   */
  async disablePlugin(name: string): Promise<void> {
    const plugin = this.plugins.get(name);
    if (!plugin) {
      throw new Error(`Plugin not found: ${name}`);
    }

    await plugin.deactivate();
  }

  /**
   * Get plugin dependency graph
   */
  getDependencyGraph(): Map<string, string[]> {
    return new Map(this.dependencyGraph);
  }

  /**
   * Get plugin load order
   */
  getLoadOrder(): string[] {
    return [...this.loadOrder];
  }

  /**
   * Create plugin context
   */
  private createContext(plugin: IPlugin): PluginContext {
    return {
      platform: this.platform,
      logger: this.createLogger(plugin.name),
      config: {},
      extensions: this.createExtensionRegistry(),
      events: this.createEventBus(),
    };
  }

  /**
   * Create logger for plugin
   */
  private createLogger(pluginName: string) {
    return {
      debug: (...args: any[]) => console.debug(`[${pluginName}]`, ...args),
      info: (...args: any[]) => console.info(`[${pluginName}]`, ...args),
      warn: (...args: any[]) => console.warn(`[${pluginName}]`, ...args),
      error: (...args: any[]) => console.error(`[${pluginName}]`, ...args),
    };
  }

  /**
   * Create extension registry for plugin
   */
  private createExtensionRegistry() {
    const extensions = new Map<string, any[]>();

    return {
      register: <T>(pointId: string, extension: T) => {
        if (!extensions.has(pointId)) {
          extensions.set(pointId, []);
        }
        extensions.get(pointId)!.push(extension);
      },
      get: <T>(pointId: string): T[] => {
        return (extensions.get(pointId) || []) as T[];
      },
      unregister: <T>(pointId: string, extension: T) => {
        const exts = extensions.get(pointId);
        if (exts) {
          const index = exts.indexOf(extension);
          if (index !== -1) {
            exts.splice(index, 1);
          }
        }
      },
    };
  }

  /**
   * Create event bus for plugin
   */
  private createEventBus() {
    const listeners = new Map<string, Array<(data: any) => void>>();

    return {
      on: (event: string, callback: (data: any) => void) => {
        if (!listeners.has(event)) {
          listeners.set(event, []);
        }
        listeners.get(event)!.push(callback);
      },
      off: (event: string, callback: (data: any) => void) => {
        const cbs = listeners.get(event);
        if (cbs) {
          const index = cbs.indexOf(callback);
          if (index !== -1) {
            cbs.splice(index, 1);
          }
        }
      },
      emit: (event: string, data: any) => {
        const cbs = listeners.get(event);
        if (cbs) {
          for (const cb of cbs) {
            try {
              cb(data);
            } catch (error) {
              console.error(`Error in event handler for ${event}:`, error);
            }
          }
        }
      },
    };
  }

  /**
   * Load plugin module (platform-specific)
   */
  private async loadPluginModule(path: string): Promise<any> {
    // This would be implemented differently for each platform
    // For now, assuming ES modules
    try {
      return await import(path);
    } catch (error) {
      throw new Error(`Failed to load plugin module from ${path}: ${error}`);
    }
  }

  /**
   * Validate plugin structure
   */
  private validatePlugin(plugin: IPlugin): void {
    if (!plugin.name) {
      throw new Error('Plugin must have a name');
    }

    if (!plugin.version) {
      throw new Error('Plugin must have a version');
    }

    if (!plugin.initialize || typeof plugin.initialize !== 'function') {
      throw new Error('Plugin must implement initialize method');
    }

    if (!plugin.activate || typeof plugin.activate !== 'function') {
      throw new Error('Plugin must implement activate method');
    }

    if (!plugin.deactivate || typeof plugin.deactivate !== 'function') {
      throw new Error('Plugin must implement deactivate method');
    }

    if (!plugin.shutdown || typeof plugin.shutdown !== 'function') {
      throw new Error('Plugin must implement shutdown method');
    }
  }

  /**
   * Build dependency graph for plugin
   */
  private buildDependencyGraph(plugin: IPlugin): void {
    const deps = plugin.dependencies.map(d => d.name);
    this.dependencyGraph.set(plugin.name, deps);
  }

  /**
   * Find plugins that depend on a given plugin
   */
  private findDependentPlugins(pluginName: string): string[] {
    const dependents: string[] = [];

    for (const [name, deps] of this.dependencyGraph) {
      if (deps.includes(pluginName)) {
        dependents.push(name);
      }
    }

    return dependents;
  }

  /**
   * Check if a version satisfies a requirement (semver)
   */
  private isVersionCompatible(version: string, requirement: string): boolean {
    // Simple semver check - would use a proper library in production
    if (requirement.startsWith('^')) {
      // Compatible with same major version
      const reqMajor = parseInt(requirement.slice(1).split('.')[0]);
      const verMajor = parseInt(version.split('.')[0]);
      return verMajor === reqMajor;
    }

    if (requirement.startsWith('~')) {
      // Compatible with same minor version
      const reqParts = requirement.slice(1).split('.');
      const verParts = version.split('.');
      return reqParts[0] === verParts[0] && reqParts[1] === verParts[1];
    }

    // Exact match
    return version === requirement;
  }

  /**
   * Check if file is a plugin file
   */
  private isPluginFile(filename: string): boolean {
    return (
      filename.endsWith('.plugin.js') ||
      filename.endsWith('.plugin.ts') ||
      filename.endsWith('.plugin.json')
    );
  }

  /**
   * Get plugin statistics
   */
  getStats(): {
    totalPlugins: number;
    activePlugins: number;
    loadOrder: string[];
    dependencies: number;
  } {
    let totalDeps = 0;
    for (const deps of this.dependencyGraph.values()) {
      totalDeps += deps.length;
    }

    return {
      totalPlugins: this.plugins.size,
      activePlugins: this.loadOrder.length,
      loadOrder: [...this.loadOrder],
      dependencies: totalDeps,
    };
  }
}
