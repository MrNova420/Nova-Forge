/**
 * SelectTool
 * Tool for selecting entities in the scene
 */

import { BaseTool } from './Tool';

export class SelectTool extends BaseTool {
  name = 'Select';
  icon = 'pointer';
  hotkey = 'Q';

  private selectedEntities: Set<string> = new Set();
  private selectionBox: {
    start: { x: number; y: number };
    end: { x: number; y: number };
  } | null = null;

  onActivate(): void {
    super.onActivate();
    // eslint-disable-next-line no-console
    console.log('Select tool activated');
  }

  onMouseDown(x: number, y: number, button: number): void {
    super.onMouseDown(x, y, button);

    if (button === 0) {
      // Start selection box
      this.selectionBox = {
        start: { x, y },
        end: { x, y },
      };

      // TODO: Raycast to select entity under cursor
      this.performRaycast(x, y);
    }
  }

  onMouseMove(x: number, y: number): void {
    if (this.isDragging && this.selectionBox) {
      // Update selection box
      this.selectionBox.end = { x, y };
    }
  }

  onMouseUp(x: number, y: number, button: number): void {
    super.onMouseUp(x, y, button);

    if (button === 0 && this.selectionBox) {
      // Finalize selection
      this.performBoxSelection();
      this.selectionBox = null;
    }
  }

  onKeyDown(key: string): void {
    if (key === 'Shift') {
      // Multi-select mode
    } else if (key === 'Escape') {
      // Clear selection
      this.clearSelection();
    }
  }

  private performRaycast(x: number, y: number): void {
    // Convert screen coordinates to normalized device coordinates (NDC)
    const canvas = document.querySelector('canvas');
    if (!canvas) {
      console.warn('No canvas found for raycasting');
      return;
    }

    const rect = canvas.getBoundingClientRect();
    const ndcX = ((x - rect.left) / rect.width) * 2 - 1;
    const ndcY = -((y - rect.top) / rect.height) * 2 + 1;

    // In a real implementation:
    // 1. Get camera's inverse projection and view matrices
    // 2. Create ray from camera through NDC point
    // 3. Test ray against all entity bounding volumes
    // 4. Select entity with closest intersection

    // eslint-disable-next-line no-console
    console.log(
      `Raycast at screen: (${x}, ${y}), NDC: (${ndcX.toFixed(2)}, ${ndcY.toFixed(2)})`
    );
  }

  private performBoxSelection(): void {
    if (!this.selectionBox) return;

    const { start, end } = this.selectionBox;
    const minX = Math.min(start.x, end.x);
    const minY = Math.min(start.y, end.y);
    const maxX = Math.max(start.x, end.x);
    const maxY = Math.max(start.y, end.y);

    // In a real implementation:
    // 1. Project all entity bounding boxes to screen space
    // 2. Test if projected bounds intersect with selection box
    // 3. Add all intersecting entities to selection set

    // eslint-disable-next-line no-console
    console.log(`Box selection from (${minX}, ${minY}) to (${maxX}, ${maxY})`);
  }

  private clearSelection(): void {
    this.selectedEntities.clear();
    // eslint-disable-next-line no-console
    console.log('Selection cleared');
  }

  update(deltaTime: number): void {
    // Update selection state
  }

  render(context: any): void {
    // Render selection box if dragging
    if (this.selectionBox && context) {
      const { start, end } = this.selectionBox;
      const width = end.x - start.x;
      const height = end.y - start.y;

      // Draw selection rectangle
      context.strokeStyle = '#4A9EFF';
      context.lineWidth = 1;
      context.setLineDash([5, 3]);
      context.strokeRect(start.x, start.y, width, height);

      context.fillStyle = 'rgba(74, 158, 255, 0.1)';
      context.fillRect(start.x, start.y, width, height);

      context.setLineDash([]);
    }

    // Render selection outline for selected entities
    for (const entityId of this.selectedEntities) {
      // In a real implementation:
      // 1. Get entity's bounding box in world space
      // 2. Project to screen space
      // 3. Draw outline around projected bounds

      if (context) {
        // Placeholder outline rendering
        context.strokeStyle = '#FFD700';
        context.lineWidth = 2;
      }
    }
  }
}
