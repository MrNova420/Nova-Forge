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
 * CreateEntityCommand
 * Command for creating a new entity
 */

import { BaseCommand } from './Command';
import type { Store } from '@reduxjs/toolkit';
import { addEntity, removeEntity } from '../store/slices/sceneSlice';
import type { Entity } from '../store/slices/sceneSlice';

export class CreateEntityCommand extends BaseCommand {
  readonly name: string = 'Create Entity';

  private entityId: string;
  private entityData: Partial<Entity>;
  private parentId: string | null;
  private store?: Store;

  constructor(
    entityData: Partial<Entity>,
    parentId: string | null = null,
    store?: Store
  ) {
    super();
    this.entityId = entityData.id || this.generateId();
    this.entityData = entityData;
    this.parentId = parentId;
    this.store = store;
  }

  execute(): void {
    if (!this.store) {
      console.warn(`No store available for CreateEntityCommand`);
      return;
    }

    // Create the entity with default values
    const entity: Entity = {
      id: this.entityId,
      name: this.entityData.name || 'New Entity',
      parent: this.parentId,
      children: [],
      components: this.entityData.components || {
        Transform: {
          position: { x: 0, y: 0, z: 0 },
          rotation: { x: 0, y: 0, z: 0 },
          scale: { x: 1, y: 1, z: 1 },
        },
      },
      enabled:
        this.entityData.enabled !== undefined ? this.entityData.enabled : true,
    };

    this.store.dispatch(addEntity(entity));
  }

  undo(): void {
    if (!this.store) {
      console.warn(`No store available for CreateEntityCommand`);
      return;
    }

    this.store.dispatch(removeEntity(this.entityId));
  }

  getEntityId(): string {
    return this.entityId;
  }

  private generateId(): string {
    return `entity_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }
}
