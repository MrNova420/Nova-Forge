/**
 * @fileoverview Desktop Platform Implementation
 * @module @nova-engine/platform
 *
 * Native desktop platform implementation for Nova Engine.
 * Supports Electron, Tauri, and Node.js environments.
 *
 * Following the autonomous development guide Phase 1.1.3
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
 * Desktop Window Implementation
 * Uses Electron BrowserWindow or similar native window
 */
class DesktopWindow implements IWindow {
  private config: WindowConfig;
  private nativeWindow: any; // Electron BrowserWindow or Tauri window
  private _width: number;
  private _height: number;
  private _title: string;
  private _fullscreen: boolean;

  constructor(config: WindowConfig) {
    this.config = config;
    this._width = config.width;
    this._height = config.height;
    this._title = config.title;
    this._fullscreen = config.fullscreen || false;

    // Note: Actual window creation would require Electron/Tauri
    // This is a stub implementation for compilation
    this.nativeWindow = null;
  }

  getWidth(): number {
    return this._width;
  }

  getHeight(): number {
    return this._height;
  }

  setSize(width: number, height: number): void {
    this._width = width;
    this._height = height;
    if (this.nativeWindow?.setSize) {
      this.nativeWindow.setSize(width, height);
    }
  }

  getTitle(): string {
    return this._title;
  }

  setTitle(title: string): void {
    this._title = title;
    if (this.nativeWindow?.setTitle) {
      this.nativeWindow.setTitle(title);
    }
  }

  isFullscreen(): boolean {
    return this._fullscreen;
  }

  setFullscreen(fullscreen: boolean): void {
    this._fullscreen = fullscreen;
    if (this.nativeWindow?.setFullScreen) {
      this.nativeWindow.setFullScreen(fullscreen);
    }
  }

  getPosition(): { x: number; y: number } {
    if (this.nativeWindow?.getPosition) {
      const [x, y] = this.nativeWindow.getPosition();
      return { x, y };
    }
    return { x: this.config.x || 0, y: this.config.y || 0 };
  }

  setPosition(x: number, y: number): void {
    if (this.nativeWindow?.setPosition) {
      this.nativeWindow.setPosition(x, y);
    }
  }

  isVisible(): boolean {
    if (this.nativeWindow?.isVisible) {
      return this.nativeWindow.isVisible();
    }
    return true;
  }

  setVisible(visible: boolean): void {
    if (visible && this.nativeWindow?.show) {
      this.nativeWindow.show();
    } else if (!visible && this.nativeWindow?.hide) {
      this.nativeWindow.hide();
    }
  }

  isFocused(): boolean {
    if (this.nativeWindow?.isFocused) {
      return this.nativeWindow.isFocused();
    }
    return false;
  }

  focus(): void {
    if (this.nativeWindow?.focus) {
      this.nativeWindow.focus();
    }
  }

  close(): void {
    if (this.nativeWindow?.close) {
      this.nativeWindow.close();
    }
  }

  getNativeHandle(): any {
    return this.nativeWindow;
  }

  requestAnimationFrame(callback: (time: number) => void): number {
    // Use setImmediate for Node.js environment
    const globalObj = globalThis as any;
    if (globalObj.setImmediate) {
      return globalObj.setImmediate(() => callback(Date.now()));
    }
    return setTimeout(() => callback(Date.now()), 16) as any;
  }

  cancelAnimationFrame(handle: number): void {
    const globalObj = globalThis as any;
    if (globalObj.clearImmediate) {
      globalObj.clearImmediate(handle);
    } else {
      clearTimeout(handle);
    }
  }
}

/**
 * Desktop Worker Implementation
 * Uses Node.js Worker Threads
 */
class DesktopWorkerWrapper implements IWorker {
  private worker: any; // Worker from 'worker_threads'
  private running = true;

  constructor(_scriptPath: string) {
    // Note: Requires 'worker_threads' module
    // This is a stub implementation
    this.worker = null;
  }

  postMessage(message: unknown, transfer?: Transferable[]): void {
    if (this.worker?.postMessage) {
      this.worker.postMessage(message, transfer);
    }
  }

  onMessage(handler: (message: unknown) => void): void {
    if (this.worker?.on) {
      this.worker.on('message', handler);
    }
  }

