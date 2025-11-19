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
 * Mobile FileSystem Implementation
 * Uses React Native or mobile-specific file APIs
 */
class MobileFileSystem implements IFileSystem {
  private basePath = '/storage/app';

  async getPath(type: PathType): Promise<string> {
    // In React Native, these would use specific libraries like react-native-fs
    switch (type) {
      case PathType.USER_DATA:
        return `${this.basePath}/user_data`;
      case PathType.APP_DATA:
        return `${this.basePath}/app_data`;
      case PathType.CACHE:
        return `${this.basePath}/cache`;
      case PathType.TEMP:
        return `${this.basePath}/temp`;
      case PathType.DOCUMENTS:
        return `${this.basePath}/documents`;
      case PathType.DOWNLOADS:
        return `${this.basePath}/downloads`;
      default:
        return this.basePath;
    }
  }

  async exists(_path: string): Promise<boolean> {
    // Would use react-native-fs or AsyncStorage
    // For now, return false as placeholder
    return false;
  }

  async readText(_path: string): Promise<string> {
    // Would use react-native-fs
    throw new Error('Mobile file system requires React Native environment');
  }

  async readBinary(_path: string): Promise<ArrayBuffer> {
    // Would use react-native-fs
    throw new Error('Mobile file system requires React Native environment');
  }

  async writeText(_path: string, _content: string): Promise<void> {
    // Would use react-native-fs
    throw new Error('Mobile file system requires React Native environment');
  }

  async writeBinary(_path: string, _content: ArrayBuffer): Promise<void> {
    // Would use react-native-fs
    throw new Error('Mobile file system requires React Native environment');
  }

  async delete(_path: string): Promise<void> {
    // Would use react-native-fs
    throw new Error('Mobile file system requires React Native environment');
  }

  async listFiles(_path: string): Promise<string[]> {
    // Would use react-native-fs
    return [];
  }

  async createDirectory(_path: string): Promise<void> {
    // Would use react-native-fs
  }

  async getFileSize(_path: string): Promise<number> {
    // Would use react-native-fs
    return 0;
  }

  async getModifiedTime(_path: string): Promise<Date> {
    // Would use react-native-fs
    return new Date();
  }
}

/**
 * Mobile Threading Implementation
 * Limited threading on mobile platforms
 */
class MobileThreading implements IThreading {
  private coreCount: number = 4;

  getCoreCount(): number {
    return this.coreCount;
  }

  async scheduleJob(job: IPlatformJob): Promise<void> {
    // Mobile platforms have limited threading
    // Execute on main thread
    await job.execute();
  }

  async scheduleJobs(jobs: IPlatformJob[]): Promise<void> {
    // Execute jobs sequentially on mobile
    for (const job of jobs) {
      await this.scheduleJob(job);
    }
  }

  isSupported(): boolean {
    // Limited threading support on mobile
    return false;
  }

  async shutdown(): Promise<void> {
    // No workers to shutdown
  }
}

/**
 * Mobile WebSocket Implementation
 */
class MobileWebSocket implements IWebSocket {
  private ws: WebSocket | null = null;
  private messageHandler: WebSocketHandler | null = null;
  private closeHandler: (() => void) | null = null;
  private errorHandler: ((error: Error) => void) | null = null;

  async connect(url: string): Promise<void> {
    if (typeof WebSocket === 'undefined') {
      throw new Error('WebSocket not available in this environment');
    }

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
 * Mobile Network Implementation
 */
class MobileNetwork implements INetwork {
  async request(options: NetworkRequest): Promise<NetworkResponse> {
    // Use fetch API (available in React Native)
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
    return new MobileWebSocket();
  }

  isOnline(): boolean {
    // Would use @react-native-community/netinfo
    // For now, assume online
    return true;
  }

  onConnectionChange(_handler: (online: boolean) => void): void {
    // Would use @react-native-community/netinfo
    // Not implemented in baseline
  }
}

/**
 * Mobile Window Implementation
 */
class MobileWindow implements IWindow {
  private callbacks: WindowCallbacks = {};
  private width = 375; // iPhone default
  private height = 667;
  private title = 'Nova Engine';

  async create(options: WindowOptions): Promise<void> {
    this.title = options.title;
    this.width = options.width;
    this.height = options.height;

    // Mobile apps typically use full screen
    console.log(`Mobile app initialized: ${this.title}`);
  }

  getSize(): { width: number; height: number } {
    // Would query from React Native Dimensions
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
    // Mobile apps are typically always fullscreen
  }

  isFullscreen(): boolean {
    return true;
  }

  setCallbacks(callbacks: WindowCallbacks): void {
    this.callbacks = callbacks;
  }

  requestAnimationFrame(callback: (time: number) => void): number {
    if (typeof requestAnimationFrame !== 'undefined') {
      return requestAnimationFrame(callback);
    }
    // Fallback
    return setTimeout(() => callback(Date.now()), 16) as unknown as number;
  }

  cancelAnimationFrame(handle: number): void {
    if (typeof cancelAnimationFrame !== 'undefined') {
      cancelAnimationFrame(handle);
    } else {
      clearTimeout(handle as any);
    }
  }

  getPixelRatio(): number {
    // Would use React Native PixelRatio
    return 2.0; // Common for mobile devices
  }

  getCanvas(): HTMLCanvasElement | null {
    // Mobile typically uses native rendering, not canvas
    return null;
  }
}

/**
 * Mobile Timing Implementation
 */
class MobileTiming implements ITiming {
  now(): number {
    return Date.now();
  }

  performance(): number {
    if (typeof performance !== 'undefined' && performance.now) {
      return performance.now();
    }
    return Date.now();
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
 * Mobile Platform Implementation
 * For iOS and Android apps via React Native
 */
export class MobilePlatform implements IPlatform {
  public readonly fileSystem: IFileSystem;
  public readonly threading: IThreading;
  public readonly network: INetwork;
  public readonly window: IWindow;
  public readonly timing: ITiming;

  constructor() {
    this.fileSystem = new MobileFileSystem();
    this.threading = new MobileThreading();
    this.network = new MobileNetwork();
    this.window = new MobileWindow();
    this.timing = new MobileTiming();
  }

  getType(): PlatformType {
    return PlatformType.MOBILE;
  }

  getOS(): OSType {
    // Would detect from React Native Platform
    // For now, return based on user agent if available
    if (typeof navigator !== 'undefined') {
      const ua = navigator.userAgent.toLowerCase();
      if (ua.includes('iphone') || ua.includes('ipad')) {
        return OSType.IOS;
      }
      if (ua.includes('android')) {
        return OSType.ANDROID;
      }
    }
    return OSType.ANDROID;
  }

  getVersion(): string {
    return `Mobile/${this.getOS()}`;
  }

  async initialize(): Promise<void> {
    // Mobile-specific initialization
    console.log('Mobile platform initialized');
  }

  async shutdown(): Promise<void> {
    await this.threading.shutdown();
  }
}
