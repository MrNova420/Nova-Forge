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
 * Physics Module
 * Export all physics-related classes
 */

export { PhysicsWorld } from './PhysicsWorld';
export type { RaycastResult } from './PhysicsWorld';
export { RigidBody, RigidBodyType } from './RigidBody';
export {
  Collider,
  BoxCollider,
  SphereCollider,
  CapsuleCollider,
  CylinderCollider,
  ConeCollider,
  PlaneCollider,
  ConvexHullCollider,
  CompoundCollider,
  TriangleMeshCollider,
} from './Collider';
export {
  Constraint,
  PointToPointConstraint,
  HingeConstraint,
  SliderConstraint,
  Generic6DofConstraint,
  FixedConstraint,
} from './Constraint';
