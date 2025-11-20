/**
 * @fileoverview Platform Abstraction Layer - Core Interface
 * @module @nova-engine/platform
 *
 * Provides a unified interface for platform-specific operations across
 * web, desktop, mobile, and other target platforms.
 *
 * Following the autonomous development guide Phase 1.1
 * Research insights from Unreal (IPlatform), Unity (internal platform layer), Godot (OS class)
 */

/**
 * Window configuration options
 */
export interface WindowConfig {
  /** Window title */
  title: string;
  /** Window width in pixels */
  width: number;
  /** Window height in pixels */
  height: number;
  /** Whether the window should be fullscreen */
  fullscreen?: boolean;
  /** Whether the window should be resizable */
  resizable?: boolean;
  /** Whether the window should have decorations (titlebar, etc) */
  decorated?: boolean;
  /** Initial window position X */
  x?: number;
  /** Initial window position Y */
  y?: number;
  /** VSync enabled */
  vsync?: boolean;
}

/**
 * Platform-agnostic window interface
 */
export interface IWindow {
  /** Get window width */
  getWidth(): number;
  /** Get window height */
  getHeight(): number;
  /** Set window size */
  setSize(width: number, height: number): void;
  /** Get window title */
  getTitle(): string;
  /** Set window title */
  setTitle(title: string): void;
  /** Check if window is fullscreen */
  isFullscreen(): boolean;
  /** Toggle fullscreen mode */
  setFullscreen(fullscreen: boolean): void;
  /** Get window position */
  getPosition(): { x: number; y: number };
  /** Set window position */
  setPosition(x: number, y: number): void;
  /** Check if window is visible */
  isVisible(): boolean;
  /** Show/hide window */
  setVisible(visible: boolean): void;
  /** Check if window is focused */
  isFocused(): boolean;
  /** Focus the window */
  focus(): void;
  /** Close the window */
  close(): void;
  /** Get the underlying canvas element (web) or native window handle */
  getNativeHandle(): unknown;
  /** Request animation frame */
  requestAnimationFrame(callback: (time: number) => void): number;
  /** Cancel animation frame */
  cancelAnimationFrame(handle: number): void;
}

/**
 * Platform-agnostic worker interface for threading
 */
export interface IWorker {
  /** Post a message to the worker */
  postMessage(message: unknown, transfer?: Transferable[]): void;
  /** Set message handler */
  onMessage(handler: (message: unknown) => void): void;
  /** Set error handler */
  onError(handler: (error: Error) => void): void;
  /** Terminate the worker */
  terminate(): void;
  /** Check if worker is running */
  isRunning(): boolean;
}

/**
 * File/directory statistics
 */
export interface FileStats {
  /** Size in bytes */
  size: number;
  /** Last modified timestamp */
  modified: number;
  /** Creation timestamp */
  created: number;
  /** Whether this is a directory */
  isDirectory: boolean;
  /** Whether this is a file */
  isFile: boolean;
  /** Permissions (platform-specific) */
  permissions?: number;
}

/**
 * Platform-agnostic file system interface
 */
export interface IFileSystem {
  /** Read file as ArrayBuffer */
  readFile(path: string): Promise<ArrayBuffer>;
  /** Read file as text */
  readTextFile(path: string): Promise<string>;
  /** Write file from ArrayBuffer */
  writeFile(path: string, data: ArrayBuffer): Promise<void>;
  /** Write file from text */
  writeTextFile(path: string, data: string): Promise<void>;
  /** Check if file/directory exists */
  exists(path: string): Promise<boolean>;
  /** List directory contents */
  listDirectory(path: string): Promise<string[]>;
  /** Create directory */
  createDirectory(path: string, recursive?: boolean): Promise<void>;
  /** Delete file */
  deleteFile(path: string): Promise<void>;
  /** Delete directory */
  deleteDirectory(path: string, recursive?: boolean): Promise<void>;
  /** Get file stats (size, modified time, etc) */
  getStats(path: string): Promise<FileStats>;
  /** Copy file */
  copyFile(source: string, destination: string): Promise<void>;
  /** Move/rename file */
  moveFile(source: string, destination: string): Promise<void>;
  /** Watch file/directory for changes */
  watch(
    path: string,
    callback: (event: 'create' | 'modify' | 'delete', path: string) => void
  ): () => void;
}

/**
 * Fetch options for network requests
 */
export interface FetchOptions {
  method?: 'GET' | 'POST' | 'PUT' | 'DELETE' | 'PATCH' | 'HEAD' | 'OPTIONS';
  headers?: Record<string, string>;
  body?: string | ArrayBuffer | Blob;
  timeout?: number;
  credentials?: 'omit' | 'same-origin' | 'include';
  mode?: 'cors' | 'no-cors' | 'same-origin';
  cache?: 'default' | 'no-cache' | 'reload' | 'force-cache';
}

/**
 * Network response
 */
export interface Response {
  status: number;
  statusText: string;
  headers: Record<string, string>;
  ok: boolean;
  arrayBuffer(): Promise<ArrayBuffer>;
  text(): Promise<string>;
  json<T = unknown>(): Promise<T>;
  blob(): Promise<Blob>;
}

/**
 * WebSocket interface
 */
export interface IWebSocket {
  /** Send data through the socket */
  send(data: string | ArrayBuffer): void;
  /** Close the socket */
  close(code?: number, reason?: string): void;
  /** Set message handler */
  onMessage(handler: (data: string | ArrayBuffer) => void): void;
  /** Set open handler */
  onOpen(handler: () => void): void;
  /** Set close handler */
  onClose(handler: (code: number, reason: string) => void): void;
  /** Set error handler */
  onError(handler: (error: Error) => void): void;
  /** Get current ready state */
  getReadyState(): number;
  /** Get buffered amount */
  getBufferedAmount(): number;
}

