import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import path from 'path'

// https://vite.dev/config/
export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
    },
  },
  server: {
    host: true, // Allow access from network (mobile testing)
    port: 5173,
    // For camera access on mobile, you may need HTTPS
    // Uncomment below for self-signed HTTPS (will show security warning)
    // https: true,
  },
})
