# Contributing to Nova Engine Editor

Thank you for your interest in contributing to Nova Engine Editor! This document provides guidelines and instructions for contributing to the project.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Workflow](#development-workflow)
4. [Coding Standards](#coding-standards)
5. [Testing Guidelines](#testing-guidelines)
6. [Commit Convention](#commit-convention)
7. [Pull Request Process](#pull-request-process)
8. [Documentation](#documentation)

---

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inclusive environment for all contributors. We expect all participants to:

- Be respectful and considerate
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

### Unacceptable Behavior

- Harassment, discrimination, or offensive comments
- Trolling or insulting/derogatory comments
- Public or private harassment
- Publishing others' private information without permission

---

## Getting Started

### Prerequisites

Before you begin, ensure you have:

- Node.js 18+ LTS installed
- Git installed and configured
- A code editor (VS Code recommended)
- Basic knowledge of TypeScript and React

### Setting Up Development Environment

1. **Fork the repository**
   ```bash
   # Navigate to https://github.com/MrNova420/Nova-Engine-editor
   # Click "Fork" button
   ```

2. **Clone your fork**
   ```bash
   git clone https://github.com/YOUR_USERNAME/Nova-Engine-editor.git
   cd Nova-Engine-editor
   ```

3. **Add upstream remote**
   ```bash
   git remote add upstream https://github.com/MrNova420/Nova-Engine-editor.git
   ```

4. **Install dependencies**
   ```bash
   npm install
   # or
   pnpm install
   ```

5. **Build the project**
   ```bash
   npm run build
   ```

6. **Run tests**
   ```bash
   npm test
   ```

---

## Development Workflow

### Branching Strategy

We follow Git Flow branching model:

- `main` - Production-ready code
- `develop` - Latest development code
- `feature/*` - New features
- `bugfix/*` - Bug fixes
- `hotfix/*` - Urgent fixes for production
- `release/*` - Release preparation

### Creating a Feature Branch

```bash
# Update your local develop branch
git checkout develop
git pull upstream develop

# Create a new feature branch
git checkout -b feature/your-feature-name

# Make your changes
# ...

# Commit your changes (see commit convention below)
git add .
git commit -m "feat: add amazing feature"

# Push to your fork
git push origin feature/your-feature-name
```

### Keeping Your Branch Updated

```bash
# Fetch latest changes from upstream
git fetch upstream

# Rebase your branch on upstream/develop
git rebase upstream/develop

# If there are conflicts, resolve them and continue
git rebase --continue

# Force push to your fork (only for your own branches!)
git push origin feature/your-feature-name --force-with-lease
```

---

## Coding Standards

### TypeScript Guidelines

#### General Rules

- Use TypeScript for all new code
- Enable strict mode in tsconfig.json
- Avoid using `any` type - use `unknown` if necessary
- Use interfaces for object shapes, types for unions/intersections

#### Naming Conventions

```typescript
// Classes and Interfaces - PascalCase
class MyClass {}
interface IMyInterface {}

// Variables and Functions - camelCase
const myVariable = 42;
function myFunction() {}

// Constants - UPPER_SNAKE_CASE
const MAX_RETRIES = 3;

// Private class members - prefix with underscore
class Example {
  private _privateField: number;
}

// Type parameters - Single uppercase letter or PascalCase
function identity<T>(arg: T): T {}
class Container<TItem> {}
```

#### Code Style

```typescript
// Use const by default, let when reassignment is needed
const value = 42;
let counter = 0;

// Use arrow functions for callbacks
array.map(item => item * 2);

// Use destructuring
const { name, age } = person;
const [first, second] = array;

// Use template literals
const message = `Hello, ${name}!`;

// Use optional chaining and nullish coalescing
const value = obj?.property ?? defaultValue;

// Prefer async/await over promises
async function fetchData() {
  try {
    const response = await fetch(url);
    return await response.json();
  } catch (error) {
    console.error('Fetch failed:', error);
  }
}
```

### React/JSX Guidelines

```tsx
// Use functional components with hooks
function MyComponent({ prop1, prop2 }: Props) {
  const [state, setState] = useState(initialValue);
  
  useEffect(() => {
    // Side effects
  }, [dependencies]);
  
  return <div>{/* JSX */}</div>;
}

// Use TypeScript for props
interface Props {
  required: string;
  optional?: number;
  children?: React.ReactNode;
}

// Export components as default
export default MyComponent;
```

### File Organization

```typescript
// 1. Imports
import React from 'react';
import { useState } from 'react';

// 2. Types and Interfaces
interface Props {
  // ...
}

// 3. Constants
const DEFAULT_VALUE = 42;

// 4. Component/Class/Function
function MyComponent(props: Props) {
  // ...
}

// 5. Exports
export default MyComponent;
export { type Props };
```

---

## Testing Guidelines

### Unit Tests

- Write tests for all new functions and classes
- Aim for at least 80% code coverage
- Use Jest for testing

```typescript
import { describe, it, expect } from '@jest/globals';
import { myFunction } from './myModule';

describe('myFunction', () => {
  it('should return expected value', () => {
    const result = myFunction(input);
    expect(result).toBe(expectedValue);
  });
  
  it('should handle edge cases', () => {
    expect(() => myFunction(null)).toThrow();
  });
});
```

### Integration Tests

- Test interactions between components
- Mock external dependencies
- Test API endpoints

### E2E Tests

- Use Playwright for end-to-end tests
- Test critical user workflows
- Run in CI/CD pipeline

---

## Commit Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/) specification.

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, missing semicolons, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Maintenance tasks
- `ci`: CI/CD changes
- `build`: Build system changes

### Examples

```bash
# Simple feature
git commit -m "feat: add particle system"

# Bug fix with scope
git commit -m "fix(renderer): correct shadow calculation"

# Breaking change
git commit -m "feat!: redesign material system

BREAKING CHANGE: Material API has been completely redesigned.
See migration guide for details."

# With issue reference
git commit -m "fix: resolve memory leak in asset loader

Fixes #123"
```

---

## Pull Request Process

### Before Submitting

1. **Ensure your code builds**
   ```bash
   npm run build
   ```

2. **Run all tests**
   ```bash
   npm test
   ```

3. **Run linter**
   ```bash
   npm run lint
   ```

4. **Update documentation** if needed

5. **Add tests** for new features

### Creating a Pull Request

1. **Push your branch** to your fork

2. **Create PR** on GitHub
   - Go to the original repository
   - Click "New Pull Request"
   - Select your branch
   - Fill in the PR template

3. **PR Title Format**
   ```
   <type>: <description>
   ```
   Example: `feat: add material editor panel`

4. **PR Description Template**
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Breaking change
   - [ ] Documentation update
   
   ## Testing
   - [ ] Unit tests added/updated
   - [ ] Integration tests added/updated
   - [ ] Manual testing completed
   
   ## Screenshots
   (if applicable)
   
   ## Related Issues
   Closes #123
   ```

### Review Process

1. **Automated checks** must pass
2. **At least one approval** required
3. **Address review comments**
4. **Squash commits** if requested
5. **Maintainer will merge** when approved

---

## Documentation

### Code Documentation

```typescript
/**
 * Brief description of the function
 * 
 * @param param1 - Description of param1
 * @param param2 - Description of param2
 * @returns Description of return value
 * 
 * @example
 * ```typescript
 * const result = myFunction('value', 42);
 * ```
 */
function myFunction(param1: string, param2: number): ReturnType {
  // Implementation
}
```

### API Documentation

- Document all public APIs
- Use TSDoc comments
- Provide examples
- Document edge cases

### User Documentation

- Update user guides for new features
- Add tutorials for complex features
- Keep documentation in sync with code

---

## Project Structure

Please familiarize yourself with the project structure:

- `packages/engine/` - Core engine runtime
- `packages/editor/` - Editor application
- `packages/server/` - Local server
- `packages/shared/` - Shared code
- `docs/` - Documentation
- `examples/` - Example projects

See [PROJECT_STRUCTURE.md](./PROJECT_STRUCTURE.md) for details.

---

## Development Tools

### Recommended VS Code Extensions

- ESLint
- Prettier
- TypeScript Vue Plugin
- GitLens
- Error Lens

### Scripts

```bash
# Development
npm run dev          # Start development server
npm run watch        # Watch mode

# Building
npm run build        # Build all packages
npm run build:engine # Build engine only
npm run build:editor # Build editor only

# Testing
npm test             # Run all tests
npm run test:watch   # Watch mode
npm run test:coverage # With coverage

# Linting
npm run lint         # Lint code
npm run lint:fix     # Fix linting issues

# Formatting
npm run format       # Format code
npm run format:check # Check formatting
```

---

## Getting Help

### Resources

- [Documentation](./docs/)
- [Roadmap](./ROADMAP.md)
- [Architecture](./ARCHITECTURE.md)
- [GitHub Issues](https://github.com/MrNova420/Nova-Engine-editor/issues)

### Communication

- **GitHub Issues** - Bug reports and feature requests
- **GitHub Discussions** - Questions and discussions
- **Discord** - Real-time chat (link TBD)

### Reporting Bugs

Use the bug report template:

```markdown
**Describe the bug**
A clear description of the bug.

**To Reproduce**
Steps to reproduce:
1. Go to '...'
2. Click on '...'
3. See error

**Expected behavior**
What you expected to happen.

**Screenshots**
If applicable.

**Environment**
- OS: [e.g. Windows 11]
- Browser: [e.g. Chrome 120]
- Version: [e.g. 0.1.0]
```

### Feature Requests

Use the feature request template:

```markdown
**Is your feature request related to a problem?**
A clear description of the problem.

**Describe the solution you'd like**
A clear description of what you want.

**Describe alternatives you've considered**
Alternative solutions or features.

**Additional context**
Any other context or screenshots.
```

---

## License

**Nova Engine is proprietary software.** By contributing to Nova Engine, you agree that:

1. Your contributions become the property of Nova Engine Team
2. Your contributions will be subject to the proprietary license
3. You retain no rights to use contributed code outside of Nova Engine
4. You have the legal right to make the contributions

**Important:** Please review our legal documentation before contributing:
- **[LEGAL.md](LEGAL.md)** - Complete legal documentation overview
- **[LICENSE](LICENSE)** - Proprietary license agreement
- **[Developer License](legal/NovaEngine_DeveloperLicense.pdf)** - Terms for using Nova Engine

⚠️ **All contributions must be original work and cannot include open source code or code under conflicting licenses.**

---

## Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes
- Project documentation

Thank you for contributing! 🎉

---

*Last Updated: 2025-11-16*  
*Version: 1.0*
