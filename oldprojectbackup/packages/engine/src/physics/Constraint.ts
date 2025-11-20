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
 * Physics Constraints
 * Various constraint types for connecting rigid bodies
 */

import Ammo from 'ammojs-typed';
import { Vector3 } from '../math/Vector3';
import { RigidBody } from './RigidBody';

/**
 * Base constraint class
 */
export abstract class Constraint {
  protected constraint: Ammo.btTypedConstraint | null = null;
  protected bodyA: RigidBody;
  protected bodyB: RigidBody | null;

  constructor(bodyA: RigidBody, bodyB: RigidBody | null = null) {
    this.bodyA = bodyA;
    this.bodyB = bodyB;
  }

  /**
   * Get the Ammo.js constraint
   */
  public getConstraint(): Ammo.btTypedConstraint | null {
    return this.constraint;
  }

  /**
   * Create the constraint
   */
  public abstract createConstraint(ammo: typeof Ammo): void;

  /**
   * Destroy the constraint
   */
  public destroy(ammo: typeof Ammo): void {
    if (this.constraint) {
      ammo.destroy(this.constraint);
      this.constraint = null;
    }
  }

  /**
   * Enable or disable the constraint
   */
  public setEnabled(enabled: boolean): void {
    if (this.constraint) {
      (this.constraint as any).setEnabled(enabled);
    }
  }

  /**
   * Check if constraint is enabled
   */
  public isEnabled(): boolean {
    return this.constraint ? (this.constraint as any).isEnabled() : false;
  }
}

/**
 * Point to point constraint (ball and socket)
 * Constrains two points on two bodies to be at the same location
 */
export class PointToPointConstraint extends Constraint {
  private pivotA: Vector3;
  private pivotB: Vector3;

  /**
   * Create a point to point constraint
   * @param bodyA - First rigid body
   * @param pivotA - Pivot point in bodyA's local space
   * @param bodyB - Second rigid body (or null for world)
   * @param pivotB - Pivot point in bodyB's local space (or world space if bodyB is null)
   */
  constructor(
    bodyA: RigidBody,
    pivotA: Vector3,
    bodyB: RigidBody | null = null,
    pivotB: Vector3 = new Vector3()
  ) {
    super(bodyA, bodyB);
    this.pivotA = pivotA;
    this.pivotB = pivotB;
  }

  public createConstraint(ammo: typeof Ammo): void {
    const ammoBodyA = this.bodyA.getAmmoBody();
    if (!ammoBodyA) return;

    const btPivotA = new ammo.btVector3(this.pivotA.x, this.pivotA.y, this.pivotA.z);

    if (this.bodyB) {
      const ammoBodyB = this.bodyB.getAmmoBody();
      if (!ammoBodyB) {
        ammo.destroy(btPivotA);
        return;
      }

      const btPivotB = new ammo.btVector3(this.pivotB.x, this.pivotB.y, this.pivotB.z);
      this.constraint = new ammo.btPoint2PointConstraint(
        ammoBodyA as any,
        ammoBodyB as any,
        btPivotA,
        btPivotB
      );
      ammo.destroy(btPivotB);
    } else {
      this.constraint = new ammo.btPoint2PointConstraint(ammoBodyA as any, btPivotA);
    }

    ammo.destroy(btPivotA);
  }
}

/**
 * Hinge constraint
 * Constrains rotation around a single axis (like a door hinge)
 */
export class HingeConstraint extends Constraint {
  private pivotA: Vector3;
  private pivotB: Vector3;
  private axisA: Vector3;
  private axisB: Vector3;
  private limits: { low: number; high: number } | null = null;
  private motorEnabled: boolean = false;
  private motorVelocity: number = 0;
  private motorMaxImpulse: number = 0;

  /**
   * Create a hinge constraint
   * @param bodyA - First rigid body
   * @param pivotA - Pivot point in bodyA's local space
   * @param axisA - Hinge axis in bodyA's local space
   * @param bodyB - Second rigid body (or null)
   * @param pivotB - Pivot point in bodyB's local space
   * @param axisB - Hinge axis in bodyB's local space
   */
  constructor(
    bodyA: RigidBody,
    pivotA: Vector3,
    axisA: Vector3,
    bodyB: RigidBody | null = null,
    pivotB: Vector3 = new Vector3(),
    axisB: Vector3 = new Vector3(1, 0, 0)
  ) {
    super(bodyA, bodyB);
    this.pivotA = pivotA;
    this.pivotB = pivotB;
    this.axisA = axisA.normalize();
    this.axisB = axisB.normalize();
  }

