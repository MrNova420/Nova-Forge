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
 * ToolManager
 * Manages editor tools and tool switching
 */

import { Tool } from './Tool';
import { SelectTool } from './SelectTool';
import { TranslateTool } from './TranslateTool';
import { RotateTool } from './RotateTool';
import { ScaleTool } from './ScaleTool';

export class ToolManager {
  private tools: Map<string, Tool> = new Map();
  private activeTool: Tool | null = null;
  private hotkeyMap: Map<string, string> = new Map();
  
  constructor() {
    this.registerDefaultTools();
  }
  
  private registerDefaultTools(): void {
    this.registerTool('select', new SelectTool());
    this.registerTool('translate', new TranslateTool());
    this.registerTool('rotate', new RotateTool());
    this.registerTool('scale', new ScaleTool());
  }
  
  registerTool(id: string, tool: Tool): void {
    this.tools.set(id, tool);
    this.hotkeyMap.set(tool.hotkey.toLowerCase(), id);
  }
  
  setActiveTool(id: string): boolean {
    const tool = this.tools.get(id);
    if (!tool) {
      console.warn(`Tool '${id}' not found`);
      return false;
    }
    
    // Deactivate current tool
    if (this.activeTool) {
      this.activeTool.onDeactivate();
    }
    
    // Activate new tool
    this.activeTool = tool;
    this.activeTool.onActivate();
    
    return true;
  }
  
  getActiveTool(): Tool | null {
    return this.activeTool;
  }
  
  handleHotkey(key: string): boolean {
    const toolId = this.hotkeyMap.get(key.toLowerCase());
    if (toolId) {
      return this.setActiveTool(toolId);
    }
    return false;
  }
  
  handleMouseDown(x: number, y: number, button: number): void {
    if (this.activeTool) {
      this.activeTool.onMouseDown(x, y, button);
    }
  }
  
  handleMouseMove(x: number, y: number): void {
    if (this.activeTool) {
      this.activeTool.onMouseMove(x, y);
    }
  }
  
  handleMouseUp(x: number, y: number, button: number): void {
    if (this.activeTool) {
      this.activeTool.onMouseUp(x, y, button);
    }
  }
  
  handleKeyDown(key: string): void {
    // Check for hotkey first
    if (this.handleHotkey(key)) {
      return;
    }
    
    // Pass to active tool
    if (this.activeTool) {
      this.activeTool.onKeyDown(key);
    }
  }
  
  handleKeyUp(key: string): void {
    if (this.activeTool) {
      this.activeTool.onKeyUp(key);
    }
  }
  
  update(deltaTime: number): void {
    if (this.activeTool) {
      this.activeTool.update(deltaTime);
    }
  }
  
  render(context: any): void {
    if (this.activeTool) {
      this.activeTool.render(context);
    }
  }
}
