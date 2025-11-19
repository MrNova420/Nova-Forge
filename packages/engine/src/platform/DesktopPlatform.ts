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
 * Desktop FileSystem Implementation
 * Uses Node.js fs module for file operations
 */
class DesktopFileSystem implements IFileSystem {
  private fs: any = null;
  private path: any = null;
  private os: any = null;

  async initialize(): Promise<void> {
    // Dynamically import Node.js modules (only available in Node environment)
    if (typeof require !== 'undefined') {
      this.fs = require('fs').promises;
      this.path = require('path');
      this.os = require('os');
    } else {
      throw new Error('Desktop platform requires Node.js environment');
    }
  }

  async getPath(type: PathType): Promise<string> {
    if (!this.os || !this.path) {
      throw new Error('FileSystem not initialized');
    }

    switch (type) {
      case PathType.USER_DATA:
        return this.path.join(this.os.homedir(), '.nova-engine', 'user_data');
      case PathType.APP_DATA:
        return this.path.join(this.os.homedir(), '.nova-engine', 'app_data');
      case PathType.CACHE:
        return this.path.join(this.os.homedir(), '.nova-engine', 'cache');
      case PathType.TEMP:
        return this.os.tmpdir();
      case PathType.DOCUMENTS:
        return this.path.join(this.os.homedir(), 'Documents');
      case PathType.DOWNLOADS:
        return this.path.join(this.os.homedir(), 'Downloads');
      default:
        return this.os.homedir();
    }
  }

  async exists(path: string): Promise<boolean> {
    if (!this.fs) throw new Error('FileSystem not initialized');

    try {
      await this.fs.access(path);
      return true;
    } catch {
      return false;
    }
  }

  async readText(path: string): Promise<string> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    return await this.fs.readFile(path, 'utf-8');
  }

  async readBinary(path: string): Promise<ArrayBuffer> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    const buffer = await this.fs.readFile(path);
    return buffer.buffer.slice(
      buffer.byteOffset,
      buffer.byteOffset + buffer.byteLength
    );
  }

  async writeText(path: string, content: string): Promise<void> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    await this.fs.writeFile(path, content, 'utf-8');
  }

  async writeBinary(path: string, content: ArrayBuffer): Promise<void> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    const buffer = Buffer.from(content);
    await this.fs.writeFile(path, buffer);
  }

  async delete(path: string): Promise<void> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    await this.fs.unlink(path);
  }

  async listFiles(path: string): Promise<string[]> {
    if (!this.fs || !this.path) throw new Error('FileSystem not initialized');
    const files = await this.fs.readdir(path);
    return files.map((file: string) => this.path.join(path, file));
  }

  async createDirectory(path: string): Promise<void> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    await this.fs.mkdir(path, { recursive: true });
  }

  async getFileSize(path: string): Promise<number> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    const stats = await this.fs.stat(path);
    return stats.size;
  }

  async getModifiedTime(path: string): Promise<Date> {
    if (!this.fs) throw new Error('FileSystem not initialized');
    const stats = await this.fs.stat(path);
    return stats.mtime;
  }
}

/**
 * Desktop Threading Implementation
 * Uses Node.js Worker threads for parallel execution
 */
class DesktopThreading implements IThreading {
  private workerThreads: any = null;
  private os: any = null;
  private workers: any[] = [];
  private coreCount: number = 1;

  async initialize(): Promise<void> {
    if (typeof require !== 'undefined') {
      this.os = require('os');
      this.coreCount = this.os.cpus().length;

      try {
        this.workerThreads = require('worker_threads');
      } catch {
        // Worker threads not available, will fall back to main thread
        console.warn(
          'Worker threads not available, jobs will run on main thread'
        );
      }
    }
  }

  getCoreCount(): number {
    return this.coreCount;
  }

  async scheduleJob(job: IPlatformJob): Promise<void> {
    // For now, execute on main thread
    // Full Worker implementation would require serialization
    await job.execute();
  }

