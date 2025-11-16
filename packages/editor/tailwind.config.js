/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        editor: {
          bg: '#1e1e1e',
          surface: '#252526',
          border: '#3e3e42',
          hover: '#2a2d2e',
          active: '#37373d',
          text: '#cccccc',
          'text-muted': '#858585',
          primary: '#007acc',
          'primary-hover': '#0098ff',
        },
      },
    },
  },
  plugins: [],
}
