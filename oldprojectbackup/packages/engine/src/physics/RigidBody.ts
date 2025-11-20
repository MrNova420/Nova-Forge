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
 * Rigid Body Component
 * Component for physics-enabled entities
 */

import Ammo from 'ammojs-typed';
import { Component } from '../ecs/Component';
import { Vector3 } from '../math/Vector3';
import { Quaternion } from '../math/Quaternion';
import { PhysicsWorld } from './PhysicsWorld';

/**
 * Rigid body type
 */
export enum RigidBodyType {
  /**
   * Static body - infinite mass, doesn't move
   */
  STATIC = 0,

  /**
   * Dynamic body - affected by forces and gravity
   */
  DYNAMIC = 1,

  /**
   * Kinematic body - moves via velocity, not affected by forces
   */
  KINEMATIC = 2,
}

/**
 * Rigid body component for physics simulation
 */
export class RigidBody extends Component {
  public static readonly typeName = 'RigidBody';

  private physicsWorld: PhysicsWorld | null = null;
  private ammoBody: Ammo.btRigidBody | null = null;
  private collisionShape: Ammo.btCollisionShape | null = null;

  private type: RigidBodyType = RigidBodyType.DYNAMIC;
  private mass: number = 1.0;
  private friction: number = 0.5;
  private restitution: number = 0.0; // Bounciness
  private linearDamping: number = 0.0;
  private angularDamping: number = 0.0;

  private linearVelocity: Vector3 = new Vector3();
  private angularVelocity: Vector3 = new Vector3();

  private disableDeactivation: boolean = false;

  /**
   * Initialize the rigid body
   * @param physicsWorld - Physics world to add this body to
   * @param collisionShape - Collision shape for this body
   */
  public initialize(
    physicsWorld: PhysicsWorld,
    collisionShape: Ammo.btCollisionShape
  ): void {
    this.physicsWorld = physicsWorld;
    this.collisionShape = collisionShape;
    this.createRigidBody();
  }

  /**
   * Create the Ammo.js rigid body
   */
  private createRigidBody(): void {
    if (!this.physicsWorld || !this.collisionShape) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    // Calculate local inertia
    const localInertia = new ammo.btVector3(0, 0, 0);
    if (this.type === RigidBodyType.DYNAMIC && this.mass > 0) {
      this.collisionShape.calculateLocalInertia(this.mass, localInertia);
    }

    // Create transform
    const transform = new ammo.btTransform();
    transform.setIdentity();
    transform.setOrigin(new ammo.btVector3(0, 0, 0));
    transform.setRotation(new ammo.btQuaternion(0, 0, 0, 1));

    // Create motion state
    const motionState = new ammo.btDefaultMotionState(transform);

    // Create construction info
    const rbInfo = new ammo.btRigidBodyConstructionInfo(
      this.type === RigidBodyType.STATIC ? 0 : this.mass,
      motionState,
      this.collisionShape,
      localInertia
    );

    // Set physics properties
    rbInfo.set_m_friction(this.friction);
    rbInfo.set_m_restitution(this.restitution);
    rbInfo.set_m_linearDamping(this.linearDamping);
    rbInfo.set_m_angularDamping(this.angularDamping);

    // Create rigid body
    this.ammoBody = new ammo.btRigidBody(rbInfo);

    // Set collision flags
    if (this.type === RigidBodyType.KINEMATIC) {
      // CF_KINEMATIC_OBJECT = 2
      this.ammoBody.setCollisionFlags(
        this.ammoBody.getCollisionFlags() | 2
      );
    }

    // Disable deactivation if requested
    if (this.disableDeactivation) {
      // DISABLE_DEACTIVATION = 4
      this.ammoBody.setActivationState(4);
    }

    // Add to physics world
    this.physicsWorld.addRigidBody(this.ammoBody);

    // Clean up temp objects
    ammo.destroy(rbInfo);
    ammo.destroy(transform);
    ammo.destroy(localInertia);
  }

