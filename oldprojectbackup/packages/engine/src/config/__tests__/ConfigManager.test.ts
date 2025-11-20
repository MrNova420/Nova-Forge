/**
 * @file ConfigManager.test.ts
 * @description Tests for configuration management
 */

import { describe, it, expect, beforeEach, vi } from 'vitest';
import { ConfigManager, ConfigSchema, COMMON_SCHEMAS } from '../ConfigManager';

describe('ConfigManager', () => {
  let configManager: ConfigManager;

  beforeEach(() => {
    configManager = new ConfigManager();
  });

  describe('Schema Registration', () => {
    it('should register schema', () => {
      configManager.registerSchema('test.value', {
        type: 'number',
        required: true,
        default: 42,
      });

      const value = configManager.get('test.value');
      expect(value).toBe(42);
    });

    it('should register multiple schemas', () => {
      configManager.registerSchemas(COMMON_SCHEMAS);
      
      const width = configManager.get('graphics.resolution.width');
      const height = configManager.get('graphics.resolution.height');
      
      expect(width).toBe(1920);
      expect(height).toBe(1080);
    });
  });

  describe('Configuration Loading', () => {
    beforeEach(() => {
      configManager.registerSchemas(COMMON_SCHEMAS);
    });

    it('should load configuration', async () => {
      await configManager.load({
        graphics: {
          resolution: {
            width: 2560,
            height: 1440,
          },
        },
      });

      expect(configManager.get('graphics.resolution.width')).toBe(2560);
      expect(configManager.get('graphics.resolution.height')).toBe(1440);
    });

    it('should load from JSON', async () => {
      const json = JSON.stringify({
        graphics: {
          vsync: false,
        },
      });

      await configManager.loadFromJSON(json);
      expect(configManager.get('graphics.vsync')).toBe(false);
    });

    it('should merge with defaults', async () => {
      await configManager.load({
        graphics: {
          resolution: {
            width: 3840,
          },
        },
      });

      expect(configManager.get('graphics.resolution.width')).toBe(3840);
      expect(configManager.get('graphics.resolution.height')).toBe(1080); // Default
    });
  });

  describe('Validation', () => {
    beforeEach(() => {
      configManager.registerSchema('test.number', {
        type: 'number',
        required: true,
        min: 0,
        max: 100,
      });

      configManager.registerSchema('test.string', {
        type: 'string',
        pattern: /^[a-z]+$/,
      });

      configManager.registerSchema('test.enum', {
        type: 'string',
        enum: ['low', 'medium', 'high'],
      });
    });

    it('should validate type', async () => {
      await expect(
        configManager.load({ test: { number: 'not a number' } })
      ).rejects.toThrow();
    });

    it('should validate required fields', async () => {
      await expect(
        configManager.load({})
      ).rejects.toThrow('Required value is missing');
    });

    it('should validate min/max', async () => {
      await expect(
        configManager.load({ test: { number: 150 } })
      ).rejects.toThrow();
    });

    it('should validate pattern', async () => {
      await expect(
        configManager.load({ test: { string: 'ABC123' } })
      ).rejects.toThrow();
    });

    it('should validate enum', async () => {
      await expect(
        configManager.load({ test: { enum: 'invalid' } })
      ).rejects.toThrow();
    });

    it('should allow valid values', async () => {
      await configManager.load({
        test: {
          number: 50,
          string: 'valid',
          enum: 'medium',
        },
      });

      expect(configManager.get('test.number')).toBe(50);
      expect(configManager.get('test.string')).toBe('valid');
      expect(configManager.get('test.enum')).toBe('medium');
    });
  });

  describe('Get/Set', () => {
    beforeEach(async () => {
      await configManager.load({
        app: {
          name: 'Test App',
          version: '1.0.0',
        },
      });
    });

    it('should get value by path', () => {
      expect(configManager.get('app.name')).toBe('Test App');
      expect(configManager.get('app.version')).toBe('1.0.0');
    });

    it('should get with default value', () => {
      expect(configManager.get('nonexistent', 'default')).toBe('default');
    });

    it('should set value', () => {
      configManager.set('app.name', 'New Name');
      expect(configManager.get('app.name')).toBe('New Name');
    });

    it('should check if has value', () => {
      expect(configManager.has('app.name')).toBe(true);
      expect(configManager.has('nonexistent')).toBe(false);
    });

    it('should delete value', () => {
      configManager.delete('app.name');
      expect(configManager.has('app.name')).toBe(false);
    });
  });

  describe('Change Watching', () => {
    beforeEach(async () => {
      await configManager.load({ test: { value: 42 } });
    });

    it('should watch for changes', () => {
      const callback = vi.fn();
      configManager.watch('test.value', callback);

      configManager.set('test.value', 100);

      expect(callback).toHaveBeenCalledWith({
        path: 'test.value',
        oldValue: 42,
        newValue: 100,
        timestamp: expect.any(Number),
      });
    });

    it('should watch wildcard', () => {
      const callback = vi.fn();
      configManager.watch('*', callback);

      configManager.set('test.value', 100);

      expect(callback).toHaveBeenCalled();
    });

    it('should unwatch', () => {
      const callback = vi.fn();
      configManager.watch('test.value', callback);
      configManager.unwatch('test.value', callback);

      configManager.set('test.value', 100);

      expect(callback).not.toHaveBeenCalled();
    });

    it('should track change history', () => {
      configManager.set('test.value', 100);
      configManager.set('test.value', 200);

      const history = configManager.getChangeHistory();
      expect(history).toHaveLength(2);
      expect(history[0].newValue).toBe(100);
      expect(history[1].newValue).toBe(200);
    });
  });

  describe('Import/Export', () => {
    beforeEach(async () => {
      configManager.registerSchemas(COMMON_SCHEMAS);
      await configManager.load({
        graphics: {
          resolution: {
            width: 1920,
            height: 1080,
          },
          vsync: true,
        },
      });
    });

    it('should export configuration', () => {
      const exported = configManager.export();

      expect(exported.config).toBeDefined();
      expect(exported.schema).toBeDefined();
      expect(exported.timestamp).toBeGreaterThan(0);
    });

    it('should import configuration', async () => {
      const exported = configManager.export();
      
      const newManager = new ConfigManager();
      await newManager.import(exported);

      expect(newManager.get('graphics.resolution.width')).toBe(1920);
    });

    it('should save to JSON', () => {
      const json = configManager.saveToJSON();
      const parsed = JSON.parse(json);

      expect(parsed.graphics.resolution.width).toBe(1920);
    });
  });

  describe('Reset and Clear', () => {
    beforeEach(async () => {
      configManager.registerSchema('test.value', {
        type: 'number',
        default: 42,
      });

      await configManager.load({ test: { value: 100 } });
    });

    it('should reset to defaults', () => {
      configManager.reset();
      expect(configManager.get('test.value')).toBe(42);
    });

    it('should clear configuration', () => {
      configManager.clear();
      expect(configManager.get('test.value')).toBe(42); // Falls back to default
    });
  });

  describe('Statistics', () => {
    beforeEach(async () => {
      configManager.registerSchemas(COMMON_SCHEMAS);
      await configManager.load({
        graphics: {
          resolution: {
            width: 1920,
            height: 1080,
          },
        },
      });
    });

    it('should get statistics', () => {
      const stats = configManager.getStats();

      expect(stats.totalKeys).toBeGreaterThan(0);
      expect(stats.schemasRegistered).toBe(Object.keys(COMMON_SCHEMAS).length);
      expect(stats.defaultsSet).toBeGreaterThan(0);
    });

    it('should track watchers', () => {
      configManager.watch('graphics.resolution.width', () => {});
      configManager.watch('graphics.resolution.height', () => {});

      const stats = configManager.getStats();
      expect(stats.watchers).toBe(2);
    });

    it('should track changes', () => {
      configManager.set('graphics.resolution.width', 2560);
      configManager.set('graphics.resolution.height', 1440);

      const stats = configManager.getStats();
      expect(stats.changes).toBe(2);
    });
  });
});
