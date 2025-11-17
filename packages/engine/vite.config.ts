import { defineConfig } from 'vite';
import { resolve } from 'path';

export default defineConfig({
  build: {
    emptyOutDir: false, // Preserve .d.ts files from tsc
    lib: {
      entry: resolve(__dirname, 'src/index.ts'),
      name: 'NovaEngine',
      formats: ['es', 'cjs'],
      fileName: (format) => `index.${format === 'es' ? 'mjs' : 'js'}`,
    },
    rollupOptions: {
      external: [],
      output: {
        // Code splitting for better tree-shaking
        manualChunks: (id) => {
          if (id.includes('node_modules')) {
            if (id.includes('ammojs-typed')) {
              return 'physics';
            }
            return 'vendor';
          }
          if (id.includes('/graphics/')) {
            return 'graphics';
          }
          if (id.includes('/physics/')) {
            return 'physics';
          }
          if (id.includes('/audio/')) {
            return 'audio';
          }
          if (id.includes('/animation/')) {
            return 'animation';
          }
        },
      },
    },
    // Enable minification for production
    minify: 'esbuild',
    // Only generate source maps in development
    sourcemap: process.env.NODE_ENV === 'development',
    // Target modern browsers for better optimization
    target: 'es2020',
    // Optimize chunk size
    chunkSizeWarningLimit: 1000,
  },
  // Optimize dependencies
  optimizeDeps: {
    include: ['ammojs-typed'],
    esbuildOptions: {
      target: 'es2020',
    },
  },
});
