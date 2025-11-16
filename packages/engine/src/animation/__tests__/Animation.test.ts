/**
 * @fileoverview Tests for animation system
 */

import { describe, it, expect } from '@jest/globals';
import {
  AnimationClip,
  AnimationTrack,
  AnimationPropertyType,
  InterpolationType,
} from '../AnimationClip';
import { AnimationState, AnimationStateMachine } from '../AnimationState';
import { Animator } from '../Animator';
import { AnimationSystem } from '../AnimationSystem';
import { AnimationMixer } from '../AnimationMixer';
import { Vector3 } from '../../math/Vector3';
import { World } from '../../ecs/World';
import { Entity } from '../../ecs/Entity';

describe('AnimationClip', () => {
  it('should create animation clip', () => {
    const clip = new AnimationClip('walk', 2.0);
    expect(clip.name).toBe('walk');
    expect(clip.duration).toBe(2.0);
  });

  it('should add and sample number track', () => {
    const clip = new AnimationClip('test', 1.0);
    const track = new AnimationTrack<number>('opacity', AnimationPropertyType.NUMBER);

    track.addKeyframe(0, 0);
    track.addKeyframe(1, 1);

    clip.addTrack(track);

    const sample = clip.sample(0.5);
    expect(sample.get('opacity')).toBeCloseTo(0.5);
  });

  it('should interpolate linearly', () => {
    const track = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track.addKeyframe(0, 0, InterpolationType.LINEAR);
    track.addKeyframe(1, 10, InterpolationType.LINEAR);

    expect(track.sample(0)).toBe(0);
    expect(track.sample(0.5)).toBeCloseTo(5);
    expect(track.sample(1)).toBe(10);
  });

  it('should handle step interpolation', () => {
    const track = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track.addKeyframe(0, 0, InterpolationType.STEP);
    track.addKeyframe(1, 10, InterpolationType.STEP);

    expect(track.sample(0.5)).toBe(0); // Should stay at first value
    expect(track.sample(1)).toBe(10);
  });

  it('should animate Vector3', () => {
    const track = new AnimationTrack<Vector3>('position', AnimationPropertyType.VECTOR3);
    track.addKeyframe(0, new Vector3(0, 0, 0));
    track.addKeyframe(1, new Vector3(10, 10, 10));

    const result = track.sample(0.5) as Vector3;
    expect(result.x).toBeCloseTo(5);
    expect(result.y).toBeCloseTo(5);
    expect(result.z).toBeCloseTo(5);
  });

  it('should loop animation', () => {
    const clip = new AnimationClip('loop', 1.0);
    clip.loop = true;

    const track = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track.addKeyframe(0, 0);
    track.addKeyframe(1, 10);
    clip.addTrack(track);

    const sample1 = clip.sample(1.5);
    const sample2 = clip.sample(0.5);
    expect(sample1.get('value')).toBeCloseTo(sample2.get('value')!);
  });
});

describe('AnimationState', () => {
  it('should create animation state', () => {
    const clip = new AnimationClip('idle', 1.0);
    const state = new AnimationState('idle', clip);

    expect(state.name).toBe('idle');
    expect(state.clip).toBe(clip);
  });

  it('should update time when playing', () => {
    const clip = new AnimationClip('test', 1.0);
    const state = new AnimationState('test', clip);

    state.play();
    state.update(0.1);

    expect(state.time).toBeCloseTo(0.1);
  });

  it('should not update when paused', () => {
    const clip = new AnimationClip('test', 1.0);
    const state = new AnimationState('test', clip);

    state.play();
    state.update(0.1);
    state.pause();
    state.update(0.1);

    expect(state.time).toBeCloseTo(0.1);
  });

  it('should reset state', () => {
    const clip = new AnimationClip('test', 1.0);
    const state = new AnimationState('test', clip);

    state.play();
    state.update(0.5);
    state.reset();

    expect(state.time).toBe(0);
  });
});

