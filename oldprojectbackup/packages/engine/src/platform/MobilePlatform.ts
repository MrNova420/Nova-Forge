/**
 * @fileoverview Mobile Platform Implementation
 * @module @nova-engine/platform
 *
 * Mobile platform implementation for Nova Engine.
 * Supports iOS and Android via React Native or Capacitor.
 *
 * Following the autonomous development guide Phase 1.1.4
 */

import type {
  IPlatform,
  IWindow,
  IWorker,
  IFileSystem,
  IWebSocket,
  IInputDevice,
  WindowConfig,
  FileStats,
  FetchOptions,
  Response,
  PlatformCapabilities,
  PlatformInfo,
} from './IPlatform';

/**
 * Mobile Window Implementation
 * Represents the mobile app screen
 */
class MobileWindow implements IWindow {
  private _width: number;
  private _height: number;
  private _title: string;
  private _visible = true;

  constructor(config: WindowConfig) {
    this._width = config.width;
    this._height = config.height;
    this._title = config.title;

    // Mobile doesn't support multiple windows, this represents the app screen
  }

  getWidth(): number {
    // Try to get actual screen dimensions
    const screen = (globalThis as any).screen;
    return screen?.width || this._width;
  }

  getHeight(): number {
    const screen = (globalThis as any).screen;
    return screen?.height || this._height;
  }

  setSize(_width: number, _height: number): void {
    // Mobile screen size is fixed, no-op
  }

  getTitle(): string {
    return this._title;
  }

  setTitle(title: string): void {
    this._title = title;
    // Could update document.title for web views
    if (typeof document !== 'undefined') {
      document.title = title;
    }
  }

  isFullscreen(): boolean {
    // Mobile apps are always fullscreen
    return true;
  }

  setFullscreen(_fullscreen: boolean): void {
    // Mobile apps are always fullscreen, no-op
  }

  getPosition(): { x: number; y: number } {
    // Mobile windows don't have position
    return { x: 0, y: 0 };
  }

  setPosition(_x: number, _y: number): void {
    // Mobile windows don't have position, no-op
  }

  isVisible(): boolean {
    return this._visible;
  }

  setVisible(visible: boolean): void {
    this._visible = visible;
  }

  isFocused(): boolean {
    // Mobile app is focused if visible
    return this._visible;
  }

  focus(): void {
    // No-op on mobile
  }

  close(): void {
    // Closing window on mobile typically exits the app
    this._visible = false;
  }

  getNativeHandle(): any {
    // Return window/screen object
    return globalThis;
  }

  requestAnimationFrame(callback: (time: number) => void): number {
    if ((globalThis as any).requestAnimationFrame) {
      return (globalThis as any).requestAnimationFrame(callback);
    }
    return setTimeout(() => callback(Date.now()), 16) as any;
  }

  cancelAnimationFrame(handle: number): void {
    if ((globalThis as any).cancelAnimationFrame) {
      (globalThis as any).cancelAnimationFrame(handle);
    } else {
      clearTimeout(handle);
    }
  }
}

/**
 * Mobile Worker Implementation
 * Limited threading support on mobile
 */
class MobileWorkerWrapper implements IWorker {
  private worker: any;
  private running = true;

  constructor(scriptPath: string) {
    // Check if Web Workers are available
    const Worker = (globalThis as any).Worker;
    if (Worker) {
      this.worker = new Worker(scriptPath);
    } else {
      this.worker = null;
    }
  }

  postMessage(message: unknown, transfer?: Transferable[]): void {
    if (this.worker?.postMessage) {
      this.worker.postMessage(message, transfer || []);
    }
  }

  onMessage(handler: (message: unknown) => void): void {
    if (this.worker) {
      this.worker.onmessage = (e: any) => handler(e.data);
    }
  }

  onError(handler: (error: Error) => void): void {
    if (this.worker) {
      this.worker.onerror = (e: any) => handler(new Error(e.message));
    }
  }

  terminate(): void {
    if (this.worker?.terminate) {
      this.worker.terminate();
    }
    this.running = false;
  }

  isRunning(): boolean {
    return this.running;
  }
}

/**
 * Mobile File System Implementation
 * Uses React Native FileSystem or Capacitor Filesystem
 */
class MobileFileSystem implements IFileSystem {
  private fsModule: any;

  constructor() {
    // Try to load filesystem module (React Native or Capacitor)
    try {
      // React Native: react-native-fs
      // Capacitor: @capacitor/filesystem
      this.fsModule = null; // Stub for now
    } catch (e) {
      this.fsModule = null;
    }
  }

