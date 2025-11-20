/**
 * ECS Performance Benchmark
 *
 * Benchmarks for Entity-Component-System operations including:
 * - Entity creation and destruction
 * - Component addition and removal
 * - System updates and queries
 * - World operations
 */

import Benchmark from 'benchmark';
import { World, Entity, Component, System } from '../packages/engine/src/ecs';
import { TransformComponent } from '../packages/engine/src/ecs/components/TransformComponent';

// Test component classes
class HealthComponent extends Component {
  health: number = 100;
  maxHealth: number = 100;
}

class VelocityComponent extends Component {
  x: number = 0;
  y: number = 0;
  z: number = 0;
}

class RenderComponent extends Component {
  visible: boolean = true;
  color: string = '#ffffff';
}

// Test system
class MovementSystem extends System {
  update(deltaTime: number): void {
    const entities = this.world.query({
      all: [TransformComponent, VelocityComponent],
    });
    for (const entity of entities) {
      const transform = entity.getComponent(TransformComponent);
      const velocity = entity.getComponent(VelocityComponent);
      if (transform && velocity) {
        transform.position.x += velocity.x * deltaTime;
        transform.position.y += velocity.y * deltaTime;
        transform.position.z += velocity.z * deltaTime;
      }
    }
  }
}

const suite = new Benchmark.Suite('ECS Performance');

// Entity creation and destruction
suite.add('Entity#create', () => {
  const world = new World();
  world.createEntity();
});

suite.add('Entity#create (100 entities)', () => {
  const world = new World();
  for (let i = 0; i < 100; i++) {
    world.createEntity();
  }
});

suite.add('Entity#destroy', () => {
  const world = new World();
  const entity = world.createEntity();
  world.destroyEntity(entity);
});

// Component operations
suite.add('Component#add', () => {
  const world = new World();
  const entity = world.createEntity();
  entity.addComponent(TransformComponent);
});

suite.add('Component#add (multiple)', () => {
  const world = new World();
  const entity = world.createEntity();
  entity.addComponent(TransformComponent);
  entity.addComponent(HealthComponent);
  entity.addComponent(VelocityComponent);
});

suite.add('Component#remove', () => {
  const world = new World();
  const entity = world.createEntity();
  entity.addComponent(TransformComponent);
  entity.removeComponent(TransformComponent);
});

suite.add('Component#get', () => {
  const world = new World();
  const entity = world.createEntity();
  entity.addComponent(TransformComponent);
  entity.getComponent(TransformComponent);
});

suite.add('Component#has', () => {
  const world = new World();
  const entity = world.createEntity();
  entity.addComponent(TransformComponent);
  entity.hasComponent(TransformComponent);
});

// Query operations
suite.add('Query#all (single component, 100 entities)', () => {
  const world = new World();
  for (let i = 0; i < 100; i++) {
    const entity = world.createEntity();
    entity.addComponent(TransformComponent);
  }
  world.query({ all: [TransformComponent] });
});

suite.add('Query#all (multiple components, 100 entities)', () => {
  const world = new World();
  for (let i = 0; i < 100; i++) {
    const entity = world.createEntity();
    entity.addComponent(TransformComponent);
    entity.addComponent(HealthComponent);
  }
  world.query({ all: [TransformComponent, HealthComponent] });
});

suite.add('Query#any (100 entities)', () => {
  const world = new World();
  for (let i = 0; i < 100; i++) {
    const entity = world.createEntity();
    if (i % 2 === 0) {
      entity.addComponent(TransformComponent);
    } else {
      entity.addComponent(HealthComponent);
    }
  }
  world.query({ any: [TransformComponent, HealthComponent] });
});

suite.add('Query#none (100 entities)', () => {
  const world = new World();
  for (let i = 0; i < 100; i++) {
    const entity = world.createEntity();
    entity.addComponent(TransformComponent);
    if (i % 2 === 0) {
      entity.addComponent(HealthComponent);
    }
  }
  world.query({ all: [TransformComponent], none: [HealthComponent] });
});

// System operations
suite.add('System#update (100 entities)', () => {
  const world = new World();
  const system = new MovementSystem(world);
  world.addSystem(system);

  for (let i = 0; i < 100; i++) {
    const entity = world.createEntity();
    entity.addComponent(TransformComponent);
    const velocity = entity.addComponent(VelocityComponent);
    velocity.x = 1;
    velocity.y = 1;
    velocity.z = 1;
  }

  world.update(0.016);
});

suite.add('System#update (1000 entities)', () => {
  const world = new World();
  const system = new MovementSystem(world);
  world.addSystem(system);

  for (let i = 0; i < 1000; i++) {
    const entity = world.createEntity();
    entity.addComponent(TransformComponent);
    const velocity = entity.addComponent(VelocityComponent);
    velocity.x = 1;
    velocity.y = 1;
    velocity.z = 1;
  }

  world.update(0.016);
});

// Complex scenario
suite.add('Complex ECS scenario (create, update, destroy)', () => {
  const world = new World();
  const system = new MovementSystem(world);
  world.addSystem(system);

  // Create entities
  const entities: Entity[] = [];
  for (let i = 0; i < 50; i++) {
    const entity = world.createEntity();
    entity.addComponent(TransformComponent);
    const velocity = entity.addComponent(VelocityComponent);
    velocity.x = Math.random();
    velocity.y = Math.random();
    velocity.z = Math.random();
    entities.push(entity);
  }

  // Update world
  world.update(0.016);

  // Destroy some entities
  for (let i = 0; i < 10; i++) {
    world.destroyEntity(entities[i]!);
  }

  // Process deferred operations
  world.update(0.016);
});

// Run the benchmark
suite
  .on('cycle', (event: Benchmark.Event) => {
    console.log(String(event.target));
  })
  .on('complete', function (this: Benchmark.Suite) {
    console.log('\n=== ECS Performance Benchmark Results ===');
    console.log('Fastest is ' + this.filter('fastest').map('name'));
    console.log('\nBaseline Metrics:');
    this.forEach((bench: Benchmark) => {
      const opsPerSec = bench.hz ? bench.hz.toFixed(0) : 'N/A';
      console.log(`  ${bench.name}: ${opsPerSec} ops/sec`);
    });
  })
  .run({ async: true });
