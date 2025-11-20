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
 * Console Panel
 * Display logs, warnings, and errors
 */

import React, { useState } from 'react';
import { InfoIcon, AlertTriangleIcon, XCircleIcon, Trash2Icon } from 'lucide-react';

interface LogEntry {
  id: string;
  type: 'log' | 'warning' | 'error';
  message: string;
  timestamp: Date;
}

const mockLogs: LogEntry[] = [
  { id: '1', type: 'log', message: 'Engine initialized successfully', timestamp: new Date() },
  { id: '2', type: 'log', message: 'Scene loaded: MainScene', timestamp: new Date() },
  { id: '3', type: 'warning', message: 'Texture resolution is too high (4096x4096)', timestamp: new Date() },
  { id: '4', type: 'log', message: 'Asset imported: player_model.gltf', timestamp: new Date() },
];

const LogIcon: React.FC<{ type: string }> = ({ type }) => {
  const size = 14;
  switch (type) {
    case 'warning':
      return <AlertTriangleIcon size={size} className="text-yellow-500" />;
    case 'error':
      return <XCircleIcon size={size} className="text-red-500" />;
    default:
      return <InfoIcon size={size} className="text-blue-500" />;
  }
};

export const Console: React.FC = () => {
  const [logs] = useState<LogEntry[]>(mockLogs);
  const [filter, setFilter] = useState<string>('all');

  const filteredLogs =
    filter === 'all' ? logs : logs.filter((log) => log.type === filter);

  const logCount = logs.filter((l) => l.type === 'log').length;
  const warningCount = logs.filter((l) => l.type === 'warning').length;
  const errorCount = logs.filter((l) => l.type === 'error').length;

  return (
    <div className="h-full flex flex-col bg-editor-surface">
      {/* Header */}
      <div className="p-2 border-b border-editor-border flex items-center justify-between">
        <div className="flex items-center gap-3">
          <h3 className="text-sm font-semibold">Console</h3>
          <div className="flex gap-1 text-xs">
            <button
              className={`px-2 py-0.5 rounded ${
                filter === 'all' ? 'bg-editor-active' : 'hover:bg-editor-hover'
              }`}
              onClick={() => setFilter('all')}
            >
              All ({logs.length})
            </button>
            <button
              className={`px-2 py-0.5 rounded ${
                filter === 'log' ? 'bg-editor-active' : 'hover:bg-editor-hover'
              } flex items-center gap-1`}
              onClick={() => setFilter('log')}
            >
              <InfoIcon size={12} className="text-blue-500" />
              {logCount}
            </button>
            <button
              className={`px-2 py-0.5 rounded ${
                filter === 'warning' ? 'bg-editor-active' : 'hover:bg-editor-hover'
              } flex items-center gap-1`}
              onClick={() => setFilter('warning')}
            >
              <AlertTriangleIcon size={12} className="text-yellow-500" />
              {warningCount}
            </button>
            <button
              className={`px-2 py-0.5 rounded ${
                filter === 'error' ? 'bg-editor-active' : 'hover:bg-editor-hover'
              } flex items-center gap-1`}
              onClick={() => setFilter('error')}
            >
              <XCircleIcon size={12} className="text-red-500" />
              {errorCount}
            </button>
          </div>
        </div>
        <button className="p-1 hover:bg-editor-hover rounded" title="Clear Console">
          <Trash2Icon size={14} />
        </button>
      </div>

      {/* Log List */}
      <div className="flex-1 overflow-auto p-2 space-y-0.5 font-mono text-xs">
        {filteredLogs.map((log) => (
          <div
            key={log.id}
            className="flex items-start gap-2 px-2 py-1 hover:bg-editor-hover rounded"
          >
            <LogIcon type={log.type} />
            <span className="flex-1">{log.message}</span>
            <span className="text-editor-text-muted">
              {log.timestamp.toLocaleTimeString()}
            </span>
          </div>
        ))}
      </div>
    </div>
  );
};
