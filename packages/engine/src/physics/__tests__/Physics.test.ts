/**
 * Physics System Tests
 */

import { PhysicsWorld } from '../PhysicsWorld';
import { RigidBody, RigidBodyType } from '../RigidBody';
import {
  BoxCollider,
  SphereCollider,
  CapsuleCollider,
  CylinderCollider,
  ConeCollider,
  PlaneCollider,
} from '../Collider';
import { Vector3 } from '../../math/Vector3';

describe('PhysicsWorld', () => {
  it('should create a physics world', async () => {
    const world = new PhysicsWorld();
    await world.initialize();
    expect(world).toBeDefined();
    expect(world.getWorld()).toBeDefined();
    world.destroy();
  });

  it('should set and get gravity', async () => {
    const world = new PhysicsWorld();
    await world.initialize();
    
    const gravity = new Vector3(0, -10, 0);
    world.setGravity(gravity);
    
    const retrieved = world.getGravity();
    expect(retrieved.y).toBeCloseTo(-10);
    
    world.destroy();
  });

  it('should set fixed time step', async () => {
    const world = new PhysicsWorld();
    await world.initialize();
    
    world.setFixedTimeStep(1 / 120);
    world.setMaxSubSteps(20);
    
    expect(world).toBeDefined();
    world.destroy();
  });
});

describe('RigidBody', () => {
  it('should create a rigid body', () => {
    const rigidBody = new RigidBody();
    expect(rigidBody).toBeDefined();
    expect(rigidBody.getType()).toBe(RigidBodyType.DYNAMIC);
  });

  it('should set and get rigid body type', () => {
    const rigidBody = new RigidBody();
    rigidBody.setType(RigidBodyType.STATIC);
    expect(rigidBody.getType()).toBe(RigidBodyType.STATIC);
  });

  it('should set and get mass', () => {
    const rigidBody = new RigidBody();
    rigidBody.setMass(5.0);
    expect(rigidBody.getMass()).toBe(5.0);
  });

  it('should set and get friction', () => {
    const rigidBody = new RigidBody();
    rigidBody.setFriction(0.8);
    expect(rigidBody.getFriction()).toBe(0.8);
  });

  it('should set and get restitution', () => {
    const rigidBody = new RigidBody();
    rigidBody.setRestitution(0.6);
    expect(rigidBody.getRestitution()).toBe(0.6);
  });
});

describe('Colliders', () => {
  it('should create a box collider', () => {
    const collider = new BoxCollider(new Vector3(1, 1, 1));
    expect(collider).toBeDefined();
    
    const halfExtents = collider.getHalfExtents();
    expect(halfExtents.x).toBe(1);
    expect(halfExtents.y).toBe(1);
    expect(halfExtents.z).toBe(1);
  });

  it('should create a sphere collider', () => {
    const collider = new SphereCollider(2.0);
    expect(collider).toBeDefined();
    expect(collider.getRadius()).toBe(2.0);
  });

  it('should create a capsule collider', () => {
    const collider = new CapsuleCollider(0.5, 2.0, 1);
    expect(collider).toBeDefined();
    expect(collider.getRadius()).toBe(0.5);
    expect(collider.getHeight()).toBe(2.0);
  });

  it('should create a cylinder collider', () => {
    const collider = new CylinderCollider(new Vector3(0.5, 1, 0.5), 1);
    expect(collider).toBeDefined();
    
    const halfExtents = collider.getHalfExtents();
    expect(halfExtents.y).toBe(1);
  });

  it('should create a cone collider', () => {
    const collider = new ConeCollider(0.5, 1.5, 1);
    expect(collider).toBeDefined();
    expect(collider.getRadius()).toBe(0.5);
    expect(collider.getHeight()).toBe(1.5);
  });

  it('should create a plane collider', () => {
    const collider = new PlaneCollider(new Vector3(0, 1, 0), 0);
    expect(collider).toBeDefined();
    
    const normal = collider.getNormal();
    expect(normal.y).toBeCloseTo(1);
    expect(collider.getPlaneConstant()).toBe(0);
  });

  it('should modify collider properties', () => {
    const box = new BoxCollider();
    box.setHalfExtents(new Vector3(2, 3, 4));
    const extents = box.getHalfExtents();
    expect(extents.x).toBe(2);
    expect(extents.y).toBe(3);
    expect(extents.z).toBe(4);

    const sphere = new SphereCollider();
    sphere.setRadius(3.5);
    expect(sphere.getRadius()).toBe(3.5);

    const capsule = new CapsuleCollider();
    capsule.setRadius(1.2);
    capsule.setHeight(3.0);
    expect(capsule.getRadius()).toBe(1.2);
    expect(capsule.getHeight()).toBe(3.0);
  });
});
