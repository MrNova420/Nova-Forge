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
 * Tool Interface
 * Base interface for all editor tools
 */

export interface Tool {
  name: string;
  icon: string;
  hotkey: string;

  /**
   * Called when tool is activated
   */
  onActivate(): void;

  /**
   * Called when tool is deactivated
   */
  onDeactivate(): void;

  /**
   * Handle mouse down event
   */
  onMouseDown(x: number, y: number, button: number): void;

  /**
   * Handle mouse move event
   */
  onMouseMove(x: number, y: number): void;

  /**
   * Handle mouse up event
   */
  onMouseUp(x: number, y: number, button: number): void;

  /**
   * Handle key down event
   */
  onKeyDown(key: string): void;

  /**
   * Handle key up event
   */
  onKeyUp(key: string): void;

  /**
   * Update tool state
   */
  update(deltaTime: number): void;

  /**
   * Render tool gizmos
   */
  render(context: any): void;
}

export abstract class BaseTool implements Tool {
  abstract name: string;
  abstract icon: string;
  abstract hotkey: string;

  protected isActive: boolean = false;
  protected isDragging: boolean = false;

  onActivate(): void {
    this.isActive = true;
  }

  onDeactivate(): void {
    this.isActive = false;
    this.isDragging = false;
  }

  onMouseDown(x: number, y: number, button: number): void {
    if (button === 0) {
      // Left mouse button
      this.isDragging = true;
    }
  }

  onMouseMove(_x: number, _y: number): void {
    // Override in subclasses
  }

  onMouseUp(x: number, y: number, button: number): void {
    if (button === 0) {
      this.isDragging = false;
    }
  }

  onKeyDown(_key: string): void {
    // Override in subclasses
  }

  onKeyUp(_key: string): void {
    // Override in subclasses
  }

  update(_deltaTime: number): void {
    // Override in subclasses
  }

  render(_context: any): void {
    // Override in subclasses
  }
}
