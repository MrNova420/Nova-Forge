/**
 * Job System Integration Layer
 * 
 * Integrates job system with engine subsystems (assets, physics, animation, particles).
 * Provides high-level job management for common game engine tasks.
 */

import { Job } from './Job';
import { ParallelForJob } from './ParallelForJob';
import { AsyncLoadJob } from './AsyncLoadJob';
import type { JobScheduler } from './JobScheduler';
import type { JobPriority } from './IJob';

/**
 * Asset metadata
 */
export interface Asset {
  id: string;
  url: string;
  type: 'mesh' | 'texture' | 'audio' | 'shader' | 'scene';
  size?: number;
  dependencies?: string[];
}

/**
 * Progress callback for asset loading
 */
export type ProgressCallback = (loaded: number, total: number) => void;

/**
 * Asset Loading Job System
 * 
 * Manages parallel asset loading with dependencies and progress tracking.
 */
export class AssetLoadingJobSystem {
  constructor(private scheduler: JobScheduler) {}

  /**
   * Load a single asset
   */
  async loadAsset(asset: Asset, priority: JobPriority = 3): Promise<any> {
    const job = new AsyncLoadJob({
      name: `Load ${asset.type}: ${asset.id}`,
      url: asset.url,
      loader: async (url) => {
        // Simulate asset loading based on type
        const response = await fetch(url);
        if (!response.ok) {
          throw new Error(`Failed to load ${asset.id}: ${response.statusText}`);
        }
        return await response.arrayBuffer();
      },
      priority,
    });

    const result = await this.scheduler.submit(job);
    return result.data;
  }

  /**
   * Load multiple assets in batch with dependencies
   */
  async loadAssetBatch(
    assets: Asset[],
    onProgress?: ProgressCallback
  ): Promise<Map<string, any>> {
    const results = new Map<string, any>();
    let loaded = 0;

    // Sort by dependencies
    const sorted = this.topologicalSort(assets);

    for (const asset of sorted) {
      try {
        const data = await this.loadAsset(asset);
        results.set(asset.id, data);
        loaded++;
        
        if (onProgress) {
          onProgress(loaded, assets.length);
        }
      } catch (error) {
        console.error(`Failed to load asset ${asset.id}:`, error);
        throw error;
      }
    }

    return results;
  }

  /**
   * Load assets progressively with priority
   */
  async loadProgressive(
    assets: Asset[],
    onProgress?: ProgressCallback
  ): Promise<void> {
    const jobs = assets.map(asset => {
      return new AsyncLoadJob({
        name: `Load ${asset.type}: ${asset.id}`,
        url: asset.url,
        loader: async (url) => {
          const response = await fetch(url);
          return await response.arrayBuffer();
        },
        priority: this.getAssetPriority(asset),
      });
    });

    let loaded = 0;
    const promises = jobs.map(async (job) => {
      const result = await this.scheduler.submit(job);
      loaded++;
      if (onProgress) {
        onProgress(loaded, assets.length);
      }
      return result;
    });

    await Promise.all(promises);
  }

  /**
   * Cancel loading operations
   */
  cancelLoading(assetIds: string[]): void {
    // Implementation would cancel jobs by ID
    for (const id of assetIds) {
      this.scheduler.cancel(id);
    }
  }

  /**
   * Topological sort for dependency resolution
   */
  private topologicalSort(assets: Asset[]): Asset[] {
    const sorted: Asset[] = [];
    const visited = new Set<string>();
    const visiting = new Set<string>();

    const visit = (asset: Asset) => {
      if (visited.has(asset.id)) return;
      if (visiting.has(asset.id)) {
        throw new Error(`Circular dependency detected: ${asset.id}`);
      }

      visiting.add(asset.id);

      if (asset.dependencies) {
        for (const depId of asset.dependencies) {
          const dep = assets.find(a => a.id === depId);
          if (dep) {
            visit(dep);
          }
        }
      }

      visiting.delete(asset.id);
      visited.add(asset.id);
      sorted.push(asset);
    };

    for (const asset of assets) {
      visit(asset);
    }

    return sorted;
  }

  /**
   * Get priority based on asset type
   */
  private getAssetPriority(asset: Asset): JobPriority {
    switch (asset.type) {
      case 'shader': return 5; // Critical
      case 'mesh': return 4; // High
      case 'texture': return 3; // Normal
      case 'audio': return 2; // Low
      default: return 3;
    }
  }
}

/**
 * Rigid body for physics
 */
export interface RigidBody {
  id: string;
  position: [number, number, number];
  velocity: [number, number, number];
  mass: number;
  update(deltaTime: number): void;
}

/**
 * Collision result
 */
export interface Collision {
  bodyA: string;
  bodyB: string;
  point: [number, number, number];
  normal: [number, number, number];
  depth: number;
}

/**
 * Constraint for physics
 */
export interface Constraint {
  id: string;
  bodyA: string;
  bodyB: string;
  type: 'distance' | 'hinge' | 'slider';
  solve(): void;
}

/**
 * Physics Job System
 * 
 * Manages parallel physics simulation tasks.
 */
export class PhysicsJobSystem {
  constructor(private scheduler: JobScheduler) {}

  /**
   * Step physics simulation in parallel
   */
  async stepParallel(
    bodies: RigidBody[],
    deltaTime: number
  ): Promise<void> {
    const job = new ParallelForJob({
      name: 'Physics Step',
      items: bodies,
      batchSize: Math.ceil(bodies.length / this.scheduler.getWorkerCount()),
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (body) => {
        body.update(deltaTime);
      },
      priority: 4, // High priority
    });

    await this.scheduler.submit(job);
  }

