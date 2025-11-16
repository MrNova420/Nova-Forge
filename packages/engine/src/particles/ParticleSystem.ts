/**
 * Particle System
 * CPU-based particle simulation for effects like fire, smoke, rain, explosions
 */

import { Vector3 } from '../math/Vector3';
import { Component } from '../ecs/Component';

/**
 * Individual particle data
 */
export interface Particle {
  position: Vector3;
  velocity: Vector3;
  acceleration: Vector3;
  color: [number, number, number, number]; // RGBA
  size: number;
  life: number; // Current lifetime
  maxLife: number; // Maximum lifetime
  rotation: number;
  rotationSpeed: number;
  active: boolean;
}

/**
 * Particle emitter shape
 */
export enum EmitterShape {
  POINT = 'POINT',
  BOX = 'BOX',
  SPHERE = 'SPHERE',
  CONE = 'CONE',
  CIRCLE = 'CIRCLE',
}

/**
 * Particle system component
 */
export class ParticleSystem extends Component {
  public static readonly typeName = 'ParticleSystem';

  // Particle pool
  private particles: Particle[] = [];
  private maxParticles: number = 1000;
  private activeCount: number = 0;

  // Emission properties
  private emissionRate: number = 10; // Particles per second
  private emissionAccumulator: number = 0;
  private emitterShape: EmitterShape = EmitterShape.POINT;
  private emitterSize: Vector3 = new Vector3(1, 1, 1);
  private emitterAngle: number = 30; // Cone angle in degrees

  // Particle properties
  private startLife: number = 2.0; // Seconds
  private startSpeed: number = 5.0;
  private startSize: number = 1.0;
  private startColor: [number, number, number, number] = [1, 1, 1, 1];
  private endColor: [number, number, number, number] = [1, 1, 1, 0];

  // Size over lifetime
  private sizeOverLifetime: boolean = false;
  private endSize: number = 0.5;

  // Rotation
  private startRotation: number = 0;
  private rotationSpeed: number = 0;

  // Physics
  private gravity: Vector3 = new Vector3(0, -9.81, 0);
  private drag: number = 0.1;

  // Velocity randomness
  private velocityRandomness: number = 0.2;

  // Control
  private playing: boolean = false;
  private looping: boolean = true;
  private duration: number = 5.0; // For non-looping systems
  private playTime: number = 0;

  /**
   * Initialize the particle system
   */
  public initialize(): void {
    // Pre-allocate particle pool
    for (let i = 0; i < this.maxParticles; i++) {
      this.particles.push({
        position: new Vector3(),
        velocity: new Vector3(),
        acceleration: new Vector3(),
        color: [1, 1, 1, 1],
        size: 1,
        life: 0,
        maxLife: 1,
        rotation: 0,
        rotationSpeed: 0,
        active: false,
      });
    }
  }

  /**
   * Update particles
   */
  public update(deltaTime: number): void {
    if (!this.playing) return;

    this.playTime += deltaTime;

    // Stop if duration reached and not looping
    if (!this.looping && this.playTime >= this.duration) {
      this.playing = false;
      return;
    }

    // Emit new particles
    if (this.looping || this.playTime < this.duration) {
      this.emit(deltaTime);
    }

    // Update existing particles
    for (let i = 0; i < this.maxParticles; i++) {
      const particle = this.particles[i];
      if (!particle || !particle.active) continue;

      // Update lifetime
      particle.life -= deltaTime;
      if (particle.life <= 0) {
        particle.active = false;
        this.activeCount--;
        continue;
      }

      // Calculate normalized life (1.0 = just born, 0.0 = about to die)
      const normalizedLife = particle.life / particle.maxLife;

      // Update physics
      particle.acceleration.set(this.gravity.x, this.gravity.y, this.gravity.z);
      
      // Apply drag
      const dragForce = particle.velocity.clone().multiplyScalar(-this.drag);
      particle.acceleration.add(dragForce);

      // Update velocity
      particle.velocity.add(particle.acceleration.clone().multiplyScalar(deltaTime));

      // Update position
      particle.position.add(particle.velocity.clone().multiplyScalar(deltaTime));

      // Update rotation
      particle.rotation += particle.rotationSpeed * deltaTime;

      // Update size over lifetime
      if (this.sizeOverLifetime) {
        particle.size = this.lerp(this.endSize, this.startSize, normalizedLife);
      }

      // Update color over lifetime
      particle.color[0] = this.lerp(this.endColor[0], this.startColor[0], normalizedLife);
      particle.color[1] = this.lerp(this.endColor[1], this.startColor[1], normalizedLife);
      particle.color[2] = this.lerp(this.endColor[2], this.startColor[2], normalizedLife);
      particle.color[3] = this.lerp(this.endColor[3], this.startColor[3], normalizedLife);
    }
  }

  /**
   * Emit new particles
   */
  private emit(deltaTime: number): void {
    this.emissionAccumulator += this.emissionRate * deltaTime;

    while (this.emissionAccumulator >= 1.0) {
      this.emissionAccumulator -= 1.0;
      this.emitParticle();
    }
  }

  /**
   * Emit a single particle
   */
  private emitParticle(): void {
    if (this.activeCount >= this.maxParticles) return;

    // Find inactive particle
    for (let i = 0; i < this.maxParticles; i++) {
      const particle = this.particles[i];
      if (!particle || particle.active) continue;

      // Reset particle
      particle.active = true;
      this.activeCount++;

      // Set position based on emitter shape
      this.setParticlePosition(particle);

      // Set velocity
      this.setParticleVelocity(particle);

      // Set properties
      particle.life = this.startLife;
      particle.maxLife = this.startLife;
      particle.size = this.startSize;
      particle.color = [...this.startColor];
      particle.rotation = this.startRotation + (Math.random() - 0.5) * Math.PI;
      particle.rotationSpeed = this.rotationSpeed;

      break;
    }
  }

