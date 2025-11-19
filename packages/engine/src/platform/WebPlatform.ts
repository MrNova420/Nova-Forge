/**
 * @fileoverview Web Platform Implementation
 * @module @nova-engine/platform
 *
 * Browser-based platform implementation for Nova Engine.
 * Implements IPlatform interface using Web APIs.
 *
 * Following the autonomous development guide Phase 1.1.2
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
 * Web Window Implementation
 * Wraps browser window/canvas for Nova Engine
 */
class WebWindow implements IWindow {
  private element: HTMLCanvasElement;
  private isFullscreenFlag = false;
  private animationFrameId: number | null = null;

  constructor(config: WindowConfig) {
    this.element = document.createElement('canvas');
    this.element.width = config.width;
    this.element.height = config.height;
    this.element.style.width = `${config.width}px`;
    this.element.style.height = `${config.height}px`;

    // Add to DOM
    document.body.appendChild(this.element);
    document.title = config.title;

    // Handle fullscreen changes
    document.addEventListener('fullscreenchange', () => {
      this.isFullscreenFlag = !!document.fullscreenElement;
    });
  }

  getWidth(): number {
    return this.element.width;
  }

  getHeight(): number {
    return this.element.height;
  }

  setSize(width: number, height: number): void {
    this.element.width = width;
    this.element.height = height;
    this.element.style.width = `${width}px`;
    this.element.style.height = `${height}px`;
  }

  getTitle(): string {
    return document.title;
  }

  setTitle(title: string): void {
    document.title = title;
  }

  isFullscreen(): boolean {
    return this.isFullscreenFlag;
  }

  setFullscreen(fullscreen: boolean): void {
    if (fullscreen && !this.isFullscreenFlag) {
      this.element.requestFullscreen().catch((err) => {
        console.error('Failed to enter fullscreen:', err);
      });
    } else if (!fullscreen && this.isFullscreenFlag) {
      document.exitFullscreen().catch((err) => {
        console.error('Failed to exit fullscreen:', err);
      });
    }
  }

  getPosition(): { x: number; y: number } {
    const rect = this.element.getBoundingClientRect();
    return { x: rect.left, y: rect.top };
  }

  setPosition(x: number, y: number): void {
    this.element.style.position = 'absolute';
    this.element.style.left = `${x}px`;
    this.element.style.top = `${y}px`;
  }

  isVisible(): boolean {
    return this.element.style.display !== 'none';
  }

  setVisible(visible: boolean): void {
    this.element.style.display = visible ? 'block' : 'none';
  }

  isFocused(): boolean {
    return document.activeElement === this.element;
  }

  focus(): void {
    this.element.focus();
  }

  close(): void {
    if (this.animationFrameId !== null) {
      window.cancelAnimationFrame(this.animationFrameId);
    }
    document.body.removeChild(this.element);
  }

  getNativeHandle(): HTMLCanvasElement {
    return this.element;
  }

  requestAnimationFrame(callback: (time: number) => void): number {
    return window.requestAnimationFrame(callback);
  }

  cancelAnimationFrame(handle: number): void {
    window.cancelAnimationFrame(handle);
  }
}

/**
 * Web Worker Implementation
 */
class WebWorkerWrapper implements IWorker {
  private worker: Worker;
  private running = true;

  constructor(scriptPath: string) {
    this.worker = new Worker(scriptPath);
  }

  postMessage(message: unknown, transfer?: Transferable[]): void {
    this.worker.postMessage(message, transfer || []);
  }

  onMessage(handler: (message: unknown) => void): void {
    this.worker.onmessage = (e) => handler(e.data);
  }

  onError(handler: (error: Error) => void): void {
    this.worker.onerror = (e) => handler(new Error(e.message));
  }

  terminate(): void {
    this.worker.terminate();
    this.running = false;
  }

  isRunning(): boolean {
    return this.running;
  }
}

/**
 * Web File System Implementation
 * Uses IndexedDB for storage
 */
class WebFileSystem implements IFileSystem {
  private dbName = 'NovaEngineFS';
  private db: IDBDatabase | null = null;

