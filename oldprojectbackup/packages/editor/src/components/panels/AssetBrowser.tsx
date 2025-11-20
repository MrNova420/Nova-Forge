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
 * AssetBrowser Panel
 * Browse and manage project assets
 */

import React, { useState } from 'react';
import { FolderIcon, FileIcon, ImageIcon, BoxIcon, MusicIcon } from 'lucide-react';

interface Asset {
  id: string;
  name: string;
  type: 'folder' | 'model' | 'texture' | 'audio' | 'script';
}

const mockAssets: Asset[] = [
  { id: '1', name: 'Models', type: 'folder' },
  { id: '2', name: 'Textures', type: 'folder' },
  { id: '3', name: 'Audio', type: 'folder' },
  { id: '4', name: 'player.gltf', type: 'model' },
  { id: '5', name: 'brick_texture.png', type: 'texture' },
  { id: '6', name: 'music.mp3', type: 'audio' },
];

const AssetIcon: React.FC<{ type: string }> = ({ type }) => {
  const size = 16;
  switch (type) {
    case 'folder':
      return <FolderIcon size={size} className="text-yellow-500" />;
    case 'model':
      return <BoxIcon size={size} className="text-blue-500" />;
    case 'texture':
      return <ImageIcon size={size} className="text-green-500" />;
    case 'audio':
      return <MusicIcon size={size} className="text-purple-500" />;
    default:
      return <FileIcon size={size} className="text-editor-text-muted" />;
  }
};

export const AssetBrowser: React.FC = () => {
  const [viewMode, setViewMode] = useState<'grid' | 'list'>('grid');

  return (
    <div className="h-full flex flex-col bg-editor-surface">
      {/* Header */}
      <div className="p-2 border-b border-editor-border flex items-center justify-between">
        <h3 className="text-sm font-semibold">Assets</h3>
        <div className="flex gap-1">
          <button
            className={`px-2 py-1 text-xs rounded ${
              viewMode === 'grid' ? 'bg-editor-active' : 'hover:bg-editor-hover'
            }`}
            onClick={() => setViewMode('grid')}
          >
            Grid
          </button>
          <button
            className={`px-2 py-1 text-xs rounded ${
              viewMode === 'list' ? 'bg-editor-active' : 'hover:bg-editor-hover'
            }`}
            onClick={() => setViewMode('list')}
          >
            List
          </button>
        </div>
      </div>

      {/* Search */}
      <div className="p-2 border-b border-editor-border">
        <input
          type="text"
          placeholder="Search assets..."
          className="w-full px-2 py-1 text-sm bg-editor-bg border border-editor-border rounded focus:outline-none focus:border-editor-primary"
        />
      </div>

      {/* Asset Grid/List */}
      <div className="flex-1 overflow-auto p-2">
        {viewMode === 'grid' ? (
          <div className="grid grid-cols-3 gap-2">
            {mockAssets.map((asset) => (
              <div
                key={asset.id}
                className="p-2 border border-editor-border rounded hover:bg-editor-hover cursor-pointer flex flex-col items-center gap-1"
              >
                <AssetIcon type={asset.type} />
                <span className="text-xs text-center truncate w-full">
                  {asset.name}
                </span>
              </div>
            ))}
          </div>
        ) : (
          <div className="space-y-0.5">
            {mockAssets.map((asset) => (
              <div
                key={asset.id}
                className="px-2 py-1.5 hover:bg-editor-hover cursor-pointer rounded flex items-center gap-2"
              >
                <AssetIcon type={asset.type} />
                <span className="text-sm">{asset.name}</span>
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Import Button */}
      <div className="p-2 border-t border-editor-border">
        <button className="w-full px-3 py-1.5 text-sm bg-editor-primary hover:bg-editor-primary-hover rounded">
          Import Assets
        </button>
      </div>
    </div>
  );
};
