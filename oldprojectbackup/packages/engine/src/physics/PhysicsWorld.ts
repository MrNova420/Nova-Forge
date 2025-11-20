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
 * Physics World
 * Manages the physics simulation using Ammo.js (Bullet Physics)
 */

import Ammo from 'ammojs-typed';
import { Vector3 } from '../math/Vector3';

/**
 * Physics world manages the entire physics simulation
 */
export class PhysicsWorld {
  private ammo: typeof Ammo | null = null;
  private world: Ammo.btDiscreteDynamicsWorld | null = null;
  private collisionConfiguration: Ammo.btDefaultCollisionConfiguration | null =
    null;
  private dispatcher: Ammo.btCollisionDispatcher | null = null;
  private broadphase: Ammo.btDbvtBroadphase | null = null;
  private solver: Ammo.btSequentialImpulseConstraintSolver | null = null;

  private rigidBodies: Ammo.btRigidBody[] = [];
  private constraints: Ammo.btTypedConstraint[] = [];

  private gravity: Vector3 = new Vector3(0, -9.81, 0);
  private fixedTimeStep: number = 1 / 60;
  private maxSubSteps: number = 1000; // Allow very fine physics simulation

  /**
   * Initialize the physics world
   */
  public async initialize(): Promise<void> {
    // Load Ammo.js
    this.ammo = await Ammo();

    if (!this.ammo) {
      throw new Error('Failed to initialize Ammo.js');
    }

    // Set up collision configuration
    this.collisionConfiguration =
      new this.ammo.btDefaultCollisionConfiguration();

    // Set up dispatcher
    this.dispatcher = new this.ammo.btCollisionDispatcher(
      this.collisionConfiguration
    );

    // Set up broadphase
    this.broadphase = new this.ammo.btDbvtBroadphase();

    // Set up solver
    this.solver = new this.ammo.btSequentialImpulseConstraintSolver();

    // Create the physics world
    this.world = new this.ammo.btDiscreteDynamicsWorld(
      this.dispatcher,
      this.broadphase,
      this.solver,
      this.collisionConfiguration
    );

    // Set gravity
    this.world.setGravity(
      new this.ammo.btVector3(this.gravity.x, this.gravity.y, this.gravity.z)
    );
  }

  /**
   * Step the physics simulation
   * @param deltaTime - Time since last frame in seconds
   */
  public step(deltaTime: number): void {
    if (!this.world) return;

    this.world.stepSimulation(deltaTime, this.maxSubSteps, this.fixedTimeStep);
  }

  /**
   * Add a rigid body to the world
   * @param body - Rigid body to add
   */
  public addRigidBody(body: Ammo.btRigidBody): void {
    if (!this.world) return;

    this.world.addRigidBody(body);
    this.rigidBodies.push(body);
  }

  /**
   * Remove a rigid body from the world
   * @param body - Rigid body to remove
   */
  public removeRigidBody(body: Ammo.btRigidBody): void {
    if (!this.world) return;

    this.world.removeRigidBody(body);
    const index = this.rigidBodies.indexOf(body);
    if (index !== -1) {
      this.rigidBodies.splice(index, 1);
    }
  }

  /**
   * Add a constraint to the world
   * @param constraint - Constraint to add
   * @param disableCollisionsBetweenLinkedBodies - Whether to disable collisions
   */
  public addConstraint(
    constraint: Ammo.btTypedConstraint,
    disableCollisionsBetweenLinkedBodies: boolean = true
  ): void {
    if (!this.world) return;

    this.world.addConstraint(constraint, disableCollisionsBetweenLinkedBodies);
    this.constraints.push(constraint);
  }

  /**
   * Remove a constraint from the world
   * @param constraint - Constraint to remove
   */
  public removeConstraint(constraint: Ammo.btTypedConstraint): void {
    if (!this.world) return;

    this.world.removeConstraint(constraint);
    const index = this.constraints.indexOf(constraint);
    if (index !== -1) {
      this.constraints.splice(index, 1);
    }
  }

  /**
   * Set the gravity vector
   * @param gravity - Gravity vector
   */
  public setGravity(gravity: Vector3): void {
    this.gravity = gravity.clone();

    if (this.world && this.ammo) {
      this.world.setGravity(
        new this.ammo.btVector3(gravity.x, gravity.y, gravity.z)
      );
    }
  }

  /**
   * Get the gravity vector
   */
  public getGravity(): Vector3 {
    return this.gravity.clone();
  }