  public createConstraint(ammo: typeof Ammo): void {
    const ammoBodyA = this.bodyA.getAmmoBody();
    if (!ammoBodyA) return;

    const btPivotA = new ammo.btVector3(this.pivotA.x, this.pivotA.y, this.pivotA.z);
    const btAxisA = new ammo.btVector3(this.axisA.x, this.axisA.y, this.axisA.z);

    if (this.bodyB) {
      const ammoBodyB = this.bodyB.getAmmoBody();
      if (!ammoBodyB) {
        ammo.destroy(btPivotA);
        ammo.destroy(btAxisA);
        return;
      }

      const btPivotB = new ammo.btVector3(this.pivotB.x, this.pivotB.y, this.pivotB.z);
      const btAxisB = new ammo.btVector3(this.axisB.x, this.axisB.y, this.axisB.z);

      this.constraint = new ammo.btHingeConstraint(
        ammoBodyA,
        ammoBodyB,
        btPivotA,
        btPivotB,
        btAxisA,
        btAxisB,
        false
      );

      ammo.destroy(btPivotB);
      ammo.destroy(btAxisB);
    } else {
      // Single body hinge - use transform instead
      const transform = new ammo.btTransform();
      transform.setIdentity();
      transform.setOrigin(btPivotA);
      this.constraint = new ammo.btHingeConstraint(ammoBodyA as any, transform, false);
      ammo.destroy(transform);
    }

    ammo.destroy(btPivotA);
    ammo.destroy(btAxisA);

    // Apply limits if set
    if (this.limits && this.constraint) {
      (this.constraint as Ammo.btHingeConstraint).setLimit(
        this.limits.low,
        this.limits.high,
        0.9,
        0.3,
        1.0
      );
    }

    // Enable motor if set
    if (this.motorEnabled && this.constraint) {
      (this.constraint as Ammo.btHingeConstraint).enableAngularMotor(
        true,
        this.motorVelocity,
        this.motorMaxImpulse
      );
    }
  }

  /**
   * Set angular limits
   * @param low - Lower limit in radians
   * @param high - Upper limit in radians
   */
  public setLimits(low: number, high: number): void {
    this.limits = { low, high };
    if (this.constraint) {
      (this.constraint as Ammo.btHingeConstraint).setLimit(low, high, 0.9, 0.3, 1.0);
    }
  }

  /**
   * Enable motor
   * @param velocity - Target velocity in radians/second
   * @param maxImpulse - Maximum impulse to apply
   */
  public enableMotor(velocity: number, maxImpulse: number): void {
    this.motorEnabled = true;
    this.motorVelocity = velocity;
    this.motorMaxImpulse = maxImpulse;

    if (this.constraint) {
      (this.constraint as Ammo.btHingeConstraint).enableAngularMotor(
        true,
        velocity,
        maxImpulse
      );
    }
  }

  /**
   * Disable motor
   */
  public disableMotor(): void {
    this.motorEnabled = false;

    if (this.constraint) {
      (this.constraint as Ammo.btHingeConstraint).enableAngularMotor(false, 0, 0);
    }
  }

  /**
   * Get current hinge angle
   */
  public getHingeAngle(): number {
    if (!this.constraint) return 0;
    // Note: getHingeAngle may not be available in all Ammo.js builds
    return 0; // Placeholder - would need proper Ammo.js API
  }
}

/**
 * Slider constraint
 * Constrains movement along a single axis (like a drawer)
 */
export class SliderConstraint extends Constraint {
  private frameA: { position: Vector3; rotation: Vector3 };
  private frameB: { position: Vector3; rotation: Vector3 };
  private limits: { lower: number; upper: number } | null = null;

  /**
   * Create a slider constraint
   * @param bodyA - First rigid body
   * @param frameA - Frame in bodyA's local space
   * @param bodyB - Second rigid body
   * @param frameB - Frame in bodyB's local space
   */
  constructor(
    bodyA: RigidBody,
    frameA: { position: Vector3; rotation: Vector3 },
    bodyB: RigidBody,
    frameB: { position: Vector3; rotation: Vector3 }
  ) {
    super(bodyA, bodyB);
    this.frameA = frameA;
    this.frameB = frameB;
  }

