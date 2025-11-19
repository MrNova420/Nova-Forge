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
 * DeleteEntityCommand
 * Command for deleting an entity
 */

import { BaseCommand } from './Command';
import type { Store } from '@reduxjs/toolkit';
import { addEntity, removeEntity } from '../store/slices/sceneSlice';
import type { Entity } from '../store/slices/sceneSlice';
import type { RootState } from '../store/store';

export class DeleteEntityCommand extends BaseCommand {
  readonly name: string = 'Delete Entity';

  private entityId: string;
  private entityData: Entity | null = null;
  private store?: Store;

  constructor(entityId: string, store?: Store) {
    super();
    this.entityId = entityId;
    this.store = store;
  }

  execute(): void {
    if (!this.store) {
      console.warn(`No store available for DeleteEntityCommand`);
      return;
    }

    // Store entity data before deletion for undo
    const state = this.store.getState() as RootState;
    const sceneState = state.scene;
    this.entityData = sceneState.entities[this.entityId] ?? null;

    if (!this.entityData) {
      console.warn(`Entity ${this.entityId} not found in scene`);
      return;
    }

    // Deep copy the entity data to preserve it for undo
    this.entityData = JSON.parse(JSON.stringify(this.entityData));

    // Delete the entity
    this.store.dispatch(removeEntity(this.entityId));
  }

  undo(): void {
    if (!this.store) {
      console.warn(`No store available for DeleteEntityCommand`);
      return;
    }

    if (!this.entityData) {
      console.warn(`No entity data to restore for ${this.entityId}`);
      return;
    }

    // Restore the entity
    this.store.dispatch(addEntity(this.entityData));
  }
}