  async readFile(_path: string): Promise<ArrayBuffer> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
    // Implementation would use React Native FS or Capacitor
    throw new Error('Not implemented: Mobile file system requires native module');
  }

  async readTextFile(_path: string): Promise<string> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
    throw new Error('Not implemented: Mobile file system requires native module');
  }

  async writeFile(_path: string, _data: ArrayBuffer): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
    throw new Error('Not implemented: Mobile file system requires native module');
  }

  async writeTextFile(_path: string, _data: string): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
    throw new Error('Not implemented: Mobile file system requires native module');
  }

  async exists(_path: string): Promise<boolean> {
    if (!this.fsModule) {
      return false;
    }
    return false;
  }

  async listDirectory(_path: string): Promise<string[]> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
    return [];
  }

  async createDirectory(_path: string, _recursive?: boolean): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
  }

  async deleteFile(_path: string): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
  }

  async deleteDirectory(_path: string, _recursive?: boolean): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
  }

  async getStats(_path: string): Promise<FileStats> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
    throw new Error('Not implemented: Mobile file system requires native module');
  }

  async copyFile(_source: string, _destination: string): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
  }

  async moveFile(_source: string, _destination: string): Promise<void> {
    if (!this.fsModule) {
      throw new Error('Mobile file system not available');
    }
  }

  watch(
    _path: string,
    _callback: (event: 'create' | 'modify' | 'delete', path: string) => void
  ): () => void {
    // File watching not typically available on mobile
    return () => {};
  }
}

/**
 * Mobile WebSocket Implementation
 * Uses native WebSocket
 */
class MobileWebSocketWrapper implements IWebSocket {
  private ws: WebSocket;

  constructor(url: string, protocols?: string | string[]) {
    this.ws = new WebSocket(url, protocols);
  }

  send(data: string | ArrayBuffer): void {
    this.ws.send(data);
  }

  close(code?: number, reason?: string): void {
    this.ws.close(code, reason);
  }

  onMessage(handler: (data: string | ArrayBuffer) => void): void {
    this.ws.onmessage = (e) => handler(e.data);
  }

  onOpen(handler: () => void): void {
    this.ws.onopen = handler;
  }

  onClose(handler: (code: number, reason: string) => void): void {
    this.ws.onclose = (e) => handler(e.code, e.reason);
  }

  onError(handler: (error: Error) => void): void {
    this.ws.onerror = () => handler(new Error('WebSocket error'));
  }

  getReadyState(): number {
    return this.ws.readyState;
  }

  getBufferedAmount(): number {
    return this.ws.bufferedAmount;
  }
}

/**
 * Mobile Platform Implementation
 *
 * Platform implementation for mobile devices (iOS/Android).
 * Supports React Native and Capacitor environments.
 * Designed for touch input and mobile-specific constraints.
 */
export class MobilePlatform implements IPlatform {
  private window: MobileWindow | null = null;
  private fileSystem: MobileFileSystem;
  private initialized = false;

  constructor() {
    this.fileSystem = new MobileFileSystem();
  }

  async initialize(_config?: Record<string, unknown>): Promise<void> {
    if (this.initialized) return;
    this.initialized = true;
  }

  async shutdown(): Promise<void> {
    if (this.window) {
      this.window.close();
      this.window = null;
    }
    this.initialized = false;
  }

  getName(): string {
    return 'mobile';
  }

  getInfo(): PlatformInfo {
    const ua = (globalThis as any).navigator?.userAgent || '';
    let os = 'unknown';
    let deviceType: 'desktop' | 'mobile' | 'tablet' | 'unknown' = 'mobile';

    if (ua.includes('Android')) {
      os = 'android';
    } else if (ua.includes('iPhone') || ua.includes('iPad') || ua.includes('iPod')) {
      os = 'ios';
      if (ua.includes('iPad')) {
        deviceType = 'tablet';
      }
    }

    return {
      name: 'mobile',
      os,
      arch: 'arm64', // Most modern mobiles use ARM64
      version: ua,
      deviceType,
      isDevelopment:
        typeof (globalThis as any).process !== 'undefined' &&
        (globalThis as any).process.env?.NODE_ENV === 'development',
      isProduction:
        typeof (globalThis as any).process !== 'undefined' &&
        (globalThis as any).process.env?.NODE_ENV === 'production',
    };
  }

