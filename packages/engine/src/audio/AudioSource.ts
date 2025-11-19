/**
 * AudioSource
 *
 * Represents an audio source that can play sounds in 3D space.
 */

import { Vector3 } from '../math/Vector3';

/**
 * Audio source for playing sounds
 */
export class AudioSource {
  private _context: AudioContext;
  private _buffer: AudioBuffer;
  private _destination: AudioNode;

  private _sourceNode: AudioBufferSourceNode | null = null;
  private _gainNode: GainNode;
  private _pannerNode: PannerNode | null = null;

  private _position: Vector3 = new Vector3(0, 0, 0);
  private _velocity: Vector3 = new Vector3(0, 0, 0);

  private _volume: number = 1.0;
  private _pitch: number = 1.0;
  private _loop: boolean = false;
  private _spatial: boolean = false;

  private _isPlaying: boolean = false;
  private _isPaused: boolean = false;
  private _startTime: number = 0;
  private _pauseTime: number = 0;

  /**
   * Create an audio source
   * @param context - Audio context
   * @param buffer - Audio buffer
   * @param destination - Destination node
   */
  constructor(
    context: AudioContext,
    buffer: AudioBuffer,
    destination: AudioNode
  ) {
    this._context = context;
    this._buffer = buffer;
    this._destination = destination;

    // Create gain node for volume control
    this._gainNode = context.createGain();
    this._gainNode.gain.value = this._volume;
  }

  /**
   * Play the audio
   */
  play(): void {
    if (this._isPlaying && !this._isPaused) return;

    if (this._isPaused) {
      // Resume from pause
      this._context.resume();
      this._isPlaying = true;
      this._isPaused = false;
      return;
    }

    // Create new source node
    this._sourceNode = this._context.createBufferSource();
    this._sourceNode.buffer = this._buffer;
    this._sourceNode.loop = this._loop;
    this._sourceNode.playbackRate.value = this._pitch;

    // Set up audio graph
    if (this._spatial) {
      if (!this._pannerNode) {
        this._pannerNode = this._context.createPanner();
        this._pannerNode.panningModel = 'HRTF';
        this._pannerNode.distanceModel = 'inverse';
        this._pannerNode.refDistance = 1;
        this._pannerNode.maxDistance = 1000000; // Massive audio distance for open worlds
        this._pannerNode.rolloffFactor = 1;
      }

      this._sourceNode.connect(this._gainNode);
      this._gainNode.connect(this._pannerNode);
      this._pannerNode.connect(this._destination);
    } else {
      this._sourceNode.connect(this._gainNode);
      this._gainNode.connect(this._destination);
    }

    // Handle end of playback
    this._sourceNode.onended = () => {
      if (!this._loop) {
        this._isPlaying = false;
      }
    };

    // Start playback
    this._sourceNode.start(0);
    this._startTime = this._context.currentTime;
    this._isPlaying = true;
    this._isPaused = false;
  }

  /**
   * Pause the audio
   */
  pause(): void {
    if (!this._isPlaying || this._isPaused) return;

    this._context.suspend();
    this._pauseTime = this._context.currentTime;
    this._isPaused = true;
  }

  /**
   * Stop the audio
   */
  stop(): void {
    if (!this._isPlaying) return;

    if (this._sourceNode) {
      try {
        this._sourceNode.stop();
      } catch (_e) {
        // Source may have already stopped
      }
      this._sourceNode = null;
    }

    this._isPlaying = false;
    this._isPaused = false;
  }

  /**
   * Update audio source (call once per frame)
   */
  update(): void {
    if (!this._spatial || !this._pannerNode) return;

    // Update position
    this._pannerNode.positionX.value = this._position.x;
    this._pannerNode.positionY.value = this._position.y;
    this._pannerNode.positionZ.value = this._position.z;

    // Update velocity (for doppler effect) - legacy API removed in modern browsers
    // Note: Velocity/Doppler effect is deprecated in Web Audio API
  }

  /**
   * Set position in 3D space
   * @param position - Position vector
   */
  setPosition(position: Vector3): void {
    this._position.copy(position);
  }

  /**
   * Set velocity for doppler effect
   * @param velocity - Velocity vector
   */
  setVelocity(velocity: Vector3): void {
    this._velocity.copy(velocity);
  }

  /**
   * Set volume
   * @param volume - Volume level (0-1)
   */
  setVolume(volume: number): void {
    this._volume = Math.max(0, Math.min(1, volume));
    this._gainNode.gain.value = this._volume;
  }

  /**
   * Set pitch/playback rate
   * @param pitch - Pitch multiplier (0.5 = half speed, 2 = double speed)
   */
  setPitch(pitch: number): void {
    this._pitch = Math.max(0.01, pitch);
    if (this._sourceNode) {
      this._sourceNode.playbackRate.value = this._pitch;
    }
  }

  /**
   * Set loop mode
   * @param loop - Whether to loop
   */
  setLoop(loop: boolean): void {
    this._loop = loop;
    if (this._sourceNode) {
      this._sourceNode.loop = loop;
    }
  }

  /**
   * Enable/disable spatial audio
   * @param spatial - Whether to use 3D positioning
   */
  setSpatial(spatial: boolean): void {
    this._spatial = spatial;

    // If changing spatial mode while playing, need to restart
    if (this._isPlaying) {
      const wasPlaying = this._isPlaying;
      this.stop();
      if (wasPlaying) {
        this.play();
      }
    }
  }

  /**
   * Get current position
   */
  get position(): Vector3 {
    return this._position.clone();
  }

  /**
   * Get current velocity
   */
  get velocity(): Vector3 {
    return this._velocity.clone();
  }

  /**
   * Get volume
   */
  get volume(): number {
    return this._volume;
  }

  /**
   * Get pitch
   */
  get pitch(): number {
    return this._pitch;
  }

  /**
   * Check if looping
   */
  get loop(): boolean {
    return this._loop;
  }

  /**
   * Check if spatial
   */
  get spatial(): boolean {
    return this._spatial;
  }

  /**
   * Check if playing
   */
  get isPlaying(): boolean {
    return this._isPlaying;
  }

  /**
   * Check if paused
   */
  get isPaused(): boolean {
    return this._isPaused;
  }

  /**
   * Get audio buffer duration
   */
  get duration(): number {
    return this._buffer.duration;
  }

  /**
   * Get current playback time
   */
  get currentTime(): number {
    if (!this._isPlaying) return 0;
    if (this._isPaused) return this._pauseTime - this._startTime;
    return this._context.currentTime - this._startTime;
  }
}
