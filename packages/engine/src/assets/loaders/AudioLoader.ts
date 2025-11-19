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
 * AudioLoader
 *
 * Loader for audio files using the Web Audio API.
 * Loads and decodes audio data into AudioBuffer objects.
 */

import { AssetLoader } from '../AssetLoader';

/**
 * Audio data container
 */
export interface AudioData {
  buffer: AudioBuffer;
  url: string;
  duration: number;
  sampleRate: number;
  numberOfChannels: number;
}

/**
 * Audio loader for loading audio files
 */
export class AudioLoader implements AssetLoader<AudioData> {
  private _audioContext: AudioContext | null = null;

  /**
   * Get or create the audio context
   */
  private getAudioContext(): AudioContext {
    if (!this._audioContext) {
      this._audioContext = new (window.AudioContext ||
        (window as any).webkitAudioContext)();
    }
    return this._audioContext;
  }

  /**
   * Load an audio file and decode it
   * @param path - Audio file URL
   * @returns Promise that resolves to decoded audio data
   */
  async load(path: string): Promise<AudioData> {
    try {
      // Fetch the audio file
      const response = await fetch(path);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      // Get array buffer
      const arrayBuffer = await response.arrayBuffer();

      // Decode audio data
      const audioContext = this.getAudioContext();
      const buffer = await audioContext.decodeAudioData(arrayBuffer);

      return {
        buffer,
        url: path,
        duration: buffer.duration,
        sampleRate: buffer.sampleRate,
        numberOfChannels: buffer.numberOfChannels,
      };
    } catch (error) {
      throw new Error(`Failed to load audio from ${path}: ${error}`);
    }
  }

  /**
   * Check if this loader supports the given file
   * @param path - File path
   * @returns True if supported
   */
  supports(path: string): boolean {
    const ext = path.toLowerCase();
    return (
      ext.endsWith('.mp3') ||
      ext.endsWith('.wav') ||
      ext.endsWith('.ogg') ||
      ext.endsWith('.m4a') ||
      ext.endsWith('.aac') ||
      ext.endsWith('.flac') ||
      ext.endsWith('.webm')
    );
  }

  /**
   * Suspend the audio context to save resources
   */
  suspend(): void {
    if (this._audioContext && this._audioContext.state === 'running') {
      this._audioContext.suspend();
    }
  }

  /**
   * Resume the audio context
   */
  resume(): void {
    if (this._audioContext && this._audioContext.state === 'suspended') {
      this._audioContext.resume();
    }
  }

  /**
   * Close the audio context and release resources
   */
  close(): void {
    if (this._audioContext) {
      this._audioContext.close();
      this._audioContext = null;
    }
  }

  /**
   * Get the current audio context
   */
  get audioContext(): AudioContext | null {
    return this._audioContext;
  }
}
