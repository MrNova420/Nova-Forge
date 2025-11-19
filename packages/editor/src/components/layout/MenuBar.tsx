/**
 * MenuBar
 * Top menu bar with File, Edit, View, etc menus
 */

import React, { useState } from 'react';
import {
  FileIcon,
  Edit3Icon,
  EyeIcon,
  BoxIcon,
  SettingsIcon,
  HelpCircleIcon,
} from 'lucide-react';

interface MenuItemProps {
  label: string;
  icon?: React.ReactNode;
  items?: { label: string; action: () => void; shortcut?: string }[];
  action?: () => void;
}

const MenuItem: React.FC<MenuItemProps> = ({ label, icon, items, action }) => {
  const [isOpen, setIsOpen] = useState(false);

  return (
    <div
      className="relative"
      onMouseEnter={() => setIsOpen(true)}
      onMouseLeave={() => setIsOpen(false)}
    >
      <button
        className="px-3 py-1.5 hover:bg-editor-hover flex items-center gap-2 text-sm"
        onClick={action}
      >
        {icon}
        {label}
      </button>

      {isOpen && items && (
        <div className="absolute top-full left-0 mt-0.5 bg-editor-surface border border-editor-border shadow-lg min-w-[200px] z-50">
          {items.map((item, index) => (
            <button
              key={index}
              className="w-full px-4 py-2 text-left hover:bg-editor-hover flex items-center justify-between text-sm"
              onClick={() => {
                item.action();
                setIsOpen(false);
              }}
            >
              <span>{item.label}</span>
              {item.shortcut && (
                <span className="text-editor-text-muted text-xs ml-4">
                  {item.shortcut}
                </span>
              )}
            </button>
          ))}
        </div>
      )}
    </div>
  );
};

export const MenuBar: React.FC = () => {
  // eslint-disable-next-line no-console
  const handleNewProject = () => console.log('New Project');
  // eslint-disable-next-line no-console
  const handleOpen = () => console.log('Open Project');
  // eslint-disable-next-line no-console
  const handleSave = () => console.log('Save Project');
  // eslint-disable-next-line no-console
  const handleExport = () => console.log('Export');

  return (
    <div className="h-8 bg-editor-surface border-b border-editor-border flex items-center px-2">
      <MenuItem
        label="File"
        icon={<FileIcon size={14} />}
        items={[
          {
            label: 'New Project',
            action: handleNewProject,
            shortcut: 'Ctrl+N',
          },
          { label: 'Open Project...', action: handleOpen, shortcut: 'Ctrl+O' },
          { label: 'Save', action: handleSave, shortcut: 'Ctrl+S' },
          { label: 'Save As...', action: handleSave, shortcut: 'Ctrl+Shift+S' },
          {
            label: 'Export Build...',
            action: handleExport,
            shortcut: 'Ctrl+E',
          },
        ]}
      />
      <MenuItem
        label="Edit"
        icon={<Edit3Icon size={14} />}
        items={[
          { label: 'Undo', action: () => {}, shortcut: 'Ctrl+Z' },
          { label: 'Redo', action: () => {}, shortcut: 'Ctrl+Y' },
          { label: 'Cut', action: () => {}, shortcut: 'Ctrl+X' },
          { label: 'Copy', action: () => {}, shortcut: 'Ctrl+C' },
          { label: 'Paste', action: () => {}, shortcut: 'Ctrl+V' },
          { label: 'Delete', action: () => {}, shortcut: 'Del' },
        ]}
      />
      <MenuItem
        label="View"
        icon={<EyeIcon size={14} />}
        items={[
          { label: 'Reset Layout', action: () => {} },
          { label: 'Toggle Grid', action: () => {}, shortcut: 'G' },
          { label: 'Toggle Gizmos', action: () => {}, shortcut: 'H' },
          { label: 'Frame Selected', action: () => {}, shortcut: 'F' },
        ]}
      />
      <MenuItem
        label="GameObject"
        icon={<BoxIcon size={14} />}
        items={[
          { label: 'Create Empty', action: () => {} },
          { label: '3D Object > Cube', action: () => {} },
          { label: '3D Object > Sphere', action: () => {} },
          { label: '3D Object > Plane', action: () => {} },
          { label: 'Light > Directional', action: () => {} },
          { label: 'Camera', action: () => {} },
        ]}
      />
      <MenuItem
        label="Tools"
        icon={<SettingsIcon size={14} />}
        items={[
          { label: 'Preferences...', action: () => {} },
          { label: 'Project Settings...', action: () => {} },
          { label: 'Asset Importer...', action: () => {} },
        ]}
      />
      <MenuItem
        label="Help"
        icon={<HelpCircleIcon size={14} />}
        items={[
          { label: 'Documentation', action: () => {} },
          { label: 'Tutorials', action: () => {} },
          { label: 'Report Issue', action: () => {} },
          { label: 'About Nova Engine', action: () => {} },
        ]}
      />
    </div>
  );
};