  /**
   * Set the rigid body type
   */
  public setType(type: RigidBodyType): void {
    this.type = type;
    
    if (this.ammoBody && this.physicsWorld) {
      // Need to recreate body with new type
      this.destroyRigidBody();
      this.createRigidBody();
    }
  }

  /**
   * Get the rigid body type
   */
  public getType(): RigidBodyType {
    return this.type;
  }

  /**
   * Set the mass
   */
  public setMass(mass: number): void {
    this.mass = mass;

    if (this.ammoBody && this.collisionShape && this.physicsWorld) {
      const ammo = this.physicsWorld.getAmmo();
      if (!ammo) return;

      const localInertia = new ammo.btVector3(0, 0, 0);
      if (this.type === RigidBodyType.DYNAMIC && mass > 0) {
        this.collisionShape.calculateLocalInertia(mass, localInertia);
      }

      this.ammoBody.setMassProps(mass, localInertia);
      this.ammoBody.updateInertiaTensor();

      ammo.destroy(localInertia);
    }
  }

  /**
   * Get the mass
   */
  public getMass(): number {
    return this.mass;
  }

  /**
   * Set friction
   */
  public setFriction(friction: number): void {
    this.friction = friction;
    if (this.ammoBody) {
      this.ammoBody.setFriction(friction);
    }
  }

  /**
   * Get friction
   */
  public getFriction(): number {
    return this.friction;
  }

  /**
   * Set restitution (bounciness)
   */
  public setRestitution(restitution: number): void {
    this.restitution = restitution;
    if (this.ammoBody) {
      this.ammoBody.setRestitution(restitution);
    }
  }

  /**
   * Get restitution
   */
  public getRestitution(): number {
    return this.restitution;
  }

  /**
   * Set linear damping
   */
  public setLinearDamping(damping: number): void {
    this.linearDamping = damping;
    if (this.ammoBody) {
      this.ammoBody.setDamping(damping, this.angularDamping);
    }
  }

  /**
   * Set angular damping
   */
  public setAngularDamping(damping: number): void {
    this.angularDamping = damping;
    if (this.ammoBody) {
      this.ammoBody.setDamping(this.linearDamping, damping);
    }
  }

  /**
   * Set position
   */
  public setPosition(position: Vector3): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const transform = new ammo.btTransform();
    this.ammoBody.getMotionState()?.getWorldTransform(transform);
    transform.setOrigin(new ammo.btVector3(position.x, position.y, position.z));
    this.ammoBody.setWorldTransform(transform);
    this.ammoBody.getMotionState()?.setWorldTransform(transform);

