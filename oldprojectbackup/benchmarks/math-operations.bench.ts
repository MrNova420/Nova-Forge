/**
 * Math Operations Benchmark
 *
 * Benchmarks for core math library operations including:
 * - Vector operations (add, subtract, multiply, normalize, dot, cross)
 * - Matrix operations (multiply, transpose, invert)
 * - Quaternion operations (multiply, slerp)
 */

import Benchmark from 'benchmark';
import {
  Vector2,
  Vector3,
  Vector4,
  Matrix4,
  Quaternion,
} from '../packages/engine/src/math';

const suite = new Benchmark.Suite('Math Operations');

// Vector2 operations
const v2a = new Vector2(1, 2);
const v2b = new Vector2(3, 4);

suite.add('Vector2#add', () => {
  v2a.clone().add(v2b);
});

suite.add('Vector2#subtract', () => {
  v2a.clone().subtract(v2b);
});

suite.add('Vector2#multiply', () => {
  v2a.clone().multiply(2);
});

suite.add('Vector2#normalize', () => {
  v2a.clone().normalize();
});

suite.add('Vector2#dot', () => {
  v2a.dot(v2b);
});

// Vector3 operations
const v3a = new Vector3(1, 2, 3);
const v3b = new Vector3(4, 5, 6);

suite.add('Vector3#add', () => {
  v3a.clone().add(v3b);
});

suite.add('Vector3#subtract', () => {
  v3a.clone().subtract(v3b);
});

suite.add('Vector3#multiply', () => {
  v3a.clone().multiply(2);
});

suite.add('Vector3#normalize', () => {
  v3a.clone().normalize();
});

suite.add('Vector3#dot', () => {
  v3a.dot(v3b);
});

suite.add('Vector3#cross', () => {
  v3a.cross(v3b);
});

// Vector4 operations
const v4a = new Vector4(1, 2, 3, 4);
const v4b = new Vector4(5, 6, 7, 8);

suite.add('Vector4#add', () => {
  v4a.clone().add(v4b);
});

suite.add('Vector4#normalize', () => {
  v4a.clone().normalize();
});

suite.add('Vector4#dot', () => {
  v4a.dot(v4b);
});

// Matrix4 operations
const m4a = Matrix4.identity();
const m4b = Matrix4.translation(new Vector3(1, 2, 3));

suite.add('Matrix4#multiply', () => {
  m4a.clone().multiply(m4b);
});

suite.add('Matrix4#transpose', () => {
  m4a.clone().transpose();
});

suite.add('Matrix4#invert', () => {
  m4a.clone().invert();
});

suite.add('Matrix4#translation', () => {
  Matrix4.translation(new Vector3(1, 2, 3));
});

suite.add('Matrix4#rotation', () => {
  Matrix4.rotation(new Vector3(0, 1, 0), Math.PI / 4);
});

suite.add('Matrix4#scale', () => {
  Matrix4.scale(new Vector3(2, 2, 2));
});

suite.add('Matrix4#perspective', () => {
  Matrix4.perspective(Math.PI / 4, 16 / 9, 0.1, 1000);
});

// Quaternion operations
const q1 = new Quaternion(0, 0, 0, 1);
const q2 = Quaternion.fromAxisAngle(new Vector3(0, 1, 0), Math.PI / 4);

suite.add('Quaternion#multiply', () => {
  q1.clone().multiply(q2);
});

suite.add('Quaternion#normalize', () => {
  q1.clone().normalize();
});

suite.add('Quaternion#slerp', () => {
  Quaternion.slerp(q1, q2, 0.5);
});

suite.add('Quaternion#fromAxisAngle', () => {
  Quaternion.fromAxisAngle(new Vector3(0, 1, 0), Math.PI / 4);
});

suite.add('Quaternion#toMatrix4', () => {
  q1.toMatrix4();
});

// Run the benchmark
suite
  .on('cycle', (event: Benchmark.Event) => {
    console.log(String(event.target));
  })
  .on('complete', function (this: Benchmark.Suite) {
    console.log('\n=== Math Operations Benchmark Results ===');
    console.log('Fastest is ' + this.filter('fastest').map('name'));
    console.log('\nBaseline Metrics:');
    this.forEach((bench: Benchmark) => {
      const opsPerSec = bench.hz ? bench.hz.toFixed(0) : 'N/A';
      console.log(`  ${bench.name}: ${opsPerSec} ops/sec`);
    });
  })
  .run({ async: true });
