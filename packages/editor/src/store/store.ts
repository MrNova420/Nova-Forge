/**
 * Redux Store Configuration
 * Central state management for the editor
 */

import { configureStore } from '@reduxjs/toolkit';
import editorReducer from './slices/editorSlice';

export const store = configureStore({
  reducer: {
    editor: editorReducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: {
        // Ignore these action types
        ignoredActions: ['editor/setScene'],
        // Ignore these field paths in all actions
        ignoredActionPaths: ['payload.scene'],
        // Ignore these paths in the state
        ignoredPaths: ['editor.scene'],
      },
    }),
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;