    ammo.destroy(transform);
  }

  /**
   * Get position
   */
  public getPosition(): Vector3 {
    if (!this.ammoBody || !this.physicsWorld) return new Vector3();

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return new Vector3();

    const transform = new ammo.btTransform();
    this.ammoBody.getMotionState()?.getWorldTransform(transform);
    const origin = transform.getOrigin();
    const position = new Vector3(origin.x(), origin.y(), origin.z());

    ammo.destroy(transform);

    return position;
  }

  /**
   * Set rotation
   */
  public setRotation(rotation: Quaternion): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const transform = new ammo.btTransform();
    this.ammoBody.getMotionState()?.getWorldTransform(transform);
    transform.setRotation(
      new ammo.btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w)
    );
    this.ammoBody.setWorldTransform(transform);
    this.ammoBody.getMotionState()?.setWorldTransform(transform);

    ammo.destroy(transform);
  }

  /**
   * Get rotation
   */
  public getRotation(): Quaternion {
    if (!this.ammoBody || !this.physicsWorld) return new Quaternion();

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return new Quaternion();

    const transform = new ammo.btTransform();
    this.ammoBody.getMotionState()?.getWorldTransform(transform);
    const quat = transform.getRotation();
    const rotation = new Quaternion(quat.x(), quat.y(), quat.z(), quat.w());

    ammo.destroy(transform);

    return rotation;
  }

  /**
   * Apply force at center of mass
   */
  public applyForce(force: Vector3): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const ammoForce = new ammo.btVector3(force.x, force.y, force.z);
    this.ammoBody.applyCentralForce(ammoForce);
    this.ammoBody.activate();

    ammo.destroy(ammoForce);
  }

  /**
   * Apply force at a specific point
   */
  public applyForceAtPoint(force: Vector3, point: Vector3): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const ammoForce = new ammo.btVector3(force.x, force.y, force.z);
    const ammoPoint = new ammo.btVector3(point.x, point.y, point.z);
    this.ammoBody.applyForce(ammoForce, ammoPoint);
    this.ammoBody.activate();

    ammo.destroy(ammoForce);
    ammo.destroy(ammoPoint);
  }

  /**
   * Apply impulse at center of mass
   */
  public applyImpulse(impulse: Vector3): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const ammoImpulse = new ammo.btVector3(impulse.x, impulse.y, impulse.z);
    this.ammoBody.applyCentralImpulse(ammoImpulse);
    this.ammoBody.activate();

    ammo.destroy(ammoImpulse);
  }

  /**
   * Apply torque
   */
  public applyTorque(torque: Vector3): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const ammoTorque = new ammo.btVector3(torque.x, torque.y, torque.z);
    this.ammoBody.applyTorque(ammoTorque);
    this.ammoBody.activate();

    ammo.destroy(ammoTorque);
  }

  /**
   * Set linear velocity
   */
  public setLinearVelocity(velocity: Vector3): void {
    this.linearVelocity = velocity.clone();

    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const ammoVelocity = new ammo.btVector3(velocity.x, velocity.y, velocity.z);
    this.ammoBody.setLinearVelocity(ammoVelocity);
    this.ammoBody.activate();

    ammo.destroy(ammoVelocity);
  }

  /**
   * Get linear velocity
   */
  public getLinearVelocity(): Vector3 {
    if (!this.ammoBody) return this.linearVelocity.clone();

    const velocity = this.ammoBody.getLinearVelocity();
    return new Vector3(velocity.x(), velocity.y(), velocity.z());
  }

  /**
   * Set angular velocity
   */
  public setAngularVelocity(velocity: Vector3): void {
    this.angularVelocity = velocity.clone();

    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    const ammoVelocity = new ammo.btVector3(velocity.x, velocity.y, velocity.z);
    this.ammoBody.setAngularVelocity(ammoVelocity);
    this.ammoBody.activate();

    ammo.destroy(ammoVelocity);
  }

  /**
   * Get angular velocity
   */
  public getAngularVelocity(): Vector3 {
    if (!this.ammoBody) return this.angularVelocity.clone();

    const velocity = this.ammoBody.getAngularVelocity();
    return new Vector3(velocity.x(), velocity.y(), velocity.z());
  }

  /**
   * Get the Ammo.js rigid body
   */
  public getAmmoBody(): Ammo.btRigidBody | null {
    return this.ammoBody;
  }

  /**
   * Set whether to disable deactivation (sleeping)
   */
  public setDisableDeactivation(disable: boolean): void {
    this.disableDeactivation = disable;

    if (this.ammoBody && this.physicsWorld) {
      if (disable) {
        // DISABLE_DEACTIVATION = 4
        this.ammoBody.setActivationState(4);
      } else {
        // ACTIVE_TAG = 1
        this.ammoBody.forceActivationState(1);
      }
    }
  }

  /**
   * Destroy the rigid body
   */
  private destroyRigidBody(): void {
    if (!this.ammoBody || !this.physicsWorld) return;

    const ammo = this.physicsWorld.getAmmo();
    if (!ammo) return;

    // Remove from world
    this.physicsWorld.removeRigidBody(this.ammoBody);

    // Destroy motion state
    const motionState = this.ammoBody.getMotionState();
    if (motionState) {
      ammo.destroy(motionState);
    }

    // Destroy body
    ammo.destroy(this.ammoBody);
    this.ammoBody = null;
  }

  /**
   * Component lifecycle: onDetach
   */
  public onDetach(): void {
    this.destroyRigidBody();
  }
}
