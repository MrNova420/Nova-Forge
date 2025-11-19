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
 * Scene
 *
 * Represents a 3D scene containing entities and resources.
 * Acts as a container for all game objects that should be rendered.
 * Now includes hierarchical scene graph support.
 */

import { Entity } from '../ecs/Entity';
import { SceneGraph } from './SceneGraph';
import { SceneNode } from './SceneNode';

/**
 * Scene class that manages a collection of entities and scene graph
 */
export class Scene {
  private _name: string;
  private _entities: Set<Entity>;
  private _sceneGraph: SceneGraph;

  /**
   * Create a new scene
   * @param name - Scene name
   */
  constructor(name: string = 'Untitled Scene') {
    this._name = name;
    this._entities = new Set<Entity>();
    this._sceneGraph = new SceneGraph();
  }

  /**
   * Get the scene name
   */
  get name(): string {
    return this._name;
  }

  /**
   * Set the scene name
   */
  set name(value: string) {
    this._name = value;
  }

  /**
   * Add an entity to the scene
   * @param entity - Entity to add
   */
  addEntity(entity: Entity): void {
    this._entities.add(entity);
  }

  /**
   * Remove an entity from the scene
   * @param entity - Entity to remove
   */
  removeEntity(entity: Entity): void {
    this._entities.delete(entity);
  }

  /**
   * Check if entity is in scene
   * @param entity - Entity to check
   */
  hasEntity(entity: Entity): boolean {
    return this._entities.has(entity);
  }

  /**
   * Get all entities in the scene
   */
  getEntities(): Entity[] {
    return Array.from(this._entities);
  }

  /**
   * Clear all entities from the scene
   */
  clear(): void {
    this._entities.clear();
  }

  /**
   * Get entity count
   */
  get entityCount(): number {
    return this._entities.size;
  }

  /**
   * Get scene graph
   */
  get sceneGraph(): SceneGraph {
    return this._sceneGraph;
  }

  /**
   * Add a scene node to the hierarchy
   * @param node - Scene node to add
   * @param parent - Parent node (defaults to root)
   */
  addNode(node: SceneNode, parent: SceneNode | null = null): void {
    this._sceneGraph.addNode(node, parent);
  }

  /**
   * Remove a scene node from the hierarchy
   * @param node - Scene node to remove
   */
  removeNode(node: SceneNode): void {
    this._sceneGraph.removeNode(node);
  }

  /**
   * Find a scene node by name
   * @param name - Node name
   */
  findNode(name: string): SceneNode | null {
    return this._sceneGraph.findNode(name);
  }

  /**
   * Update all scene transforms
   */
  updateTransforms(): void {
    this._sceneGraph.updateTransforms();
  }
}
