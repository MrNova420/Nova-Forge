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
 * TransformCommand
 * Command for transforming entity position, rotation, or scale
 */

import { BaseCommand } from './Command';
import type { Store } from '@reduxjs/toolkit';
import { updateComponent } from '../store/slices/sceneSlice';

export interface TransformData {
  position?: { x: number; y: number; z: number };
  rotation?: { x: number; y: number; z: number };
  scale?: { x: number; y: number; z: number };
}

interface TransformComponentData {
  position?: { x: number; y: number; z: number };
  rotation?: { x: number; y: number; z: number };
  scale?: { x: number; y: number; z: number };
}

export class TransformCommand extends BaseCommand {
  readonly name: string;

  private entityId: string;
  private oldTransform: TransformData;
  private newTransform: TransformData;
  private store?: Store;

  constructor(
    entityId: string,
    oldTransform: TransformData,
    newTransform: TransformData,
    name: string = 'Transform Entity',
    store?: Store
  ) {
    super();
    this.entityId = entityId;
    this.oldTransform = oldTransform;
    this.newTransform = newTransform;
    this.name = name;
    this.store = store;
  }

  execute(): void {
    this.applyTransform(this.newTransform);
  }

  undo(): void {
    this.applyTransform(this.oldTransform);
  }

  private applyTransform(transform: TransformData): void {
    if (!this.store) {
      console.warn(`No store available for TransformCommand`);
      return;
    }

    // Update the Transform component on the entity
    const transformComponentData: TransformComponentData = {};

    if (transform.position) {
      transformComponentData.position = transform.position;
    }
    if (transform.rotation) {
      transformComponentData.rotation = transform.rotation;
    }
    if (transform.scale) {
      transformComponentData.scale = transform.scale;
    }

    this.store.dispatch(
      updateComponent({
        entityId: this.entityId,
        componentType: 'Transform',
        data: transformComponentData,
      })
    );
  }

  override canMergeWith(other: import('./Command').Command): boolean {
    if (!(other instanceof TransformCommand)) {
      return false;
    }

    // Only merge if it's the same entity
    return this.entityId === other.entityId;
  }

  override mergeWith(other: import('./Command').Command): void {
    if (other instanceof TransformCommand) {
      // Update new transform to the other command's new transform
      this.newTransform = other.newTransform;
    }
  }
}
