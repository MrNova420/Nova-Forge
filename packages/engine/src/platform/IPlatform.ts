/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

/**
 * Platform Abstraction Layer
 *
 * Provides a unified interface for cross-platform functionality across
 * Web, Desktop, Mobile, and Console platforms.
 *
 * @module platform
 */

/**
 * Platform types supported by Nova Engine
 */
export enum PlatformType {
  WEB = 'web',
  DESKTOP = 'desktop',
  MOBILE = 'mobile',
  CONSOLE = 'console',
}

/**
 * Operating system types
 */
export enum OSType {
  WINDOWS = 'windows',
  MACOS = 'macos',
  LINUX = 'linux',
  IOS = 'ios',
  ANDROID = 'android',
  WEB_BROWSER = 'web_browser',
}

/**
 * Window creation options
 */
export interface WindowOptions {
  title: string;
  width: number;
  height: number;
  fullscreen?: boolean;
  resizable?: boolean;
  vsync?: boolean;
  msaa?: number;
}

/**
 * Window events
 */
export interface WindowCallbacks {
  onResize?: (width: number, height: number) => void;
  onClose?: () => void;
  onFocus?: (focused: boolean) => void;
  onMinimize?: (minimized: boolean) => void;
}

/**
 * File system path types
 */
export enum PathType {
  USER_DATA = 'user_data',
  APP_DATA = 'app_data',
  CACHE = 'cache',
  TEMP = 'temp',
  DOCUMENTS = 'documents',
  DOWNLOADS = 'downloads',
}

/**
 * File open modes
 */
export enum FileMode {
  READ = 'read',
  WRITE = 'write',
  APPEND = 'append',
  READ_WRITE = 'read_write',
}

/**
 * File system interface
 */
export interface IFileSystem {
  /**
   * Get a platform-specific path
   */
  getPath(type: PathType): Promise<string>;

  /**
   * Check if a file exists
   */
  exists(path: string): Promise<boolean>;

  /**
   * Read file contents as text
   */
  readText(path: string): Promise<string>;

  /**
   * Read file contents as binary
   */
  readBinary(path: string): Promise<ArrayBuffer>;

  /**
   * Write text to file
   */
  writeText(path: string, content: string): Promise<void>;

  /**
   * Write binary to file
   */
  writeBinary(path: string, content: ArrayBuffer): Promise<void>;

  /**
   * Delete a file
   */
  delete(path: string): Promise<void>;

  /**
   * List files in a directory
   */
  listFiles(path: string): Promise<string[]>;

  /**
   * Create a directory
   */
  createDirectory(path: string): Promise<void>;

  /**
   * Get file size in bytes
   */
  getFileSize(path: string): Promise<number>;

  /**
   * Get file modification time
   */
  getModifiedTime(path: string): Promise<Date>;
}

/**
 * Job/task to be executed on platform threading
 */
export interface IPlatformJob {
  /**
   * Execute the job
   */
  execute(): Promise<void> | void;

  /**
   * Job priority (higher = more important)
   */
  priority?: number;

  /**
   * Job name for debugging
   */
  name?: string;
}

/**
 * Threading interface
 */
export interface IThreading {
  /**
   * Get number of logical CPU cores
   */
  getCoreCount(): number;

  /**
   * Schedule a job to run on a worker thread
   */
  scheduleJob(job: IPlatformJob): Promise<void>;

  /**
   * Schedule multiple jobs in parallel
   */
  scheduleJobs(jobs: IPlatformJob[]): Promise<void>;

  /**
   * Check if threading is supported
   */
  isSupported(): boolean;

  /**
   * Shutdown threading system
   */
  shutdown(): Promise<void>;
}

/**
 * Network request options
 */
export interface NetworkRequest {
  url: string;
  method?: 'GET' | 'POST' | 'PUT' | 'DELETE' | 'PATCH';
  headers?: Record<string, string>;
  body?: string | ArrayBuffer;
  timeout?: number;
}

/**
 * Network response
 */