  public createConstraint(ammo: typeof Ammo): void {
    const ammoBodyA = this.bodyA.getAmmoBody();
    const ammoBodyB = this.bodyB?.getAmmoBody();
    if (!ammoBodyA || !ammoBodyB) return;

    // Create transforms
    const transformA = new ammo.btTransform();
    transformA.setIdentity();
    transformA.setOrigin(
      new ammo.btVector3(this.frameA.position.x, this.frameA.position.y, this.frameA.position.z)
    );

    const transformB = new ammo.btTransform();
    transformB.setIdentity();
    transformB.setOrigin(
      new ammo.btVector3(this.frameB.position.x, this.frameB.position.y, this.frameB.position.z)
    );

    this.constraint = new ammo.btSliderConstraint(
      ammoBodyA,
      ammoBodyB,
      transformA,
      transformB,
      false
    );

    // Apply limits if set
    if (this.limits) {
      (this.constraint as Ammo.btSliderConstraint).setLowerLinLimit(this.limits.lower);
      (this.constraint as Ammo.btSliderConstraint).setUpperLinLimit(this.limits.upper);
    }

    ammo.destroy(transformA);
    ammo.destroy(transformB);
  }

  /**
   * Set linear limits
   * @param lower - Lower limit
   * @param upper - Upper limit
   */
  public setLimits(lower: number, upper: number): void {
    this.limits = { lower, upper };

    if (this.constraint) {
      (this.constraint as Ammo.btSliderConstraint).setLowerLinLimit(lower);
      (this.constraint as Ammo.btSliderConstraint).setUpperLinLimit(upper);
    }
  }
}

/**
 * Generic 6DOF constraint
 * Constrains motion along and rotation around 3 axes
 */
export class Generic6DofConstraint extends Constraint {
  private frameA: { position: Vector3; rotation: Vector3 };
  private frameB: { position: Vector3; rotation: Vector3 };
  private linearLowerLimit: Vector3 = new Vector3(0, 0, 0);
  private linearUpperLimit: Vector3 = new Vector3(0, 0, 0);
  private angularLowerLimit: Vector3 = new Vector3(0, 0, 0);
  private angularUpperLimit: Vector3 = new Vector3(0, 0, 0);

  /**
   * Create a 6DOF constraint
   * @param bodyA - First rigid body
   * @param frameA - Frame in bodyA's local space
   * @param bodyB - Second rigid body
   * @param frameB - Frame in bodyB's local space
   */
  constructor(
    bodyA: RigidBody,
    frameA: { position: Vector3; rotation: Vector3 },
    bodyB: RigidBody,
    frameB: { position: Vector3; rotation: Vector3 }
  ) {
    super(bodyA, bodyB);
    this.frameA = frameA;
    this.frameB = frameB;
  }

  public createConstraint(ammo: typeof Ammo): void {
    const ammoBodyA = this.bodyA.getAmmoBody();
    const ammoBodyB = this.bodyB?.getAmmoBody();
    if (!ammoBodyA || !ammoBodyB) return;

    // Create transforms
    const transformA = new ammo.btTransform();
    transformA.setIdentity();
    transformA.setOrigin(
      new ammo.btVector3(this.frameA.position.x, this.frameA.position.y, this.frameA.position.z)
    );

    const transformB = new ammo.btTransform();
    transformB.setIdentity();
    transformB.setOrigin(
      new ammo.btVector3(this.frameB.position.x, this.frameB.position.y, this.frameB.position.z)
    );

    this.constraint = new ammo.btGeneric6DofConstraint(
      ammoBodyA,
      ammoBodyB,
      transformA,
      transformB,
      false
    );

    // Set limits
    const btLinearLower = new ammo.btVector3(
      this.linearLowerLimit.x,
      this.linearLowerLimit.y,
      this.linearLowerLimit.z
    );
    const btLinearUpper = new ammo.btVector3(
      this.linearUpperLimit.x,
      this.linearUpperLimit.y,
      this.linearUpperLimit.z
    );
    const btAngularLower = new ammo.btVector3(
      this.angularLowerLimit.x,
      this.angularLowerLimit.y,
      this.angularLowerLimit.z
    );
    const btAngularUpper = new ammo.btVector3(
      this.angularUpperLimit.x,
      this.angularUpperLimit.y,
      this.angularUpperLimit.z
    );

    (this.constraint as Ammo.btGeneric6DofConstraint).setLinearLowerLimit(btLinearLower);
    (this.constraint as Ammo.btGeneric6DofConstraint).setLinearUpperLimit(btLinearUpper);
    (this.constraint as Ammo.btGeneric6DofConstraint).setAngularLowerLimit(btAngularLower);
    (this.constraint as Ammo.btGeneric6DofConstraint).setAngularUpperLimit(btAngularUpper);

    ammo.destroy(transformA);
    ammo.destroy(transformB);
    ammo.destroy(btLinearLower);
    ammo.destroy(btLinearUpper);
    ammo.destroy(btAngularLower);
    ammo.destroy(btAngularUpper);
  }