  /**
   * Set the fixed time step
   * @param timeStep - Fixed time step in seconds
   */
  public setFixedTimeStep(timeStep: number): void {
    this.fixedTimeStep = timeStep;
  }

  /**
   * Set the maximum number of substeps
   * @param maxSubSteps - Maximum substeps per frame
   */
  public setMaxSubSteps(maxSubSteps: number): void {
    this.maxSubSteps = maxSubSteps;
  }

  /**
   * Raycast from start to end
   * @param from - Start position
   * @param to - End position
   * @returns Raycast result or null if no hit
   */
  public raycast(from: Vector3, to: Vector3): RaycastResult | null {
    if (!this.world || !this.ammo) return null;

    const rayFrom = new this.ammo.btVector3(from.x, from.y, from.z);
    const rayTo = new this.ammo.btVector3(to.x, to.y, to.z);

    const rayCallback = new this.ammo.ClosestRayResultCallback(rayFrom, rayTo);
    this.world.rayTest(rayFrom, rayTo, rayCallback);

    if (rayCallback.hasHit()) {
      const hitPoint = rayCallback.get_m_hitPointWorld();
      const hitNormal = rayCallback.get_m_hitNormalWorld();
      const body = rayCallback.get_m_collisionObject();

      const result: RaycastResult = {
        hasHit: true,
        body: body as any as Ammo.btRigidBody, // Type cast for Ammo.js compatibility
        point: new Vector3(hitPoint.x(), hitPoint.y(), hitPoint.z()),
        normal: new Vector3(hitNormal.x(), hitNormal.y(), hitNormal.z()),
        distance: from.distance(
          new Vector3(hitPoint.x(), hitPoint.y(), hitPoint.z())
        ),
      };

      // Clean up
      this.ammo.destroy(rayCallback);
      this.ammo.destroy(rayFrom);
      this.ammo.destroy(rayTo);

      return result;
    }

    // Clean up
    this.ammo.destroy(rayCallback);
    this.ammo.destroy(rayFrom);
    this.ammo.destroy(rayTo);

    return null;
  }

  /**
   * Get all rigid bodies
   */
  public getRigidBodies(): Ammo.btRigidBody[] {
    return [...this.rigidBodies];
  }

  /**
   * Get all constraints
   */
  public getConstraints(): Ammo.btTypedConstraint[] {
    return [...this.constraints];
  }

  /**
   * Get the Ammo.js instance
   */
  public getAmmo(): typeof Ammo | null {
    return this.ammo;
  }

  /**
   * Get the underlying btDiscreteDynamicsWorld
   */
  public getWorld(): Ammo.btDiscreteDynamicsWorld | null {
    return this.world;
  }

  /**
   * Destroy the physics world and clean up resources
   */
  public destroy(): void {
    if (!this.ammo) return;

    // Remove all constraints
    for (const constraint of this.constraints) {
      if (this.world) {
        this.world.removeConstraint(constraint);
      }
      this.ammo.destroy(constraint);
    }
    this.constraints = [];

    // Remove all rigid bodies
    for (const body of this.rigidBodies) {
      if (this.world) {
        this.world.removeRigidBody(body);
      }

      // Destroy motion state
      const motionState = body.getMotionState();
      if (motionState) {
        this.ammo.destroy(motionState);
      }

      // Destroy collision shape
      const shape = body.getCollisionShape();
      if (shape) {
        this.ammo.destroy(shape);
      }

      this.ammo.destroy(body);
    }
    this.rigidBodies = [];

    // Destroy world
    if (this.world) {
      this.ammo.destroy(this.world);
      this.world = null;
    }

    // Destroy solver
    if (this.solver) {
      this.ammo.destroy(this.solver);
      this.solver = null;
    }

    // Destroy broadphase
    if (this.broadphase) {
      this.ammo.destroy(this.broadphase);
      this.broadphase = null;
    }

    // Destroy dispatcher
    if (this.dispatcher) {
      this.ammo.destroy(this.dispatcher);
      this.dispatcher = null;
    }

    // Destroy collision configuration
    if (this.collisionConfiguration) {
      this.ammo.destroy(this.collisionConfiguration);
      this.collisionConfiguration = null;
    }
  }
}

/**
 * Raycast result
 */
export interface RaycastResult {
  hasHit: boolean;
  body: Ammo.btRigidBody;
  point: Vector3;
  normal: Vector3;
  distance: number;
}
