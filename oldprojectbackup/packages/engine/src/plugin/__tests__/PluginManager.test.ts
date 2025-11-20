/**
 * @file PluginManager.test.ts
 * @description Tests for plugin management system
 */

import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { PluginManager } from '../PluginManager';
import { IPlugin, PluginMetadata, PluginContext } from '../IPlugin';

describe('PluginManager', () => {
  let pluginManager: PluginManager;
  let mockPlatform: any;
  let mockLogger: any;

  beforeEach(() => {
    mockLogger = {
      info: vi.fn(),
      warn: vi.fn(),
      error: vi.fn(),
    };

    mockPlatform = {
      filesystem: {
        readdir: vi.fn(() => Promise.resolve(['plugin1.plugin.js', 'plugin2.plugin.js'])),
        readFile: vi.fn(() => Promise.resolve('{}')),
      },
    };

    pluginManager = new PluginManager(mockPlatform, mockLogger);
  });

  afterEach(() => {
    vi.clearAllMocks();
  });

  describe('Plugin Discovery', () => {
    it('should discover plugins from search paths', async () => {
      const plugins = await pluginManager.discoverPlugins(['/plugins']);
      
      expect(plugins.length).toBeGreaterThan(0);
      expect(mockPlatform.filesystem.readdir).toHaveBeenCalledWith('/plugins');
    });

    it('should filter .plugin.js files', async () => {
      mockPlatform.filesystem.readdir.mockResolvedValue([
        'plugin1.plugin.js',
        'not-a-plugin.js',
        'plugin2.plugin.ts',
        'readme.md',
      ]);

      const plugins = await pluginManager.discoverPlugins(['/plugins']);
      
      expect(plugins.length).toBe(2);
    });

    it('should discover from multiple paths', async () => {
      const plugins = await pluginManager.discoverPlugins([
        '/plugins/core',
        '/plugins/custom',
      ]);

      expect(mockPlatform.filesystem.readdir).toHaveBeenCalledTimes(2);
    });
  });

  describe('Plugin Loading', () => {
    it('should load plugin', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      const plugin = await pluginManager.loadPlugin(testPlugin);

      expect(plugin.name).toBe('test-plugin');
    });

    it('should initialize plugin', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      const initSpy = vi.spyOn(testPlugin, 'initialize');
      
      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.initializeAll();

      expect(initSpy).toHaveBeenCalled();
    });

    it('should activate plugin after initialization', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      const activateSpy = vi.spyOn(testPlugin, 'activate');
      
      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.initializeAll();

      expect(activateSpy).toHaveBeenCalled();
    });

    it('should provide context to plugin', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      let receivedContext: PluginContext | undefined;

      testPlugin.initialize = async (context: PluginContext) => {
        receivedContext = context;
      };

      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.initializeAll();

      expect(receivedContext).toBeDefined();
      expect(receivedContext!.logger).toBeDefined();
      expect(receivedContext!.extensions).toBeDefined();
    });
  });

  describe('Dependency Resolution', () => {
    it('should resolve dependencies in order', async () => {
      const corePlugin = createTestPlugin('core');
      const dependentPlugin = createTestPlugin('dependent', [
        { name: 'core', version: '^1.0.0' },
      ]);

      const plugins = pluginManager.resolveDependencies([dependentPlugin, corePlugin]);

      expect(plugins[0].name).toBe('core');
      expect(plugins[1].name).toBe('dependent');
    });

    it('should detect circular dependencies', async () => {
      const plugin1 = createTestPlugin('plugin1', [{ name: 'plugin2', version: '^1.0.0' }]);
      const plugin2 = createTestPlugin('plugin2', [{ name: 'plugin1', version: '^1.0.0' }]);

      expect(() => {
        pluginManager.resolveDependencies([plugin1, plugin2]);
      }).toThrow('Circular dependency');
    });

    it('should throw error for missing dependencies', async () => {
      const plugin = createTestPlugin('plugin', [{ name: 'missing', version: '^1.0.0' }]);

      expect(() => {
        pluginManager.resolveDependencies([plugin]);
      }).toThrow('Missing dependency');
    });

    it('should check version compatibility', async () => {
      const corePlugin = createTestPlugin('core', [], '2.0.0');
      const dependentPlugin = createTestPlugin('dependent', [
        { name: 'core', version: '^1.0.0' }, // Requires 1.x
      ]);

      expect(() => {
        pluginManager.resolveDependencies([corePlugin, dependentPlugin]);
      }).toThrow('Version mismatch');
    });
  });

  describe('Hot Reload', () => {
    it('should reload plugin', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.initializeAll();

      const deactivateSpy = vi.spyOn(testPlugin, 'deactivate');
      const initializeSpy = vi.spyOn(testPlugin, 'initialize');

      await pluginManager.reloadPlugin('test-plugin');

      expect(deactivateSpy).toHaveBeenCalled();
      expect(initializeSpy).toHaveBeenCalled();
    });

    it('should call onHotReload hook', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      testPlugin.onHotReload = vi.fn();

      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.initializeAll();
      await pluginManager.reloadPlugin('test-plugin');

      expect(testPlugin.onHotReload).toHaveBeenCalled();
    });
  });

  describe('Plugin Lifecycle', () => {
    it('should initialize all plugins', async () => {
      const plugin1 = createTestPlugin('plugin1');
      const plugin2 = createTestPlugin('plugin2');

      await pluginManager.loadPlugin(plugin1);
      await pluginManager.loadPlugin(plugin2);

      const init1Spy = vi.spyOn(plugin1, 'initialize');
      const init2Spy = vi.spyOn(plugin2, 'initialize');

      await pluginManager.initializeAll();

      expect(init1Spy).toHaveBeenCalled();
      expect(init2Spy).toHaveBeenCalled();
    });

    it('should shutdown all plugins', async () => {
      const plugin1 = createTestPlugin('plugin1');
      const plugin2 = createTestPlugin('plugin2');

      await pluginManager.loadPlugin(plugin1);
      await pluginManager.loadPlugin(plugin2);
      await pluginManager.initializeAll();

      const shutdown1Spy = vi.spyOn(plugin1, 'shutdown');
      const shutdown2Spy = vi.spyOn(plugin2, 'shutdown');

      await pluginManager.shutdownAll();

      expect(shutdown1Spy).toHaveBeenCalled();
      expect(shutdown2Spy).toHaveBeenCalled();
    });

    it('should handle plugin errors gracefully', async () => {
      const faultyPlugin = createTestPlugin('faulty');
      faultyPlugin.initialize = async () => {
        throw new Error('Plugin error');
      };

      await pluginManager.loadPlugin(faultyPlugin);

      await expect(pluginManager.initializeAll()).rejects.toThrow();
      expect(mockLogger.error).toHaveBeenCalled();
    });
  });

  describe('Plugin Unloading', () => {
    it('should unload plugin', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.initializeAll();

      const shutdownSpy = vi.spyOn(testPlugin, 'shutdown');
      await pluginManager.unloadPlugin('test-plugin');

      expect(shutdownSpy).toHaveBeenCalled();
    });

    it('should remove plugin from registry', async () => {
      const testPlugin = createTestPlugin('test-plugin');
      await pluginManager.loadPlugin(testPlugin);
      await pluginManager.unloadPlugin('test-plugin');

      const plugins = pluginManager.getLoadedPlugins();
      expect(plugins).not.toContain('test-plugin');
    });
  });

  describe('Plugin Queries', () => {
    beforeEach(async () => {
      await pluginManager.loadPlugin(createTestPlugin('plugin1'));
      await pluginManager.loadPlugin(createTestPlugin('plugin2'));
    });

    it('should get loaded plugins', () => {
      const plugins = pluginManager.getLoadedPlugins();
      expect(plugins).toContain('plugin1');
      expect(plugins).toContain('plugin2');
    });

    it('should check if plugin is loaded', () => {
      expect(pluginManager.hasPlugin('plugin1')).toBe(true);
      expect(pluginManager.hasPlugin('nonexistent')).toBe(false);
    });

    it('should get plugin by name', () => {
      const plugin = pluginManager.getPlugin('plugin1');
      expect(plugin).toBeDefined();
      expect(plugin!.name).toBe('plugin1');
    });
  });
});

// Helper function to create test plugin
function createTestPlugin(
  name: string,
  dependencies: Array<{ name: string; version: string }> = [],
  version: string = '1.0.0'
): IPlugin {
  const metadata: PluginMetadata = {
    name,
    version,
    description: `Test plugin ${name}`,
    author: 'Test',
    dependencies,
  };

  return {
    name,
    version,
    dependencies,
    metadata,
    initialize: async (context: PluginContext) => {},
    activate: async () => {},
    deactivate: async () => {},
    shutdown: async () => {},
  };
}
