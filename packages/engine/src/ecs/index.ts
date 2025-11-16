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
