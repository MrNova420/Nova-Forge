/**
 * @fileoverview Animator component for entity animation playback
 */

import { Component } from '../ecs/Component';
import { AnimationClip } from './AnimationClip';
import { AnimationState, AnimationStateMachine } from './AnimationState';

/**
 * Animator component - manages animation playback for an entity
 */
export class Animator extends Component {
  /** Animation state machine */
  private stateMachine: AnimationStateMachine;
  /** Cache of property setters for performance */
  private propertySetters: Map<string, (value: unknown) => void> = new Map();

  constructor() {
    super();
    this.stateMachine = new AnimationStateMachine();
  }

  /**
   * Add an animation clip as a state
   */
  public addClip(name: string, clip: AnimationClip): void {
    const state = new AnimationState(name, clip);
    this.stateMachine.addState(state);
  }

  /**
   * Add a pre-configured animation state
   */
  public addState(state: AnimationState): void {
    this.stateMachine.addState(state);
  }

  /**
   * Get a state by name
   */
  public getState(name: string): AnimationState | undefined {
    return this.stateMachine.getState(name);
  }

  /**
   * Get current state
   */
  public getCurrentState(): AnimationState | null {
    return this.stateMachine.getCurrentState();
  }

  /**
   * Play an animation by name
   */
  public play(stateName: string, transitionDuration: number = 0.3): boolean {
    return this.stateMachine.transitionTo(stateName, transitionDuration);
  }

  /**
   * Stop the current animation
   */
  public stop(): void {
    const current = this.stateMachine.getCurrentState();
    if (current) current.stop();
  }

  /**
   * Pause the current animation
   */
  public pause(): void {
    const current = this.stateMachine.getCurrentState();
    if (current) current.pause();
  }

  /**
   * Resume the current animation
   */
  public resume(): void {
    const current = this.stateMachine.getCurrentState();
    if (current) current.play();
  }

  /**
   * Check if currently playing
   */
  public isPlaying(): boolean {
    const current = this.stateMachine.getCurrentState();
    return current ? current.isPlaying : false;
  }

  /**
   * Get current animation time
   */
  public getTime(): number {
    const current = this.stateMachine.getCurrentState();
    return current ? current.time : 0;
  }

  /**
   * Set current animation time
   */
  public setTime(time: number): void {
    const current = this.stateMachine.getCurrentState();
    if (current) current.time = time;
  }

  /**
   * Set playback speed
   */
  public setSpeed(speed: number): void {
    const current = this.stateMachine.getCurrentState();
    if (current) current.speed = speed;
  }

  /**
   * Add a state transition
   */
  public addTransition(
    from: string,
    to: string,
    duration: number = 0.3,
    condition?: () => boolean,
  ): void {
    this.stateMachine.addTransition({ from, to, duration, condition });
  }

  /**
   * Update animation - called by AnimationSystem
   */
  public update(deltaTime: number): void {
    // Update state machine
    this.stateMachine.update(deltaTime);

    // Sample current animation values
    const values = this.stateMachine.sample();

    // Apply values to entity
    if (this.entity && values.size > 0) {
      this.applyAnimationValues(values);
    }
  }

  /**
   * Apply sampled animation values to entity
   */
  private applyAnimationValues(values: Map<string, unknown>): void {
    for (const [property, value] of values) {
      this.setProperty(property, value);
    }
  }

  /**
   * Set a property on the entity using path notation
   * Example: "transform.position.x" -> entity.transform.position.x = value
   */
  private setProperty(path: string, value: unknown): void {
    if (!this.entity) return;

    // Check cache first
    let setter = this.propertySetters.get(path);

    if (!setter) {
      // Build setter and cache it
      const parts = path.split('.');
      setter = this.buildPropertySetter(parts);
      this.propertySetters.set(path, setter);
    }

    // Apply value
    if (setter) {
      setter(value);
    }
  }

  /**
   * Build a property setter function from path parts
   */
  private buildPropertySetter(parts: string[]): (value: unknown) => void {
    return (value: unknown) => {
      if (!this.entity) return;

      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      let target: any = this.entity;

      // Navigate to parent object
      for (let i = 0; i < parts.length - 1; i++) {
        const part = parts[i];
        if (part) {
          target = target[part];
          if (!target) return;
        }
      }

      // Set final property
      const finalProperty = parts[parts.length - 1];
      if (finalProperty) {
        target[finalProperty] = value;
      }
    };
  }

  /**
   * Reset animator to initial state
   */
  public reset(): void {
    this.stateMachine.reset();
    this.propertySetters.clear();
  }

  /**
   * Component lifecycle - called when detached
   */
  public override onDetach(): void {
    this.stop();
    this.propertySetters.clear();
  }
}
