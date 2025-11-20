#!/bin/bash

# Nova Engine - Easy Setup Script
# One command to install everything!

set -e

echo "🎮 Nova Engine - Easy Setup"
echo "============================"
echo ""

# Check Node.js version
if ! command -v node &> /dev/null; then
    echo "❌ Node.js not found!"
    echo "Please install Node.js 18+ from https://nodejs.org/"
    exit 1
fi

NODE_VERSION=$(node -v | cut -d'v' -f2 | cut -d'.' -f1)
if [ "$NODE_VERSION" -lt 18 ]; then
    echo "❌ Node.js version too old (need 18+, have $NODE_VERSION)"
    echo "Please upgrade Node.js from https://nodejs.org/"
    exit 1
fi

echo "✅ Node.js $(node -v) detected"
echo ""

# Install dependencies
echo "📦 Installing all dependencies..."
npm install

echo ""
echo "🏗️  Building all packages..."
npm run build

echo ""
echo "🧪 Running tests..."
npm test

echo ""
echo "✅ Setup Complete!"
echo ""
echo "🚀 Next steps:"
echo "   npm run dev:editor    # Start the editor"
echo "   npm run dev:engine    # Start engine dev server"
echo ""
echo "📚 Documentation:"
echo "   SETUP.md             # Detailed setup guide"
echo "   USER_GUIDE.md        # How to use Nova Engine"
echo "   examples/            # Example projects"
echo ""
echo "Happy game development! 🎮✨"
