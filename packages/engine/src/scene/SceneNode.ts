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
 * SceneNode
 *
 * Hierarchical node in a scene graph.
 * Represents a transformation node that can have children.
 */

import { Transform } from '../math/Transform';
import { Matrix4 } from '../math/Matrix4';
import { Vector3 } from '../math/Vector3';
import { Entity } from '../ecs/Entity';

/**
 * Scene node for hierarchical transformations
 */
export class SceneNode {
  private _parent: SceneNode | null = null;
  private _children: SceneNode[] = [];
  private _transform: Transform;
  private _worldTransform: Transform;
  private _entity: Entity | null = null;
  private _name: string;
  private _active: boolean = true;

  /**
   * Create a new scene node
   * @param name - Node name for debugging
   */
  constructor(name: string = 'SceneNode') {
    this._name = name;
    this._transform = new Transform();
    this._worldTransform = new Transform();
  }

  /**
   * Get node name
   */
  get name(): string {
    return this._name;
  }

  /**
   * Set node name
   */
  set name(value: string) {
    this._name = value;
  }

  /**
   * Get parent node
   */
  get parent(): SceneNode | null {
    return this._parent;
  }

  /**
   * Get children nodes
   */
  get children(): ReadonlyArray<SceneNode> {
    return this._children;
  }

  /**
   * Get local transform
   */
  get transform(): Transform {
    return this._transform;
  }

  /**
   * Get world transform (accumulated from root)
   */
  get worldTransform(): Transform {
    return this._worldTransform;
  }

  /**
   * Get associated entity
   */
  get entity(): Entity | null {
    return this._entity;
  }

  /**
   * Set associated entity
   */
  set entity(value: Entity | null) {
    this._entity = value;
  }

  /**
   * Get active state
   */
  get active(): boolean {
    return this._active;
  }

  /**
   * Set active state
   */
  set active(value: boolean) {
    this._active = value;
  }

  /**
   * Add a child node
   * @param child - Node to add as child
   */
  addChild(child: SceneNode): void {
    if (child._parent === this) {
      return; // Already a child
    }

    // Remove from previous parent
    if (child._parent) {
      child._parent.removeChild(child);
    }

    // Add to this node
    this._children.push(child);
    child._parent = this;

    // Update world transform
    child.updateWorldTransform();
  }

  /**
   * Remove a child node
   * @param child - Node to remove
   * @returns True if child was removed
   */
  removeChild(child: SceneNode): boolean {
    const index = this._children.indexOf(child);
    if (index === -1) {
      return false;
    }

    this._children.splice(index, 1);
    child._parent = null;
    child.updateWorldTransform();

    return true;
  }

  /**
   * Remove this node from its parent
   */
  removeFromParent(): void {
    if (this._parent) {
      this._parent.removeChild(this);
    }
  }

  /**
   * Remove all children
   */
  removeAllChildren(): void {
    for (const child of this._children) {
      child._parent = null;
    }
    this._children = [];
  }

  /**
   * Get child by name
   * @param name - Child name
   * @returns Child node or null if not found
   */
  getChildByName(name: string): SceneNode | null {
    for (const child of this._children) {
      if (child.name === name) {
        return child;
      }
    }
    return null;
  }

  /**
   * Find node in hierarchy by name (depth-first search)
   * @param name - Node name to find
   * @returns Node or null if not found
   */
  find(name: string): SceneNode | null {
    if (this.name === name) {
      return this;
    }

    for (const child of this._children) {
      const found = child.find(name);
      if (found) {
        return found;
      }
    }

    return null;
  }

  /**
   * Traverse the hierarchy (depth-first)
   * @param callback - Function to call for each node
   * @param includeInactive - Whether to include inactive nodes
   */
  traverse(
    callback: (node: SceneNode) => void,
    includeInactive: boolean = false
  ): void {
    if (!includeInactive && !this._active) {
      return;
    }

    callback(this);

    for (const child of this._children) {
      child.traverse(callback, includeInactive);
    }
  }

  /**
   * Traverse ancestors (parent chain)
   * @param callback - Function to call for each ancestor
   */
  traverseAncestors(callback: (node: SceneNode) => void): void {
    if (this._parent) {
      callback(this._parent);
      this._parent.traverseAncestors(callback);
    }
  }

  /**
   * Update world transform from parent
   */
  updateWorldTransform(): void {
    if (this._parent) {
      // Combine parent's world transform with this node's local transform
      // For now, we use a simplified approach that concatenates transforms
      // TODO: Implement proper matrix composition and decomposition
      const parentTransform = this._parent.worldTransform;

      // Apply parent's transform to this node's local transform
      this._worldTransform.position = parentTransform.position
        .clone()
        .add(this._transform.position);
      this._worldTransform.rotation = parentTransform.rotation
        .clone()
        .multiply(this._transform.rotation);
      // Scale is component-wise multiplication
      const parentScale = parentTransform.scale;
      const localScale = this._transform.scale;
      this._worldTransform.scale = new Vector3(
        parentScale.x * localScale.x,
        parentScale.y * localScale.y,
        parentScale.z * localScale.z
      );
    } else {
      // No parent - world transform is same as local
      this._worldTransform = this._transform.clone();
    }

    // Update children
    for (const child of this._children) {
      child.updateWorldTransform();
    }
  }

  /**
   * Get world transformation matrix
   */
  getWorldMatrix(): Matrix4 {
    return this._worldTransform.toMatrix();
  }

  /**
   * Get number of children
   */
  get childCount(): number {
    return this._children.length;
  }

  /**
   * Check if this node is an ancestor of another node
   * @param node - Node to check
   */
  isAncestorOf(node: SceneNode): boolean {
    let current = node._parent;
    while (current) {
      if (current === this) {
        return true;
      }
      current = current._parent;
    }
    return false;
  }

  /**
   * Get depth in hierarchy (0 = root)
   */
  get depth(): number {
    let depth = 0;
    let current = this._parent;
    while (current) {
      depth++;
      current = current._parent;
    }
    return depth;
  }

  /**
   * Get root node
   */
  getRoot(): SceneNode {
    let root: SceneNode = this;
    while (root._parent) {
      root = root._parent;
    }
    return root;
  }
}
