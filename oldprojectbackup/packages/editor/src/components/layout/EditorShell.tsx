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
 * EditorShell
 * Main editor layout with menu, toolbar, panels, and status bar
 */

import React, { useState } from 'react';
import { MenuBar } from './MenuBar';
import { Toolbar } from './Toolbar';
import { StatusBar } from './StatusBar';
import { PanelContainer } from './PanelContainer';
import { Viewport } from '../panels/Viewport';
import { Hierarchy } from '../panels/Hierarchy';
import { Inspector } from '../panels/Inspector';
import { AssetBrowser } from '../panels/AssetBrowser';
import { Console } from '../panels/Console';

export const EditorShell: React.FC = () => {
  const [leftPanelWidth, setLeftPanelWidth] = useState(250);
  const [rightPanelWidth, setRightPanelWidth] = useState(300);
  const [bottomPanelHeight, setBottomPanelHeight] = useState(200);

  return (
    <div className="w-full h-full flex flex-col bg-editor-bg text-editor-text">
      {/* Menu Bar */}
      <MenuBar />

      {/* Toolbar */}
      <Toolbar />

      {/* Main Content Area */}
      <div className="flex-1 flex overflow-hidden">
        {/* Left Panel - Hierarchy */}
        <PanelContainer
          width={leftPanelWidth}
          onResize={setLeftPanelWidth}
          resizable="right"
          minWidth={200}
          maxWidth={400}
        >
          <Hierarchy />
        </PanelContainer>

        {/* Center - Viewport and Console */}
        <div className="flex-1 flex flex-col min-w-0">
          {/* Viewport */}
          <div className="flex-1 min-h-0">
            <Viewport />
          </div>

          {/* Bottom Panel - Console */}
          <PanelContainer
            height={bottomPanelHeight}
            onResize={setBottomPanelHeight}
            resizable="top"
            minHeight={150}
            maxHeight={400}
          >
            <Console />
          </PanelContainer>
        </div>

        {/* Right Panel - Inspector and Assets */}
        <PanelContainer
          width={rightPanelWidth}
          onResize={setRightPanelWidth}
          resizable="left"
          minWidth={250}
          maxWidth={500}
        >
          <div className="h-full flex flex-col">
            <div className="flex-1 min-h-0 overflow-auto">
              <Inspector />
            </div>
            <div className="h-64 border-t border-editor-border">
              <AssetBrowser />
            </div>
          </div>
        </PanelContainer>
      </div>

      {/* Status Bar */}
      <StatusBar />
    </div>
  );
};
