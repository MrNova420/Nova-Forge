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
 * AudioSystem
 *
 * Main audio manager using Web Audio API.
 * Manages audio context, sources, and listener.
 */

import { AudioSource } from './AudioSource';
import { AudioListener } from './AudioListener';

/**
 * Audio system manager
 */
export class AudioSystem {
  private _context: AudioContext | null = null;
  private _masterGain: GainNode | null = null;
  private _sources: Set<AudioSource> = new Set();
  private _listener: AudioListener | null = null;
  private _masterVolume: number = 1.0;
  private _muted: boolean = false;

  /**
   * Initialize the audio system
   */
  initialize(): void {
    // Create audio context
    this._context = new (window.AudioContext ||
      (window as any).webkitAudioContext)();

    // Create master gain node
    this._masterGain = this._context.createGain();
    this._masterGain.connect(this._context.destination);
    this._masterGain.gain.value = this._masterVolume;

    // Create listener
    this._listener = new AudioListener(this._context);
  }

  /**
   * Update audio system (call once per frame)
   */
  update(): void {
    if (!this._context || !this._listener) return;

    // Update listener
    this._listener.update();

    // Update all sources
    for (const source of this._sources) {
      source.update();
    }

    // Clean up stopped sources
    const stoppedSources: AudioSource[] = [];
    for (const source of this._sources) {
      if (!source.isPlaying && !source.isPaused) {
        stoppedSources.push(source);
      }
    }
    for (const source of stoppedSources) {
      this._sources.delete(source);
    }
  }

  /**
   * Suspend audio context (save resources)
   */
  async suspend(): Promise<void> {
    if (this._context && this._context.state === 'running') {
      await this._context.suspend();
    }
  }

  /**
   * Resume audio context
   */
  async resume(): Promise<void> {
    if (this._context && this._context.state === 'suspended') {
      await this._context.resume();
    }
  }

  /**
   * Close audio context and cleanup
   */
  async destroy(): Promise<void> {
    // Stop all sources
    for (const source of this._sources) {
      source.stop();
    }
    this._sources.clear();

    // Close context
    if (this._context) {
      await this._context.close();
      this._context = null;
    }

    this._masterGain = null;
    this._listener = null;
  }

  /**
   * Create an audio source
   * @param buffer - Audio buffer to play
   * @returns New audio source
   */
  createSource(buffer: AudioBuffer): AudioSource {
    if (!this._context || !this._masterGain) {
      throw new Error('Audio system not initialized');
    }

    const source = new AudioSource(this._context, buffer, this._masterGain);
    this._sources.add(source);
    return source;
  }

  /**
   * Get the audio listener
   */
  get listener(): AudioListener | null {
    return this._listener;
  }

  /**
   * Get the audio context
   */
  get context(): AudioContext | null {
    return this._context;
  }

  /**
   * Get master gain node
   */
  get masterGain(): GainNode | null {
    return this._masterGain;
  }

  /**
   * Set master volume
   * @param volume - Volume level (0-1)
   */
  setMasterVolume(volume: number): void {
    this._masterVolume = Math.max(0, Math.min(1, volume));
    if (this._masterGain && !this._muted) {
      this._masterGain.gain.value = this._masterVolume;
    }
  }

  /**
   * Get master volume
   */
  get masterVolume(): number {
    return this._masterVolume;
  }

  /**
   * Mute all audio
   */
  mute(): void {
    this._muted = true;
    if (this._masterGain) {
      this._masterGain.gain.value = 0;
    }
  }

  /**
   * Unmute audio
   */
  unmute(): void {
    this._muted = false;
    if (this._masterGain) {
      this._masterGain.gain.value = this._masterVolume;
    }
  }

  /**
   * Check if audio is muted
   */
  get isMuted(): boolean {
    return this._muted;
  }

  /**
   * Get number of active sources
   */
  get activeSourceCount(): number {
    return this._sources.size;
  }

  /**
   * Get all active sources
   */
  getSources(): AudioSource[] {
    return Array.from(this._sources);
  }
}
