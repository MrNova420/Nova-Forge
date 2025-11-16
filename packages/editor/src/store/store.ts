/**
 * Redux Store Configuration
 * Central state management for the editor
 */

import { configureStore } from '@reduxjs/toolkit';
import editorReducer from './slices/editorSlice';
import sceneReducer from './slices/sceneSlice';
import selectionReducer from './slices/selectionSlice';
import assetReducer from './slices/assetSlice';

export const store = configureStore({
  reducer: {
    editor: editorReducer,
    scene: sceneReducer,
    selection: selectionReducer,
    asset: assetReducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: {
        // Ignore these action types
        ignoredActions: ['editor/setScene', 'scene/loadScene'],
        // Ignore these field paths in all actions
        ignoredActionPaths: ['payload.scene', 'payload.entities', 'payload.components'],
        // Ignore these paths in the state
        ignoredPaths: ['editor.scene', 'scene.entities'],
      },
    }),
  devTools: {
    name: 'Nova Engine Editor',
    trace: true,
    traceLimit: 25,
  },
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;
