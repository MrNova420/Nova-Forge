/**
 * Editor Slice
 * Manages the main editor state
 */

import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export interface EditorState {
  isPlaying: boolean;
  selectedEntityId: string | null;
  viewportSize: { width: number; height: number };
  panelLayout: string;
}

const initialState: EditorState = {
  isPlaying: false,
  selectedEntityId: null,
  viewportSize: { width: 800, height: 600 },
  panelLayout: 'default',
};

const editorSlice = createSlice({
  name: 'editor',
  initialState,
  reducers: {
    setPlaying: (state, action: PayloadAction<boolean>) => {
      state.isPlaying = action.payload;
    },
    setSelectedEntity: (state, action: PayloadAction<string | null>) => {
      state.selectedEntityId = action.payload;
    },
    setViewportSize: (
      state,
      action: PayloadAction<{ width: number; height: number }>
    ) => {
      state.viewportSize = action.payload;
    },
    setPanelLayout: (state, action: PayloadAction<string>) => {
      state.panelLayout = action.payload;
    },
  },
});

export const { setPlaying, setSelectedEntity, setViewportSize, setPanelLayout } =
  editorSlice.actions;

export default editorSlice.reducer;
