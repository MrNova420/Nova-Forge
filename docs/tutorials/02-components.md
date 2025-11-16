# Working with Components

Components are the building blocks of Nova Engine's Entity-Component-System (ECS) architecture. This tutorial covers built-in components and how to create custom ones.

## Built-in Components

### Transform Component

Every entity automatically has a Transform component:

```javascript
const entity = scene.createEntity('MyEntity');

// Position
entity.transform.position.set(5, 0, -3);
entity.transform.position.x += 1;

// Rotation (in radians)
entity.transform.rotation.set(0, Math.PI / 4, 0);

// Scale
entity.transform.scale.set(2, 1, 1); // Scale X by 2

// Look at a target
entity.transform.lookAt({ x: 10, y: 0, z: 0 });

// Parent-child relationships
const parent = scene.createEntity('Parent');
const child = scene.createEntity('Child');
child.transform.setParent(parent.transform);
```

### MeshRenderer Component

Renders 3D meshes:

```javascript
import { MeshRenderer, Material } from '@nova-engine/engine';

const entity = scene.createEntity('Cube');
const renderer = entity.addComponent(MeshRenderer);

// Set mesh
renderer.mesh = engine.primitives.createCube();
// or load from file
renderer.mesh = await engine.assetManager.load('models/character.gltf');

// Set material
renderer.material = new Material(engine.shaders.get('standard'));
renderer.material.setProperty('baseColor', { r: 1, g: 0, b: 0, a: 1 });

// Visibility
renderer.visible = true;
renderer.castShadows = true;
renderer.receiveShadows = true;
```

### Camera Component

Controls the viewing frustum:

```javascript
import { Camera } from '@nova-engine/engine';

const camEntity = scene.createEntity('MainCamera');
const camera = camEntity.addComponent(Camera);

// Perspective projection
camera.setPerspective(
  Math.PI / 4, // FOV (45 degrees)
  16 / 9, // Aspect ratio
  0.1, // Near plane
  1000 // Far plane
);

// Orthographic projection
camera.setOrthographic(
  -10,
  10, // Left, right
  -10,
  10, // Bottom, top
  0.1,
  100 // Near, far
);

// Set as active camera
engine.setActiveCamera(camera);
```

### Light Components

#### Directional Light (Sun)

```javascript
import { DirectionalLight } from '@nova-engine/engine';

const sunEntity = scene.createEntity('Sun');
const sun = sunEntity.addComponent(DirectionalLight);
sun.color = { r: 1, g: 1, b: 0.9 };
sun.intensity = 1.5;
sunEntity.transform.rotation.set(-Math.PI / 4, Math.PI / 4, 0);
```

#### Point Light

```javascript
import { PointLight } from '@nova-engine/engine';

const lightEntity = scene.createEntity('PointLight');
const light = lightEntity.addComponent(PointLight);
light.color = { r: 1, g: 0.5, b: 0 };
light.intensity = 2.0;
light.range = 10;
lightEntity.transform.position.set(0, 5, 0);
```

#### Spot Light

```javascript
import { SpotLight } from '@nova-engine/engine';

const spotEntity = scene.createEntity('Spotlight');
const spot = spotEntity.addComponent(SpotLight);
spot.color = { r: 1, g: 1, b: 1 };
spot.intensity = 3.0;
spot.range = 20;
spot.innerAngle = Math.PI / 8;
spot.outerAngle = Math.PI / 6;
```

### RigidBody Component (Physics)

```javascript
import { RigidBody, BoxCollider } from '@nova-engine/engine';

const entity = scene.createEntity('PhysicsBox');

// Add rigidbody
const rb = entity.addComponent(RigidBody);
rb.type = 'dynamic'; // or 'static', 'kinematic'
rb.mass = 1.0;
rb.friction = 0.5;
rb.restitution = 0.3; // Bounciness

// Add collider
const collider = entity.addComponent(BoxCollider);
collider.size.set(1, 1, 1);

// Apply forces
rb.applyForce({ x: 0, y: 100, z: 0 });
rb.applyImpulse({ x: 10, y: 0, z: 0 });
rb.applyTorque({ x: 0, y: 5, z: 0 });

// Set velocity
rb.linearVelocity = { x: 5, y: 0, z: 0 };
rb.angularVelocity = { x: 0, y: 1, z: 0 };
```

### AudioSource Component

```javascript
import { AudioSource } from '@nova-engine/engine';

const entity = scene.createEntity('MusicPlayer');
const audio = entity.addComponent(AudioSource);

// Load and play audio
const clip = await engine.assetManager.load('sounds/music.mp3', 'audio');
audio.clip = clip;
audio.play();

// Audio properties
audio.volume = 0.8;
audio.pitch = 1.0;
audio.loop = true;

// 3D audio
audio.spatial = true;
audio.minDistance = 1;
audio.maxDistance = 50;
```

### Animator Component

```javascript
import { Animator, AnimationClip } from '@nova-engine/engine';

const entity = scene.createEntity('Character');
const animator = entity.addComponent(Animator);

// Load animation clips
const walkClip = await engine.assetManager.load('anims/walk.gltf');
const runClip = await engine.assetManager.load('anims/run.gltf');

// Play animations
animator.play('walk');
animator.crossFade('run', 0.3); // Blend over 0.3 seconds

// Animation events
animator.onAnimationEnd('walk', () => {
  console.log('Walk animation finished');
});
```

