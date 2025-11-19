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
 * PanelContainer
 * Resizable panel container with drag handle
 */

import React, { useRef, useState, useEffect } from 'react';

interface PanelContainerProps {
  children: React.ReactNode;
  width?: number;
  height?: number;
  onResize?: (size: number) => void;
  resizable?: 'left' | 'right' | 'top' | 'bottom';
  minWidth?: number;
  maxWidth?: number;
  minHeight?: number;
  maxHeight?: number;
}

export const PanelContainer: React.FC<PanelContainerProps> = ({
  children,
  width,
  height,
  onResize,
  resizable,
  minWidth = 100,
  maxWidth = 1000,
  minHeight = 100,
  maxHeight = 800,
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const containerRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!isDragging) return;

    const handleMouseMove = (e: MouseEvent) => {
      if (!containerRef.current || !onResize) return;

      const rect = containerRef.current.getBoundingClientRect();

      if (resizable === 'right') {
        const newWidth = e.clientX - rect.left;
        onResize(Math.max(minWidth, Math.min(maxWidth, newWidth)));
      } else if (resizable === 'left') {
        const newWidth = rect.right - e.clientX;
        onResize(Math.max(minWidth, Math.min(maxWidth, newWidth)));
      } else if (resizable === 'bottom') {
        const newHeight = e.clientY - rect.top;
        onResize(Math.max(minHeight, Math.min(maxHeight, newHeight)));
      } else if (resizable === 'top') {
        const newHeight = rect.bottom - e.clientY;
        onResize(Math.max(minHeight, Math.min(maxHeight, newHeight)));
      }
    };

    const handleMouseUp = () => {
      setIsDragging(false);
    };

    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);

    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isDragging, onResize, resizable, minWidth, maxWidth, minHeight, maxHeight]);

  const style: React.CSSProperties = {};
  if (width !== undefined) style.width = `${width}px`;
  if (height !== undefined) style.height = `${height}px`;

  const resizerClass =
    resizable === 'right' || resizable === 'left'
      ? 'panel-resizer-horizontal'
      : 'panel-resizer-vertical';

  const resizerPosition =
    resizable === 'right'
      ? 'right-0 top-0'
      : resizable === 'left'
      ? 'left-0 top-0'
      : resizable === 'bottom'
      ? 'bottom-0 left-0'
      : 'top-0 left-0';

  return (
    <div
      ref={containerRef}
      className="relative bg-editor-surface border-editor-border overflow-hidden"
      style={style}
    >
      {children}

      {resizable && (
        <div
          className={`absolute ${resizerPosition} ${resizerClass} panel-resizer cursor-col-resize hover:bg-editor-primary`}
          onMouseDown={(e) => {
            e.preventDefault();
            setIsDragging(true);
          }}
        />
      )}
    </div>
  );
};