  /**
   * Detect collisions between bodies
   */
  async detectCollisions(bodies: RigidBody[]): Promise<Collision[]> {
    const collisions: Collision[] = [];

    // Broad phase - spatial partitioning would go here
    const job = new ParallelForJob({
      name: 'Collision Detection',
      items: bodies,
      batchSize: 50,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (bodyA) => {
        // Simplified collision detection
        for (const bodyB of bodies) {
          if (bodyA.id === bodyB.id) continue;
          
          // Simple sphere-sphere collision
          const dx = bodyA.position[0] - bodyB.position[0];
          const dy = bodyA.position[1] - bodyB.position[1];
          const dz = bodyA.position[2] - bodyB.position[2];
          const distSq = dx * dx + dy * dy + dz * dz;
          const minDist = 1.0; // Assume unit radius
          
          if (distSq < minDist * minDist) {
            const dist = Math.sqrt(distSq);
            collisions.push({
              bodyA: bodyA.id,
              bodyB: bodyB.id,
              point: [
                (bodyA.position[0] + bodyB.position[0]) / 2,
                (bodyA.position[1] + bodyB.position[1]) / 2,
                (bodyA.position[2] + bodyB.position[2]) / 2,
              ],
              normal: [dx / dist, dy / dist, dz / dist],
              depth: minDist - dist,
            });
          }
        }
      },
      priority: 4,
    });

    await this.scheduler.submit(job);
    return collisions;
  }

  /**
   * Solve constraints in parallel
   */
  async solveConstraints(constraints: Constraint[]): Promise<void> {
    const job = new ParallelForJob({
      name: 'Constraint Solver',
      items: constraints,
      batchSize: 25,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (constraint) => {
        constraint.solve();
      },
      priority: 4,
    });

    await this.scheduler.submit(job);
  }
}

/**
 * Skeleton for animation
 */
export interface Skeleton {
  id: string;
  bones: Bone[];
  updateBones(): void;
}

/**
 * Bone data
 */
export interface Bone {
  id: string;
  position: [number, number, number];
  rotation: [number, number, number, number];
  parent?: string;
}

/**
 * IK chain
 */
export interface IKChain {
  id: string;
  bones: string[];
  target: [number, number, number];
  solve(): void;
}

/**
 * Blend tree
 */
export interface BlendTree {
  id: string;
  animations: string[];
  weights: number[];
  evaluate(): void;
}

/**
 * Animation Job System
 * 
 * Manages parallel animation updates.
 */
export class AnimationJobSystem {
  constructor(private scheduler: JobScheduler) {}

  /**
   * Update skeleton bones in parallel
   */
  async updateBones(skeletons: Skeleton[]): Promise<void> {
    const job = new ParallelForJob({
      name: 'Update Bones',
      items: skeletons,
      batchSize: 10,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (skeleton) => {
        skeleton.updateBones();
      },
      priority: 3,
    });

    await this.scheduler.submit(job);
  }

  /**
   * Solve IK chains in parallel
   */
  async solveIK(chains: IKChain[]): Promise<void> {
    const job = new ParallelForJob({
      name: 'Solve IK',
      items: chains,
      batchSize: 5,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (chain) => {
        chain.solve();
      },
      priority: 3,
    });

    await this.scheduler.submit(job);
  }

  /**
   * Evaluate blend trees in parallel
   */
  async evaluateBlendTrees(trees: BlendTree[]): Promise<void> {
    const job = new ParallelForJob({
      name: 'Evaluate Blend Trees',
      items: trees,
      batchSize: 10,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (tree) => {
        tree.evaluate();
      },
      priority: 3,
    });

    await this.scheduler.submit(job);
  }
}

/**
 * Particle system
 */
export interface ParticleSystem {
  id: string;
  particles: Particle[];
  update(deltaTime: number): void;
}

/**
 * Particle data
 */
export interface Particle {
  id: string;
  position: [number, number, number];
  velocity: [number, number, number];
  life: number;
  maxLife: number;
  update(deltaTime: number): void;
}

/**
 * Particle Job System
 * 
 * Manages parallel particle updates.
 */
export class ParticleJobSystem {
  constructor(private scheduler: JobScheduler) {}

  /**
   * Update particle systems in parallel
   */
  async updateParticles(systems: ParticleSystem[], deltaTime: number): Promise<void> {
    const job = new ParallelForJob({
      name: 'Update Particles',
      items: systems,
      batchSize: 5,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (system) => {
        system.update(deltaTime);
      },
      priority: 2, // Low priority
    });

    await this.scheduler.submit(job);
  }

  /**
   * Detect particle collisions
   */
  async detectParticleCollisions(particles: Particle[]): Promise<Collision[]> {
    const collisions: Collision[] = [];

    const job = new ParallelForJob({
      name: 'Particle Collisions',
      items: particles,
      batchSize: 100,
      maxWorkers: this.scheduler.getWorkerCount(),
      process: (particle) => {
        // Simplified collision detection
        // In a real implementation, this would use spatial partitioning
      },
      priority: 2,
    });

    await this.scheduler.submit(job);
    return collisions;
  }
}

/**
 * Integrated job system manager
 * 
 * Provides unified access to all job system integrations.
 */
export class IntegratedJobSystem {
  readonly assets: AssetLoadingJobSystem;
  readonly physics: PhysicsJobSystem;
  readonly animation: AnimationJobSystem;
  readonly particles: ParticleJobSystem;

  constructor(scheduler: JobScheduler) {
    this.assets = new AssetLoadingJobSystem(scheduler);
    this.physics = new PhysicsJobSystem(scheduler);
    this.animation = AnimationJobSystem(scheduler);
    this.particles = new ParticleJobSystem(scheduler);
  }
}
