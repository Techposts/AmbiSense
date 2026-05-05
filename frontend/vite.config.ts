import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [preact(), viteSingleFile()],
  build: {
    target: 'es2020',
    minify: 'terser',
    cssCodeSplit: false,
    assetsInlineLimit: 100000000,
    chunkSizeWarningLimit: 100000000,
    outDir: 'dist',
    rollupOptions: {
      output: { inlineDynamicImports: true },
    },
  },
  server: {
    port: 5173,
    proxy: {
      '/api': 'http://192.168.4.1',
    },
  },
});