describe('AnimationStateMachine', () => {
  it('should create state machine', () => {
    const machine = new AnimationStateMachine();
    expect(machine).toBeDefined();
  });

  it('should add and get states', () => {
    const machine = new AnimationStateMachine();
    const clip = new AnimationClip('idle', 1.0);
    const state = new AnimationState('idle', clip);

    machine.addState(state);

    expect(machine.getState('idle')).toBe(state);
    expect(machine.getCurrentState()).toBe(state);
  });

  it('should transition between states', () => {
    const machine = new AnimationStateMachine();

    const clip1 = new AnimationClip('idle', 1.0);
    const state1 = new AnimationState('idle', clip1);
    machine.addState(state1);

    const clip2 = new AnimationClip('walk', 1.0);
    const state2 = new AnimationState('walk', clip2);
    machine.addState(state2);

    machine.transitionTo('walk', 0); // Instant transition

    expect(machine.getCurrentState()).toBe(state2);
  });

  it('should blend during transition', () => {
    const machine = new AnimationStateMachine();

    const clip1 = new AnimationClip('idle', 1.0);
    const track1 = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track1.addKeyframe(0, 0);
    clip1.addTrack(track1);
    const state1 = new AnimationState('idle', clip1);
    machine.addState(state1);

    const clip2 = new AnimationClip('walk', 1.0);
    const track2 = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track2.addKeyframe(0, 10);
    clip2.addTrack(track2);
    const state2 = new AnimationState('walk', clip2);
    machine.addState(state2);

    machine.transitionTo('walk', 1.0); // 1 second transition
    machine.update(0.5); // 50% through transition

    const sample = machine.sample();
    const value = sample.get('value') as number;
    expect(value).toBeGreaterThan(0);
    expect(value).toBeLessThan(10);
  });
});

describe('Animator', () => {
  it('should create animator', () => {
    const animator = new Animator();
    expect(animator).toBeDefined();
  });

  it('should add and play clips', () => {
    const animator = new Animator();
    const clip = new AnimationClip('test', 1.0);

    animator.addClip('test', clip);
    animator.play('test', 0);

    expect(animator.isPlaying()).toBe(true);
  });

  it('should stop animation', () => {
    const animator = new Animator();
    const clip = new AnimationClip('test', 1.0);

    animator.addClip('test', clip);
    animator.play('test', 0);
    animator.stop();

    expect(animator.isPlaying()).toBe(false);
  });

  it('should update animation time', () => {
    const animator = new Animator();
    const clip = new AnimationClip('test', 1.0);

    animator.addClip('test', clip);
    animator.play('test', 0);
    animator.update(0.5);

    expect(animator.getTime()).toBeCloseTo(0.5);
  });
});

describe('AnimationSystem', () => {
  it('should create animation system', () => {
    const system = new AnimationSystem();
    expect(system).toBeDefined();
    expect(system.enabled).toBe(true);
  });

  it('should update animators in world', () => {
    const world = new World();
    const system = new AnimationSystem();

    const entity = new Entity();
    const animator = new Animator();
    const clip = new AnimationClip('test', 1.0);

    animator.addClip('test', clip);
    entity.addComponent(animator);
    world.addEntity(entity);

    animator.play('test', 0);
    system.update(world, 0.1);

    expect(animator.getTime()).toBeCloseTo(0.1);
  });
});

describe('AnimationMixer', () => {
  it('should create mixer', () => {
    const mixer = new AnimationMixer();
    expect(mixer).toBeDefined();
  });

  it('should play animation', () => {
    const mixer = new AnimationMixer();
    const clip = new AnimationClip('test', 1.0);

    mixer.play('test', clip, 0);
    mixer.update(0.5);

    expect(mixer.isPlaying('test')).toBe(true);
    expect(mixer.getTime('test')).toBeCloseTo(0.5);
  });

  it('should stop animation', () => {
    const mixer = new AnimationMixer();
    const clip = new AnimationClip('test', 1.0);

    mixer.play('test', clip, 0);
    mixer.stop('test', 0);

    expect(mixer.isPlaying('test')).toBe(false);
  });

  it('should blend multiple animations', () => {
    const mixer = new AnimationMixer();

    const clip1 = new AnimationClip('anim1', 1.0);
    const track1 = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track1.addKeyframe(0, 0);
    clip1.addTrack(track1);

    const clip2 = new AnimationClip('anim2', 1.0);
    const track2 = new AnimationTrack<number>('value', AnimationPropertyType.NUMBER);
    track2.addKeyframe(0, 10);
    clip2.addTrack(track2);

    mixer.play('anim1', clip1, 0, false, 0.5);
    mixer.play('anim2', clip2, 0, false, 0.5);

    const sample = mixer.sample();
    const value = sample.get('value') as number;

    expect(value).toBeGreaterThan(0);
    expect(value).toBeLessThan(10);
  });

  it('should fade in animation', () => {
    const mixer = new AnimationMixer();
    const clip = new AnimationClip('test', 1.0);

    mixer.play('test', clip, 0.5); // 0.5 second fade in
    mixer.update(0.25); // 50% through fade

    // Weight should be around 0.5
    const sample = mixer.sample();
    expect(sample.size).toBeGreaterThan(0);
  });

  it('should fade out animation', () => {
    const mixer = new AnimationMixer();
    const clip = new AnimationClip('test', 1.0);

    mixer.play('test', clip, 0);
    mixer.stop('test', 0.5); // 0.5 second fade out
    mixer.update(0.5); // Complete fade out

    expect(mixer.getActiveCount()).toBe(0);
  });
});
