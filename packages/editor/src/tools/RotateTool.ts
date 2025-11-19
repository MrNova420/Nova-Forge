/**
 * RotateTool
 * Tool for rotating entities
 */

import { BaseTool } from './Tool';

export class RotateTool extends BaseTool {
  name = 'Rotate';
  icon = 'rotate-ccw';
  hotkey = 'E';

  private dragStart: { x: number; y: number } | null = null;
  private dragAxis: 'x' | 'y' | 'z' | null = null;
  private initialRotation: { x: number; y: number; z: number } | null = null;
  private snapAngle: number = 15; // degrees
  private enableSnap: boolean = false;

  onActivate(): void {
    super.onActivate();
    // eslint-disable-next-line no-console
    console.log('Rotate tool activated');
  }

  onMouseDown(x: number, y: number, button: number): void {
    super.onMouseDown(x, y, button);

    if (button === 0) {
      this.dragStart = { x, y };

      // TODO: Detect which rotation gizmo was clicked
      this.dragAxis = this.detectGizmoAxis(x, y);

      // Store initial rotation
      // TODO: Get from selected entity
      this.initialRotation = { x: 0, y: 0, z: 0 };
    }
  }

  onMouseMove(x: number, y: number): void {
    if (this.isDragging && this.dragStart && this.dragAxis) {
      const deltaX = x - this.dragStart.x;
      const deltaY = y - this.dragStart.y;

      // Calculate rotation angle based on mouse movement
      this.updateEntityRotation(deltaX, deltaY);
    }
  }

  onMouseUp(x: number, y: number, button: number): void {
    super.onMouseUp(x, y, button);

    this.dragStart = null;
    this.dragAxis = null;
    this.initialRotation = null;
  }

  onKeyDown(key: string): void {
    if (key === 'Shift') {
      this.enableSnap = true;
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
      this.enableSnap = false;
    }
  }

  private detectGizmoAxis(x: number, y: number): 'x' | 'y' | 'z' | null {
    const canvas = document.querySelector('canvas');
    if (!canvas) return null;

    const rect = canvas.getBoundingClientRect();
    const centerX = rect.width / 2;
    const centerY = rect.height / 2;

    const dx = x - centerX;
    const dy = y - centerY;
    const distance = Math.sqrt(dx * dx + dy * dy);

    const gizmoRadius = 80;
    const tolerance = 12;

    // Check if clicking near the rotation circles
    if (Math.abs(distance - gizmoRadius) < tolerance) {
      const angle = Math.atan2(dy, dx);
      const degrees = (angle * 180) / Math.PI;

      // X axis (red - vertical ellipse)
      if (Math.abs(degrees - 90) < 30 || Math.abs(degrees + 90) < 30) {
        return 'x';
      }
      // Y axis (green - full circle)
      if (Math.abs(degrees) < 30 || Math.abs(degrees - 180) < 30) {
        return 'y';
      }
      // Z axis (blue - horizontal ellipse)
      return 'z';
    }

    return null;
  }

  private updateEntityRotation(deltaX: number, deltaY: number): void {
    if (!this.initialRotation) return;

    // Calculate rotation angle (in degrees)
    const rotationSensitivity = 0.5;
    let angle = 0;

    switch (this.dragAxis) {
      case 'x':
        angle = deltaY * rotationSensitivity;
        break;
      case 'y':
        angle = deltaX * rotationSensitivity;
        break;
      case 'z':
        angle = -deltaX * rotationSensitivity;
        break;
    }

    // Apply angle snapping
    if (this.enableSnap) {
      angle = Math.round(angle / this.snapAngle) * this.snapAngle;
    }

    let newRotX = this.initialRotation.x;
    let newRotY = this.initialRotation.y;
    let newRotZ = this.initialRotation.z;

    switch (this.dragAxis) {
      case 'x':
        newRotX += angle;
        break;
      case 'y':
        newRotY += angle;
        break;
      case 'z':
        newRotZ += angle;
        break;
    }

    // Normalize to 0-360 range
    newRotX = ((newRotX % 360) + 360) % 360;
    newRotY = ((newRotY % 360) + 360) % 360;
    newRotZ = ((newRotZ % 360) % 360) % 360;

    // eslint-disable-next-line no-console
    console.log(
      `New rotation: (${newRotX.toFixed(1)}°, ${newRotY.toFixed(1)}°, ${newRotZ.toFixed(1)}°)`
    );
  }

