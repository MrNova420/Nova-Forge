import { defineConfig } from 'vite';
import { resolve } from 'path';

export default defineConfig({
  build: {
    emptyOutDir: false,
    lib: {
      entry: resolve(__dirname, 'src/index.ts'),
      name: 'NovaEngineWeb',
      formats: ['es', 'cjs'],
      fileName: (format) => `index.${format === 'es' ? 'mjs' : 'js'}`,
    },
    rollupOptions: {
      external: ['three', 'cannon-es'],
      output: {
        globals: {
          three: 'THREE',
          'cannon-es': 'CANNON',
        },
      },
    },
    sourcemap: true,
    target: 'es2020',
  },
  optimizeDeps: {
    include: ['three', 'cannon-es'],
  },
});
