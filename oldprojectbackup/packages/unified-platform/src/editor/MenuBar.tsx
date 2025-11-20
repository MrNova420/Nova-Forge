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

import React, { useState } from 'react';

interface MenuBarProps {
  onFileAction: (action: string) => void;
  onEditAction: (action: string) => void;
  onGameObjectAction: (action: string) => void;
  onComponentAction: (action: string) => void;
  onWindowAction: (action: string) => void;
  onHelpAction: (action: string) => void;
}

export const MenuBar: React.FC<MenuBarProps> = ({
  onFileAction,
  onEditAction,
  onGameObjectAction,
  onComponentAction,
  onWindowAction,
  onHelpAction,
}) => {
  const [openMenu, setOpenMenu] = useState<string | null>(null);

  const handleMenuClick = (menu: string) => {
    setOpenMenu(openMenu === menu ? null : menu);
  };

  const handleItemClick = (action: string, handler: (action: string) => void) => {
    handler(action);
    setOpenMenu(null);
  };

  return (
    <div className="menu-bar" style={styles.menuBar}>
      {/* File Menu */}
      <MenuDropdown
        title="File"
        isOpen={openMenu === 'file'}
        onToggle={() => handleMenuClick('file')}
        items={[
          { label: 'New Project', shortcut: 'Ctrl+N', action: () => handleItemClick('new', onFileAction) },
          { label: 'Open Project', shortcut: 'Ctrl+O', action: () => handleItemClick('open', onFileAction) },
          { label: 'Save', shortcut: 'Ctrl+S', action: () => handleItemClick('save', onFileAction) },
          { label: 'Save As...', shortcut: 'Ctrl+Shift+S', action: () => handleItemClick('saveAs', onFileAction) },
          { separator: true },
          { label: 'Export', action: () => handleItemClick('export', onFileAction) },
          { label: 'Import', action: () => handleItemClick('import', onFileAction) },
          { separator: true },
          { label: 'Build Settings', shortcut: 'Ctrl+B', action: () => handleItemClick('build', onFileAction) },
          { label: 'Publish to Hub', shortcut: 'Ctrl+P', action: () => handleItemClick('publish', onFileAction) },
          { separator: true },
          { label: 'Recent Projects', action: () => handleItemClick('recent', onFileAction) },
          { separator: true },
          { label: 'Exit', action: () => handleItemClick('exit', onFileAction) },
        ]}
      />

      {/* Edit Menu */}
      <MenuDropdown
        title="Edit"
        isOpen={openMenu === 'edit'}
        onToggle={() => handleMenuClick('edit')}
        items={[
          { label: 'Undo', shortcut: 'Ctrl+Z', action: () => handleItemClick('undo', onEditAction) },
          { label: 'Redo', shortcut: 'Ctrl+Y', action: () => handleItemClick('redo', onEditAction) },
          { separator: true },
          { label: 'Cut', shortcut: 'Ctrl+X', action: () => handleItemClick('cut', onEditAction) },
          { label: 'Copy', shortcut: 'Ctrl+C', action: () => handleItemClick('copy', onEditAction) },
          { label: 'Paste', shortcut: 'Ctrl+V', action: () => handleItemClick('paste', onEditAction) },
          { label: 'Duplicate', shortcut: 'Ctrl+D', action: () => handleItemClick('duplicate', onEditAction) },
          { label: 'Delete', shortcut: 'Delete', action: () => handleItemClick('delete', onEditAction) },
          { separator: true },
          { label: 'Select All', shortcut: 'Ctrl+A', action: () => handleItemClick('selectAll', onEditAction) },
          { label: 'Deselect All', shortcut: 'Ctrl+Shift+A', action: () => handleItemClick('deselectAll', onEditAction) },
          { separator: true },
          { label: 'Preferences', shortcut: 'Ctrl+,', action: () => handleItemClick('preferences', onEditAction) },
        ]}
      />

      {/* GameObject Menu */}
      <MenuDropdown
        title="GameObject"
        isOpen={openMenu === 'gameObject'}
        onToggle={() => handleMenuClick('gameObject')}
        items={[
          { label: 'Create Empty', shortcut: 'Ctrl+Shift+N', action: () => handleItemClick('createEmpty', onGameObjectAction) },
          { separator: true },
          { label: '3D Object → Cube', action: () => handleItemClick('cube', onGameObjectAction) },
          { label: '3D Object → Sphere', action: () => handleItemClick('sphere', onGameObjectAction) },
          { label: '3D Object → Capsule', action: () => handleItemClick('capsule', onGameObjectAction) },
          { label: '3D Object → Cylinder', action: () => handleItemClick('cylinder', onGameObjectAction) },
          { label: '3D Object → Plane', action: () => handleItemClick('plane', onGameObjectAction) },
          { label: '3D Object → Quad', action: () => handleItemClick('quad', onGameObjectAction) },
          { separator: true },
          { label: '2D Object → Sprite', action: () => handleItemClick('sprite', onGameObjectAction) },
          { separator: true },
          { label: 'Effects → Particle System', action: () => handleItemClick('particles', onGameObjectAction) },
          { label: 'Effects → Trail', action: () => handleItemClick('trail', onGameObjectAction) },
          { label: 'Effects → Line', action: () => handleItemClick('line', onGameObjectAction) },
          { separator: true },
          { label: 'Light → Directional Light', action: () => handleItemClick('directionalLight', onGameObjectAction) },
          { label: 'Light → Point Light', action: () => handleItemClick('pointLight', onGameObjectAction) },
          { label: 'Light → Spot Light', action: () => handleItemClick('spotLight', onGameObjectAction) },
          { separator: true },
          { label: 'Camera', action: () => handleItemClick('camera', onGameObjectAction) },
          { label: 'Audio → Audio Source', action: () => handleItemClick('audioSource', onGameObjectAction) },
        ]}
      />

      {/* Component Menu */}
      <MenuDropdown
        title="Component"
        isOpen={openMenu === 'component'}
        onToggle={() => handleMenuClick('component')}
        items={[
          { label: 'Add Component', shortcut: 'Ctrl+Shift+A', action: () => handleItemClick('add', onComponentAction) },
          { label: 'Remove Component', action: () => handleItemClick('remove', onComponentAction) },
          { separator: true },
          { label: 'Reset', action: () => handleItemClick('reset', onComponentAction) },
          { label: 'Copy Component', action: () => handleItemClick('copyComponent', onComponentAction) },
          { label: 'Paste Component Values', action: () => handleItemClick('pasteComponent', onComponentAction) },
        ]}
      />

      {/* Window Menu */}
      <MenuDropdown
        title="Window"
        isOpen={openMenu === 'window'}
        onToggle={() => handleMenuClick('window')}
        items={[
          { label: 'Scene', action: () => handleItemClick('scene', onWindowAction) },
          { label: 'Game', action: () => handleItemClick('game', onWindowAction) },
          { label: 'Inspector', action: () => handleItemClick('inspector', onWindowAction) },
          { label: 'Hierarchy', action: () => handleItemClick('hierarchy', onWindowAction) },
          { label: 'Project', action: () => handleItemClick('project', onWindowAction) },
          { label: 'Console', shortcut: 'Ctrl+Shift+C', action: () => handleItemClick('console', onWindowAction) },
          { separator: true },
          { label: 'Profiler', action: () => handleItemClick('profiler', onWindowAction) },
          { label: 'Animator', action: () => handleItemClick('animator', onWindowAction) },
          { label: 'Timeline', action: () => handleItemClick('timeline', onWindowAction) },
          { separator: true },
          { label: 'Asset Store', action: () => handleItemClick('assetStore', onWindowAction) },
          { separator: true },
          { label: 'Layouts → Default', action: () => handleItemClick('layoutDefault', onWindowAction) },
          { label: 'Layouts → 2 by 3', action: () => handleItemClick('layout2Col', onWindowAction) },
          { label: 'Layouts → 4 Split', action: () => handleItemClick('layout4Split', onWindowAction) },
          { label: 'Layouts → Save Layout...', action: () => handleItemClick('layoutSave', onWindowAction) },
        ]}
      />

      {/* Help Menu */}
      <MenuDropdown
        title="Help"
        isOpen={openMenu === 'help'}
        onToggle={() => handleMenuClick('help')}
        items={[
          { label: 'Documentation', action: () => handleItemClick('docs', onHelpAction) },
          { label: 'Tutorials', action: () => handleItemClick('tutorials', onHelpAction) },
          { label: 'API Reference', action: () => handleItemClick('api', onHelpAction) },
          { separator: true },
          { label: 'Community Forums', action: () => handleItemClick('community', onHelpAction) },
          { label: 'Join Discord', action: () => handleItemClick('discord', onHelpAction) },
          { separator: true },
          { label: 'Report a Bug', action: () => handleItemClick('reportBug', onHelpAction) },
          { label: 'Check for Updates', action: () => handleItemClick('checkUpdates', onHelpAction) },
          { label: 'About Nova Engine', action: () => handleItemClick('about', onHelpAction) },
        ]}
      />
    </div>
  );
};