  getCapabilities(): PlatformCapabilities {
    const hasWebGL = typeof (globalThis as any).WebGLRenderingContext !== 'undefined';
    const hasWebGL2 = typeof (globalThis as any).WebGL2RenderingContext !== 'undefined';

    return {
      supportsWorkers: typeof (globalThis as any).Worker !== 'undefined',
      supportsWebGL: hasWebGL,
      supportsWebGL2: hasWebGL2,
      supportsWebGPU: 'gpu' in ((globalThis as any).navigator || {}),
      supportsWebAudio:
        typeof (globalThis as any).AudioContext !== 'undefined' ||
        typeof (globalThis as any).webkitAudioContext !== 'undefined',
      supportsIndexedDB: 'indexedDB' in (globalThis as any),
      supportsLocalStorage: typeof (globalThis as any).Storage !== 'undefined',
      supportsTouch: true, // Mobile always has touch
      supportsGamepad: false, // Typically no gamepad on mobile
      supportsPointerLock: false,
      supportsFullscreen: true,
      maxTextureSize: 4096, // Conservative mobile limit
      maxRenderTargets: 4,
      devicePixelRatio: (globalThis as any).devicePixelRatio || 2.0,
    };
  }

  createWindow(config: WindowConfig): IWindow {
    if (!this.window) {
      this.window = new MobileWindow(config);
    }
    return this.window;
  }

  getWindows(): IWindow[] {
    return this.window ? [this.window] : [];
  }

  getMainWindow(): IWindow | null {
    return this.window;
  }

  getFileSystem(): IFileSystem {
    return this.fileSystem;
  }

  createWorker(scriptPath: string): IWorker {
    return new MobileWorkerWrapper(scriptPath);
  }

  getHardwareConcurrency(): number {
    return (globalThis as any).navigator?.hardwareConcurrency || 2;
  }

  now(): number {
    return (globalThis as any).performance?.now?.() || Date.now();
  }

  sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }

  getInputDevices(): IInputDevice[] {
    const devices: IInputDevice[] = [];

    // Touch input is primary on mobile
    devices.push({
      type: 'touch',
      id: 'touch-0',
      name: 'Touch Screen',
      isConnected: () => true,
      getCapabilities: () => ({
        maxTouchPoints: (globalThis as any).navigator?.maxTouchPoints || 5,
      }),
    });

    return devices;
  }

  async fetch(url: string, options?: FetchOptions): Promise<Response> {
    const fetchFunc = (globalThis as any).fetch;
    if (!fetchFunc) {
      throw new Error('Fetch not available');
    }

    const fetchOptions: RequestInit = {
      method: options?.method || 'GET',
      headers: options?.headers,
      body: options?.body as any,
      credentials: options?.credentials,
      mode: options?.mode,
      cache: options?.cache,
    };

    const response = await fetchFunc(url, fetchOptions);

    return {
      status: response.status,
      statusText: response.statusText,
      headers: Object.fromEntries(response.headers.entries()),
      ok: response.ok,
      arrayBuffer: () => response.arrayBuffer(),
      text: () => response.text(),
      json: () => response.json(),
      blob: () => response.blob(),
    };
  }

  createWebSocket(url: string, protocols?: string | string[]): IWebSocket {
    return new MobileWebSocketWrapper(url, protocols);
  }

  async readClipboard(): Promise<string> {
    const clipboard = (globalThis as any).navigator?.clipboard;
    if (clipboard?.readText) {
      try {
        return await clipboard.readText();
      } catch {
        return '';
      }
    }
    return '';
  }

  async writeClipboard(text: string): Promise<void> {
    const clipboard = (globalThis as any).navigator?.clipboard;
    if (clipboard?.writeText) {
      await clipboard.writeText(text);
    }
  }

  async alert(message: string): Promise<void> {
    if ((globalThis as any).alert) {
      (globalThis as any).alert(message);
    }
  }

  async confirm(message: string): Promise<boolean> {
    if ((globalThis as any).confirm) {
      return (globalThis as any).confirm(message);
    }
    return false;
  }

  async prompt(message: string, defaultValue = ''): Promise<string | null> {
    if ((globalThis as any).prompt) {
      return (globalThis as any).prompt(message, defaultValue);
    }
    return null;
  }

  addEventListener(event: string, handler: (...args: unknown[]) => void): void {
    (globalThis as any).addEventListener?.(event, handler as EventListener);
  }

  removeEventListener(
    event: string,
    handler: (...args: unknown[]) => void
  ): void {
    (globalThis as any).removeEventListener?.(event, handler as EventListener);
  }
}
