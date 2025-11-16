/**
 * @fileoverview Animation state for state machine transitions
 */

import { AnimationClip } from './AnimationClip';

/**
 * Blend modes for animation mixing
 */
export enum BlendMode {
  /** Override previous animations */
  OVERRIDE = 'override',
  /** Add to previous animations */
  ADDITIVE = 'additive',
}

/**
 * Animation state in a state machine
 */
export class AnimationState {
  /** State name */
  public readonly name: string;
  /** Animation clip to play */
  public readonly clip: AnimationClip;
  /** Playback speed multiplier */
  public speed: number = 1.0;
  /** Blend mode */
  public blendMode: BlendMode = BlendMode.OVERRIDE;
  /** Whether to loop this animation */
  public loop: boolean;
  /** Current playback time */
  public time: number = 0;
  /** Current blend weight (0-1) */
  public weight: number = 1.0;
  /** Whether state is currently playing */
  public isPlaying: boolean = false;

  constructor(name: string, clip: AnimationClip) {
    this.name = name;
    this.clip = clip;
    this.loop = clip.loop;
  }

  /**
   * Update state time
   */
  public update(deltaTime: number): void {
    if (!this.isPlaying) return;

    this.time += deltaTime * this.speed;

    // Handle looping or clamping
    if (this.loop && this.clip.duration > 0) {
      this.time = this.time % this.clip.duration;
    } else if (this.time >= this.clip.duration) {
      this.time = this.clip.duration;
      this.isPlaying = false;
    }
  }

  /**
   * Reset state to beginning
   */
  public reset(): void {
    this.time = 0;
    this.weight = 1.0;
  }

  /**
   * Start playing the state
   */
  public play(): void {
    this.isPlaying = true;
  }

  /**
   * Stop playing the state
   */
  public stop(): void {
    this.isPlaying = false;
    this.reset();
  }

  /**
   * Pause the state
   */
  public pause(): void {
    this.isPlaying = false;
  }

  /**
   * Sample the current animation values
   */
  public sample(): Map<string, unknown> {
    return this.clip.sample(this.time);
  }
}

/**
 * Transition between animation states
 */
export interface StateTransition {
  /** Source state name */
  from: string;
  /** Target state name */
  to: string;
  /** Transition duration in seconds */
  duration: number;
  /** Condition function to check if transition should occur */
  condition?: () => boolean;
}

/**
 * Animation state machine for managing state transitions
 */
export class AnimationStateMachine {
  /** All states in the machine */
  private states: Map<string, AnimationState> = new Map();
  /** Transitions between states */
  private transitions: StateTransition[] = [];
  /** Currently active state */
  private currentState: AnimationState | null = null;
  /** Target state during transition */
  private targetState: AnimationState | null = null;
  /** Current transition progress (0-1) */
  private transitionProgress: number = 0;
  /** Current transition duration */
  private transitionDuration: number = 0;

  /**
   * Add a state to the machine
   */
  public addState(state: AnimationState): void {
    this.states.set(state.name, state);

    // Set first state as current if none set
    if (!this.currentState) {
      this.currentState = state;
      this.currentState.play();
    }
  }

  /**
   * Get a state by name
   */
  public getState(name: string): AnimationState | undefined {
    return this.states.get(name);
  }

  /**
   * Get current state
   */
  public getCurrentState(): AnimationState | null {
    return this.currentState;
  }

  /**
   * Add a transition between states
   */
  public addTransition(transition: StateTransition): void {
    this.transitions.push(transition);
  }

  /**
   * Manually transition to a state
   */
  public transitionTo(stateName: string, duration: number = 0.3): boolean {
    const state = this.states.get(stateName);
    if (!state || state === this.currentState) return false;

    if (duration <= 0) {
      // Instant transition
      if (this.currentState) this.currentState.stop();
      this.currentState = state;
      this.currentState.reset();
      this.currentState.play();
      this.targetState = null;
      this.transitionProgress = 0;
    } else {
      // Smooth transition
      this.targetState = state;
      this.targetState.reset();
      this.targetState.play();
      this.transitionDuration = duration;
      this.transitionProgress = 0;
    }

    return true;
  }

  /**
   * Update state machine
   */
  public update(deltaTime: number): void {
    // Check for automatic transitions
    if (!this.targetState) {
      for (const transition of this.transitions) {
        if (
          this.currentState?.name === transition.from &&
          (!transition.condition || transition.condition())
        ) {
          this.transitionTo(transition.to, transition.duration);
          break;
        }
      }
    }

    // Update transition
    if (this.targetState) {
      this.transitionProgress += deltaTime / this.transitionDuration;

      if (this.transitionProgress >= 1.0) {
        // Transition complete
        if (this.currentState) this.currentState.stop();
        this.currentState = this.targetState;
        this.targetState = null;
        this.transitionProgress = 0;
      } else {
        // Update blend weights during transition
        if (this.currentState) {
          this.currentState.weight = 1.0 - this.transitionProgress;
          this.currentState.update(deltaTime);
        }
        this.targetState.weight = this.transitionProgress;
        this.targetState.update(deltaTime);
      }
    } else if (this.currentState) {
      // Normal update
      this.currentState.weight = 1.0;
      this.currentState.update(deltaTime);
    }
  }

  /**
   * Sample the current animation state(s) with blending
   */
  public sample(): Map<string, unknown> {
    const result = new Map<string, unknown>();

    if (!this.currentState) return result;

    // Sample current state
    const currentSample = this.currentState.sample();

    if (!this.targetState || this.transitionProgress === 0) {
      // No blending needed
      return currentSample;
    }

    // Sample target state
    const targetSample = this.targetState.sample();

    // Blend values
    for (const [property, value1] of currentSample) {
      const value2 = targetSample.get(property);

      if (value2 === undefined) {
        result.set(property, value1);
        continue;
      }

      // Blend based on type
      if (typeof value1 === 'number' && typeof value2 === 'number') {
        result.set(
          property,
          value1 * this.currentState.weight + value2 * this.targetState.weight,
        );
      } else {
        // For complex types, use current value
        // (proper blending would require type information)
        result.set(property, this.transitionProgress < 0.5 ? value1 : value2);
      }
    }

    // Add properties only in target
    for (const [property, value] of targetSample) {
      if (!result.has(property)) {
        result.set(property, value);
      }
    }

    return result;
  }

  /**
   * Reset the state machine
   */
  public reset(): void {
    for (const state of this.states.values()) {
      state.stop();
    }

    if (this.currentState) {
      this.currentState.reset();
      this.currentState.play();
    }

    this.targetState = null;
    this.transitionProgress = 0;
  }
}
