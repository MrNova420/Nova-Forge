/**
 * Hierarchy Panel
 * Scene hierarchy tree view
 */

import React, { useState } from 'react';
import { useSelector, useDispatch } from 'react-redux';
import type { RootState } from '../../store/store';
import { selectEntity } from '../../store/slices/selectionSlice';
import {
  ChevronRightIcon,
  ChevronDownIcon,
  BoxIcon,
  EyeIcon,
  EyeOffIcon,
} from 'lucide-react';

interface SceneNode {
  id: string;
  name: string;
  type: string;
  children?: SceneNode[];
  visible?: boolean;
}

interface TreeNodeProps {
  node: SceneNode;
  level: number;
  onSelect: (id: string) => void;
  selectedId: string | null;
}

const TreeNode: React.FC<TreeNodeProps> = ({
  node,
  level,
  onSelect,
  selectedId,
}) => {
  const [isExpanded, setIsExpanded] = useState(true);
  const hasChildren = node.children && node.children.length > 0;

  return (
    <div>
      <div
        className={`tree-item ${selectedId === node.id ? 'selected' : ''}`}
        style={{ paddingLeft: `${level * 16 + 8}px` }}
        onClick={() => onSelect(node.id)}
      >
        {hasChildren && (
          <button
            onClick={(e) => {
              e.stopPropagation();
              setIsExpanded(!isExpanded);
            }}
            className="p-0.5"
          >
            {isExpanded ? (
              <ChevronDownIcon size={14} />
            ) : (
              <ChevronRightIcon size={14} />
            )}
          </button>
        )}
        {!hasChildren && <div className="w-4" />}

        <BoxIcon size={14} className="text-editor-text-muted" />
        <span className="flex-1 text-sm">{node.name}</span>

        {node.visible ? (
          <EyeIcon size={14} className="text-editor-text-muted" />
        ) : (
          <EyeOffIcon size={14} className="text-editor-text-muted" />
        )}
      </div>

      {isExpanded && hasChildren && (
        <div>
          {node.children!.map((child) => (
            <TreeNode
              key={child.id}
              node={child}
              level={level + 1}
              onSelect={onSelect}
              selectedId={selectedId}
            />
          ))}
        </div>
      )}
    </div>
  );
};

export const Hierarchy: React.FC = () => {
  const dispatch = useDispatch();
  const scene = useSelector((state: RootState) => state.scene);
  const selectedEntityIds = useSelector(
    (state: RootState) => state.selection.selectedEntityIds
  );
  const selectedEntityId = selectedEntityIds[0] || null;
  const [searchTerm, setSearchTerm] = useState('');

  // Convert entities to tree structure
  const buildSceneTree = (): SceneNode[] => {
    return scene.rootEntities.map((entityId) => {
      const entity = scene.entities[entityId];
      return entityToNode(entity);
    });
  };

  const entityToNode = (entity: any): SceneNode => {
    return {
      id: entity.id,
      name: entity.name,
      type: entity.components.camera
        ? 'camera'
        : entity.components.light
          ? 'light'
          : 'object',
      visible: entity.enabled,
      children: entity.children.map((childId: string) =>
        entityToNode(scene.entities[childId])
      ),
    };
  };

  const handleSelect = (id: string) => {
    dispatch(selectEntity(id));
  };

  const sceneTree = buildSceneTree();

  return (
    <div className="h-full flex flex-col bg-editor-surface">
      {/* Header */}
      <div className="p-2 border-b border-editor-border">
        <h3 className="text-sm font-semibold mb-2">Hierarchy</h3>
        <input
          type="text"
          placeholder="Search..."
          value={searchTerm}
          onChange={(e) => setSearchTerm(e.target.value)}
          className="w-full px-2 py-1 text-sm bg-editor-bg border border-editor-border rounded focus:outline-none focus:border-editor-primary"
        />
      </div>

      {/* Tree View */}
      <div className="flex-1 overflow-auto p-1">
        {sceneTree.length > 0 ? (
          sceneTree.map((node) => (
            <TreeNode
              key={node.id}
              node={node}
              level={0}
              onSelect={handleSelect}
              selectedId={selectedEntityId}
            />
          ))
        ) : (
          <div className="p-4 text-center text-sm text-gray-500">
            Scene is empty. Click Create to add entities.
          </div>
        )}
      </div>

      {/* Actions */}
      <div className="p-2 border-t border-editor-border flex gap-2">
        <button className="px-3 py-1 text-xs bg-editor-hover hover:bg-editor-active rounded">
          + Create
        </button>
        <button className="px-3 py-1 text-xs bg-editor-hover hover:bg-editor-active rounded">
          Delete
        </button>
      </div>
    </div>
  );
};
