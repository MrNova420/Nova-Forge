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
 * Entity-Component-System architecture
 * @packageDocumentation
 */

export { Entity } from './Entity';
export type { EntityId } from './Entity';
export { Component, ComponentRegistry } from './Component';
export type {
  IComponent,
  ComponentConstructor,
  ComponentType,
} from './Component';
export { System } from './System';
export type { ISystem } from './System';
export { World } from './World';
export type { ComponentQuery } from './World';

// Built-in components
export { TransformComponent } from './components/TransformComponent';
