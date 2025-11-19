/**
 * @file Serializer.ts
 * @description Advanced serialization system
 * Supports multiple formats and custom type serialization
 */

/**
 * Serialization format
 */
export enum SerializationFormat {
  JSON = 'json',
  BINARY = 'binary',
  YAML = 'yaml',
  XML = 'xml',
  MessagePack = 'messagepack',
}

/**
 * Serializable type metadata
 */
export interface TypeMetadata {
  typeName: string;
  version: number;
  serialize: (obj: any) => any;
  deserialize: (data: any) => any;
}

/**
 * Serialization context
 */
export interface SerializationContext {
  format: SerializationFormat;
  references: Map<any, number>;
  nextId: number;
  includeMetadata: boolean;
}

/**
 * Serializer - handles object serialization and deserialization
 */
export class Serializer {
  private typeRegistry: Map<string, TypeMetadata> = new Map();
  private formatHandlers: Map<SerializationFormat, FormatHandler> = new Map();

  constructor() {
    // Register default format handlers
    this.formatHandlers.set(SerializationFormat.JSON, new JSONHandler());
    this.formatHandlers.set(SerializationFormat.BINARY, new BinaryHandler());
  }

  /**
   * Register custom type for serialization
   */
  registerType(metadata: TypeMetadata): void {
    this.typeRegistry.set(metadata.typeName, metadata);
  }

  /**
   * Register multiple types
   */
  registerTypes(types: TypeMetadata[]): void {
    for (const type of types) {
      this.registerType(type);
    }
  }

  /**
   * Serialize object
   */
  serialize(
    obj: any,
    format: SerializationFormat = SerializationFormat.JSON,
    options: SerializationOptions = {}
  ): string | ArrayBuffer {
    const context: SerializationContext = {
      format,
      references: new Map(),
      nextId: 0,
      includeMetadata: options.includeMetadata !== false,
    };

    const serialized = this.serializeValue(obj, context);

    const handler = this.formatHandlers.get(format);
    if (!handler) {
      throw new Error(`No handler registered for format: ${format}`);
    }

    return handler.encode(serialized, options);
  }

  /**
   * Deserialize object
   */
  deserialize(
    data: string | ArrayBuffer,
    format: SerializationFormat = SerializationFormat.JSON,
    options: DeserializationOptions = {}
  ): any {
    const handler = this.formatHandlers.get(format);
    if (!handler) {
      throw new Error(`No handler registered for format: ${format}`);
    }

    const decoded = handler.decode(data, options);
    return this.deserializeValue(decoded, format);
  }

  /**
   * Serialize value (internal)
   */
  private serializeValue(value: any, context: SerializationContext): any {
    // Handle null and undefined
    if (value === null || value === undefined) {
      return value;
    }

    // Handle primitives
    const type = typeof value;
    if (type === 'string' || type === 'number' || type === 'boolean') {
      return value;
    }

    // Handle circular references
    if (context.references.has(value)) {
      return {
        __ref: context.references.get(value),
      };
    }

    // Register reference
    context.references.set(value, context.nextId++);

    // Handle arrays
    if (Array.isArray(value)) {
      return value.map(item => this.serializeValue(item, context));
    }

    // Handle dates
    if (value instanceof Date) {
      return {
        __type: 'Date',
        __value: value.toISOString(),
      };
    }

    // Handle registered types
    const typeName = value.constructor?.name;
    if (typeName && this.typeRegistry.has(typeName)) {
      const metadata = this.typeRegistry.get(typeName)!;
      return {
        __type: typeName,
        __version: metadata.version,
        __value: metadata.serialize(value),
      };
    }

    // Handle plain objects
    if (value.constructor === Object || !value.constructor) {
      const serialized: any = {};
      for (const [key, val] of Object.entries(value)) {
        serialized[key] = this.serializeValue(val, context);
      }
      return serialized;
    }

    // Unknown type - try to serialize as plain object
    console.warn(`Serializing unknown type: ${typeName}`);
    const serialized: any = {};
    for (const [key, val] of Object.entries(value)) {
      serialized[key] = this.serializeValue(val, context);
    }
    return {
      __type: typeName || 'Unknown',
      __value: serialized,
    };
  }

