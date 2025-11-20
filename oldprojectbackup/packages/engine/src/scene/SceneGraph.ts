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
 * SceneGraph
 *
 * Manages the hierarchical organization of scene nodes.
 * Provides efficient traversal and query methods.
 */

import { SceneNode } from './SceneNode';

/**
 * Scene graph manager
 */
export class SceneGraph {
  private _root: SceneNode;
  private _nodeMap: Map<string, SceneNode> = new Map();

  /**
   * Create a new scene graph
   */
  constructor() {
    this._root = new SceneNode('Root');
  }

  /**
   * Get root node
   */
  get root(): SceneNode {
    return this._root;
  }

  /**
   * Add a node to the graph
   * @param node - Node to add
   * @param parent - Parent node (defaults to root)
   */
  addNode(node: SceneNode, parent: SceneNode | null = null): void {
    const targetParent = parent || this._root;
    targetParent.addChild(node);
    this._nodeMap.set(node.name, node);
  }

  /**
   * Remove a node from the graph
   * @param node - Node to remove
   */
  removeNode(node: SceneNode): void {
    node.removeFromParent();
    this._nodeMap.delete(node.name);

    // Remove all descendants from map
    node.traverse((child) => {
      this._nodeMap.delete(child.name);
    });
  }

  /**
   * Find node by name
   * @param name - Node name
   * @returns Node or null if not found
   */
  findNode(name: string): SceneNode | null {
    return this._nodeMap.get(name) || null;
  }

  /**
   * Find node by predicate
   * @param predicate - Function to test each node
   * @returns First node that matches or null
   */
  findNodeBy(predicate: (node: SceneNode) => boolean): SceneNode | null {
    let found: SceneNode | null = null;

    this._root.traverse((node) => {
      if (!found && predicate(node)) {
        found = node;
      }
    });

    return found;
  }

  /**
   * Find all nodes by predicate
   * @param predicate - Function to test each node
   * @returns Array of nodes that match
   */
  findNodesBy(predicate: (node: SceneNode) => boolean): SceneNode[] {
    const results: SceneNode[] = [];

    this._root.traverse((node) => {
      if (predicate(node)) {
        results.push(node);
      }
    });

    return results;
  }

  /**
   * Traverse entire graph
   * @param callback - Function to call for each node
   * @param includeInactive - Whether to include inactive nodes
   */
  traverse(
    callback: (node: SceneNode) => void,
    includeInactive: boolean = false
  ): void {
    this._root.traverse(callback, includeInactive);
  }

  /**
   * Update all world transforms
   */
  updateTransforms(): void {
    this._root.updateWorldTransform();
  }

  /**
   * Get all nodes in the graph
   * @param includeRoot - Whether to include root node
   * @returns Array of all nodes
   */
  getAllNodes(includeRoot: boolean = false): SceneNode[] {
    const nodes: SceneNode[] = [];

    this._root.traverse((node) => {
      if (node !== this._root || includeRoot) {
        nodes.push(node);
      }
    });

    return nodes;
  }

  /**
   * Get total node count
   * @param includeRoot - Whether to include root node
   */
  getNodeCount(includeRoot: boolean = false): number {
    let count = 0;

    this._root.traverse(() => {
      count++;
    });

    return includeRoot ? count : count - 1;
  }

  /**
   * Clear all nodes except root
   */
  clear(): void {
    this._root.removeAllChildren();
    this._nodeMap.clear();
    this._nodeMap.set(this._root.name, this._root);
  }
}
