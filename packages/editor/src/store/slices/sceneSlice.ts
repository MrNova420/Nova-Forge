/**
 * Scene Slice
 * Manages scene state including entities, components, and scene graph
 */

import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export interface Entity {
  id: string;
  name: string;
  parent: string | null;
  children: string[];
  components: Record<string, any>;
  enabled: boolean;
}

export interface SceneState {
  name: string;
  entities: Record<string, Entity>;
  rootEntities: string[];
  isDirty: boolean;
  lastSaved: number | null;
}

const initialState: SceneState = {
  name: 'Untitled Scene',
  entities: {},
  rootEntities: [],
  isDirty: false,
  lastSaved: null,
};

const sceneSlice = createSlice({
  name: 'scene',
  initialState,
  reducers: {
    setSceneName: (state, action: PayloadAction<string>) => {
      state.name = action.payload;
      state.isDirty = true;
    },

    addEntity: (state, action: PayloadAction<Entity>) => {
      const entity = action.payload;
      state.entities[entity.id] = entity;
      
      if (entity.parent === null) {
        state.rootEntities.push(entity.id);
      } else {
        const parent = state.entities[entity.parent];
        if (parent) {
          parent.children.push(entity.id);
        }
      }
      
      state.isDirty = true;
    },

    removeEntity: (state, action: PayloadAction<string>) => {
      const entityId = action.payload;
      const entity = state.entities[entityId];
      
      if (!entity) return;

      // Remove from parent's children or root
      if (entity.parent === null) {
        state.rootEntities = state.rootEntities.filter(id => id !== entityId);
      } else {
        const parent = state.entities[entity.parent];
        if (parent) {
          parent.children = parent.children.filter(id => id !== entityId);
        }
      }

      // Recursively remove children
      const removeRecursive = (id: string) => {
        const ent = state.entities[id];
        if (ent) {
          ent.children.forEach(removeRecursive);
          delete state.entities[id];
        }
      };
      
      removeRecursive(entityId);
      state.isDirty = true;
    },

    updateEntity: (state, action: PayloadAction<{ id: string; updates: Partial<Entity> }>) => {
      const { id, updates } = action.payload;
      const entity = state.entities[id];
      
      if (entity) {
        Object.assign(entity, updates);
        state.isDirty = true;
      }
    },

    setEntityParent: (state, action: PayloadAction<{ entityId: string; parentId: string | null }>) => {
      const { entityId, parentId } = action.payload;
      const entity = state.entities[entityId];
      
      if (!entity) return;

      // Remove from old parent
      if (entity.parent === null) {
        state.rootEntities = state.rootEntities.filter(id => id !== entityId);
      } else {
        const oldParent = state.entities[entity.parent];
        if (oldParent) {
          oldParent.children = oldParent.children.filter(id => id !== entityId);
        }
      }

      // Add to new parent
      entity.parent = parentId;
      if (parentId === null) {
        state.rootEntities.push(entityId);
      } else {
        const newParent = state.entities[parentId];
        if (newParent) {
          newParent.children.push(entityId);
        }
      }
      
      state.isDirty = true;
    },

    addComponent: (state, action: PayloadAction<{ entityId: string; componentType: string; data: any }>) => {
      const { entityId, componentType, data } = action.payload;
      const entity = state.entities[entityId];
      
      if (entity) {
        entity.components[componentType] = data;
        state.isDirty = true;
      }
    },

    removeComponent: (state, action: PayloadAction<{ entityId: string; componentType: string }>) => {
      const { entityId, componentType } = action.payload;
      const entity = state.entities[entityId];
      
      if (entity) {
        delete entity.components[componentType];
        state.isDirty = true;
      }
    },

    updateComponent: (state, action: PayloadAction<{ entityId: string; componentType: string; data: any }>) => {
      const { entityId, componentType, data } = action.payload;
      const entity = state.entities[entityId];
      
      if (entity && entity.components[componentType]) {
        entity.components[componentType] = { ...entity.components[componentType], ...data };
        state.isDirty = true;
      }
    },

    loadScene: (state, action: PayloadAction<SceneState>) => {
      return { ...action.payload, isDirty: false, lastSaved: Date.now() };
    },

    clearScene: (state) => {
      state.entities = {};
      state.rootEntities = [];
      state.name = 'Untitled Scene';
      state.isDirty = false;
      state.lastSaved = null;
    },

    markSaved: (state) => {
      state.isDirty = false;
      state.lastSaved = Date.now();
    },
  },
});

export const {
  setSceneName,
  addEntity,
  removeEntity,
  updateEntity,
  setEntityParent,
  addComponent,
  removeComponent,
  updateComponent,
  loadScene,
  clearScene,
  markSaved,
} = sceneSlice.actions;

export default sceneSlice.reducer;
