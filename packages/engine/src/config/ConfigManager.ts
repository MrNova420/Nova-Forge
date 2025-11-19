/**
 * @file ConfigManager.ts
 * @description Configuration management system
 * Handles loading, saving, and validation of configuration data
 */

/**
 * Configuration schema for validation
 */
export interface ConfigSchema {
  type: 'string' | 'number' | 'boolean' | 'object' | 'array';
  required?: boolean;
  default?: any;
  min?: number;
  max?: number;
  pattern?: RegExp;
  enum?: any[];
  items?: ConfigSchema;
  properties?: Record<string, ConfigSchema>;
  validate?: (value: any) => boolean;
  description?: string;
}

/**
 * Configuration validation error
 */
export interface ValidationError {
  path: string;
  message: string;
  value: any;
}

/**
 * Configuration change event
 */
export interface ConfigChangeEvent {
  path: string;
  oldValue: any;
  newValue: any;
  timestamp: number;
}

/**
 * Configuration manager - handles all configuration operations
 */
export class ConfigManager {
  private config: Record<string, any> = {};
  private schema: Record<string, ConfigSchema> = {};
  private defaults: Record<string, any> = {};
  private listeners: Map<string, Array<(event: ConfigChangeEvent) => void>> = new Map();
  private changeHistory: ConfigChangeEvent[] = [];

  /**
   * Register configuration schema
   */
  registerSchema(path: string, schema: ConfigSchema): void {
    this.schema[path] = schema;
    
    if (schema.default !== undefined) {
      this.defaults[path] = schema.default;
    }
  }

  /**
   * Register multiple schemas
   */
  registerSchemas(schemas: Record<string, ConfigSchema>): void {
    for (const [path, schema] of Object.entries(schemas)) {
      this.registerSchema(path, schema);
    }
  }

  /**
   * Load configuration from object
   */
  async load(config: Record<string, any>): Promise<void> {
    // Validate configuration
    const errors = this.validate(config);
    if (errors.length > 0) {
      throw new Error(
        `Configuration validation failed:\n${errors.map(e => `  ${e.path}: ${e.message}`).join('\n')}`
      );
    }

    this.config = { ...this.defaults, ...config };
  }

  /**
   * Load configuration from JSON string
   */
  async loadFromJSON(json: string): Promise<void> {
    try {
      const config = JSON.parse(json);
      await this.load(config);
    } catch (error) {
      throw new Error(`Failed to parse configuration JSON: ${error}`);
    }
  }

  /**
   * Load configuration from file
   */
  async loadFromFile(path: string, fs: any): Promise<void> {
    try {
      const content = await fs.readFile(path);
      const contentStr = typeof content === 'string' ? content : content.toString();
      
      if (path.endsWith('.json')) {
        await this.loadFromJSON(contentStr);
      } else if (path.endsWith('.yaml') || path.endsWith('.yml')) {
        await this.loadFromYAML(contentStr);
      } else {
        throw new Error(`Unsupported configuration file format: ${path}`);
      }
    } catch (error) {
      throw new Error(`Failed to load configuration from ${path}: ${error}`);
    }
  }

  /**
   * Load configuration from YAML string
   */
  private async loadFromYAML(yaml: string): Promise<void> {
    // Would use a YAML parser in production
    throw new Error('YAML parsing not implemented');
  }

  /**
   * Save configuration to JSON string
   */
  saveToJSON(): string {
    return JSON.stringify(this.config, null, 2);
  }

  /**
   * Save configuration to file
   */
  async saveToFile(path: string, fs: any): Promise<void> {
    try {
      let content: string;
      
      if (path.endsWith('.json')) {
        content = this.saveToJSON();
      } else if (path.endsWith('.yaml') || path.endsWith('.yml')) {
        content = this.saveToYAML();
      } else {
        throw new Error(`Unsupported configuration file format: ${path}`);
      }

      await fs.writeFile(path, content);
    } catch (error) {
      throw new Error(`Failed to save configuration to ${path}: ${error}`);
    }
  }

  /**
   * Save configuration to YAML string
   */
  private saveToYAML(): string {
    // Would use a YAML serializer in production
    throw new Error('YAML serialization not implemented');
  }

  /**
   * Get configuration value
   */
  get<T = any>(path: string, defaultValue?: T): T {
    const value = this.getByPath(this.config, path);
    
    if (value === undefined) {
      if (defaultValue !== undefined) {
        return defaultValue;
      }
      
      const schemaDefault = this.defaults[path];
      if (schemaDefault !== undefined) {
        return schemaDefault as T;
      }
    }

    return value as T;
  }