  async scheduleJobs(jobs: IPlatformJob[]): Promise<void> {
    // Execute jobs in parallel using Promise.all
    await Promise.all(jobs.map((job) => this.scheduleJob(job)));
  }

  isSupported(): boolean {
    return this.workerThreads !== null;
  }

  async shutdown(): Promise<void> {
    // Terminate all workers
    this.workers.forEach((worker) => {
      if (worker && typeof worker.terminate === 'function') {
        worker.terminate();
      }
    });
    this.workers = [];
  }
}

/**
 * Desktop WebSocket Implementation
 */
class DesktopWebSocket implements IWebSocket {
  private ws: any = null;
  private WebSocket: any = null;
  private messageHandler: WebSocketHandler | null = null;
  private closeHandler: (() => void) | null = null;
  private errorHandler: ((error: Error) => void) | null = null;

  constructor() {
    if (typeof require !== 'undefined') {
      try {
        this.WebSocket = require('ws');
      } catch {
        // WebSocket library not available
        console.warn('ws package not available for WebSocket support');
      }
    }
  }

  async connect(url: string): Promise<void> {
    if (!this.WebSocket) {
      throw new Error('WebSocket library not available');
    }

    return new Promise((resolve, reject) => {
      this.ws = new this.WebSocket(url);

      this.ws.on('open', () => resolve());
      this.ws.on('error', (error: Error) => {
        if (this.errorHandler) this.errorHandler(error);
        reject(error);
      });
      this.ws.on('message', (data: any) => {
        if (this.messageHandler) {
          this.messageHandler(data);
        }
      });
      this.ws.on('close', () => {
        if (this.closeHandler) this.closeHandler();
      });
    });
  }

  send(data: string | ArrayBuffer): void {
    if (!this.ws) {
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
    return this.ws !== null && this.ws.readyState === 1; // OPEN
  }
}

/**
 * Desktop Network Implementation
 */
class DesktopNetwork implements INetwork {
  private https: any = null;
  private http: any = null;

  async initialize(): Promise<void> {
    if (typeof require !== 'undefined') {
      this.https = require('https');
      this.http = require('http');
    }
  }

  async request(options: NetworkRequest): Promise<NetworkResponse> {
    if (!this.https || !this.http) {
      throw new Error('Network not initialized');
    }

    return new Promise((resolve, reject) => {
      const url = new URL(options.url);
      const protocol = url.protocol === 'https:' ? this.https : this.http;

      const requestOptions = {
        hostname: url.hostname,
        port: url.port,
        path: url.pathname + url.search,
        method: options.method || 'GET',
        headers: options.headers || {},
        timeout: options.timeout || 30000,
      };

      const req = protocol.request(requestOptions, (res: any) => {
        const chunks: Buffer[] = [];

        res.on('data', (chunk: Buffer) => chunks.push(chunk));
        res.on('end', () => {
          const buffer = Buffer.concat(chunks);
          const arrayBuffer = buffer.buffer.slice(
            buffer.byteOffset,
            buffer.byteOffset + buffer.byteLength
          );

          const headers: Record<string, string> = {};
          Object.keys(res.headers).forEach((key) => {
            headers[key] = res.headers[key];
          });

          resolve({
            status: res.statusCode,
            statusText: res.statusMessage || '',
            headers,
            body: arrayBuffer,
            text: () => buffer.toString('utf-8'),
            json: () => JSON.parse(buffer.toString('utf-8')),
          });
        });
      });

      req.on('error', reject);
      req.on('timeout', () => {
        req.destroy();
        reject(new Error('Request timeout'));
      });

      if (options.body) {
        req.write(options.body);
      }

      req.end();
    });
  }

  createWebSocket(): IWebSocket {
    return new DesktopWebSocket();
  }

  isOnline(): boolean {
    // In Node.js, we assume online unless explicitly checking network interfaces
    return true;
  }

