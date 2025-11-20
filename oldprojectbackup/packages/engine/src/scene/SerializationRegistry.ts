/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * SerializationRegistry
 *
 * Registry for component serializers and deserializers.
 * Allows custom components to register their own serialization logic.
 */

import { IComponent } from '../ecs/Component';

/**
 * Component serializer function
 */
export type ComponentSerializer<T extends IComponent = IComponent> = (
  component: T
) => any;

/**
 * Component deserializer function
 */
export type ComponentDeserializer<T extends IComponent = IComponent> = (
  data: any
) => T;

/**
 * Registry for component serialization
 */
export class SerializationRegistry {
  private _serializers: Map<string, ComponentSerializer> = new Map();
  private _deserializers: Map<string, ComponentDeserializer> = new Map();

  /**
   * Register a component serializer
   * @param componentType - Component class name
   * @param serializer - Serialization function
   * @param deserializer - Deserialization function
   */
  register<T extends IComponent>(
    componentType: string,
    serializer: ComponentSerializer<T>,
    deserializer: ComponentDeserializer<T>
  ): void {
    this._serializers.set(componentType, serializer as ComponentSerializer);
    this._deserializers.set(
      componentType,
      deserializer as ComponentDeserializer
    );
  }

  /**
   * Unregister a component serializer
   * @param componentType - Component class name
   */
  unregister(componentType: string): void {
    this._serializers.delete(componentType);
    this._deserializers.delete(componentType);
  }

  /**
   * Get serializer for a component type
   * @param componentType - Component class name
   * @returns Serializer function or undefined
   */
  getSerializer(componentType: string): ComponentSerializer | undefined {
    return this._serializers.get(componentType);
  }

  /**
   * Get deserializer for a component type
   * @param componentType - Component class name
   * @returns Deserializer function or undefined
   */
  getDeserializer(componentType: string): ComponentDeserializer | undefined {
    return this._deserializers.get(componentType);
  }

  /**
   * Check if a component type is registered
   * @param componentType - Component class name
   */
  has(componentType: string): boolean {
    return (
      this._serializers.has(componentType) &&
      this._deserializers.has(componentType)
    );
  }

  /**
   * Get all registered component types
   */
  getRegisteredTypes(): string[] {
    return Array.from(this._serializers.keys());
  }

  /**
   * Clear all registrations
   */
  clear(): void {
    this._serializers.clear();
    this._deserializers.clear();
  }
}
