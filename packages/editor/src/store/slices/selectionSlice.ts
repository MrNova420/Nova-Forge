/**
 * Selection Slice
 * Manages selected entities and components in the editor
 */

import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export interface SelectionState {
  selectedEntityIds: string[];
  selectedComponentType: string | null;
  hoveredEntityId: string | null;
  selectionMode: 'single' | 'multi';
  locked: boolean;
}

const initialState: SelectionState = {
  selectedEntityIds: [],
  selectedComponentType: null,
  hoveredEntityId: null,
  selectionMode: 'single',
  locked: false,
};

const selectionSlice = createSlice({
  name: 'selection',
  initialState,
  reducers: {
    selectEntity: (state, action: PayloadAction<string>) => {
      if (state.locked) return;
      
      if (state.selectionMode === 'single') {
        state.selectedEntityIds = [action.payload];
      } else {
        if (!state.selectedEntityIds.includes(action.payload)) {
          state.selectedEntityIds.push(action.payload);
        }
      }
      state.selectedComponentType = null;
    },

    selectEntities: (state, action: PayloadAction<string[]>) => {
      if (state.locked) return;
      state.selectedEntityIds = action.payload;
      state.selectedComponentType = null;
    },

    deselectEntity: (state, action: PayloadAction<string>) => {
      if (state.locked) return;
      state.selectedEntityIds = state.selectedEntityIds.filter(id => id !== action.payload);
    },

    toggleEntitySelection: (state, action: PayloadAction<string>) => {
      if (state.locked) return;
      
      const index = state.selectedEntityIds.indexOf(action.payload);
      if (index >= 0) {
        state.selectedEntityIds.splice(index, 1);
      } else {
        state.selectedEntityIds.push(action.payload);
      }
    },

    clearSelection: (state) => {
      if (state.locked) return;
      state.selectedEntityIds = [];
      state.selectedComponentType = null;
    },

    selectComponent: (state, action: PayloadAction<string>) => {
      state.selectedComponentType = action.payload;
    },

    setHoveredEntity: (state, action: PayloadAction<string | null>) => {
      state.hoveredEntityId = action.payload;
    },

    setSelectionMode: (state, action: PayloadAction<'single' | 'multi'>) => {
      state.selectionMode = action.payload;
    },

    lockSelection: (state) => {
      state.locked = true;
    },

    unlockSelection: (state) => {
      state.locked = false;
    },

    toggleSelectionLock: (state) => {
      state.locked = !state.locked;
    },
  },
});

export const {
  selectEntity,
  selectEntities,
  deselectEntity,
  toggleEntitySelection,
  clearSelection,
  selectComponent,
  setHoveredEntity,
  setSelectionMode,
  lockSelection,
  unlockSelection,
  toggleSelectionLock,
} = selectionSlice.actions;

export default selectionSlice.reducer;
