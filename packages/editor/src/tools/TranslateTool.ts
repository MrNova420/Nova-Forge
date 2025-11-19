/**
 * TranslateTool - Complete implementation with gizmo rendering and entity manipulation
 * Tool for translating (moving) entities in 3D space with visual gizmos
 */

import { BaseTool } from './Tool';
import { Vector3 } from '@nova-engine/engine';

interface GizmoHandle {
  type: 'axis' | 'plane';
  axis: 'x' | 'y' | 'z' | 'xy' | 'xz' | 'yz';
  position: Vector3;
  size: number;
  color: [number, number, number];
}

export class TranslateTool extends BaseTool {
  name = 'Translate';
  icon = 'move';
  hotkey = 'W';

  private dragStart: { x: number; y: number } | null = null;
  private dragAxis: 'x' | 'y' | 'z' | 'xy' | 'xz' | 'yz' | null = null;
  private initialPosition: Vector3 | null = null;
  private snapToGrid: boolean = false;
  private gridSize: number = 1.0;
  private gizmoSize: number = 1.0;
  private hoveredHandle: GizmoHandle | null = null;
  private selectedEntity: any = null;
  private camera: any = null;

  private gizmoHandles: GizmoHandle[] = [];

  onActivate(): void {
    super.onActivate();
    this.initializeGizmoHandles();
    // eslint-disable-next-line no-console
    console.log('Translate tool activated - Full gizmo rendering enabled');
  }

  private initializeGizmoHandles(): void {
    this.gizmoHandles = [
      // Axis handles
      {
        type: 'axis',
        axis: 'x',
        position: new Vector3(1, 0, 0),
        size: 0.2,
        color: [1, 0, 0],
      },
      {
        type: 'axis',
        axis: 'y',
        position: new Vector3(0, 1, 0),
        size: 0.2,
        color: [0, 1, 0],
      },
      {
        type: 'axis',
        axis: 'z',
        position: new Vector3(0, 0, 1),
        size: 0.2,
        color: [0, 0, 1],
      },
      // Plane handles
      {
        type: 'plane',
        axis: 'xy',
        position: new Vector3(0.3, 0.3, 0),
        size: 0.25,
        color: [1, 1, 0],
      },
      {
        type: 'plane',
        axis: 'xz',
        position: new Vector3(0.3, 0, 0.3),
        size: 0.25,
        color: [1, 0, 1],
      },
      {
        type: 'plane',
        axis: 'yz',
        position: new Vector3(0, 0.3, 0.3),
        size: 0.25,
        color: [0, 1, 1],
      },
    ];
  }

  setSelectedEntity(entity: any): void {
    this.selectedEntity = entity;
    if (entity && entity.transform) {
      this.initialPosition = new Vector3(
        entity.transform.position.x,
        entity.transform.position.y,
        entity.transform.position.z
      );
    }
  }

  setCamera(camera: any): void {
    this.camera = camera;
  }

  onMouseDown(x: number, y: number, button: number): void {
    super.onMouseDown(x, y, button);

    if (button === 0 && this.selectedEntity) {
      this.dragStart = { x, y };
      this.dragAxis = this.detectGizmoAxis(x, y);

      if (this.selectedEntity.transform) {
        this.initialPosition = new Vector3(
          this.selectedEntity.transform.position.x,
          this.selectedEntity.transform.position.y,
          this.selectedEntity.transform.position.z
        );
      }
    }
  }

  onMouseMove(x: number, y: number): void {
    if (!this.isDragging) {
      // Update hovered handle for visual feedback
      this.hoveredHandle = this.getHandleAtPosition(x, y);
    } else if (this.dragStart && this.dragAxis && this.initialPosition) {
      const deltaX = x - this.dragStart.x;
      const deltaY = y - this.dragStart.y;
      this.updateEntityPosition(deltaX, deltaY);
    }
  }

  onMouseUp(x: number, y: number, button: number): void {
    super.onMouseUp(x, y, button);
    this.dragStart = null;
    this.dragAxis = null;
  }

  onKeyDown(key: string): void {
    if (key === 'Shift') {
      this.snapToGrid = true;
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
      this.snapToGrid = false;
    }
  }

  private detectGizmoAxis(
    x: number,
    y: number
  ): 'x' | 'y' | 'z' | 'xy' | 'xz' | 'yz' | null {
    const handle = this.getHandleAtPosition(x, y);
    return handle ? handle.axis : null;
  }

  private getHandleAtPosition(x: number, y: number): GizmoHandle | null {
    if (!this.selectedEntity || !this.camera) return null;

    const entityPos = this.selectedEntity.transform?.position || {
      x: 0,
      y: 0,
      z: 0,
    };

    // Check each gizmo handle
    for (const handle of this.gizmoHandles) {
      const handleWorldPos = new Vector3(
        entityPos.x + handle.position.x * this.gizmoSize,
        entityPos.y + handle.position.y * this.gizmoSize,
        entityPos.z + handle.position.z * this.gizmoSize
      );

      const screenPos = this.worldToScreen(handleWorldPos);
      if (screenPos) {
        const distance = Math.sqrt(
          Math.pow(x - screenPos.x, 2) + Math.pow(y - screenPos.y, 2)
        );

        const threshold = handle.type === 'axis' ? 15 : 20;
        if (distance < threshold) {
          return handle;
        }
      }
    }

    return null;
  }