interface MenuDropdownProps {
  title: string;
  isOpen: boolean;
  onToggle: () => void;
  items: Array<{
    label?: string;
    shortcut?: string;
    action?: () => void;
    separator?: boolean;
  }>;
}

const MenuDropdown: React.FC<MenuDropdownProps> = ({ title, isOpen, onToggle, items }) => {
  return (
    <div style={styles.menuContainer}>
      <button style={styles.menuButton} onClick={onToggle}>
        {title}
      </button>
      {isOpen && (
        <div style={styles.dropdown}>
          {items.map((item, index) => {
            if (item.separator) {
              return <div key={index} style={styles.separator} />;
            }
            return (
              <button key={index} style={styles.dropdownItem} onClick={item.action}>
                {item.label}
                {item.shortcut && <span style={styles.shortcut}>{item.shortcut}</span>}
              </button>
            );
          })}
        </div>
      )}
    </div>
  );
};

const styles = {
  menuBar: {
    display: 'flex',
    backgroundColor: '#1e1e1e',
    borderBottom: '1px solid #333',
    height: '30px',
    alignItems: 'center',
    padding: '0 8px',
    fontFamily: 'Arial, sans-serif',
    fontSize: '13px',
    userSelect: 'none' as const,
  },
  menuContainer: {
    position: 'relative' as const,
  },
  menuButton: {
    background: 'transparent',
    border: 'none',
    color: '#ccc',
    padding: '6px 12px',
    cursor: 'pointer',
    fontSize: '13px',
  },
  dropdown: {
    position: 'absolute' as const,
    top: '100%',
    left: 0,
    backgroundColor: '#2d2d2d',
    border: '1px solid #444',
    boxShadow: '0 4px 8px rgba(0,0,0,0.3)',
    minWidth: '200px',
    zIndex: 1000,
    display: 'flex',
    flexDirection: 'column' as const,
  },
  dropdownItem: {
    background: 'transparent',
    border: 'none',
    color: '#ccc',
    padding: '6px 12px',
    textAlign: 'left' as const,
    cursor: 'pointer',
    fontSize: '13px',
    width: '100%',
    display: 'flex',
    justifyContent: 'space-between',
    alignItems: 'center',
  },
  separator: {
    height: '1px',
    backgroundColor: '#444',
    margin: '4px 0',
  },
  shortcut: {
    color: '#888',
    fontSize: '11px',
    marginLeft: '20px',
  },
};
