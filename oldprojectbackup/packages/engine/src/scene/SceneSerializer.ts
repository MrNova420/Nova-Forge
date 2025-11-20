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
 * SceneSerializer
 *
 * Handles serialization and deserialization of scenes to/from JSON.
 * Supports saving and loading entire scenes with entities, components, and hierarchy.
 */

import { Scene } from './Scene';
import { SceneNode } from './SceneNode';
import { Entity } from '../ecs/Entity';
import { World } from '../ecs/World';
import { SerializationRegistry } from './SerializationRegistry';

/**
 * Serialized scene data structure
 */
export interface SerializedScene {
  name: string;
  version: string;
  nodes: SerializedNode[];
  entities: SerializedEntity[];
}

/**
 * Serialized scene node
 */
export interface SerializedNode {
  name: string;
  active: boolean;
  parent: string | null;
  transform: {
    position: [number, number, number];
    rotation: [number, number, number, number]; // Quaternion
    scale: [number, number, number];
  };
  entityId?: number;
}

/**
 * Serialized entity
 */
export interface SerializedEntity {
  id: number;
  name: string;
  active: boolean;
  components: SerializedComponent[];
}

/**
 * Serialized component
 */
export interface SerializedComponent {
  type: string;
  data: any;
}

/**
 * Scene serializer for save/load functionality
 */
export class SceneSerializer {
  private _registry: SerializationRegistry;
  private _version: string = '1.0.0';

  constructor(registry?: SerializationRegistry) {
    this._registry = registry || new SerializationRegistry();
  }

  /**
   * Get serialization registry
   */
  get registry(): SerializationRegistry {
    return this._registry;
  }

  /**
   * Serialize a scene to JSON
   * @param scene - Scene to serialize
   * @param world - World containing entity components
   * @returns Serialized scene data
   */
  serialize(scene: Scene, world?: World): SerializedScene {
    const serializedNodes: SerializedNode[] = [];
    const serializedEntities: SerializedEntity[] = [];

    // Serialize scene graph nodes
    scene.sceneGraph.traverse((node) => {
      serializedNodes.push(this.serializeNode(node));
    }, true); // Include inactive nodes

    // Serialize entities if world is provided
    if (world) {
      const entities = scene.getEntities();
      for (const entity of entities) {
        serializedEntities.push(this.serializeEntity(entity, world));
      }
    }

    return {
      name: scene.name,
      version: this._version,
      nodes: serializedNodes,
      entities: serializedEntities,
    };
  }

  /**
   * Serialize a scene node
   */
  private serializeNode(node: SceneNode): SerializedNode {
    const transform = node.transform;
    const parentName = node.parent ? node.parent.name : null;

    return {
      name: node.name,
      active: node.active,
      parent: parentName,
      transform: {
        position: [
          transform.position.x,
          transform.position.y,
          transform.position.z,
        ],
        rotation: [
          transform.rotation.x,
          transform.rotation.y,
          transform.rotation.z,
          transform.rotation.w,
        ],
        scale: [transform.scale.x, transform.scale.y, transform.scale.z],
      },
      entityId: node.entity?.id,
    };
  }

  /**
   * Serialize an entity
   */
  private serializeEntity(entity: Entity, world: World): SerializedEntity {
    const components = world.getComponents(entity);
    const serializedComponents: SerializedComponent[] = [];

    for (const component of components) {
      const componentType = component.constructor.name;
      const serializer = this._registry.getSerializer(componentType);

      if (serializer) {
        serializedComponents.push({
          type: componentType,
          data: serializer(component),
        });
      }
    }

    return {
      id: entity.id,
      name: entity.name,
      active: entity.active,
      components: serializedComponents,
    };
  }

  /**
   * Deserialize JSON to a scene
   * @param data - Serialized scene data
   * @param world - World to add entity components to
   * @returns Deserialized scene
   */
  deserialize(data: SerializedScene, world?: World): Scene {
    const scene = new Scene(data.name);

    // Deserialize nodes
    const nodeMap = new Map<string, SceneNode>();

    // First pass: Create all nodes
    for (const nodeData of data.nodes) {
      const node = this.deserializeNode(nodeData);
      nodeMap.set(node.name, node);
    }

    // Second pass: Build hierarchy
    for (const nodeData of data.nodes) {
      const node = nodeMap.get(nodeData.name);
      if (!node) continue;

      if (nodeData.parent) {
        const parent = nodeMap.get(nodeData.parent);
        if (parent) {
          scene.addNode(node, parent);
        } else {
          scene.addNode(node); // Add to root if parent not found
        }
      } else {
        scene.addNode(node); // Add to root
      }
    }

    // Deserialize entities if world is provided
    if (world) {
      const entityMap = new Map<number, Entity>();

      for (const entityData of data.entities) {
        const entity = this.deserializeEntity(entityData, world);
        entityMap.set(entityData.id, entity);
        scene.addEntity(entity);
      }

      // Link entities to nodes
      for (const nodeData of data.nodes) {
        if (nodeData.entityId !== undefined) {
          const node = nodeMap.get(nodeData.name);
          const entity = entityMap.get(nodeData.entityId);
          if (node && entity) {
            node.entity = entity;
          }
        }
      }
    }

    // Update transforms
    scene.updateTransforms();

    return scene;
  }

  /**
   * Deserialize a scene node
   */
  private deserializeNode(data: SerializedNode): SceneNode {
    const node = new SceneNode(data.name);
    node.active = data.active;

    // Set transform
    node.transform.position.x = data.transform.position[0];
    node.transform.position.y = data.transform.position[1];
    node.transform.position.z = data.transform.position[2];

    node.transform.rotation.x = data.transform.rotation[0];
    node.transform.rotation.y = data.transform.rotation[1];
    node.transform.rotation.z = data.transform.rotation[2];
    node.transform.rotation.w = data.transform.rotation[3];

    node.transform.scale.x = data.transform.scale[0];
    node.transform.scale.y = data.transform.scale[1];
    node.transform.scale.z = data.transform.scale[2];

    return node;
  }

  /**
   * Deserialize an entity
   */
  private deserializeEntity(data: SerializedEntity, world: World): Entity {
    const entity = new Entity(data.name);
    entity.active = data.active;

    // Deserialize components
    for (const componentData of data.components) {
      const deserializer = this._registry.getDeserializer(componentData.type);

      if (deserializer) {
        const component = deserializer(componentData.data);
        world.addComponent(entity, component);
      }
    }

    return entity;
  }

  /**
   * Serialize scene to JSON string
   * @param scene - Scene to serialize
   * @param world - World containing entity components
   * @param pretty - Whether to format JSON with indentation
   * @returns JSON string
   */
  toJSON(scene: Scene, world?: World, pretty: boolean = true): string {
    const data = this.serialize(scene, world);
    return pretty ? JSON.stringify(data, null, 2) : JSON.stringify(data);
  }

  /**
   * Deserialize scene from JSON string
   * @param json - JSON string
   * @param world - World to add entity components to
   * @returns Deserialized scene
   */
  fromJSON(json: string, world?: World): Scene {
    const data = JSON.parse(json) as SerializedScene;
    return this.deserialize(data, world);
  }
}
