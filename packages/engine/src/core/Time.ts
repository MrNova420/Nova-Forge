/**
 * Manages time-related information for the engine
 */
export class Time {
  private _deltaTime: number = 0;
  private _totalTime: number = 0;
  private _frameCount: number = 0;
  private _lastFrameTime: number = 0;
  private _timeScale: number = 1.0;
  private _maxDeltaTime: number = 0.1; // 100ms max to prevent spiral of death

  /**
   * Time elapsed since the last frame in seconds
   */
  get deltaTime(): number {
    return this._deltaTime * this._timeScale;
  }

  /**
   * Unscaled time elapsed since the last frame in seconds
   */
  get unscaledDeltaTime(): number {
    return this._deltaTime;
  }

  /**
   * Total time elapsed since engine start in seconds
   */
  get totalTime(): number {
    return this._totalTime;
  }

  /**
   * Total number of frames rendered
   */
  get frameCount(): number {
    return this._frameCount;
  }

  /**
   * Current frames per second (calculated)
   */
  get fps(): number {
    return this._deltaTime > 0 ? 1 / this._deltaTime : 0;
  }

  /**
   * Time scale factor (1.0 = normal speed, 0.5 = half speed, etc.)
   */
  get timeScale(): number {
    return this._timeScale;
  }

  set timeScale(value: number) {
    this._timeScale = Math.max(0, value);
  }

  /**
   * Maximum allowed delta time to prevent large jumps
   */
  get maxDeltaTime(): number {
    return this._maxDeltaTime;
  }

  set maxDeltaTime(value: number) {
    this._maxDeltaTime = Math.max(0.001, value);
  }

  /**
   * Updates time information
   * @param currentTime - Current time in milliseconds
   * @internal
   */
  update(currentTime: number): void {
    if (this._lastFrameTime === 0) {
      this._lastFrameTime = currentTime;
      this._deltaTime = 0;
      return;
    }

    // Calculate delta time in seconds
    const rawDelta = (currentTime - this._lastFrameTime) / 1000;
    this._deltaTime = Math.min(rawDelta, this._maxDeltaTime);

    // Update total time and frame count
    this._totalTime += this._deltaTime * this._timeScale;
    this._frameCount++;
    this._lastFrameTime = currentTime;
  }

  /**
   * Resets time information
   * @internal
   */
  reset(): void {
    this._deltaTime = 0;
    this._totalTime = 0;
    this._frameCount = 0;
    this._lastFrameTime = 0;
  }
}