  onConnectionChange(_handler: (online: boolean) => void): void {
    // Network change detection would require platform-specific APIs
    // Not implemented for desktop platform baseline
  }
}

/**
 * Desktop Window Implementation
 * Placeholder for Electron/Tauri window management
 */
class DesktopWindow implements IWindow {
  private callbacks: WindowCallbacks = {};
  private width = 1280;
  private height = 720;
  private title = 'Nova Engine';

  async create(options: WindowOptions): Promise<void> {
    this.title = options.title;
    this.width = options.width;
    this.height = options.height;

    // In a real desktop app, this would create an Electron/Tauri window
    // For now, this is a placeholder
    console.log(
      `Desktop window created: ${this.title} (${this.width}x${this.height})`
    );
  }

  getSize(): { width: number; height: number } {
    return { width: this.width, height: this.height };
  }

  setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    if (this.callbacks.onResize) {
      this.callbacks.onResize(width, height);
    }
  }

  getTitle(): string {
    return this.title;
  }

  setTitle(title: string): void {
    this.title = title;
  }

  setFullscreen(_fullscreen: boolean): void {
    // Would integrate with Electron/Tauri fullscreen API
  }

  isFullscreen(): boolean {
    return false;
  }

  setCallbacks(callbacks: WindowCallbacks): void {
    this.callbacks = callbacks;
  }

  requestAnimationFrame(callback: (time: number) => void): number {
    // Use Node.js setImmediate for frame callbacks
    return setImmediate(() => callback(Date.now())) as unknown as number;
  }

  cancelAnimationFrame(handle: number): void {
    clearImmediate(handle as any);
  }

  getPixelRatio(): number {
    // Would query from Electron/Tauri
    return 1.0;
  }

  getCanvas(): HTMLCanvasElement | null {
    // Would return canvas from Electron/Tauri window
    return null;
  }
}

/**
 * Desktop Timing Implementation
 */
class DesktopTiming implements ITiming {
  now(): number {
    return Date.now();
  }

  performance(): number {
    if (typeof performance !== 'undefined') {
      return performance.now();
    }
    // Node.js fallback
    const [seconds, nanoseconds] = process.hrtime();
    return seconds * 1000 + nanoseconds / 1000000;
  }

  async sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }

  setInterval(callback: () => void, ms: number): number {
    return setInterval(callback, ms) as unknown as number;
  }

  clearInterval(handle: number): void {
    clearInterval(handle as any);
  }

  setTimeout(callback: () => void, ms: number): number {
    return setTimeout(callback, ms) as unknown as number;
  }

  clearTimeout(handle: number): void {
    clearTimeout(handle as any);
  }
}

/**
 * Desktop Platform Implementation
 * For Electron, Tauri, and native Node.js applications
 */
export class DesktopPlatform implements IPlatform {
  public readonly fileSystem: IFileSystem;
  public readonly threading: IThreading;
  public readonly network: INetwork;
  public readonly window: IWindow;
  public readonly timing: ITiming;

  constructor() {
    this.fileSystem = new DesktopFileSystem();
    this.threading = new DesktopThreading();
    this.network = new DesktopNetwork();
    this.window = new DesktopWindow();
    this.timing = new DesktopTiming();
  }

  getType(): PlatformType {
    return PlatformType.DESKTOP;
  }

  getOS(): OSType {
    if (typeof process !== 'undefined' && process.platform) {
      switch (process.platform) {
        case 'win32':
          return OSType.WINDOWS;
        case 'darwin':
          return OSType.MACOS;
        case 'linux':
          return OSType.LINUX;
        default:
          return OSType.LINUX;
      }
    }
    return OSType.LINUX;
  }

  getVersion(): string {
    if (typeof process !== 'undefined') {
      return `Desktop/${process.platform} Node/${process.version}`;
    }
    return 'Desktop/Unknown';
  }

  async initialize(): Promise<void> {
    await (this.fileSystem as DesktopFileSystem).initialize();
    await (this.threading as DesktopThreading).initialize();
    await (this.network as DesktopNetwork).initialize();
  }

  async shutdown(): Promise<void> {
    await this.threading.shutdown();
  }
}