  async initialize(): Promise<void> {
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(this.dbName, 1);

      request.onerror = () => reject(request.error);
      request.onsuccess = () => {
        this.db = request.result;
        resolve();
      };

      request.onupgradeneeded = (event) => {
        const db = (event.target as IDBOpenDBRequest).result;
        if (!db.objectStoreNames.contains('files')) {
          db.createObjectStore('files', { keyPath: 'path' });
        }
      };
    });
  }

  async readFile(path: string): Promise<ArrayBuffer> {
    if (!this.db) await this.initialize();

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction(['files'], 'readonly');
      const store = transaction.objectStore('files');
      const request = store.get(path);

      request.onsuccess = () => {
        if (request.result) {
          resolve(request.result.data);
        } else {
          reject(new Error(`File not found: ${path}`));
        }
      };

      request.onerror = () => reject(request.error);
    });
  }

  async readTextFile(path: string): Promise<string> {
    const buffer = await this.readFile(path);
    const decoder = new TextDecoder();
    return decoder.decode(buffer);
  }

  async writeFile(path: string, data: ArrayBuffer): Promise<void> {
    if (!this.db) await this.initialize();

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction(['files'], 'readwrite');
      const store = transaction.objectStore('files');
      const request = store.put({
        path,
        data,
        modified: Date.now(),
        created: Date.now(),
      });

      request.onsuccess = () => resolve();
      request.onerror = () => reject(request.error);
    });
  }

  async writeTextFile(path: string, data: string): Promise<void> {
    const encoder = new TextEncoder();
    const encoded = encoder.encode(data);
    await this.writeFile(path, encoded.buffer);
  }

  async exists(path: string): Promise<boolean> {
    if (!this.db) await this.initialize();

    return new Promise((resolve) => {
      const transaction = this.db!.transaction(['files'], 'readonly');
      const store = transaction.objectStore('files');
      const request = store.get(path);

      request.onsuccess = () => resolve(!!request.result);
      request.onerror = () => resolve(false);
    });
  }

  async listDirectory(path: string): Promise<string[]> {
    if (!this.db) await this.initialize();

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction(['files'], 'readonly');
      const store = transaction.objectStore('files');
      const request = store.getAllKeys();

      request.onsuccess = () => {
        const prefix = path.endsWith('/') ? path : `${path}/`;
        const files = (request.result as string[]).filter((key) =>
          key.startsWith(prefix)
        );
        resolve(files);
      };

      request.onerror = () => reject(request.error);
    });
  }

  async createDirectory(_path: string, _recursive?: boolean): Promise<void> {
    // No-op for web (directories are implicit in paths)
  }

  async deleteFile(path: string): Promise<void> {
    if (!this.db) await this.initialize();

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction(['files'], 'readwrite');
      const store = transaction.objectStore('files');
      const request = store.delete(path);

      request.onsuccess = () => resolve();
      request.onerror = () => reject(request.error);
    });
  }

  async deleteDirectory(path: string, recursive?: boolean): Promise<void> {
    const files = await this.listDirectory(path);
    if (recursive) {
      await Promise.all(files.map((file) => this.deleteFile(file)));
    }
  }

  async getStats(path: string): Promise<FileStats> {
    if (!this.db) await this.initialize();

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction(['files'], 'readonly');
      const store = transaction.objectStore('files');
      const request = store.get(path);

      request.onsuccess = () => {
        if (request.result) {
          const data = request.result;
          resolve({
            size: data.data.byteLength,
            modified: data.modified,
            created: data.created,
            isDirectory: false,
            isFile: true,
          });
        } else {
          reject(new Error(`File not found: ${path}`));
        }
      };

      request.onerror = () => reject(request.error);
    });
  }

  async copyFile(source: string, destination: string): Promise<void> {
    const data = await this.readFile(source);
    await this.writeFile(destination, data);
  }

  async moveFile(source: string, destination: string): Promise<void> {
    await this.copyFile(source, destination);
    await this.deleteFile(source);
  }

  watch(
    _path: string,
    _callback: (event: 'create' | 'modify' | 'delete', path: string) => void
  ): () => void {
    // File watching not supported in web environment
    return () => {};
  }
}

/**
 * Web WebSocket Implementation
 */
class WebWebSocketWrapper implements IWebSocket {
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
 * Web Platform Implementation
 *
 * Primary platform implementation for browser-based Nova Engine.
 * Uses standard Web APIs: Canvas, Web Workers, IndexedDB, Fetch, WebSocket.
 */
export class WebPlatform implements IPlatform {
  private windows: WebWindow[] = [];
  private fileSystem: WebFileSystem;
  private initialized = false;

  constructor() {
    this.fileSystem = new WebFileSystem();
  }

  async initialize(_config?: Record<string, unknown>): Promise<void> {
    if (this.initialized) return;

    await this.fileSystem.initialize();
    this.initialized = true;
  }

  async shutdown(): Promise<void> {
    // Close all windows
    this.windows.forEach((window) => window.close());
    this.windows = [];
    this.initialized = false;
  }

  getName(): string {
    return 'web';
  }

  getInfo(): PlatformInfo {
    const ua = navigator.userAgent;
    let os = 'unknown';
    if (ua.includes('Win')) os = 'windows';
    else if (ua.includes('Mac')) os = 'macos';
    else if (ua.includes('Linux')) os = 'linux';
    else if (ua.includes('Android')) os = 'android';
    else if (ua.includes('iOS') || ua.includes('iPhone') || ua.includes('iPad'))
      os = 'ios';

    const isMobile =
      /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(
        ua
      );

    return {
      name: 'web',
      os,
      arch: 'wasm',
      version: navigator.userAgent,
      deviceType: isMobile ? 'mobile' : 'desktop',
      isDevelopment:
        typeof (globalThis as any).process !== 'undefined' &&
        (globalThis as any).process.env?.NODE_ENV === 'development',
      isProduction:
        typeof (globalThis as any).process !== 'undefined' &&
        (globalThis as any).process.env?.NODE_ENV === 'production',
    };
  }

