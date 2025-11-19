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
 * AudioEffects
 *
 * Collection of audio effect processors.
 */

/**
 * Reverb effect
 */
export class ReverbEffect {
  private _convolver: ConvolverNode;
  private _wetGain: GainNode;
  private _dryGain: GainNode;
  private _mix: number = 0.5;

  /**
   * Create a reverb effect
   * @param context - Audio context
   */
  constructor(context: AudioContext) {
    // Create nodes
    this._convolver = context.createConvolver();
    this._wetGain = context.createGain();
    this._dryGain = context.createGain();

    // Set initial mix
    this.setMix(this._mix);
  }

  /**
   * Load an impulse response
   * @param buffer - Impulse response buffer
   */
  loadImpulse(buffer: AudioBuffer): void {
    this._convolver.buffer = buffer;
  }

  /**
   * Connect the effect
   * @param input - Input node
   * @param output - Output node
   */
  connect(input: AudioNode, output: AudioNode): void {
    // Dry path (direct)
    input.connect(this._dryGain);
    this._dryGain.connect(output);

    // Wet path (processed)
    input.connect(this._convolver);
    this._convolver.connect(this._wetGain);
    this._wetGain.connect(output);
  }

  /**
   * Set wet/dry mix
   * @param mix - Mix amount (0 = dry, 1 = wet)
   */
  setMix(mix: number): void {
    this._mix = Math.max(0, Math.min(1, mix));
    this._wetGain.gain.value = this._mix;
    this._dryGain.gain.value = 1 - this._mix;
  }

  /**
   * Get convolver node
   */
  get convolver(): ConvolverNode {
    return this._convolver;
  }
}

/**
 * Delay effect
 */
export class DelayEffect {
  private _delay: DelayNode;
  private _feedback: GainNode;
  private _wetGain: GainNode;
  private _dryGain: GainNode;

  /**
   * Create a delay effect
   * @param context - Audio context
   * @param maxDelay - Maximum delay time in seconds
   */
  constructor(context: AudioContext, maxDelay: number = 1.0) {
    // Create nodes
    this._delay = context.createDelay(maxDelay);
    this._feedback = context.createGain();
    this._wetGain = context.createGain();
    this._dryGain = context.createGain();

    // Connect feedback loop
    this._delay.connect(this._feedback);
    this._feedback.connect(this._delay);

    // Set initial values
    this.setDelayTime(0.3);
    this.setFeedback(0.5);
    this.setMix(0.5);
  }

  /**
   * Connect the effect
   * @param input - Input node
   * @param output - Output node
   */
  connect(input: AudioNode, output: AudioNode): void {
    // Dry path
    input.connect(this._dryGain);
    this._dryGain.connect(output);

    // Wet path
    input.connect(this._delay);
    this._delay.connect(this._wetGain);
    this._wetGain.connect(output);
  }

  /**
   * Set delay time
   * @param time - Delay time in seconds
   */
  setDelayTime(time: number): void {
    this._delay.delayTime.value = time;
  }

  /**
   * Set feedback amount
   * @param feedback - Feedback amount (0-1)
   */
  setFeedback(feedback: number): void {
    this._feedback.gain.value = Math.max(0, Math.min(0.99, feedback));
  }

  /**
   * Set wet/dry mix
   * @param mix - Mix amount (0 = dry, 1 = wet)
   */
  setMix(mix: number): void {
    const mixValue = Math.max(0, Math.min(1, mix));
    this._wetGain.gain.value = mixValue;
    this._dryGain.gain.value = 1 - mixValue;
  }
}

/**
 * Filter effect
 */
export class FilterEffect {
  private _filter: BiquadFilterNode;

  /**
   * Create a filter effect
   * @param context - Audio context
   * @param type - Filter type
   */
  constructor(context: AudioContext, type: BiquadFilterType = 'lowpass') {
    this._filter = context.createBiquadFilter();
    this._filter.type = type;
  }

  /**
   * Connect the effect
   * @param input - Input node
   * @param output - Output node
   */
  connect(input: AudioNode, output: AudioNode): void {
    input.connect(this._filter);
    this._filter.connect(output);
  }

  /**
   * Set filter type
   * @param type - Filter type
   */
  setType(type: BiquadFilterType): void {
    this._filter.type = type;
  }

  /**
   * Set frequency
   * @param frequency - Frequency in Hz
   */
  setFrequency(frequency: number): void {
    this._filter.frequency.value = frequency;
  }

  /**
   * Set Q factor (resonance)
   * @param q - Q factor
   */
  setQ(q: number): void {
    this._filter.Q.value = q;
  }

  /**
   * Set gain (for peaking and shelving filters)
   * @param gain - Gain in dB
   */
  setGain(gain: number): void {
    this._filter.gain.value = gain;
  }

  /**
   * Get filter node
   */
  get filter(): BiquadFilterNode {
    return this._filter;
  }
}

/**
 * Compressor effect
 */
export class CompressorEffect {
  private _compressor: DynamicsCompressorNode;

  /**
   * Create a compressor effect
   * @param context - Audio context
   */
  constructor(context: AudioContext) {
    this._compressor = context.createDynamicsCompressor();

    // Set default values
    this.setThreshold(-24);
    this.setKnee(30);
    this.setRatio(12);
    this.setAttack(0.003);
    this.setRelease(0.25);
  }

  /**
   * Connect the effect
   * @param input - Input node
   * @param output - Output node
   */
  connect(input: AudioNode, output: AudioNode): void {
    input.connect(this._compressor);
    this._compressor.connect(output);
  }

  /**
   * Set threshold
   * @param threshold - Threshold in dB
   */
  setThreshold(threshold: number): void {
    this._compressor.threshold.value = threshold;
  }

  /**
   * Set knee
   * @param knee - Knee in dB
   */
  setKnee(knee: number): void {
    this._compressor.knee.value = knee;
  }

  /**
   * Set ratio
   * @param ratio - Compression ratio
   */
  setRatio(ratio: number): void {
    this._compressor.ratio.value = ratio;
  }

  /**
   * Set attack time
   * @param attack - Attack time in seconds
   */
  setAttack(attack: number): void {
    this._compressor.attack.value = attack;
  }

  /**
   * Set release time
   * @param release - Release time in seconds
   */
  setRelease(release: number): void {
    this._compressor.release.value = release;
  }

  /**
   * Get compressor node
   */
  get compressor(): DynamicsCompressorNode {
    return this._compressor;
  }

  /**
   * Get current reduction amount
   */
  get reduction(): number {
    return this._compressor.reduction;
  }
}