  /**
   * Deserialize value (internal)
   */
  private deserializeValue(value: any, format: SerializationFormat): any {
    // Handle null and undefined
    if (value === null || value === undefined) {
      return value;
    }

    // Handle primitives
    const type = typeof value;
    if (type === 'string' || type === 'number' || type === 'boolean') {
      return value;
    }

    // Handle arrays
    if (Array.isArray(value)) {
      return value.map(item => this.deserializeValue(item, format));
    }

    // Handle special types
    if (value.__type) {
      // Handle Date
      if (value.__type === 'Date') {
        return new Date(value.__value);
      }

      // Handle registered types
      if (this.typeRegistry.has(value.__type)) {
        const metadata = this.typeRegistry.get(value.__type)!;
        return metadata.deserialize(value.__value);
      }

      // Unknown type - return as plain object
      console.warn(`Deserializing unknown type: ${value.__type}`);
      return this.deserializeValue(value.__value, format);
    }

    // Handle references
    if (value.__ref !== undefined) {
      // Would need to track references during deserialization
      console.warn('Reference deserialization not fully implemented');
      return null;
    }

    // Handle plain objects
    const deserialized: any = {};
    for (const [key, val] of Object.entries(value)) {
      deserialized[key] = this.deserializeValue(val, format);
    }
    return deserialized;
  }

  /**
   * Clone object via serialization
   */
  clone<T>(obj: T, format: SerializationFormat = SerializationFormat.JSON): T {
    const serialized = this.serialize(obj, format);
    return this.deserialize(serialized, format) as T;
  }

  /**
   * Check if type is registered
   */
  hasType(typeName: string): boolean {
    return this.typeRegistry.has(typeName);
  }

  /**
   * Get registered types
   */
  getRegisteredTypes(): string[] {
    return Array.from(this.typeRegistry.keys());
  }

  /**
   * Register format handler
   */
  registerFormatHandler(format: SerializationFormat, handler: FormatHandler): void {
    this.formatHandlers.set(format, handler);
  }

  /**
   * Get statistics
   */
  getStats(): {
    registeredTypes: number;
    registeredFormats: number;
  } {
    return {
      registeredTypes: this.typeRegistry.size,
      registeredFormats: this.formatHandlers.size,
    };
  }
}

/**
 * Format handler interface
 */
export interface FormatHandler {
  encode(data: any, options: SerializationOptions): string | ArrayBuffer;
  decode(data: string | ArrayBuffer, options: DeserializationOptions): any;
}

/**
 * JSON format handler
 */
export class JSONHandler implements FormatHandler {
  encode(data: any, options: SerializationOptions): string {
    return JSON.stringify(data, null, options.pretty ? 2 : undefined);
  }

  decode(data: string | ArrayBuffer, options: DeserializationOptions): any {
    const str = typeof data === 'string' ? data : new TextDecoder().decode(data);
    return JSON.parse(str);
  }
}

/**
 * Binary format handler (simplified)
 */
export class BinaryHandler implements FormatHandler {
  encode(data: any, options: SerializationOptions): ArrayBuffer {
    // Simplified binary encoding - would use MessagePack or similar in production
    const json = JSON.stringify(data);
    const encoder = new TextEncoder();
    return encoder.encode(json).buffer;
  }

  decode(data: string | ArrayBuffer, options: DeserializationOptions): any {
    const buffer = typeof data === 'string' 
      ? new TextEncoder().encode(data).buffer 
      : data;
    const decoder = new TextDecoder();
    const json = decoder.decode(buffer);
    return JSON.parse(json);
  }
}

/**
 * Serialization options
 */
export interface SerializationOptions {
  pretty?: boolean;
  includeMetadata?: boolean;
  includeVersion?: boolean;
}

/**
 * Deserialization options
 */
export interface DeserializationOptions {
  strict?: boolean;
  allowUnknownTypes?: boolean;
}

/**
 * Common type serializers
 */
export const COMMON_TYPE_SERIALIZERS: TypeMetadata[] = [
  {
    typeName: 'Vector3',
    version: 1,
    serialize: (v: any) => ({ x: v.x, y: v.y, z: v.z }),
    deserialize: (d: any) => ({ x: d.x, y: d.y, z: d.z }),
  },
  {
    typeName: 'Quaternion',
    version: 1,
    serialize: (q: any) => ({ x: q.x, y: q.y, z: q.z, w: q.w }),
    deserialize: (d: any) => ({ x: d.x, y: d.y, z: d.z, w: d.w }),
  },
  {
    typeName: 'Matrix4',
    version: 1,
    serialize: (m: any) => ({ elements: Array.from(m.elements || []) }),
    deserialize: (d: any) => ({ elements: d.elements }),
  },
  {
    typeName: 'Color',
    version: 1,
    serialize: (c: any) => ({ r: c.r, g: c.g, b: c.b, a: c.a || 1 }),
    deserialize: (d: any) => ({ r: d.r, g: d.g, b: d.b, a: d.a }),
  },
];

/**
 * Create default serializer with common types
 */
export function createDefaultSerializer(): Serializer {
  const serializer = new Serializer();
  serializer.registerTypes(COMMON_TYPE_SERIALIZERS);
  return serializer;
}