  getCapabilities(): PlatformCapabilities {
    const canvas = document.createElement('canvas');
    const gl2 = canvas.getContext('webgl2') as WebGL2RenderingContext | null;
    const gl1 = canvas.getContext('webgl') as WebGLRenderingContext | null;

    let maxTextureSize = 2048;
    let maxRenderTargets = 1;

    if (gl2) {
      maxTextureSize = gl2.getParameter(gl2.MAX_TEXTURE_SIZE) as number;
      maxRenderTargets = gl2.getParameter(gl2.MAX_DRAW_BUFFERS) as number;
    } else if (gl1) {
      maxTextureSize = gl1.getParameter(gl1.MAX_TEXTURE_SIZE) as number;
    }

    return {
      supportsWorkers: typeof Worker !== 'undefined',
      supportsWebGL: !!gl1,
      supportsWebGL2: !!gl2,
      supportsWebGPU: 'gpu' in navigator,
      supportsWebAudio:
        typeof AudioContext !== 'undefined' ||
        typeof (window as any).webkitAudioContext !== 'undefined',
      supportsIndexedDB: 'indexedDB' in window,
      supportsLocalStorage: typeof Storage !== 'undefined',
      supportsTouch: 'ontouchstart' in window,
      supportsGamepad: 'getGamepads' in navigator,
      supportsPointerLock: 'pointerLockElement' in document,
      supportsFullscreen: 'fullscreenEnabled' in document,
      maxTextureSize,
      maxRenderTargets,
      devicePixelRatio: window.devicePixelRatio || 1,
    };
  }

  createWindow(config: WindowConfig): IWindow {
    const window = new WebWindow(config);
    this.windows.push(window);
    return window;
  }

  getWindows(): IWindow[] {
    return this.windows;
  }

  getMainWindow(): IWindow | null {
    return this.windows[0] || null;
  }

  getFileSystem(): IFileSystem {
    return this.fileSystem;
  }

  createWorker(scriptPath: string): IWorker {
    return new WebWorkerWrapper(scriptPath);
  }

  getHardwareConcurrency(): number {
    return navigator.hardwareConcurrency || 4;
  }

  now(): number {
    return performance.now();
  }

  sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }

  getInputDevices(): IInputDevice[] {
    const devices: IInputDevice[] = [];

    // Keyboard
    devices.push({
      type: 'keyboard',
      id: 'keyboard-0',
      name: 'Keyboard',
      isConnected: () => true,
      getCapabilities: () => ({}),
    });

    // Mouse
    devices.push({
      type: 'mouse',
      id: 'mouse-0',
      name: 'Mouse',
      isConnected: () => true,
      getCapabilities: () => ({}),
    });

    // Touch (if supported)
    if ('ontouchstart' in window) {
      devices.push({
        type: 'touch',
        id: 'touch-0',
        name: 'Touch Screen',
        isConnected: () => true,
        getCapabilities: () => ({}),
      });
    }

    // Gamepads
    if ('getGamepads' in navigator) {
      const gamepads = navigator.getGamepads();
      gamepads.forEach((gamepad, index) => {
        if (gamepad) {
          devices.push({
            type: 'gamepad',
            id: `gamepad-${index}`,
            name: gamepad.id,
            isConnected: () => gamepad.connected,
            getCapabilities: () => ({
              buttons: gamepad.buttons.length,
              axes: gamepad.axes.length,
            }),
          });
        }
      });
    }

    return devices;
  }

  async fetch(url: string, options?: FetchOptions): Promise<Response> {
    const fetchOptions: RequestInit = {
      method: options?.method || 'GET',
      headers: options?.headers,
      body: options?.body,
      credentials: options?.credentials,
      mode: options?.mode,
      cache: options?.cache,
    };

    const response = await fetch(url, fetchOptions);

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
    return new WebWebSocketWrapper(url, protocols);
  }

  async readClipboard(): Promise<string> {
    if (navigator.clipboard && navigator.clipboard.readText) {
      return navigator.clipboard.readText();
    }
    return '';
  }

  async writeClipboard(text: string): Promise<void> {
    if (navigator.clipboard && navigator.clipboard.writeText) {
      await navigator.clipboard.writeText(text);
    }
  }

  async alert(message: string): Promise<void> {
    window.alert(message);
  }

  async confirm(message: string): Promise<boolean> {
    return window.confirm(message);
  }

  async prompt(message: string, defaultValue?: string): Promise<string | null> {
    return window.prompt(message, defaultValue);
  }

  addEventListener(event: string, handler: (...args: unknown[]) => void): void {
    window.addEventListener(event, handler as EventListener);
  }

  removeEventListener(
    event: string,
    handler: (...args: unknown[]) => void
  ): void {
    window.removeEventListener(event, handler as EventListener);
  }
}
