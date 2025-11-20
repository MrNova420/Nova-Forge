/**
 * @file ExtensionPointRegistry.ts
 * @description Extension point system for plugin extensibility
 * Allows plugins to register extensions at predefined extension points
 */

/**
 * Extension point schema definition
 */
export interface ExtensionSchema {
  id: string;
  description: string;
  required: string[];
  optional?: string[];
  validate?: (extension: any) => boolean;
}

/**
 * Extension point definition
 */
export interface ExtensionPoint<T = any> {
  id: string;
  schema: ExtensionSchema;
  extensions: T[];
}

/**
 * Extension metadata
 */
export interface ExtensionMetadata {
  id: string;
  pluginName: string;
  priority: number;
  enabled: boolean;
}

/**
 * Extension point registry - manages all extension points
 */
export class ExtensionPointRegistry {
  private points: Map<string, ExtensionPoint> = new Map();
  private extensions: Map<string, Array<{ extension: any; metadata: ExtensionMetadata }>> = new Map();

  /**
   * Define a new extension point
   */
  defineExtensionPoint<T>(schema: ExtensionSchema): void {
    if (this.points.has(schema.id)) {
      throw new Error(`Extension point already defined: ${schema.id}`);
    }

    this.points.set(schema.id, {
      id: schema.id,
      schema,
      extensions: [],
    });

    this.extensions.set(schema.id, []);
  }

  /**
   * Register an extension at a point
   */
  registerExtension<T>(
    pointId: string,
    extension: T,
    metadata: Partial<ExtensionMetadata> = {}
  ): void {
    const point = this.points.get(pointId);
    if (!point) {
      throw new Error(`Extension point not found: ${pointId}`);
    }

    // Validate extension against schema
    if (point.schema.validate && !point.schema.validate(extension)) {
      throw new Error(`Extension validation failed for point: ${pointId}`);
    }

    // Check required fields
    for (const field of point.schema.required) {
      if (!(extension as any)[field]) {
        throw new Error(`Extension missing required field: ${field}`);
      }
    }

    // Create full metadata
    const fullMetadata: ExtensionMetadata = {
      id: metadata.id || this.generateExtensionId(),
      pluginName: metadata.pluginName || 'unknown',
      priority: metadata.priority || 0,
      enabled: metadata.enabled !== false,
    };

    const exts = this.extensions.get(pointId)!;
    exts.push({ extension, metadata: fullMetadata });

    // Sort by priority (higher priority first)
    exts.sort((a, b) => b.metadata.priority - a.metadata.priority);

    // Update point extensions
    point.extensions = exts.map(e => e.extension);
  }

  /**
   * Unregister an extension
   */
  unregisterExtension<T>(pointId: string, extensionIdOrInstance: string | T): void {
    const exts = this.extensions.get(pointId);
    if (!exts) return;

    let index = -1;
    if (typeof extensionIdOrInstance === 'string') {
      index = exts.findIndex(e => e.metadata.id === extensionIdOrInstance);
    } else {
      index = exts.findIndex(e => e.extension === extensionIdOrInstance);
    }

    if (index !== -1) {
      exts.splice(index, 1);

      // Update point extensions
      const point = this.points.get(pointId);
      if (point) {
        point.extensions = exts.map(e => e.extension);
      }
    }
  }

  /**
   * Get all extensions at a point
   */
  getExtensions<T>(pointId: string): T[] {
    const point = this.points.get(pointId);
    if (!point) {
      return [];
    }

    const exts = this.extensions.get(pointId) || [];
    return exts
      .filter(e => e.metadata.enabled)
      .map(e => e.extension as T);
  }

  /**
   * Get extensions sorted by priority
   */
  getExtensionsSorted<T>(pointId: string): T[] {
    return this.getExtensions<T>(pointId);
  }

  /**
   * Get extension metadata
   */
  getExtensionMetadata(pointId: string, extensionId: string): ExtensionMetadata | null {
    const exts = this.extensions.get(pointId);
    if (!exts) return null;

    const ext = exts.find(e => e.metadata.id === extensionId);
    return ext ? ext.metadata : null;
  }

  /**
   * Enable extension
   */
  enableExtension(pointId: string, extensionId: string): void {
    const metadata = this.getExtensionMetadata(pointId, extensionId);
    if (metadata) {
      metadata.enabled = true;
      this.updatePointExtensions(pointId);
    }
  }

  /**
   * Disable extension
   */
  disableExtension(pointId: string, extensionId: string): void {
    const metadata = this.getExtensionMetadata(pointId, extensionId);
    if (metadata) {
      metadata.enabled = false;
      this.updatePointExtensions(pointId);
    }
  }

