import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import Windi from 'vite-plugin-windicss'
import { transform } from "windicss/helpers"

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [Windi({ config: { plugins: [transform("daisyui")] } }), svelte()],
})