  update(_deltaTime: number): void {
    // Update tool state
  }

  render(context: any): void {
    if (!context) return;

    const canvas = context.canvas;
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const radius = 80;

    context.save();
    context.lineWidth = 3;

    // Draw X axis ring (red - vertical ellipse for YZ plane rotation)
    context.strokeStyle = this.dragAxis === 'x' ? '#FF6666' : '#FF0000';
    context.beginPath();
    context.ellipse(centerX, centerY, radius * 0.3, radius, 0, 0, Math.PI * 2);
    context.stroke();

    // Add axis label
    context.fillStyle = context.strokeStyle;
    context.font = 'bold 16px Arial';
    context.fillText('X', centerX + radius * 0.3 + 10, centerY);

    // Draw Y axis ring (green - full circle for XZ plane rotation)
    context.strokeStyle = this.dragAxis === 'y' ? '#66FF66' : '#00FF00';
    context.beginPath();
    context.arc(centerX, centerY, radius, 0, Math.PI * 2);
    context.stroke();

    // Add axis label
    context.fillStyle = context.strokeStyle;
    context.fillText('Y', centerX, centerY - radius - 10);

    // Draw Z axis ring (blue - horizontal ellipse for XY plane rotation)
    context.strokeStyle = this.dragAxis === 'z' ? '#6666FF' : '#0000FF';
    context.beginPath();
    context.ellipse(centerX, centerY, radius, radius * 0.3, 0, 0, Math.PI * 2);
    context.stroke();

    // Add axis label
    context.fillStyle = context.strokeStyle;
    context.fillText('Z', centerX + radius + 10, centerY);

    // Draw screen-space rotation ring (white outer ring)
    context.strokeStyle = '#FFFFFF';
    context.lineWidth = 2;
    context.setLineDash([5, 5]);
    context.beginPath();
    context.arc(centerX, centerY, radius + 20, 0, Math.PI * 2);
    context.stroke();
    context.setLineDash([]);

    // Draw center sphere
    context.fillStyle = '#FFFFFF';
    context.strokeStyle = '#000000';
    context.lineWidth = 1;
    context.beginPath();
    context.arc(centerX, centerY, 6, 0, Math.PI * 2);
    context.fill();
    context.stroke();

    // Display current rotation angle if actively rotating
    if (this.isDragging && this.dragAxis) {
      const angle = this.getCurrentRotationAngle();
      context.fillStyle = '#FFFFFF';
      context.strokeStyle = '#000000';
      context.lineWidth = 3;
      context.font = 'bold 18px monospace';
      const text = `${angle.toFixed(1)}°`;
      const metrics = context.measureText(text);
      const textX = centerX + radius + 40;
      const textY = centerY - radius - 20;

      // Draw background
      context.fillStyle = 'rgba(0, 0, 0, 0.7)';
      context.fillRect(textX - 5, textY - 20, metrics.width + 10, 30);

      // Draw text
      context.fillStyle = '#FFFFFF';
      context.fillText(text, textX, textY);
    }

    context.restore();
  }

  private getCurrentRotationAngle(): number {
    if (!this.initialRotation) return 0;

    // Calculate current angle based on drag axis
    const deltaX = this.dragStart ? 0 : 0; // Would use actual drag delta
    const deltaY = this.dragStart ? 0 : 0;

    const rotationSensitivity = 0.5;
    switch (this.dragAxis) {
      case 'x':
        return deltaY * rotationSensitivity;
      case 'y':
        return deltaX * rotationSensitivity;
      case 'z':
        return -deltaX * rotationSensitivity;
      default:
        return 0;
    }
  }
}