  onError(handler: (error: Error) => void): void {
    if (this.worker?.on) {
      this.worker.on('error', handler);
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
 * Desktop File System Implementation
 * Uses Node.js fs module
 */
class DesktopFileSystem implements IFileSystem {
  private fs: any;
  private path: any;

  constructor() {
    // Note: Requires 'fs' and 'path' modules
    // This is a stub implementation for compilation
    try {
      this.fs = (globalThis as any).require?.('fs');
      this.path = (globalThis as any).require?.('path');
    } catch (_e) {
      this.fs = null;
      this.path = null;
    }
    // Path module reserved for future use
    void this.path;
  }

  async readFile(path: string): Promise<ArrayBuffer> {
    if (!this.fs?.promises?.readFile) {
      throw new Error('File system not available');
    }
    const buffer = await this.fs.promises.readFile(path);
    return buffer.buffer.slice(
      buffer.byteOffset,
      buffer.byteOffset + buffer.byteLength
    );
  }

  async readTextFile(path: string): Promise<string> {
    if (!this.fs?.promises?.readFile) {
      throw new Error('File system not available');
    }
    return this.fs.promises.readFile(path, 'utf8');
  }

  async writeFile(path: string, data: ArrayBuffer): Promise<void> {
    if (!this.fs?.promises?.writeFile) {
      throw new Error('File system not available');
    }
    const BufferClass = (globalThis as any).Buffer;
    const buffer = BufferClass ? BufferClass.from(data) : new Uint8Array(data);
    await this.fs.promises.writeFile(path, buffer);
  }

  async writeTextFile(path: string, data: string): Promise<void> {
    if (!this.fs?.promises?.writeFile) {
      throw new Error('File system not available');
    }
    await this.fs.promises.writeFile(path, data, 'utf8');
  }

  async exists(path: string): Promise<boolean> {
    if (!this.fs?.promises?.access) {
      throw new Error('File system not available');
    }
    try {
      await this.fs.promises.access(path);
      return true;
    } catch {
      return false;
    }
  }

  async listDirectory(path: string): Promise<string[]> {
    if (!this.fs?.promises?.readdir) {
      throw new Error('File system not available');
    }
    return this.fs.promises.readdir(path);
  }

  async createDirectory(path: string, recursive = false): Promise<void> {
    if (!this.fs?.promises?.mkdir) {
      throw new Error('File system not available');
    }
    await this.fs.promises.mkdir(path, { recursive });
  }

  async deleteFile(path: string): Promise<void> {
    if (!this.fs?.promises?.unlink) {
      throw new Error('File system not available');
    }
    await this.fs.promises.unlink(path);
  }

  async deleteDirectory(path: string, recursive = false): Promise<void> {
    if (!this.fs?.promises?.rmdir) {
      throw new Error('File system not available');
    }
    await this.fs.promises.rmdir(path, { recursive });
  }

  async getStats(path: string): Promise<FileStats> {
    if (!this.fs?.promises?.stat) {
      throw new Error('File system not available');
    }
    const stats = await this.fs.promises.stat(path);
    return {
      size: stats.size,
      modified: stats.mtimeMs,
      created: stats.birthtimeMs || stats.ctimeMs,
      isDirectory: stats.isDirectory(),
      isFile: stats.isFile(),
      permissions: stats.mode,
    };
  }

  async copyFile(source: string, destination: string): Promise<void> {
    if (!this.fs?.promises?.copyFile) {
      throw new Error('File system not available');
    }
    await this.fs.promises.copyFile(source, destination);
  }

  async moveFile(source: string, destination: string): Promise<void> {
    if (!this.fs?.promises?.rename) {
      throw new Error('File system not available');
    }
    await this.fs.promises.rename(source, destination);
  }

  watch(
    path: string,
    callback: (event: 'create' | 'modify' | 'delete', path: string) => void
  ): () => void {
    if (!this.fs?.watch) {
      return () => {};
    }

    const watcher = this.fs.watch(path, (eventType: string, filename: string) => {
      const event =
        eventType === 'rename'
          ? 'create'
          : eventType === 'change'
            ? 'modify'
            : 'delete';
      callback(event as any, filename);
    });

    return () => watcher.close();
  }
}

/**
 * Desktop WebSocket Implementation
 * Uses ws package or native WebSocket
 */
class DesktopWebSocketWrapper implements IWebSocket {
  private ws: any;

  constructor(url: string, protocols?: string | string[]) {
    // Note: Requires 'ws' package or native WebSocket
    const WebSocketClass = (globalThis as any).WebSocket;
    if (WebSocketClass) {
      this.ws = new WebSocketClass(url, protocols);
    }
  }

  send(data: string | ArrayBuffer): void {
    if (this.ws?.send) {
      this.ws.send(data);
    }
  }

  close(code?: number, reason?: string): void {
    if (this.ws?.close) {
      this.ws.close(code, reason);
    }
  }

  onMessage(handler: (data: string | ArrayBuffer) => void): void {
    if (this.ws) {
      this.ws.onmessage = (e: any) => handler(e.data);
    }
  }

  onOpen(handler: () => void): void {
    if (this.ws) {
      this.ws.onopen = handler;
    }
  }

  onClose(handler: (code: number, reason: string) => void): void {
    if (this.ws) {
      this.ws.onclose = (e: any) => handler(e.code, e.reason);
    }
  }

  onError(handler: (error: Error) => void): void {
    if (this.ws) {
      this.ws.onerror = () => handler(new Error('WebSocket error'));
    }
  }

  getReadyState(): number {
    return this.ws?.readyState || 0;
  }

  getBufferedAmount(): number {
    return this.ws?.bufferedAmount || 0;
  }
}

/**
 * Desktop Platform Implementation
 *
 * Platform implementation for native desktop applications.
 * Supports Electron, Tauri, and Node.js environments.
 * Uses native file system, worker threads, and system APIs.
 */
export class DesktopPlatform implements IPlatform {
  private windows: DesktopWindow[] = [];
  private fileSystem: DesktopFileSystem;
  private initialized = false;

  constructor() {
    this.fileSystem = new DesktopFileSystem();
  }

  async initialize(_config?: Record<string, unknown>): Promise<void> {
    if (this.initialized) return;
    this.initialized = true;
  }

  async shutdown(): Promise<void> {
    this.windows.forEach((window) => window.close());
    this.windows = [];
    this.initialized = false;
  }

  getName(): string {
    return 'desktop';
  }

  getInfo(): PlatformInfo {
    const process = (globalThis as any).process;
    return {
      name: 'desktop',
      os: process?.platform || 'unknown',
      arch: process?.arch || 'unknown',
      version: process?.version || 'unknown',
      deviceType: 'desktop',
      isDevelopment: process?.env?.NODE_ENV === 'development',
      isProduction: process?.env?.NODE_ENV === 'production',
    };
  }

  getCapabilities(): PlatformCapabilities {
    return {
      supportsWorkers: typeof (globalThis as any).Worker !== 'undefined',
      supportsWebGL: false, // Desktop uses native graphics APIs
      supportsWebGL2: false,
      supportsWebGPU: false,
      supportsWebAudio: false,
      supportsIndexedDB: false,
      supportsLocalStorage: false,
      supportsTouch: false,
      supportsGamepad: true,
      supportsPointerLock: false,
      supportsFullscreen: true,
      maxTextureSize: 16384, // Typical desktop limit
      maxRenderTargets: 8,
      devicePixelRatio: 1.0,
    };
  }

  createWindow(config: WindowConfig): IWindow {
    const window = new DesktopWindow(config);
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
    return new DesktopWorkerWrapper(scriptPath);
  }

  getHardwareConcurrency(): number {
    const os = (globalThis as any).require?.('os');
    return os?.cpus?.()?.length || 4;
  }

  now(): number {
    return (globalThis as any).performance?.now?.() || Date.now();
  }

  sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }

  getInputDevices(): IInputDevice[] {
    const devices: IInputDevice[] = [];

    // Desktop always has keyboard and mouse
    devices.push({
      type: 'keyboard',
      id: 'keyboard-0',
      name: 'Keyboard',
      isConnected: () => true,
      getCapabilities: () => ({}),
    });

    devices.push({
      type: 'mouse',
      id: 'mouse-0',
      name: 'Mouse',
      isConnected: () => true,
      getCapabilities: () => ({}),
    });

    return devices;
  }

  async fetch(url: string, options?: FetchOptions): Promise<Response> {
    // Use native fetch if available, otherwise require node-fetch
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
    return new DesktopWebSocketWrapper(url, protocols);
  }

  async readClipboard(): Promise<string> {
    const clipboard = (globalThis as any).require?.('electron')?.clipboard;
    if (clipboard?.readText) {
      return clipboard.readText();
    }
    return '';
  }

  async writeClipboard(text: string): Promise<void> {
    const clipboard = (globalThis as any).require?.('electron')?.clipboard;
    if (clipboard?.writeText) {
      clipboard.writeText(text);
    }
  }

  async alert(message: string): Promise<void> {
    const dialog = (globalThis as any).require?.('electron')?.dialog;
    if (dialog?.showMessageBox) {
      await dialog.showMessageBox({ message, type: 'info' });
    } else {
      console.log('Alert:', message);
    }
  }

  async confirm(message: string): Promise<boolean> {
    const dialog = (globalThis as any).require?.('electron')?.dialog;
    if (dialog?.showMessageBox) {
      const result = await dialog.showMessageBox({
        message,
        type: 'question',
        buttons: ['OK', 'Cancel'],
      });
      return result.response === 0;
    }
    return false;
  }

  async prompt(message: string, defaultValue = ''): Promise<string | null> {
    // Electron doesn't have native prompt, would need custom dialog
    console.log('Prompt:', message, defaultValue);
    return null;
  }

  addEventListener(event: string, handler: (...args: unknown[]) => void): void {
    const app = (globalThis as any).require?.('electron')?.app;
    if (app?.on) {
      app.on(event, handler);
    }
  }

  removeEventListener(
    event: string,
    handler: (...args: unknown[]) => void
  ): void {
    const app = (globalThis as any).require?.('electron')?.app;
    if (app?.removeListener) {
      app.removeListener(event, handler);
    }
  }
}