  /**
   * Set particle position based on emitter shape
   */
  private setParticlePosition(particle: Particle): void {
    switch (this.emitterShape) {
      case EmitterShape.POINT:
        particle.position.set(0, 0, 0);
        break;

      case EmitterShape.BOX:
        particle.position.set(
          (Math.random() - 0.5) * this.emitterSize.x,
          (Math.random() - 0.5) * this.emitterSize.y,
          (Math.random() - 0.5) * this.emitterSize.z
        );
        break;

      case EmitterShape.SPHERE:
        {
          const theta = Math.random() * Math.PI * 2;
          const phi = Math.acos(2 * Math.random() - 1);
          const r = Math.cbrt(Math.random()) * this.emitterSize.x;
          particle.position.set(
            r * Math.sin(phi) * Math.cos(theta),
            r * Math.sin(phi) * Math.sin(theta),
            r * Math.cos(phi)
          );
        }
        break;

      case EmitterShape.CONE:
        {
          const angle = Math.random() * Math.PI * 2;
          const radius = Math.random() * this.emitterSize.x;
          particle.position.set(
            Math.cos(angle) * radius,
            0,
            Math.sin(angle) * radius
          );
        }
        break;

      case EmitterShape.CIRCLE:
        {
          const angle = Math.random() * Math.PI * 2;
          const radius = Math.sqrt(Math.random()) * this.emitterSize.x;
          particle.position.set(
            Math.cos(angle) * radius,
            0,
            Math.sin(angle) * radius
          );
        }
        break;
    }
  }

  /**
   * Set particle velocity
   */
  private setParticleVelocity(particle: Particle): void {
    let direction = new Vector3();

    switch (this.emitterShape) {
      case EmitterShape.POINT:
      case EmitterShape.BOX:
        // Random direction
        direction.set(
          Math.random() - 0.5,
          Math.random() - 0.5,
          Math.random() - 0.5
        );
        direction.normalize();
        break;

      case EmitterShape.SPHERE:
        // Outward from center
        direction = particle.position.clone().normalize();
        break;

      case EmitterShape.CONE:
        {
          // Cone direction with angle
          const coneAngle = (this.emitterAngle * Math.PI) / 180;
          const angle = Math.random() * Math.PI * 2;
          const elevation = Math.random() * coneAngle;
          direction.set(
            Math.sin(elevation) * Math.cos(angle),
            Math.cos(elevation),
            Math.sin(elevation) * Math.sin(angle)
          );
        }
        break;

      case EmitterShape.CIRCLE:
        // Outward in XZ plane
        direction = particle.position.clone();
        direction.y = 0;
        direction.normalize();
        break;
    }

    // Add randomness
    direction.x += (Math.random() - 0.5) * this.velocityRandomness;
    direction.y += (Math.random() - 0.5) * this.velocityRandomness;
    direction.z += (Math.random() - 0.5) * this.velocityRandomness;
    direction.normalize();

    // Set velocity
    particle.velocity = direction.multiplyScalar(this.startSpeed);
    particle.acceleration.set(0, 0, 0);
  }

  /**
   * Linear interpolation
   */
  private lerp(a: number, b: number, t: number): number {
    return a + (b - a) * t;
  }

  /**
   * Play the particle system
   */
  public play(): void {
    this.playing = true;
    this.playTime = 0;
  }

  /**
   * Stop the particle system
   */
  public stop(): void {
    this.playing = false;
  }

  /**
   * Clear all particles
   */
  public clear(): void {
    for (const particle of this.particles) {
      particle.active = false;
    }
    this.activeCount = 0;
  }

  // Getters and setters
  public getParticles(): Particle[] {
    return this.particles;
  }

  public getActiveCount(): number {
    return this.activeCount;
  }

  public setMaxParticles(max: number): void {
    this.maxParticles = max;
  }

  public setEmissionRate(rate: number): void {
    this.emissionRate = rate;
  }

  public setEmitterShape(shape: EmitterShape): void {
    this.emitterShape = shape;
  }

  public setEmitterSize(size: Vector3): void {
    this.emitterSize = size.clone();
  }

  public setEmitterAngle(angle: number): void {
    this.emitterAngle = angle;
  }

  public setStartLife(life: number): void {
    this.startLife = life;
  }

  public setStartSpeed(speed: number): void {
    this.startSpeed = speed;
  }

  public setStartSize(size: number): void {
    this.startSize = size;
  }

  public setEndSize(size: number): void {
    this.endSize = size;
  }

  public setSizeOverLifetime(enabled: boolean): void {
    this.sizeOverLifetime = enabled;
  }

  public setStartColor(color: [number, number, number, number]): void {
    this.startColor = [...color];
  }

  public setEndColor(color: [number, number, number, number]): void {
    this.endColor = [...color];
  }

  public setGravity(gravity: Vector3): void {
    this.gravity = gravity.clone();
  }

  public setDrag(drag: number): void {
    this.drag = drag;
  }

  public setVelocityRandomness(randomness: number): void {
    this.velocityRandomness = randomness;
  }

  public setLooping(looping: boolean): void {
    this.looping = looping;
  }

  public setDuration(duration: number): void {
    this.duration = duration;
  }

  public isPlaying(): boolean {
    return this.playing;
  }
}