/**
 * Input device interface
 */
export interface IInputDevice {
  /** Device type */
  type: 'keyboard' | 'mouse' | 'gamepad' | 'touch';
  /** Device ID */
  id: string;
  /** Device name */
  name: string;
  /** Whether device is connected */
  isConnected(): boolean;
  /** Get device capabilities */
  getCapabilities(): Record<string, unknown>;
}

/**
 * Platform capabilities flags
 */
export interface PlatformCapabilities {
  /** Supports web workers / threads */
  supportsWorkers: boolean;
  /** Supports WebGL */
  supportsWebGL: boolean;
  /** Supports WebGL 2 */
  supportsWebGL2: boolean;
  /** Supports WebGPU */
  supportsWebGPU: boolean;
  /** Supports Web Audio */
  supportsWebAudio: boolean;
  /** Supports IndexedDB */
  supportsIndexedDB: boolean;
  /** Supports LocalStorage */
  supportsLocalStorage: boolean;
  /** Supports touch input */
  supportsTouch: boolean;
  /** Supports gamepad input */
  supportsGamepad: boolean;
  /** Supports pointer lock */
  supportsPointerLock: boolean;
  /** Supports fullscreen API */
  supportsFullscreen: boolean;
  /** Maximum texture size */
  maxTextureSize: number;
  /** Maximum render targets */
  maxRenderTargets: number;
  /** Available memory (if detectable) */
  availableMemory?: number;
  /** Device pixel ratio */
  devicePixelRatio: number;
}

/**
 * Platform information
 */
export interface PlatformInfo {
  /** Platform name (web, desktop, mobile) */
  name: string;
  /** Operating system */
  os: string;
  /** Architecture (x64, arm64, etc) */
  arch: string;
  /** Browser/runtime version */
  version: string;
  /** Device type (desktop, mobile, tablet) */
  deviceType: 'desktop' | 'mobile' | 'tablet' | 'unknown';
  /** Is development build */
  isDevelopment: boolean;
  /** Is production build */
  isProduction: boolean;
}

/**
 * Main platform interface that all platform implementations must provide
 *
 * This abstraction allows Nova Engine to run on multiple platforms
 * (web, desktop, mobile, console) with the same codebase.
 *
 * Inspired by:
 * - Unreal Engine's IPlatform interface
 * - Unity's internal platform layer
 * - Godot's OS class
 */
export interface IPlatform {
  /**
   * Initialize the platform
   * @param config Platform-specific configuration
   */
  initialize(config?: Record<string, unknown>): Promise<void>;

  /**
   * Shutdown the platform and cleanup resources
   */
  shutdown(): Promise<void>;

  /**
   * Get platform name
   */
  getName(): string;

  /**
   * Get platform information
   */
  getInfo(): PlatformInfo;

  /**
   * Get platform capabilities
   */
  getCapabilities(): PlatformCapabilities;

  // Window Management

  /**
   * Create a new window
   * @param config Window configuration
   */
  createWindow(config: WindowConfig): IWindow;

  /**
   * Get all active windows
   */
  getWindows(): IWindow[];

  /**
   * Get the main window
   */
  getMainWindow(): IWindow | null;

  // File System

  /**
   * Get the file system interface
   */
  getFileSystem(): IFileSystem;

  // Threading

  /**
   * Create a worker for multi-threading
   * @param scriptPath Path to worker script or script content
   */
  createWorker(scriptPath: string): IWorker;

  /**
   * Get number of available CPU cores
   */
  getHardwareConcurrency(): number;

  // Time

  /**
   * Get high-resolution timestamp in milliseconds
   * Uses performance.now() on web, similar high-res timers on other platforms
   */
  now(): number;

  /**
   * Sleep for specified milliseconds
   * @param ms Milliseconds to sleep
   */
  sleep(ms: number): Promise<void>;

  // Input

  /**
   * Get all available input devices
   */
  getInputDevices(): IInputDevice[];

  // Network

  /**
   * Make an HTTP request
   * @param url Request URL
   * @param options Request options
   */
  fetch(url: string, options?: FetchOptions): Promise<Response>;

  /**
   * Create a WebSocket connection
   * @param url WebSocket URL
   * @param protocols Optional protocols
   */
  createWebSocket(url: string, protocols?: string | string[]): IWebSocket;

  // Clipboard

  /**
   * Read text from clipboard
   */
  readClipboard(): Promise<string>;

  /**
   * Write text to clipboard
   * @param text Text to write
   */
  writeClipboard(text: string): Promise<void>;

  // Dialog

  /**
   * Show alert dialog
   * @param message Message to display
   */
  alert(message: string): Promise<void>;

  /**
   * Show confirm dialog
   * @param message Message to display
   * @returns True if confirmed, false if cancelled
   */
  confirm(message: string): Promise<boolean>;

  /**
   * Show prompt dialog
   * @param message Message to display
   * @param defaultValue Default input value
   * @returns Input value or null if cancelled
   */
  prompt(message: string, defaultValue?: string): Promise<string | null>;

  // Events

  /**
   * Add event listener
   * @param event Event name
   * @param handler Event handler
   */
  addEventListener(event: string, handler: (...args: unknown[]) => void): void;

  /**
   * Remove event listener
   * @param event Event name
   * @param handler Event handler
   */
  removeEventListener(
    event: string,
    handler: (...args: unknown[]) => void
  ): void;
}