export interface NetworkResponse {
  status: number;
  statusText: string;
  headers: Record<string, string>;
  body: ArrayBuffer;
  text(): string;
  json(): unknown;
}

/**
 * WebSocket message handler
 */
export type WebSocketHandler = (data: string | ArrayBuffer) => void;

/**
 * WebSocket interface
 */
export interface IWebSocket {
  /**
   * Connect to WebSocket server
   */
  connect(url: string): Promise<void>;

  /**
   * Send data over WebSocket
   */
  send(data: string | ArrayBuffer): void;

  /**
   * Set message handler
   */
  onMessage(handler: WebSocketHandler): void;

  /**
   * Set close handler
   */
  onClose(handler: () => void): void;

  /**
   * Set error handler
   */
  onError(handler: (error: Error) => void): void;

  /**
   * Close WebSocket connection
   */
  close(): void;

  /**
   * Check if connected
   */
  isConnected(): boolean;
}

/**
 * Network interface
 */
export interface INetwork {
  /**
   * Make an HTTP request
   */
  request(options: NetworkRequest): Promise<NetworkResponse>;

  /**
   * Create a WebSocket connection
   */
  createWebSocket(): IWebSocket;

  /**
   * Check if network is available
   */
  isOnline(): boolean;

  /**
   * Add online/offline listener
   */
  onConnectionChange(handler: (online: boolean) => void): void;
}

/**
 * Window management interface
 */
export interface IWindow {
  /**
   * Create and show a window
   */
  create(options: WindowOptions): Promise<void>;

  /**
   * Get window dimensions
   */
  getSize(): { width: number; height: number };

  /**
   * Set window size
   */
  setSize(width: number, height: number): void;

  /**
   * Get window title
   */
  getTitle(): string;

  /**
   * Set window title
   */
  setTitle(title: string): void;

  /**
   * Toggle fullscreen
   */
  setFullscreen(fullscreen: boolean): void;

  /**
   * Check if fullscreen
   */
  isFullscreen(): boolean;

  /**
   * Set window callbacks
   */
  setCallbacks(callbacks: WindowCallbacks): void;

  /**
   * Request animation frame
   */
  requestAnimationFrame(callback: (time: number) => void): number;

  /**
   * Cancel animation frame
   */
  cancelAnimationFrame(handle: number): void;

  /**
   * Get device pixel ratio
   */
  getPixelRatio(): number;

  /**
   * Get canvas/rendering context
   */
  getCanvas(): HTMLCanvasElement | OffscreenCanvas | null;
}

/**
 * Timing interface
 */
export interface ITiming {
  /**
   * Get high-resolution timestamp in milliseconds
   */
  now(): number;

  /**
   * Get performance timestamp (monotonic, not affected by clock changes)
   */
  performance(): number;

  /**
   * Sleep for specified milliseconds (async)
   */
  sleep(ms: number): Promise<void>;

  /**
   * Set interval timer
   */
  setInterval(callback: () => void, ms: number): number;

  /**
   * Clear interval timer
   */
  clearInterval(handle: number): void;

  /**
   * Set timeout timer
   */
  setTimeout(callback: () => void, ms: number): number;

  /**
   * Clear timeout timer
   */
  clearTimeout(handle: number): void;
}

/**
 * Main platform interface
 *
 * Provides unified access to platform-specific functionality
 */
export interface IPlatform {
  /**
   * Get platform type
   */
  getType(): PlatformType;

  /**
   * Get operating system type
   */
  getOS(): OSType;

  /**
   * Get platform name/version
   */
  getVersion(): string;

  /**
   * Initialize platform
   */
  initialize(): Promise<void>;

  /**
   * Shutdown platform
   */
  shutdown(): Promise<void>;

  /**
   * File system interface
   */
  readonly fileSystem: IFileSystem;

  /**
   * Threading interface
   */
  readonly threading: IThreading;

  /**
   * Network interface
   */
  readonly network: INetwork;

  /**
   * Window interface
   */
  readonly window: IWindow;

  /**
   * Timing interface
   */
  readonly timing: ITiming;
}
