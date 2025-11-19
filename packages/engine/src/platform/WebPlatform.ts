/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

import {
  IPlatform,
  PlatformType,
  OSType,
  IFileSystem,
  IThreading,
  INetwork,
  IWindow,
  ITiming,
  PathType,
  WindowOptions,
  WindowCallbacks,
  IPlatformJob,
  NetworkRequest,
  NetworkResponse,
  IWebSocket,
  WebSocketHandler,
} from './IPlatform';

/**
 * Web FileSystem Implementation
 * Uses IndexedDB for persistent storage and in-memory for temporary storage
 */
class WebFileSystem implements IFileSystem {
  private dbName = 'NovaEngineFS';
  private storeName = 'files';
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
        if (!db.objectStoreNames.contains(this.storeName)) {
          db.createObjectStore(this.storeName);
        }
      };
    });
  }

  async getPath(type: PathType): Promise<string> {
    // In web, we use virtual paths
    switch (type) {
      case PathType.USER_DATA:
        return '/user_data';
      case PathType.APP_DATA:
        return '/app_data';
      case PathType.CACHE:
        return '/cache';
      case PathType.TEMP:
        return '/temp';
      case PathType.DOCUMENTS:
        return '/documents';
      case PathType.DOWNLOADS:
        return '/downloads';
      default:
        return '/';
    }
  }

  async exists(path: string): Promise<boolean> {
    if (!this.db) throw new Error('FileSystem not initialized');

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction([this.storeName], 'readonly');
      const store = transaction.objectStore(this.storeName);
      const request = store.get(path);

      request.onsuccess = () => resolve(request.result !== undefined);
      request.onerror = () => reject(request.error);
    });
  }

  async readText(path: string): Promise<string> {
    const buffer = await this.readBinary(path);
    const decoder = new TextDecoder();
    return decoder.decode(buffer);
  }

  async readBinary(path: string): Promise<ArrayBuffer> {
    if (!this.db) throw new Error('FileSystem not initialized');

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction([this.storeName], 'readonly');
      const store = transaction.objectStore(this.storeName);
      const request = store.get(path);

      request.onsuccess = () => {
        if (request.result === undefined) {
          reject(new Error(`File not found: ${path}`));
        } else {
          resolve(request.result as ArrayBuffer);
        }
      };
      request.onerror = () => reject(request.error);
    });
  }

  async writeText(path: string, content: string): Promise<void> {
    const encoder = new TextEncoder();
    const buffer = encoder.encode(content);
    await this.writeBinary(path, buffer.buffer);
  }

  async writeBinary(path: string, content: ArrayBuffer): Promise<void> {
    if (!this.db) throw new Error('FileSystem not initialized');

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction([this.storeName], 'readwrite');
      const store = transaction.objectStore(this.storeName);
      const request = store.put(content, path);

      request.onsuccess = () => resolve();
      request.onerror = () => reject(request.error);
    });
  }

  async delete(path: string): Promise<void> {
    if (!this.db) throw new Error('FileSystem not initialized');

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction([this.storeName], 'readwrite');
      const store = transaction.objectStore(this.storeName);
      const request = store.delete(path);

      request.onsuccess = () => resolve();
      request.onerror = () => reject(request.error);
    });
  }

  async listFiles(path: string): Promise<string[]> {
    if (!this.db) throw new Error('FileSystem not initialized');

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction([this.storeName], 'readonly');
      const store = transaction.objectStore(this.storeName);
      const request = store.getAllKeys();

      request.onsuccess = () => {
        const keys = request.result as string[];
        // Filter keys that start with the path
        const filtered = keys.filter((key) => key.startsWith(path));
        resolve(filtered);
      };
      request.onerror = () => reject(request.error);
    });
  }

  async createDirectory(_path: string): Promise<void> {
    // In IndexedDB, directories are implicit
    // No operation needed
  }

  async getFileSize(path: string): Promise<number> {
    const buffer = await this.readBinary(path);
    return buffer.byteLength;
  }

  async getModifiedTime(_path: string): Promise<Date> {
    // IndexedDB doesn't track modification time
    // Return current time as fallback
    return new Date();
  }
}

