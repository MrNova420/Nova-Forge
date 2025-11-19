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
 * Inspector Panel
 * Property inspector for selected entities
 */

import React from 'react';
import { useAppSelector } from '../../hooks';

type PropertyValue = string | number | boolean | { x: number; y: number; z: number } | null;

interface PropertyFieldProps {
  label: string;
  value: PropertyValue;
  onChange: (value: PropertyValue) => void;
  type?: 'number' | 'text' | 'boolean' | 'vector3' | 'color';
}

const PropertyField: React.FC<PropertyFieldProps> = ({
  label,
  value,
  onChange,
  type = 'text',
}) => {
  if (type === 'boolean') {
    return (
      <div className="flex items-center justify-between py-1">
        <label className="text-sm text-editor-text-muted">{label}</label>
        <input
          type="checkbox"
          checked={value}
          onChange={(e) => onChange(e.target.checked)}
          className="w-4 h-4"
        />
      </div>
    );
  }

  if (type === 'vector3') {
    return (
      <div className="py-1">
        <label className="text-sm text-editor-text-muted block mb-1">
          {label}
        </label>
        <div className="grid grid-cols-3 gap-1">
          <input
            type="number"
            value={value.x}
            onChange={(e) =>
              onChange({ ...value, x: parseFloat(e.target.value) })
            }
            className="px-2 py-1 text-xs bg-editor-bg border border-editor-border rounded"
            placeholder="X"
          />
          <input
            type="number"
            value={value.y}
            onChange={(e) =>
              onChange({ ...value, y: parseFloat(e.target.value) })
            }
            className="px-2 py-1 text-xs bg-editor-bg border border-editor-border rounded"
            placeholder="Y"
          />
          <input
            type="number"
            value={value.z}
            onChange={(e) =>
              onChange({ ...value, z: parseFloat(e.target.value) })
            }
            className="px-2 py-1 text-xs bg-editor-bg border border-editor-border rounded"
            placeholder="Z"
          />
        </div>
      </div>
    );
  }

  return (
    <div className="flex items-center justify-between py-1">
      <label className="text-sm text-editor-text-muted">{label}</label>
      <input
        type={type}
        value={value}
        onChange={(e) =>
          onChange(
            type === 'number' ? parseFloat(e.target.value) : e.target.value
          )
        }
        className="w-32 px-2 py-1 text-xs bg-editor-bg border border-editor-border rounded focus:outline-none focus:border-editor-primary"
      />
    </div>
  );
};

export const Inspector: React.FC = () => {
  const selectedEntityIds = useAppSelector(
    (state) => state.selection.selectedEntityIds
  );
  const selectedEntityId = selectedEntityIds[0] || null;
  const entity = useAppSelector((state) =>
    selectedEntityId ? state.scene.entities[selectedEntityId] : null
  );

  if (!selectedEntityId || !entity) {
    return (
      <div className="h-full flex items-center justify-center bg-editor-surface text-editor-text-muted">
        <p className="text-sm">No entity selected</p>
      </div>
    );
  }

  const transform = entity.components.transform || {
    position: [0, 0, 0],
    rotation: [0, 0, 0],
    scale: [1, 1, 1],
  };

  return (
    <div className="h-full bg-editor-surface overflow-auto">
      {/* Header */}
      <div className="p-3 border-b border-editor-border">
        <h3 className="text-sm font-semibold">Inspector</h3>
        <p className="text-xs text-editor-text-muted mt-1">{entity.name}</p>
      </div>

      {/* Properties */}
      <div className="p-3 space-y-4">
        {/* Entity Info */}
        <div className="border border-editor-border rounded p-3">
          <h4 className="text-sm font-semibold mb-3">Entity</h4>
          <div className="py-1">
            <label className="text-sm text-editor-text-muted block mb-1">
              Name
            </label>
            <input
              type="text"
              value={entity.name}
              className="w-full px-2 py-1 text-sm bg-editor-bg border border-editor-border rounded"
              readOnly
            />
          </div>
          <PropertyField
            label="Enabled"
            value={entity.enabled}
            onChange={() => {}}
            type="boolean"
          />
        </div>

        {/* Transform Component */}
        {transform && (
          <div className="border border-editor-border rounded p-3">
            <h4 className="text-sm font-semibold mb-3">Transform</h4>
            <PropertyField
              label="Position"
              value={{
                x: transform.position[0],
                y: transform.position[1],
                z: transform.position[2],
              }}
              onChange={() => {}}
              type="vector3"
            />
            <PropertyField
              label="Rotation"
              value={{
                x: transform.rotation[0],
                y: transform.rotation[1],
                z: transform.rotation[2],
              }}
              onChange={() => {}}
              type="vector3"
            />
            <PropertyField
              label="Scale"
              value={{
                x: transform.scale[0],
                y: transform.scale[1],
                z: transform.scale[2],
              }}
              onChange={() => {}}
              type="vector3"
            />
          </div>
        )}

        {/* Mesh Renderer Component */}
        <div className="border border-editor-border rounded p-3">
          <h4 className="text-sm font-semibold mb-3">Mesh Renderer</h4>
          <PropertyField
            label="Cast Shadows"
            value={true}
            onChange={() => {}}
            type="boolean"
          />
          <PropertyField
            label="Receive Shadows"
            value={true}
            onChange={() => {}}
            type="boolean"
          />
          <div className="py-1">
            <button className="w-full px-3 py-1.5 text-xs bg-editor-hover hover:bg-editor-active rounded">
              Select Material
            </button>
          </div>
        </div>

        {/* Add Component Button */}
        <button className="w-full px-3 py-2 text-sm bg-editor-primary hover:bg-editor-primary-hover rounded">
          + Add Component
        </button>
      </div>
    </div>
  );
};