  /**
   * Check if extension point exists
   */
  hasExtensionPoint(pointId: string): boolean {
    return this.points.has(pointId);
  }

  /**
   * Get all extension points
   */
  getAllExtensionPoints(): ExtensionPoint[] {
    return Array.from(this.points.values());
  }

  /**
   * Get extensions by plugin
   */
  getExtensionsByPlugin(pluginName: string): Map<string, any[]> {
    const result = new Map<string, any[]>();

    for (const [pointId, exts] of this.extensions) {
      const pluginExts = exts
        .filter(e => e.metadata.pluginName === pluginName)
        .map(e => e.extension);

      if (pluginExts.length > 0) {
        result.set(pointId, pluginExts);
      }
    }

    return result;
  }

  /**
   * Remove all extensions from a plugin
   */
  removePluginExtensions(pluginName: string): void {
    for (const [pointId, exts] of this.extensions) {
      const filtered = exts.filter(e => e.metadata.pluginName !== pluginName);
      this.extensions.set(pointId, filtered);
      this.updatePointExtensions(pointId);
    }
  }

  /**
   * Clear all extensions at a point
   */
  clearExtensions(pointId: string): void {
    if (this.extensions.has(pointId)) {
      this.extensions.set(pointId, []);
      this.updatePointExtensions(pointId);
    }
  }

  /**
   * Clear all extension points
   */
  clearAll(): void {
    this.points.clear();
    this.extensions.clear();
  }

  /**
   * Update point extensions from registry
   */
  private updatePointExtensions(pointId: string): void {
    const point = this.points.get(pointId);
    if (!point) return;

    const exts = this.extensions.get(pointId) || [];
    point.extensions = exts
      .filter(e => e.metadata.enabled)
      .map(e => e.extension);
  }

  /**
   * Generate unique extension ID
   */
  private generateExtensionId(): string {
    return `ext_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }

  /**
   * Get statistics
   */
  getStats(): {
    totalPoints: number;
    totalExtensions: number;
    enabledExtensions: number;
    averageExtensionsPerPoint: number;
  } {
    let totalExts = 0;
    let enabledExts = 0;

    for (const exts of this.extensions.values()) {
      totalExts += exts.length;
      enabledExts += exts.filter(e => e.metadata.enabled).length;
    }

    return {
      totalPoints: this.points.size,
      totalExtensions: totalExts,
      enabledExtensions: enabledExts,
      averageExtensionsPerPoint:
        this.points.size > 0 ? totalExts / this.points.size : 0,
    };
  }
}

/**
 * Built-in extension points for the engine
 */
export const BUILTIN_EXTENSION_POINTS: ExtensionSchema[] = [
  {
    id: 'engine.renderer',
    description: 'Custom renderer implementations',
    required: ['name', 'render'],
    optional: ['initialize', 'shutdown', 'resize'],
  },
  {
    id: 'engine.input',
    description: 'Input handler extensions',
    required: ['name', 'handleInput'],
    optional: ['initialize', 'shutdown'],
  },
  {
    id: 'engine.asset-loader',
    description: 'Custom asset loaders',
    required: ['extensions', 'load'],
    optional: ['canLoad', 'priority'],
  },
  {
    id: 'engine.serializer',
    description: 'Serialization format handlers',
    required: ['format', 'serialize', 'deserialize'],
    optional: ['canHandle', 'priority'],
  },
  {
    id: 'engine.editor-tool',
    description: 'Editor tooling extensions',
    required: ['name', 'icon', 'activate'],
    optional: ['deactivate', 'shortcuts'],
  },
  {
    id: 'engine.debug-visualizer',
    description: 'Debug visualization overlays',
    required: ['name', 'render'],
    optional: ['enabled', 'priority'],
  },
  {
    id: 'engine.physics-solver',
    description: 'Custom physics solvers',
    required: ['name', 'solve'],
    optional: ['initialize', 'shutdown'],
  },
  {
    id: 'engine.audio-processor',
    description: 'Audio effect processors',
    required: ['name', 'process'],
    optional: ['parameters', 'initialize'],
  },
  {
    id: 'engine.network-protocol',
    description: 'Network protocol handlers',
    required: ['protocol', 'send', 'receive'],
    optional: ['connect', 'disconnect'],
  },
  {
    id: 'engine.script-runtime',
    description: 'Scripting language runtimes',
    required: ['language', 'execute'],
    optional: ['initialize', 'cleanup'],
  },
];

/**
 * Initialize built-in extension points
 */
export function initializeBuiltinExtensionPoints(registry: ExtensionPointRegistry): void {
  for (const schema of BUILTIN_EXTENSION_POINTS) {
    registry.defineExtensionPoint(schema);
  }
}
