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
 * StatusBar
 * Bottom status bar with notifications and progress
 */

import React from 'react';
import { AlertCircleIcon, CheckCircleIcon, InfoIcon, Loader2Icon } from 'lucide-react';

export const StatusBar: React.FC = () => {
  const [messages] = React.useState([
    { type: 'success', text: 'Project loaded successfully' },
  ]);

  const getIcon = (type: string) => {
    switch (type) {
      case 'success':
        return <CheckCircleIcon size={14} className="text-green-500" />;
      case 'error':
        return <AlertCircleIcon size={14} className="text-red-500" />;
      case 'loading':
        return <Loader2Icon size={14} className="animate-spin text-blue-500" />;
      default:
        return <InfoIcon size={14} className="text-blue-500" />;
    }
  };

  return (
    <div className="h-6 bg-editor-surface border-t border-editor-border flex items-center px-3 text-xs">
      <div className="flex items-center gap-2 flex-1">
        {messages.map((msg, index) => (
          <div key={index} className="flex items-center gap-1.5">
            {getIcon(msg.type)}
            <span className="text-editor-text-muted">{msg.text}</span>
          </div>
        ))}
      </div>
      <div className="flex items-center gap-4 text-editor-text-muted">
        <span>FPS: 60</span>
        <span>Draw Calls: 24</span>
        <span>Vertices: 12.5K</span>
      </div>
    </div>
  );
};