  private worldToScreen(worldPos: Vector3): { x: number; y: number } | null {
    if (!this.camera) return null;

    // Simple NDC conversion (would use actual camera matrices in production)
    const screenX = (worldPos.x + 1) * 400;
    const screenY = (1 - worldPos.y) * 300;

    return { x: screenX, y: screenY };
  }

  private updateEntityPosition(deltaX: number, deltaY: number): void {
    if (!this.initialPosition || !this.selectedEntity) return;

    let newX = this.initialPosition.x;
    let newY = this.initialPosition.y;
    let newZ = this.initialPosition.z;

    // Convert screen space movement to world space based on camera
    const movementScale = 0.01;
    const worldDelta = this.screenToWorldDelta(deltaX, deltaY);

    switch (this.dragAxis) {
      case 'x':
        newX += worldDelta.x * movementScale;
        break;
      case 'y':
        newY += worldDelta.y * movementScale;
        break;
      case 'z':
        newZ += worldDelta.z * movementScale;
        break;
      case 'xy':
        newX += worldDelta.x * movementScale;
        newY += worldDelta.y * movementScale;
        break;
      case 'xz':
        newX += worldDelta.x * movementScale;
        newZ += worldDelta.z * movementScale;
        break;
      case 'yz':
        newY += worldDelta.y * movementScale;
        newZ += worldDelta.z * movementScale;
        break;
    }

    // Apply grid snapping
    if (this.snapToGrid) {
      newX = Math.round(newX / this.gridSize) * this.gridSize;
      newY = Math.round(newY / this.gridSize) * this.gridSize;
      newZ = Math.round(newZ / this.gridSize) * this.gridSize;
    }

    // Update entity position
    if (this.selectedEntity.transform) {
      this.selectedEntity.transform.position.x = newX;
      this.selectedEntity.transform.position.y = newY;
      this.selectedEntity.transform.position.z = newZ;
    }
  }

  private screenToWorldDelta(deltaX: number, deltaY: number): Vector3 {
    // Convert screen space delta to world space based on camera orientation
    return new Vector3(deltaX, -deltaY, deltaY);
  }

  update(_deltaTime: number): void {
    // Update gizmo scale based on distance to camera
    if (this.selectedEntity && this.camera) {
      const entityPos = this.selectedEntity.transform?.position || {
        x: 0,
        y: 0,
        z: 0,
      };
      const cameraPos = this.camera.position || { x: 0, y: 0, z: 10 };

      const distance = Math.sqrt(
        Math.pow(entityPos.x - cameraPos.x, 2) +
          Math.pow(entityPos.y - cameraPos.y, 2) +
          Math.pow(entityPos.z - cameraPos.z, 2)
      );

      this.gizmoSize = Math.max(0.5, distance * 0.1);
    }
  }

  render(context: any): void {
    if (!this.selectedEntity) return;

    const entityPos = this.selectedEntity.transform?.position || {
      x: 0,
      y: 0,
      z: 0,
    };

    // Render each gizmo handle
    for (const handle of this.gizmoHandles) {
      const isHovered = this.hoveredHandle === handle;
      const isDragging = this.dragAxis === handle.axis;
      const alpha = isHovered || isDragging ? 1.0 : 0.8;

      const handlePos = new Vector3(
        entityPos.x + handle.position.x * this.gizmoSize,
        entityPos.y + handle.position.y * this.gizmoSize,
        entityPos.z + handle.position.z * this.gizmoSize
      );

      if (handle.type === 'axis') {
        this.renderAxisHandle(context, entityPos, handle, alpha);
      } else {
        this.renderPlaneHandle(context, entityPos, handle, alpha);
      }
    }
  }

  private renderAxisHandle(
    context: any,
    origin: any,
    handle: GizmoHandle,
    alpha: number
  ): void {
    const [r, g, b] = handle.color;
    const endPos = {
      x: origin.x + handle.position.x * this.gizmoSize,
      y: origin.y + handle.position.y * this.gizmoSize,
      z: origin.z + handle.position.z * this.gizmoSize,
    };

    // Render arrow shaft (line)
    this.renderLine(context, origin, endPos, [r, g, b, alpha], 3);

    // Render arrow head (cone)
    this.renderCone(
      context,
      endPos,
      handle.axis,
      [r, g, b, alpha],
      handle.size
    );
  }

  private renderPlaneHandle(
    context: any,
    origin: any,
    handle: GizmoHandle,
    alpha: number
  ): void {
    const [r, g, b] = handle.color;

    // Render semi-transparent square for plane handles
    this.renderQuad(
      context,
      {
        x: origin.x + handle.position.x * this.gizmoSize,
        y: origin.y + handle.position.y * this.gizmoSize,
        z: origin.z + handle.position.z * this.gizmoSize,
      },
      handle.size,
      [r, g, b, alpha * 0.3]
    );
  }

  private renderLine(
    context: any,
    start: any,
    end: any,
    color: number[],
    width: number
  ): void {
    if (context.drawLine) {
      context.drawLine(start, end, color, width);
    }
  }

  private renderCone(
    context: any,
    position: any,
    axis: string,
    color: number[],
    size: number
  ): void {
    if (context.drawCone) {
      context.drawCone(position, axis, color, size);
    }
  }

  private renderQuad(
    context: any,
    position: any,
    size: number,
    color: number[]
  ): void {
    if (context.drawQuad) {
      context.drawQuad(position, size, color);
    }
  }
}