## Creating Custom Components

### Basic Custom Component

```javascript
import { Component } from '@nova-engine/engine';

export class Rotator extends Component {
  constructor() {
    super();
    this.speed = 1.0;
  }

  // Called when component is added to entity
  onAttach() {
    console.log('Rotator attached to', this.entity.name);
  }

  // Called every frame
  update(deltaTime) {
    this.entity.transform.rotation.y += this.speed * deltaTime;
  }

  // Called when component is removed
  onDetach() {
    console.log('Rotator detached');
  }

  // Serialize component data
  serialize() {
    return { speed: this.speed };
  }

  // Deserialize component data
  deserialize(data) {
    this.speed = data.speed;
  }
}

// Usage
const entity = scene.createEntity('RotatingCube');
const rotator = entity.addComponent(Rotator);
rotator.speed = 2.0;
```

### Component with Properties

```javascript
import { Component, Vector3 } from '@nova-engine/engine';

export class Oscillator extends Component {
  constructor() {
    super();
    this.amplitude = 1.0;
    this.frequency = 1.0;
    this.axis = new Vector3(0, 1, 0); // Oscillate on Y axis
    this._initialPosition = new Vector3();
    this._time = 0;
  }

  onAttach() {
    this._initialPosition.copy(this.entity.transform.position);
  }

  update(deltaTime) {
    this._time += deltaTime;
    const offset = Math.sin(this._time * this.frequency) * this.amplitude;

    this.entity.transform.position.copy(this._initialPosition);
    this.entity.transform.position.add(
      this.axis.clone().multiplyScalar(offset)
    );
  }

  reset() {
    this._time = 0;
    this.entity.transform.position.copy(this._initialPosition);
  }
}
```

### Component Communication

```javascript
// Component A
export class HealthComponent extends Component {
  constructor() {
    super();
    this.maxHealth = 100;
    this.currentHealth = 100;
  }

  takeDamage(amount) {
    this.currentHealth -= amount;
    if (this.currentHealth <= 0) {
      this.entity.emit('died', this.entity);
    }
  }
}

// Component B
export class DeathHandler extends Component {
  onAttach() {
    this.entity.on('died', (entity) => {
      console.log(entity.name, 'has died!');
      this.playDeathAnimation();
    });
  }

  playDeathAnimation() {
    const animator = this.entity.getComponent(Animator);
    animator?.play('death');
  }
}

// Usage
const enemy = scene.createEntity('Enemy');
const health = enemy.addComponent(HealthComponent);
const deathHandler = enemy.addComponent(DeathHandler);

health.takeDamage(150); // Triggers death
```

## Component Lifecycle

```javascript
export class MyComponent extends Component {
  constructor() {
    super();
    // 1. Constructor - Initialize properties
  }

  onAttach() {
    // 2. Component added to entity
    // Access entity, scene, engine
  }

  onEnable() {
    // 3. Component enabled (default: true)
  }

  start() {
    // 4. First frame after onAttach
    // All other components are attached
  }

  update(deltaTime) {
    // 5. Called every frame
  }

  lateUpdate(deltaTime) {
    // 6. After all update() calls
  }

  onDisable() {
    // 7. Component disabled
  }

  onDetach() {
    // 8. Component removed from entity
  }
}
```

## Best Practices

### 1. Keep Components Focused

```javascript
// ❌ Bad: Doing too much
class Player extends Component {
  handleInput() {
    /*...*/
  }
  updatePhysics() {
    /*...*/
  }
  playAnimations() {
    /*...*/
  }
  checkCollisions() {
    /*...*/
  }
}

// ✅ Good: Separate responsibilities
class PlayerInput extends Component {
  /*...*/
}
class PlayerMovement extends Component {
  /*...*/
}
class PlayerAnimator extends Component {
  /*...*/
}
```

### 2. Use Systems for Cross-Component Logic

```javascript
// For logic that affects multiple components
class MovementSystem {
  update(entities, deltaTime) {
    for (const entity of entities) {
      const movement = entity.getComponent(Movement);
      const rigidbody = entity.getComponent(RigidBody);

      if (movement && rigidbody) {
        rigidbody.velocity = movement.velocity;
      }
    }
  }
}
```

### 3. Cache Component References

```javascript
// ❌ Bad: Looking up every frame
update(deltaTime) {
  const rb = this.entity.getComponent(RigidBody); // Slow!
  rb.applyForce(this.force);
}

// ✅ Good: Cache in onAttach
onAttach() {
  this._rigidbody = this.entity.getComponent(RigidBody);
}

update(deltaTime) {
  this._rigidbody?.applyForce(this.force); // Fast!
}
```

### 4. Clean Up Resources

```javascript
onDetach() {
  // Cancel timers
  if (this._timer) clearInterval(this._timer);

  // Remove event listeners
  this.entity.off('damaged', this._onDamaged);

  // Release resources
  if (this._texture) this._texture.dispose();
}
```

## Next Steps

- [Materials and Shaders](./03-materials.md) - Customize visual appearance
- [Physics Integration](./04-physics.md) - Add realistic physics
- [Input Handling](./05-input.md) - Respond to user input

---

**Master the component system to build anything! 🚀**
