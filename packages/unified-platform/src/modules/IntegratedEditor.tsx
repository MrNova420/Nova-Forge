/**
 * Integrated Editor - REAL Nova Engine Editor
 * 
 * This embeds the actual editor from @nova-engine/editor package
 * into the unified platform, with full functionality
 */

import React, { useState, useEffect } from 'react';
import { Provider } from 'react-redux';
import { store } from '../../../editor/src/store/store';
import { EditorShell } from '../../../editor/src/components/layout/EditorShell';
import { UnifiedPlatformCore } from '../core/UnifiedPlatformCore';

interface IntegratedEditorProps {
  platform: UnifiedPlatformCore;
}

export const IntegratedEditor: React.FC<IntegratedEditorProps> = ({ platform }) => {
  const [isInitialized, setIsInitialized] = useState(false);

  useEffect(() => {
    // Initialize connection between unified platform and editor
    console.log('Initializing integrated editor...');
    setIsInitialized(true);

    // Notify platform that editor is ready
    platform.showNotification({
      type: 'success',
      message: 'Editor initialized and ready',
    });

    return () => {
      console.log('Editor cleanup');
    };
  }, [platform]);

  if (!isInitialized) {
    return (
      <div style={{ 
        width: '100%', 
        height: '100%', 
        display: 'flex', 
        alignItems: 'center', 
        justifyContent: 'center',
        background: '#1a1a1a',
        color: 'white',
        fontSize: '18px'
      }}>
        <div>
          <div>🎮 Nova Engine Editor</div>
          <div style={{ fontSize: '14px', color: '#888', marginTop: '10px' }}>
            Initializing Unity/Unreal-style editor...
          </div>
        </div>
      </div>
    );
  }

  return (
    <div style={{ width: '100%', height: '100%', overflow: 'hidden' }}>
      <Provider store={store}>
        <EditorShell />
      </Provider>
    </div>
  );
};
