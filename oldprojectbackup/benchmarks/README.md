# Nova Engine Benchmarks

Performance benchmarks for the Nova Engine project.

## Overview

This directory contains performance benchmarks for critical engine systems. Benchmarks help us:

- Monitor performance regressions
- Compare different implementation approaches
- Establish baseline performance metrics
- Identify optimization opportunities

## Available Benchmarks

### Math Operations (`math-operations.bench.ts`)

Benchmarks for core math library operations:

- **Vector2/3/4**: add, subtract, multiply, normalize, dot, cross
- **Matrix4**: multiply, transpose, invert, transformations (translation, rotation, scale), projections (perspective, orthographic)
- **Quaternion**: multiply, normalize, slerp, conversions

**Run:** `npm run bench:math`

### ECS Performance (`ecs-performance.bench.ts`)

Benchmarks for Entity-Component-System operations:

- **Entity**: creation, destruction, batch operations
- **Component**: add, remove, get, has, multiple components
- **Query**: all, any, none conditions with various entity counts
- **System**: update loops with 100, 1000, 10000 entities
- **Complex scenarios**: mixed operations (create, update, destroy)

**Run:** `npm run bench:ecs`

### Rendering Performance (`rendering.bench.ts`)

Template for future rendering benchmarks (Phase 2):

- Shader compilation and switching
- Mesh rendering and batching
- Texture operations
- Scene rendering
- Post-processing

**Status:** Template only, to be implemented in Phase 2

## Running Benchmarks

### Run All Benchmarks

```bash
npm run bench
```

### Run Specific Benchmark

```bash
npm run bench:math
npm run bench:ecs
npm run bench:rendering  # Template only
```

### Run with tsx

```bash
npx tsx benchmarks/math-operations.bench.ts
npx tsx benchmarks/ecs-performance.bench.ts
```

## Interpreting Results

Benchmark results show operations per second (ops/sec):

```
Vector3#add x 12,345,678 ops/sec ±1.23% (90 runs sampled)
```

- **ops/sec**: Higher is better
- **±%**: Margin of error (lower is more consistent)
- **runs sampled**: Number of test runs

## Performance Targets

### Math Operations

- Vector operations: > 10M ops/sec
- Matrix multiply: > 1M ops/sec
- Quaternion operations: > 5M ops/sec

### ECS Operations

- Entity creation: > 100K ops/sec
- Component queries (100 entities): > 100K ops/sec
- System update (1000 entities): > 1K ops/sec

### Rendering (Future - Phase 2)

- Draw calls: > 1000 per frame (60 FPS)
- Shader switches: > 100 per frame
- Scene render (100 objects): 60 FPS minimum

## Adding New Benchmarks

1. Create a new `.bench.ts` file in this directory
2. Use the Benchmark.js API:

```typescript
import Benchmark from 'benchmark';

const suite = new Benchmark.Suite('My Benchmark');

suite.add('Operation#test', () => {
  // Code to benchmark
});

suite
  .on('cycle', (event: Benchmark.Event) => {
    console.log(String(event.target));
  })
  .on('complete', function (this: Benchmark.Suite) {
    console.log('Fastest is ' + this.filter('fastest').map('name'));
  })
  .run({ async: true });
```

3. Add a script to `package.json`:

```json
"bench:mytest": "tsx benchmarks/my-test.bench.ts"
```

4. Document it in this README

## CI Integration

Benchmarks can optionally be integrated into CI/CD:

```yaml
# .github/workflows/benchmarks.yml
name: Benchmarks
on: [push, pull_request]
jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-node@v3
      - run: npm install
      - run: npm run bench
```

## Best Practices

1. **Isolate operations**: Benchmark one thing at a time
2. **Warm up**: Let the JIT compiler optimize before measuring
3. **Avoid side effects**: Don't modify shared state
4. **Use realistic data**: Test with typical use case scenarios
5. **Run multiple times**: Benchmark.js handles this automatically
6. **Document baselines**: Record initial metrics for comparison

## Resources

- [Benchmark.js Documentation](https://benchmarkjs.com/)
- [DEVELOPMENT_GUIDE.md](../DEVELOPMENT_GUIDE.md) - Task 1.3.2
- [AUTOMATION.md](../AUTOMATION.md) - Performance Monitoring

## Status

- ✅ Math Operations - Complete and running
- ✅ ECS Performance - Complete and running
- ⏳ Rendering Performance - Template (awaiting Phase 2)

Last Updated: 2025-11-16
