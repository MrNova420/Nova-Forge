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
 * Toolbar
 * Quick action toolbar with tool buttons
 */

import React from 'react';
import { useAppDispatch, useAppSelector } from '../../hooks';
import { setPlaying } from '../../store/slices/editorSlice';
import {
  PlayIcon,
  PauseIcon,
  MousePointerIcon,
  MoveIcon,
  RotateCcwIcon,
  MaximizeIcon,
  GlobeIcon,
  GridIcon,
} from 'lucide-react';

interface ToolButtonProps {
  icon: React.ReactNode;
  label: string;
  active?: boolean;
  onClick: () => void;
  separator?: boolean;
}

const ToolButton: React.FC<ToolButtonProps> = ({
  icon,
  label,
  active,
  onClick,
  separator,
}) => {
  return (
    <>
      <button
        className={`p-2 hover:bg-editor-hover transition-colors ${
          active ? 'bg-editor-active text-editor-primary' : ''
        }`}
        onClick={onClick}
        title={label}
      >
        {icon}
      </button>
      {separator && <div className="w-px h-6 bg-editor-border mx-1" />}
    </>
  );
};

export const Toolbar: React.FC = () => {
  const dispatch = useAppDispatch();
  const isPlaying = useAppSelector((state) => state.editor.isPlaying);
  const [activeTool, setActiveTool] = React.useState<string>('select');
  const [transformSpace, setTransformSpace] = React.useState<string>('world');
  const [gridVisible, setGridVisible] = React.useState(true);

  const handlePlay = () => {
    dispatch(setPlaying(!isPlaying));
  };

  return (
    <div className="h-10 bg-editor-surface border-b border-editor-border flex items-center px-2 gap-1">
      {/* Play Controls */}
      <ToolButton
        icon={isPlaying ? <PauseIcon size={18} /> : <PlayIcon size={18} />}
        label={isPlaying ? 'Stop' : 'Play'}
        active={isPlaying}
        onClick={handlePlay}
        separator
      />

      {/* Transform Tools */}
      <ToolButton
        icon={<MousePointerIcon size={18} />}
        label="Select (Q)"
        active={activeTool === 'select'}
        onClick={() => setActiveTool('select')}
      />
      <ToolButton
        icon={<MoveIcon size={18} />}
        label="Move (W)"
        active={activeTool === 'move'}
        onClick={() => setActiveTool('move')}
      />
      <ToolButton
        icon={<RotateCcwIcon size={18} />}
        label="Rotate (E)"
        active={activeTool === 'rotate'}
        onClick={() => setActiveTool('rotate')}
      />
      <ToolButton
        icon={<MaximizeIcon size={18} />}
        label="Scale (R)"
        active={activeTool === 'scale'}
        onClick={() => setActiveTool('scale')}
        separator
      />

      {/* Transform Space */}
      <ToolButton
        icon={<GlobeIcon size={18} />}
        label="Transform Space"
        active={transformSpace === 'world'}
        onClick={() =>
          setTransformSpace(transformSpace === 'world' ? 'local' : 'world')
        }
        separator
      />

      {/* View Options */}
      <ToolButton
        icon={<GridIcon size={18} />}
        label="Toggle Grid (G)"
        active={gridVisible}
        onClick={() => setGridVisible(!gridVisible)}
      />

      {/* Status Text */}
      <div className="flex-1 flex items-center justify-end px-4 text-sm text-editor-text-muted">
        <span>
          {isPlaying ? 'Playing' : 'Edit Mode'} | {activeTool.charAt(0).toUpperCase() + activeTool.slice(1)} Tool |{' '}
          {transformSpace === 'world' ? 'World' : 'Local'} Space
        </span>
      </div>
    </div>
  );
};
