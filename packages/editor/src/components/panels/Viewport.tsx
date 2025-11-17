/**
 * Viewport Panel
 * 3D viewport for scene visualization with WebGL integration
 */

import React, { useRef, useEffect, useState } from 'react';

export const Viewport: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [engineStatus, setEngineStatus] = useState<string>('Initializing...');
  const [_renderLoop, setRenderLoop] = useState<number | null>(null);

  useEffect(() => {
    if (!canvasRef.current) return undefined;

    const canvas = canvasRef.current;
    let gl: WebGLRenderingContext | null = null;
    let animationId: number | null = null;

    try {
      // Initialize WebGL context
      gl =
        (canvas.getContext('webgl2') as WebGLRenderingContext) ||
        (canvas.getContext('webgl') as WebGLRenderingContext) ||
        (canvas.getContext('experimental-webgl') as WebGLRenderingContext);

      if (!gl) {
        setEngineStatus('WebGL not supported');
        return undefined;
      }

      setEngineStatus('Engine Ready');

      // Set canvas size
      const resizeCanvas = () => {
        const displayWidth = canvas.clientWidth;
        const displayHeight = canvas.clientHeight;

        if (canvas.width !== displayWidth || canvas.height !== displayHeight) {
          canvas.width = displayWidth;
          canvas.height = displayHeight;
          if (gl) {
            gl.viewport(0, 0, canvas.width, canvas.height);
          }
        }
      };

      resizeCanvas();
      window.addEventListener('resize', resizeCanvas);

      // Simple render loop
      const render = () => {
        if (!gl) return;

        // Clear to dark gray
        gl.clearColor(0.15, 0.15, 0.15, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        animationId = requestAnimationFrame(render);
      };

      render();
      setRenderLoop(animationId);

      // Cleanup
      return () => {
        window.removeEventListener('resize', resizeCanvas);
        if (animationId) {
          cancelAnimationFrame(animationId);
        }
      };
    } catch (error) {
      console.error('Failed to initialize viewport:', error);
      setEngineStatus('Error: ' + (error as Error).message);
      return undefined;
    }
  }, []);

  return (
    <div className="relative w-full h-full bg-gray-900">
      {/* Viewport Canvas */}
      <canvas
        ref={canvasRef}
        className="w-full h-full"
        style={{ display: 'block' }}
      />

      {/* Viewport Overlay */}
      <div className="absolute top-2 left-2 bg-black/50 px-3 py-1 rounded text-xs text-white">
        Perspective | Free Camera | {engineStatus}
      </div>

      {/* Grid Toggle */}
      <div className="absolute bottom-2 right-2 flex gap-2">
        <button className="bg-editor-surface hover:bg-editor-hover px-3 py-1.5 rounded text-xs border border-editor-border">
          Grid: On
        </button>
        <button className="bg-editor-surface hover:bg-editor-hover px-3 py-1.5 rounded text-xs border border-editor-border">
          Gizmos: On
        </button>
      </div>
    </div>
  );
};