  /**
   * Set linear limits
   * @param lower - Lower limit for each axis
   * @param upper - Upper limit for each axis
   */
  public setLinearLimits(lower: Vector3, upper: Vector3): void {
    this.linearLowerLimit = lower;
    this.linearUpperLimit = upper;

    if (this.constraint) {
      const ammo = (this.constraint as any).constructor;
      const btLinearLower = new ammo.btVector3(lower.x, lower.y, lower.z);
      const btLinearUpper = new ammo.btVector3(upper.x, upper.y, upper.z);
      (this.constraint as Ammo.btGeneric6DofConstraint).setLinearLowerLimit(btLinearLower);
      (this.constraint as Ammo.btGeneric6DofConstraint).setLinearUpperLimit(btLinearUpper);
      ammo.destroy(btLinearLower);
      ammo.destroy(btLinearUpper);
    }
  }

  /**
   * Set angular limits
   * @param lower - Lower limit for each axis (radians)
   * @param upper - Upper limit for each axis (radians)
   */
  public setAngularLimits(lower: Vector3, upper: Vector3): void {
    this.angularLowerLimit = lower;
    this.angularUpperLimit = upper;

    if (this.constraint) {
      const ammo = (this.constraint as any).constructor;
      const btAngularLower = new ammo.btVector3(lower.x, lower.y, lower.z);
      const btAngularUpper = new ammo.btVector3(upper.x, upper.y, upper.z);
      (this.constraint as Ammo.btGeneric6DofConstraint).setAngularLowerLimit(btAngularLower);
      (this.constraint as Ammo.btGeneric6DofConstraint).setAngularUpperLimit(btAngularUpper);
      ammo.destroy(btAngularLower);
      ammo.destroy(btAngularUpper);
    }
  }
}

/**
 * Fixed constraint
 * Locks two bodies together (no relative movement)
 */
export class FixedConstraint extends Constraint {
  private frameA: { position: Vector3; rotation: Vector3 };
  private frameB: { position: Vector3; rotation: Vector3 };

  /**
   * Create a fixed constraint
   * @param bodyA - First rigid body
   * @param frameA - Frame in bodyA's local space
   * @param bodyB - Second rigid body
   * @param frameB - Frame in bodyB's local space
   */
  constructor(
    bodyA: RigidBody,
    frameA: { position: Vector3; rotation: Vector3 },
    bodyB: RigidBody,
    frameB: { position: Vector3; rotation: Vector3 }
  ) {
    super(bodyA, bodyB);
    this.frameA = frameA;
    this.frameB = frameB;
  }

  public createConstraint(ammo: typeof Ammo): void {
    const ammoBodyA = this.bodyA.getAmmoBody();
    const ammoBodyB = this.bodyB?.getAmmoBody();
    if (!ammoBodyA || !ammoBodyB) return;

    // Create transforms
    const transformA = new ammo.btTransform();
    transformA.setIdentity();
    transformA.setOrigin(
      new ammo.btVector3(this.frameA.position.x, this.frameA.position.y, this.frameA.position.z)
    );

    const transformB = new ammo.btTransform();
    transformB.setIdentity();
    transformB.setOrigin(
      new ammo.btVector3(this.frameB.position.x, this.frameB.position.y, this.frameB.position.z)
    );

    this.constraint = new ammo.btFixedConstraint(ammoBodyA, ammoBodyB, transformA, transformB);

    ammo.destroy(transformA);
    ammo.destroy(transformB);
  }
}
