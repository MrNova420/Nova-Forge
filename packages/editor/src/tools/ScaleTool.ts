/**
 * ScaleTool
 * Tool for scaling entities
 */

import { BaseTool } from './Tool';

export class ScaleTool extends BaseTool {
  name = 'Scale';
  icon = 'maximize';
  hotkey = 'R';
  
  private dragStart: { x: number; y: number } | null = null;
  private dragAxis: 'x' | 'y' | 'z' | 'uniform' | null = null;
  private initialScale: { x: number; y: number; z: number } | null = null;
  private uniformScale: boolean = false;
  
  onActivate(): void {
    super.onActivate();
    console.log('Scale tool activated');
  }
  
  onMouseDown(x: number, y: number, button: number): void {
    super.onMouseDown(x, y, button);
    
    if (button === 0) {
      this.dragStart = { x, y };
      
      // TODO: Detect which scale gizmo was clicked
      this.dragAxis = this.detectGizmoAxis(x, y);
      
      // Store initial scale
      // TODO: Get from selected entity
      this.initialScale = { x: 1, y: 1, z: 1 };
    }
  }
  
  onMouseMove(x: number, y: number): void {
    if (this.isDragging && this.dragStart && this.dragAxis) {
      const deltaX = x - this.dragStart.x;
      const deltaY = y - this.dragStart.y;
      
      // Calculate scale factor based on mouse movement
      this.updateEntityScale(deltaX, deltaY);
    }
  }
  
  onMouseUp(x: number, y: number, button: number): void {
    super.onMouseUp(x, y, button);
    
    this.dragStart = null;
    this.dragAxis = null;
    this.initialScale = null;
  }
  
  onKeyDown(key: string): void {
    if (key === 'Shift') {
      this.uniformScale = true;
      this.dragAxis = 'uniform';
    } else if (key === 'x' || key === 'X') {
      this.dragAxis = 'x';
    } else if (key === 'y' || key === 'Y') {
      this.dragAxis = 'y';
    } else if (key === 'z' || key === 'Z') {
      this.dragAxis = 'z';
    }
  }
  
  onKeyUp(key: string): void {
    if (key === 'Shift') {
      this.uniformScale = false;
    }
  }
  
  private detectGizmoAxis(x: number, y: number): 'x' | 'y' | 'z' | 'uniform' | null {
    // Gizmo handle positions (in screen space)
    const gizmoHandles = {
      x: { x: 100, y: 50, size: 10 },
      y: { x: 50, y: 100, size: 10 },
      z: { x: 50, y: 50, size: 10 },
      uniform: { x: 50, y: 50, size: 8 }
    };
    
    // Check which handle is closest to click position
    for (const [axis, handle] of Object.entries(gizmoHandles)) {
      const dx = x - handle.x;
      const dy = y - handle.y;
      const distance = Math.sqrt(dx * dx + dy * dy);
      
      if (distance < handle.size) {
        return axis as 'x' | 'y' | 'z' | 'uniform';
      }
    }
    
    return null;
  }
  
  private updateEntityScale(deltaX: number, deltaY: number): void {
    if (!this.initialScale) return;
    
    // Calculate scale factor
    const scaleSensitivity = 0.01;
    const delta = (deltaX + deltaY) * scaleSensitivity;
    const scaleFactor = 1 + delta;
    
    // Prevent negative or zero scale
    const clampedFactor = Math.max(0.01, scaleFactor);
    
    let newScaleX = this.initialScale.x;
    let newScaleY = this.initialScale.y;
    let newScaleZ = this.initialScale.z;
    
    if (this.dragAxis === 'uniform' || this.uniformScale) {
      // Uniform scaling
      newScaleX *= clampedFactor;
      newScaleY *= clampedFactor;
      newScaleZ *= clampedFactor;
    } else {
      // Axis-specific scaling
      switch (this.dragAxis) {
        case 'x':
          newScaleX *= clampedFactor;
          break;
        case 'y':
          newScaleY *= clampedFactor;
          break;
        case 'z':
          newScaleZ *= clampedFactor;
          break;
      }
    }
    
    // Clamp scale values
    newScaleX = Math.max(0.01, newScaleX);
    newScaleY = Math.max(0.01, newScaleY);
    newScaleZ = Math.max(0.01, newScaleZ);
    
    console.log(`New scale: (${newScaleX.toFixed(2)}, ${newScaleY.toFixed(2)}, ${newScaleZ.toFixed(2)})`);
  }
  
  update(deltaTime: number): void {
    // Update tool state if needed
  }
  
  render(context: any): void {
    if (!context || !context.ctx) return;
    
    const ctx = context.ctx;
    const centerX = context.width / 2;
    const centerY = context.height / 2;
    const gizmoSize = 80;
    
    // Save context state
    ctx.save();
    
    // Draw X axis (red) - horizontal line with cube
    ctx.strokeStyle = this.dragAxis === 'x' ? '#ff6666' : '#ff0000';
    ctx.lineWidth = this.dragAxis === 'x' ? 3 : 2;
    ctx.beginPath();
    ctx.moveTo(centerX, centerY);
    ctx.lineTo(centerX + gizmoSize, centerY);
    ctx.stroke();
    
    // X axis cube handle
    ctx.fillStyle = this.dragAxis === 'x' ? '#ff6666' : '#ff0000';
    ctx.fillRect(centerX + gizmoSize - 5, centerY - 5, 10, 10);
    
    // Draw Y axis (green) - vertical line with cube
    ctx.strokeStyle = this.dragAxis === 'y' ? '#66ff66' : '#00ff00';
    ctx.lineWidth = this.dragAxis === 'y' ? 3 : 2;
    ctx.beginPath();
    ctx.moveTo(centerX, centerY);
    ctx.lineTo(centerX, centerY - gizmoSize);
    ctx.stroke();
    
    // Y axis cube handle
    ctx.fillStyle = this.dragAxis === 'y' ? '#66ff66' : '#00ff00';
    ctx.fillRect(centerX - 5, centerY - gizmoSize - 5, 10, 10);
    
    // Draw Z axis (blue) - diagonal line with cube
    ctx.strokeStyle = this.dragAxis === 'z' ? '#6666ff' : '#0000ff';
    ctx.lineWidth = this.dragAxis === 'z' ? 3 : 2;
    ctx.beginPath();
    ctx.moveTo(centerX, centerY);
    ctx.lineTo(centerX + gizmoSize * 0.6, centerY + gizmoSize * 0.6);
    ctx.stroke();
    
    // Z axis cube handle
    ctx.fillStyle = this.dragAxis === 'z' ? '#6666ff' : '#0000ff';
    ctx.fillRect(centerX + gizmoSize * 0.6 - 5, centerY + gizmoSize * 0.6 - 5, 10, 10);
    
    // Draw center cube for uniform scaling (white)
    ctx.fillStyle = this.dragAxis === 'uniform' ? '#cccccc' : '#ffffff';
    ctx.strokeStyle = '#000000';
    ctx.lineWidth = 1;
    ctx.fillRect(centerX - 6, centerY - 6, 12, 12);
    ctx.strokeRect(centerX - 6, centerY - 6, 12, 12);
    
    // Restore context state
    ctx.restore();
  }
}
