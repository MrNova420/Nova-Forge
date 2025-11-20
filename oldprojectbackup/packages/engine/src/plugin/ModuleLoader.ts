/**
 * @file ModuleLoader.ts
 * @description ES Module-style dynamic module loading system
 * Supports dynamic imports, path resolution, and dependency management
 */

/**
 * Module import specification
 */
export interface ModuleImport {
  moduleId: string;
  name: string;
  as?: string;
}

/**
 * Module definition
 */
export interface IModule {
  readonly id: string;
  readonly exports: Record<string, any>;
  readonly imports: ModuleImport[];
  
  load(): Promise<void>;
  unload(): Promise<void>;
  resolveDependencies(): Promise<void>;
}

/**
 * Module factory function
 */
export type ModuleFactory = (require: ModuleRequire, exports: any, module: any) => void;

/**
 * Module require function
 */
export type ModuleRequire = (moduleId: string) => any;

/**
 * Module resolver interface
 */
export interface ModuleResolver {
  resolve(moduleId: string, fromModule?: string): string | null;
  canResolve(moduleId: string): boolean;
}

/**
 * Circular dependency error
 */
export interface CircularDepError {
  cycle: string[];
  message: string;
}

/**
 * Module loader - handles dynamic module loading
 */
export class ModuleLoader {
  private modules: Map<string, IModule> = new Map();
  private factories: Map<string, ModuleFactory> = new Map();
  private resolvers: ModuleResolver[] = [];
  private loading: Set<string> = new Set();

  /**
   * Register a module factory
   */
  registerModule(id: string, factory: ModuleFactory): void {
    this.factories.set(id, factory);
  }

  /**
   * Add module resolver
   */
  addResolver(resolver: ModuleResolver): void {
    this.resolvers.push(resolver);
  }

  /**
   * Dynamic import with async loading
   */
  async import<T = any>(moduleId: string): Promise<T> {
    // Check cache
    if (this.modules.has(moduleId)) {
      const module = this.modules.get(moduleId)!;
      return module.exports as T;
    }

    // Check if currently loading (circular)
    if (this.loading.has(moduleId)) {
      throw new Error(`Circular dependency detected while loading: ${moduleId}`);
    }

    // Mark as loading
    this.loading.add(moduleId);

    try {
      // Resolve module path
      const resolved = this.resolveModule(moduleId);
      if (!resolved) {
        throw new Error(`Cannot resolve module: ${moduleId}`);
      }

      // Load module
      const module = await this.loadModule(resolved);

      // Store in cache
      this.modules.set(moduleId, module);
      this.modules.set(resolved, module);

      // Unmark as loading
      this.loading.delete(moduleId);

      return module.exports as T;
    } catch (error) {
      this.loading.delete(moduleId);
      throw error;
    }
  }

  /**
   * Load module from factory or file
   */
  private async loadModule(moduleId: string): Promise<IModule> {
    // Check if factory is registered
    if (this.factories.has(moduleId)) {
      return await this.loadModuleFromFactory(moduleId);
    }

    // Try to load from file
    return await this.loadModuleFromFile(moduleId);
  }

  /**
   * Load module from registered factory
   */
  private async loadModuleFromFactory(moduleId: string): Promise<IModule> {
    const factory = this.factories.get(moduleId)!;
    const exports: Record<string, any> = {};
    const imports: ModuleImport[] = [];

    const moduleObj = {
      id: moduleId,
      exports,
      imports,
    };

    const require: ModuleRequire = (depId: string) => {
      const dep = this.modules.get(depId);
      if (!dep) {
        throw new Error(`Module not found: ${depId}`);
      }
      imports.push({ moduleId: depId, name: '*' });
      return dep.exports;
    };

    // Execute factory
    factory(require, exports, moduleObj);

    return {
      id: moduleId,
      exports: moduleObj.exports,
      imports,
      async load() {},
      async unload() {},
      async resolveDependencies() {},
    };
  }

  /**
   * Load module from file (platform-specific)
   */
  private async loadModuleFromFile(moduleId: string): Promise<IModule> {
    try {
      // This would be implemented differently for each platform
      const imported = await import(moduleId);

      return {
        id: moduleId,
        exports: imported,
        imports: [],
        async load() {},
        async unload() {},
        async resolveDependencies() {},
      };
    } catch (error) {
      throw new Error(`Failed to load module from file: ${moduleId} - ${error}`);
    }
  }

  /**
   * Resolve module path
   */
  resolveModule(id: string, fromModule?: string): string | null {
    // Absolute path
    if (id.startsWith('/')) {
      return id;
    }

    // Relative path
    if (id.startsWith('./') || id.startsWith('../')) {
      if (!fromModule) {
        throw new Error('Cannot resolve relative module without context');
      }
      return this.resolveRelative(id, fromModule);
    }

    // Try resolvers
    for (const resolver of this.resolvers) {
      if (resolver.canResolve(id)) {
        const resolved = resolver.resolve(id, fromModule);
        if (resolved) {
          return resolved;
        }
      }
    }

    // Package name - try node_modules style resolution
    return this.resolvePackage(id);
  }

