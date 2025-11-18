import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path';

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
      '@nova-engine/engine': path.resolve(__dirname, '../engine/src'),
      '@nova-engine/editor': path.resolve(__dirname, '../editor/src'),
      '@nova-engine/shared': path.resolve(__dirname, '../shared/src'),
    },
  },
  server: {
    port: 3000,
    host: true,
    open: true,
    proxy: {
      '/api': {
        target: 'http://localhost:3001',
        changeOrigin: true,
        secure: false,
      },
    },
  },
  build: {
    emptyOutDir: false, // Preserve .d.ts files from tsc
    outDir: 'dist',
    sourcemap: true,
    rollupOptions: {
      output: {
        manualChunks: {
          vendor: ['react', 'react-dom', 'react-router-dom'],
          editor: ['@nova-engine/editor'],
          engine: ['@nova-engine/engine'],
        },
      },
    },
  },
});