  /**
   * Set configuration value
   */
  set(path: string, value: any): void {
    const oldValue = this.get(path);

    // Validate value against schema
    const schema = this.schema[path];
    if (schema) {
      const errors = this.validateValue(path, value, schema);
      if (errors.length > 0) {
        throw new Error(
          `Configuration validation failed:\n${errors.map(e => `  ${e.path}: ${e.message}`).join('\n')}`
        );
      }
    }

    // Set value
    this.setByPath(this.config, path, value);

    // Emit change event
    const event: ConfigChangeEvent = {
      path,
      oldValue,
      newValue: value,
      timestamp: Date.now(),
    };

    this.changeHistory.push(event);
    this.notifyListeners(path, event);
  }

  /**
   * Check if configuration has value
   */
  has(path: string): boolean {
    return this.getByPath(this.config, path) !== undefined;
  }

  /**
   * Delete configuration value
   */
  delete(path: string): void {
    this.set(path, undefined);
  }

  /**
   * Get all configuration
   */
  getAll(): Record<string, any> {
    return { ...this.config };
  }

  /**
   * Clear all configuration
   */
  clear(): void {
    this.config = { ...this.defaults };
    this.changeHistory = [];
  }

  /**
   * Reset configuration to defaults
   */
  reset(): void {
    for (const path of Object.keys(this.config)) {
      const defaultValue = this.defaults[path];
      if (defaultValue !== undefined) {
        this.set(path, defaultValue);
      }
    }
  }

  /**
   * Validate entire configuration
   */
  validate(config: Record<string, any>): ValidationError[] {
    const errors: ValidationError[] = [];

    for (const [path, schema] of Object.entries(this.schema)) {
      const value = this.getByPath(config, path);
      errors.push(...this.validateValue(path, value, schema));
    }

    return errors;
  }

  /**
   * Validate single value
   */
  private validateValue(path: string, value: any, schema: ConfigSchema): ValidationError[] {
    const errors: ValidationError[] = [];

    // Check required
    if (schema.required && value === undefined) {
      errors.push({
        path,
        message: 'Required value is missing',
        value,
      });
      return errors;
    }

    if (value === undefined) {
      return errors;
    }

    // Check type
    const actualType = Array.isArray(value) ? 'array' : typeof value;
    if (actualType !== schema.type) {
      errors.push({
        path,
        message: `Expected type ${schema.type}, got ${actualType}`,
        value,
      });
      return errors;
    }

    // Type-specific validation
    switch (schema.type) {
      case 'number':
        if (schema.min !== undefined && value < schema.min) {
          errors.push({
            path,
            message: `Value must be at least ${schema.min}`,
            value,
          });
        }
        if (schema.max !== undefined && value > schema.max) {
          errors.push({
            path,
            message: `Value must be at most ${schema.max}`,
            value,
          });
        }
        break;

      case 'string':
        if (schema.pattern && !schema.pattern.test(value)) {
          errors.push({
            path,
            message: `Value must match pattern ${schema.pattern}`,
            value,
          });
        }
        break;

      case 'array':
        if (schema.items) {
          for (let i = 0; i < value.length; i++) {
            errors.push(...this.validateValue(`${path}[${i}]`, value[i], schema.items));
          }
        }
        break;

      case 'object':
        if (schema.properties) {
          for (const [key, propSchema] of Object.entries(schema.properties)) {
            errors.push(...this.validateValue(`${path}.${key}`, value[key], propSchema));
          }
        }
        break;
    }

    // Check enum
    if (schema.enum && !schema.enum.includes(value)) {
      errors.push({
        path,
        message: `Value must be one of: ${schema.enum.join(', ')}`,
        value,
      });
    }

    // Custom validation
    if (schema.validate && !schema.validate(value)) {
      errors.push({
        path,
        message: 'Custom validation failed',
        value,
      });
    }

    return errors;
  }

  /**
   * Get value by path (dot notation)
   */
  private getByPath(obj: any, path: string): any {
    const parts = path.split('.');
    let current = obj;

    for (const part of parts) {
      if (current === undefined || current === null) {
        return undefined;
      }
      current = current[part];
    }

    return current;
  }

  /**
   * Set value by path (dot notation)
   */
  private setByPath(obj: any, path: string, value: any): void {
    const parts = path.split('.');
    let current = obj;

    for (let i = 0; i < parts.length - 1; i++) {
      const part = parts[i];
      if (!(part in current) || typeof current[part] !== 'object') {
        current[part] = {};
      }
      current = current[part];
    }

    const lastPart = parts[parts.length - 1];
    if (value === undefined) {
      delete current[lastPart];
    } else {
      current[lastPart] = value;
    }
  }

