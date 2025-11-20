#!/bin/bash

# Nova Engine - Version Bump Script
# Updates version across all packages

set -e

VERSION=$1

if [ -z "$VERSION" ]; then
    echo "Usage: ./scripts/version-bump.sh <version>"
    echo "Example: ./scripts/version-bump.sh 0.1.0"
    exit 1
fi

echo "🚀 Nova Engine Version Bump to $VERSION"
echo "========================================"

# Update root package.json
echo "📦 Updating root package.json..."
npm version $VERSION --no-git-tag-version

# Update engine package
echo "📦 Updating @nova-engine/engine..."
cd packages/engine
npm version $VERSION --no-git-tag-version
cd ../..

# Update editor package
echo "📦 Updating @nova-engine/editor..."
cd packages/editor
npm version $VERSION --no-git-tag-version
cd ../..

# Update server package
echo "📦 Updating @nova-engine/server..."
cd packages/server
npm version $VERSION --no-git-tag-version
cd ../..

# Update shared package
echo "📦 Updating @nova-engine/shared..."
cd packages/shared
npm version $VERSION --no-git-tag-version
cd ../..

echo ""
echo "✅ Version bump complete!"
echo "📝 Updated to version: $VERSION"
echo ""
echo "Next steps:"
echo "1. Review changes with: git diff"
echo "2. Commit changes: git add . && git commit -m \"chore: bump version to $VERSION\""
echo "3. Create tag: git tag v$VERSION"
echo "4. Push: git push && git push --tags"