/**
 * Web Threading Implementation
 * Uses Web Workers for parallel execution
 */
class WebThreading implements IThreading {
  private workers: Worker[] = [];
  private coreCount: number;

  constructor() {
    this.coreCount = navigator.hardwareConcurrency || 4;
  }

  getCoreCount(): number {
    return this.coreCount;
  }

  async scheduleJob(job: IPlatformJob): Promise<void> {
    // For now, execute jobs on main thread
    // Full Web Worker implementation would require more setup
    await job.execute();
  }

  async scheduleJobs(jobs: IPlatformJob[]): Promise<void> {
    // Execute jobs in parallel using Promise.all
    await Promise.all(jobs.map((job) => this.scheduleJob(job)));
  }

  isSupported(): boolean {
    return typeof Worker !== 'undefined';
  }

  async shutdown(): Promise<void> {
    this.workers.forEach((worker) => worker.terminate());
    this.workers = [];
  }
}

/**
 * Web WebSocket Implementation
 */
class WebSocketImpl implements IWebSocket {
  private ws: WebSocket | null = null;
  private messageHandler: WebSocketHandler | null = null;
  private closeHandler: (() => void) | null = null;
  private errorHandler: ((error: Error) => void) | null = null;

  async connect(url: string): Promise<void> {
    return new Promise((resolve, reject) => {
      this.ws = new WebSocket(url);

      this.ws.onopen = () => resolve();
      this.ws.onerror = (_event) => {
        const error = new Error('WebSocket connection error');
        if (this.errorHandler) this.errorHandler(error);
        reject(error);
      };
      this.ws.onmessage = (event) => {
        if (this.messageHandler) {
          this.messageHandler(event.data);
        }
      };
      this.ws.onclose = () => {
        if (this.closeHandler) this.closeHandler();
      };
    });
  }

  send(data: string | ArrayBuffer): void {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      throw new Error('WebSocket is not connected');
    }
    this.ws.send(data);
  }

  onMessage(handler: WebSocketHandler): void {
    this.messageHandler = handler;
  }

  onClose(handler: () => void): void {
    this.closeHandler = handler;
  }

  onError(handler: (error: Error) => void): void {
    this.errorHandler = handler;
  }

  close(): void {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  isConnected(): boolean {
    return this.ws !== null && this.ws.readyState === WebSocket.OPEN;
  }
}

/**
 * Web Network Implementation
 */
class WebNetwork implements INetwork {
  async request(options: NetworkRequest): Promise<NetworkResponse> {
    const response = await fetch(options.url, {
      method: options.method || 'GET',
      headers: options.headers,
      body: options.body,
      signal: options.timeout
        ? AbortSignal.timeout(options.timeout)
        : undefined,
    });

    const body = await response.arrayBuffer();
    const headers: Record<string, string> = {};
    response.headers.forEach((value: string, key: string) => {
      headers[key] = value;
    });

    return {
      status: response.status,
      statusText: response.statusText,
      headers,
      body,
      text: () => new TextDecoder().decode(body),
      json: () => JSON.parse(new TextDecoder().decode(body)),
    };
  }

  createWebSocket(): IWebSocket {
    return new WebSocketImpl();
  }

  isOnline(): boolean {
    return navigator.onLine;
  }

  onConnectionChange(handler: (online: boolean) => void): void {
    window.addEventListener('online', () => handler(true));
    window.addEventListener('offline', () => handler(false));
  }
}

/**
 * Web Window Implementation
 */
class WebWindow implements IWindow {
  private canvas: HTMLCanvasElement | null = null;
  private callbacks: WindowCallbacks = {};