  /**
   * Watch for configuration changes
   */
  watch(path: string, callback: (event: ConfigChangeEvent) => void): void {
    if (!this.listeners.has(path)) {
      this.listeners.set(path, []);
    }
    this.listeners.get(path)!.push(callback);
  }

  /**
   * Stop watching for changes
   */
  unwatch(path: string, callback: (event: ConfigChangeEvent) => void): void {
    const callbacks = this.listeners.get(path);
    if (callbacks) {
      const index = callbacks.indexOf(callback);
      if (index !== -1) {
        callbacks.splice(index, 1);
      }
    }
  }

  /**
   * Notify listeners of changes
   */
  private notifyListeners(path: string, event: ConfigChangeEvent): void {
    // Notify exact path listeners
    const callbacks = this.listeners.get(path);
    if (callbacks) {
      for (const callback of callbacks) {
        try {
          callback(event);
        } catch (error) {
          console.error(`Error in config change listener:`, error);
        }
      }
    }

    // Notify wildcard listeners
    const wildcardCallbacks = this.listeners.get('*');
    if (wildcardCallbacks) {
      for (const callback of wildcardCallbacks) {
        try {
          callback(event);
        } catch (error) {
          console.error(`Error in config change listener:`, error);
        }
      }
    }

    // Notify parent path listeners
    const parts = path.split('.');
    for (let i = 1; i < parts.length; i++) {
      const parentPath = parts.slice(0, i).join('.');
      const parentCallbacks = this.listeners.get(parentPath);
      if (parentCallbacks) {
        for (const callback of parentCallbacks) {
          try {
            callback(event);
          } catch (error) {
            console.error(`Error in config change listener:`, error);
          }
        }
      }
    }
  }

  /**
   * Get change history
   */
  getChangeHistory(): ConfigChangeEvent[] {
    return [...this.changeHistory];
  }

  /**
   * Clear change history
   */
  clearChangeHistory(): void {
    this.changeHistory = [];
  }

  /**
   * Export configuration with metadata
   */
  export(): {
    config: Record<string, any>;
    schema: Record<string, ConfigSchema>;
    timestamp: number;
  } {
    return {
      config: this.getAll(),
      schema: { ...this.schema },
      timestamp: Date.now(),
    };
  }

  /**
   * Import configuration with metadata
   */
  async import(data: {
    config: Record<string, any>;
    schema?: Record<string, ConfigSchema>;
  }): Promise<void> {
    if (data.schema) {
      this.registerSchemas(data.schema);
    }
    await this.load(data.config);
  }

  /**
   * Get statistics
   */
  getStats(): {
    totalKeys: number;
    schemasRegistered: number;
    defaultsSet: number;
    watchers: number;
    changes: number;
  } {
    let totalKeys = 0;
    const countKeys = (obj: any) => {
      for (const value of Object.values(obj)) {
        totalKeys++;
        if (value && typeof value === 'object' && !Array.isArray(value)) {
          countKeys(value);
        }
      }
    };
    countKeys(this.config);

    let totalWatchers = 0;
    for (const callbacks of this.listeners.values()) {
      totalWatchers += callbacks.length;
    }

    return {
      totalKeys,
      schemasRegistered: Object.keys(this.schema).length,
      defaultsSet: Object.keys(this.defaults).length,
      watchers: totalWatchers,
      changes: this.changeHistory.length,
    };
  }
}

/**
 * Common configuration schemas
 */
export const COMMON_SCHEMAS: Record<string, ConfigSchema> = {
  'graphics.resolution.width': {
    type: 'number',
    required: true,
    default: 1920,
    min: 640,
    max: 7680,
    description: 'Screen width in pixels',
  },
  'graphics.resolution.height': {
    type: 'number',
    required: true,
    default: 1080,
    min: 480,
    max: 4320,
    description: 'Screen height in pixels',
  },
  'graphics.vsync': {
    type: 'boolean',
    required: false,
    default: true,
    description: 'Enable vertical sync',
  },
  'graphics.fullscreen': {
    type: 'boolean',
    required: false,
    default: false,
    description: 'Fullscreen mode',
  },
  'audio.masterVolume': {
    type: 'number',
    required: false,
    default: 1.0,
    min: 0.0,
    max: 1.0,
    description: 'Master audio volume',
  },
  'input.mouseSensitivity': {
    type: 'number',
    required: false,
    default: 1.0,
    min: 0.1,
    max: 10.0,
    description: 'Mouse sensitivity',
  },
};