  /**
   * Resolve relative module path
   */
  private resolveRelative(id: string, fromModule: string): string {
    const fromParts = fromModule.split('/');
    fromParts.pop(); // Remove filename

    const idParts = id.split('/');

    for (const part of idParts) {
      if (part === '.') {
        continue;
      } else if (part === '..') {
        fromParts.pop();
      } else {
        fromParts.push(part);
      }
    }

    return fromParts.join('/');
  }

  /**
   * Resolve package name
   */
  private resolvePackage(id: string): string | null {
    // Simplified - would search node_modules in production
    return `/node_modules/${id}/index.js`;
  }

  /**
   * Detect circular dependencies
   */
  detectCircularDependencies(modules: IModule[]): CircularDepError[] {
    const errors: CircularDepError[] = [];
    const visited = new Set<string>();
    const stack: string[] = [];

    const visit = (module: IModule) => {
      if (stack.includes(module.id)) {
        // Found cycle
        const cycleStart = stack.indexOf(module.id);
        const cycle = [...stack.slice(cycleStart), module.id];
        errors.push({
          cycle,
          message: `Circular dependency: ${cycle.join(' -> ')}`,
        });
        return;
      }

      if (visited.has(module.id)) {
        return;
      }

      stack.push(module.id);
      visited.add(module.id);

      for (const imp of module.imports) {
        const depModule = modules.find(m => m.id === imp.moduleId);
        if (depModule) {
          visit(depModule);
        }
      }

      stack.pop();
    };

    for (const module of modules) {
      visit(module);
    }

    return errors;
  }

  /**
   * Get module by ID
   */
  getModule(id: string): IModule | undefined {
    return this.modules.get(id);
  }

  /**
   * Check if module is loaded
   */
  hasModule(id: string): boolean {
    return this.modules.has(id);
  }

  /**
   * Unload module
   */
  async unloadModule(id: string): Promise<void> {
    const module = this.modules.get(id);
    if (!module) {
      return;
    }

    await module.unload();
    this.modules.delete(id);
  }

  /**
   * Clear all modules
   */
  clearAll(): void {
    this.modules.clear();
    this.factories.clear();
    this.loading.clear();
  }

  /**
   * Get all loaded modules
   */
  getAllModules(): IModule[] {
    return Array.from(this.modules.values());
  }

  /**
   * Get module dependency graph
   */
  getDependencyGraph(): Map<string, string[]> {
    const graph = new Map<string, string[]>();

    for (const module of this.modules.values()) {
      const deps = module.imports.map(imp => imp.moduleId);
      graph.set(module.id, deps);
    }

    return graph;
  }

  /**
   * Get statistics
   */
  getStats(): {
    totalModules: number;
    loadingModules: number;
    totalDependencies: number;
    averageDependencies: number;
  } {
    let totalDeps = 0;

    for (const module of this.modules.values()) {
      totalDeps += module.imports.length;
    }

    return {
      totalModules: this.modules.size,
      loadingModules: this.loading.size,
      totalDependencies: totalDeps,
      averageDependencies:
        this.modules.size > 0 ? totalDeps / this.modules.size : 0,
    };
  }
}

/**
 * Node.js style module resolver
 */
export class NodeModuleResolver implements ModuleResolver {
  private basePaths: string[] = [];

  constructor(basePaths: string[] = []) {
    this.basePaths = basePaths;
  }

  addBasePath(path: string): void {
    if (!this.basePaths.includes(path)) {
      this.basePaths.push(path);
    }
  }

  canResolve(moduleId: string): boolean {
    // Can resolve package names (not starting with ./ or /)
    return !moduleId.startsWith('./') && !moduleId.startsWith('/');
  }

  resolve(moduleId: string, fromModule?: string): string | null {
    // Try each base path
    for (const basePath of this.basePaths) {
      const candidates = [
        `${basePath}/node_modules/${moduleId}/index.js`,
        `${basePath}/node_modules/${moduleId}.js`,
        `${basePath}/node_modules/${moduleId}/index.ts`,
        `${basePath}/node_modules/${moduleId}.ts`,
      ];

      // In production, would check if file exists
      // For now, return first candidate
      return candidates[0];
    }

    return null;
  }
}

/**
 * URL-based module resolver
 */
export class URLModuleResolver implements ModuleResolver {
  private baseUrl: string;

  constructor(baseUrl: string) {
    this.baseUrl = baseUrl;
  }

  canResolve(moduleId: string): boolean {
    return moduleId.startsWith('http://') || moduleId.startsWith('https://');
  }

  resolve(moduleId: string): string | null {
    if (this.canResolve(moduleId)) {
      return moduleId;
    }

    return `${this.baseUrl}/${moduleId}`;
  }
}