  async create(options: WindowOptions): Promise<void> {
    // Set document title
    document.title = options.title;

    // Create or get canvas
    this.canvas =
      (document.querySelector('canvas') as HTMLCanvasElement) ||
      document.createElement('canvas');

    if (!this.canvas.parentElement) {
      document.body.appendChild(this.canvas);
    }

    // Set canvas size
    this.setSize(options.width, options.height);

    // Set up event listeners
    window.addEventListener('resize', () => {
      if (this.callbacks.onResize) {
        this.callbacks.onResize(window.innerWidth, window.innerHeight);
      }
    });

    window.addEventListener('beforeunload', () => {
      if (this.callbacks.onClose) {
        this.callbacks.onClose();
      }
    });

    window.addEventListener('focus', () => {
      if (this.callbacks.onFocus) {
        this.callbacks.onFocus(true);
      }
    });

    window.addEventListener('blur', () => {
      if (this.callbacks.onFocus) {
        this.callbacks.onFocus(false);
      }
    });
  }

  getSize(): { width: number; height: number } {
    return {
      width: window.innerWidth,
      height: window.innerHeight,
    };
  }

  setSize(width: number, height: number): void {
    if (this.canvas) {
      this.canvas.width = width;
      this.canvas.height = height;
    }
  }

  getTitle(): string {
    return document.title;
  }

  setTitle(title: string): void {
    document.title = title;
  }

  setFullscreen(fullscreen: boolean): void {
    if (fullscreen) {
      document.documentElement.requestFullscreen();
    } else {
      if (document.fullscreenElement) {
        document.exitFullscreen();
      }
    }
  }

  isFullscreen(): boolean {
    return document.fullscreenElement !== null;
  }

  setCallbacks(callbacks: WindowCallbacks): void {
    this.callbacks = callbacks;
  }

  requestAnimationFrame(callback: (time: number) => void): number {
    return window.requestAnimationFrame(callback);
  }

  cancelAnimationFrame(handle: number): void {
    window.cancelAnimationFrame(handle);
  }

  getPixelRatio(): number {
    return window.devicePixelRatio || 1;
  }

  getCanvas(): HTMLCanvasElement | null {
    return this.canvas;
  }
}

/**
 * Web Timing Implementation
 */
class WebTiming implements ITiming {
  now(): number {
    return Date.now();
  }

  performance(): number {
    return performance.now();
  }

  async sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }

  setInterval(callback: () => void, ms: number): number {
    return (typeof window !== 'undefined'
      ? window.setInterval(callback, ms)
      : setInterval(callback, ms)) as unknown as number;
  }

  clearInterval(handle: number): void {
    if (typeof window !== 'undefined') {
      window.clearInterval(handle);
    } else {
      clearInterval(handle as any);
    }
  }

  setTimeout(callback: () => void, ms: number): number {
    return (typeof window !== 'undefined'
      ? window.setTimeout(callback, ms)
      : setTimeout(callback, ms)) as unknown as number;
  }

  clearTimeout(handle: number): void {
    if (typeof window !== 'undefined') {
      window.clearTimeout(handle);
    } else {
      clearTimeout(handle as any);
    }
  }
}

/**
 * Web Platform Implementation
 */
export class WebPlatform implements IPlatform {
  public readonly fileSystem: IFileSystem;
  public readonly threading: IThreading;
  public readonly network: INetwork;
  public readonly window: IWindow;
  public readonly timing: ITiming;

  constructor() {
    this.fileSystem = new WebFileSystem();
    this.threading = new WebThreading();
    this.network = new WebNetwork();
    this.window = new WebWindow();
    this.timing = new WebTiming();
  }

  getType(): PlatformType {
    return PlatformType.WEB;
  }

  getOS(): OSType {
    // Detect OS from user agent
    const ua = navigator.userAgent.toLowerCase();
    if (ua.includes('win')) return OSType.WINDOWS;
    if (ua.includes('mac')) return OSType.MACOS;
    if (ua.includes('linux')) return OSType.LINUX;
    if (ua.includes('iphone') || ua.includes('ipad')) return OSType.IOS;
    if (ua.includes('android')) return OSType.ANDROID;
    return OSType.WEB_BROWSER;
  }

  getVersion(): string {
    return `Web/${navigator.userAgent}`;
  }

  async initialize(): Promise<void> {
    // Initialize file system
    await (this.fileSystem as WebFileSystem).initialize();
  }

  async shutdown(): Promise<void> {
    await this.threading.shutdown();
  }
}
