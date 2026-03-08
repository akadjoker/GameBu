# BuGame Script API (Bindings)

This file documents the script API currently exposed by the C++ bindings in `main/src`.

Source files used:
- `main/src/bindings.cpp`
- `main/src/input.cpp`
- `main/src/process.cpp`
- `main/src/draw.cpp`
- `main/src/particles.cpp`
- `main/src/main.cpp`


## Notes

- `number` means numeric values accepted by the VM (`int`/`double`).
- Process-native functions are only valid inside a `process`.
- A few names intentionally follow current code spelling (example: `atach`).
- This doc mirrors runtime names exactly as registered.

## Core Language Features & Concepts

Beyond the API functions, the scripting language has features that are crucial for using the API effectively.

### Modules and Plugins

- **`import <module>;`**: Makes a module's functions and constants available. You must access them with the module name as a prefix (e.g., `math.sin(x)`). `import *;` is also supported to import all available modules.
- **`using <module>;`**: After importing, this allows you to use a module's members directly without a prefix (e.g., `sin(x)` instead of `math.sin(x)`). The compiler will flag an error if the same name exists in multiple `using` modules.
- **`require "<plugin>";`**: Loads a dynamic plugin (e.g., a `.dll` or `.so` file) that can register new native functions, classes, and modules with the engine. You can require multiple plugins by separating them with commas or semicolons inside the string (e.g., `require "sdl,net";`).

### Multiple Return Values

Many functions in the API can return more than one value. This is a core feature of the language. You can capture these values using a special multi-assignment syntax.

**Example:**
```
var (mx, my) = get_mouse_position();
log("Mouse is at: ", mx, ", ", my);
```

### Processes

A `process` is a fundamental concept in the engine, typically representing a game object or actor with its own logic and state. Many functions are "process-native" and can only be called from within a `process` block.

### User-Defined Types

The language supports creating your own `struct`s and `class`es. While this document focuses on the native API, you can define your own data structures to organize your game's code and state.

## Global Constants

- `SKILL = 0`
- `SFREEZE = 1`
- `SHIDE = 2`
- `SSHOW = 3`
- `PATH_ASTAR`
- `PATH_DIJKSTRA`
- `PF_MANHATTAN`
- `PF_EUCLIDEAN`
- `PF_OCTILE`
- `PF_CHEBYSHEV`

- `BODY_DYNAMIC = 0`
- `BODY_STATIC = 1`
- `BODY_KINEMATIC = 2`

- `b2_pbdStretchingModel`
- `b2_xpbdStretchingModel`
- `b2_springAngleBendingModel`
- `b2_pbdAngleBendingModel`
- `b2_xpbdAngleBendingModel`
- `b2_pbdDistanceBendingModel`
- `b2_pbdHeightBendingModel`
- `b2_pbdTriangleBendingModel`

- `BLEND_ALPHA`
- `BLEND_ADDITIVE`
- `BLEND_MULTIPLIED`
- `BLEND_ADD_COLORS`
- `BLEND_SUBTRACT_COLORS`
- `BLEND_ALPHA_PREMULTIPLY`
- `SHADER_NONE = -1`

## Native Structs

- `Color(r, g, b, a)`
- `Vec2(x, y)`

## Native Class: `Path`

Constructor:
- `Path(width, height, resolution)`

Methods:
- `set_occupied(x, y)`
- `set_free(x, y)`
- `clear_all()`
- `is_occupied(x, y) -> bool`
- `is_walkable(x, y) -> bool`
- `load_from_image(path, [threshold=128])`
- `get_width() -> int`
- `get_height() -> int`
- `get_resolution() -> int`
- `world_to_grid(x, y) -> (gx, gy)`
- `grid_to_world(x, y) -> (wx, wy)`
- `find(sx, sy, ex, ey, [diag=1], [algo=PATH_ASTAR], [heur=PF_MANHATTAN]) -> x1, y1, x2, y2, ...`
- `fill_from_layer(layer, [use_solid=1], [clear_first=1]) -> int`

`fill_from_layer()` behavior:
- `use_solid=1`: use `tile.solid`
- `use_solid=0`: use `tile.id != 0`
- return value is the number of mask cells marked occupied

`find()` return behavior:
- pushes flat coordinates on stack (`x, y` pairs)
- function return count is `2 * number_of_points`

## Native Class: `Emitter`

`Emitter` instances are created by factory functions (below), not by direct constructor call.

Methods:
- `set_position(x, y)`
- `set_direction(x, y)`
- `set_emission_rate(rate)`
- `set_life(life)`
- `set_speed_range(min, max)`
- `set_spread(radians)`
- `set_color_curve(startColor, endColor)`
- `set_size_curve(startSize, endSize)`
- `set_spawn_zone(x, y, w, h)`
- `set_lifetime(time)`
- `set_gravity(x, y)`
- `set_drag(drag)`
- `set_rotation_range(min, max)`
- `set_angular_vel_range(min, max)`
- `set_blend_mode(blendMode)`
- `set_layer(layer)`

## Particle Factory Functions

All return an `Emitter`.

- `create_emitter(persistent, graph, maxParticles)`
- `create_fire(x, y, graph)`
- `create_smoke(x, y, graph)`
- `create_explosion(x, y, graph, color)`
- `create_sparks(x, y, graph, color)`
- `create_landing_dust(x, y, graph, facingRight)`
- `create_wall_impact(x, y, graph, hitFromLeft, size_start, size_end)`
- `create_water_splash(x, y, graph)`
- `create_run_trail(x, y, graph, size_min, size_max)`
- `create_speed_lines(x, y, graph, velX, velY)`
- `create_collect_effect(x, y, graph, itemColor)`
- `create_power_up_aura(x, y, graph, auraColor)`
- `create_sparkle(x, y, graph)`
- `create_blood_splatter(x, y, graph, hitDirectionX, hitDirectionY)`
- `create_rain(x, y, graph, width)`
- `create_shell_ejection(x, y, graph, facingRight)`
- `create_muzzle_flash(x, y, graph, shootDirection)`

## Core Global Functions

### Graphics / Assets

- `load_graph(path) -> graphId`
- `load_atlas(texturePath, countX, countY) -> graphId`
- `load_subgraph(parentId, name, x, y, width, height) -> graphId`
- `save_graphics(filename)`
- `load_graphics(filename)`
- `set_graphics_point(graphics, x, y)`

### Mesh (PolyMesh)

- `create_mesh() -> meshId`
- `mesh_clear(meshId)`
- `mesh_add_point(meshId, x, y)`
- `mesh_build_track(meshId, depth)`
- `mesh_build_polygon(meshId, [uv_scale=0.01])`
- `mesh_set_texture(meshId, graphId)`
- `mesh_set_body_texture(meshId, graphId)`
- `mesh_set_edge_texture(meshId, graphId)`
- `mesh_set_scale_top(meshId, scaleX, scaleY)`
- `mesh_set_scale_bottom(meshId, scaleX, scaleY)`
- `mesh_draw(meshId, x, y, rotation, scale, [screen_space=false])`

Aliases for scale (same behavior):
- `set_scale_top(meshId, scaleX, scaleY)`
- `set_scale_bottom(meshId, scaleX, scaleY)`
- `set_scalke_top(meshId, scaleX, scaleY)` (legacy typo alias)
- `set_sclae_bottom(meshId, scaleX, scaleY)` (legacy typo alias)

`mesh_build_track(meshId, depth)` current fixed internal parameters (from bindings):
- `body_u_scale = 0.010`
- `edge_width = 26.0`
- `edge_u_scale = 0.020`
- `body_v_scale = 1.0`
- `edge_v_scale = 1.0`

 

Example:
 
var m = create_mesh();
mesh_add_point(m, 0, 340);
mesh_add_point(m, 320, 300);
mesh_add_point(m, 640, 360);

mesh_set_scale_top(m, 1.0, 1.0);
mesh_set_scale_bottom(m, 2.0, 10.0);
mesh_build_track(m, 860);
mesh_set_body_texture(m, BODY_TEX);
mesh_set_edge_texture(m, EDGE_TEX);
mesh_draw(m, 0, 0, 0, 1);
 

### Collision / Process Helpers

- `proc(processId) -> processHandle|nil`
- `type(processId) -> string`
- `signal(processId, signalType)`
- `exists(processId) -> bool`
- `get_id(typeName) -> processHandle|-1`

### Sound

Functions for playing short sound effects, which are loaded completely into memory.

- `load_sound(path) -> soundId`
- `play_sound(soundId, [volume=1.0], [pitch=1.0])`
- `stop_sound(soundId)`
- `pause_sound(soundId)`
- `resume_sound(soundId)`
- `is_sound_playing(soundId) -> bool`

### Music

Functions for streaming long audio files like background music from disk.

**IMPORTANT:** You must call `update_music_streams()` once per frame in your main loop for music to play correctly.

- `load_music(path) -> musicId`
- `play_music(musicId)`
- `stop_music(musicId)`
- `pause_music(musicId)`
- `resume_music(musicId)`
- `set_music_volume(musicId, volume)`
- `is_music_playing(musicId) -> bool`

### Audio Control
- `update_music_streams()`: Updates all active music streams. **Must be called once per frame.**

### Layer / Scroll

- `set_layer_mode(layer, mode)`
- `set_layer_scroll_factor(layer, x, y)`
- `set_layer_size(layer, x, y, width, height)`
- `set_layer_back_graph(layer, graph)`
- `set_layer_front_graph(layer, graph)`
- `set_scroll(x, y)`

### Tilemap

- `set_tile_map(layer, map_width, map_height, tile_width, tile_height, columns, graph)`
- `set_tile_map_spacing(layer, spacing)`
- `set_tile_map_free(layer, tile_id)`
- `set_tile_map_solid(layer, tile_id)`
- `set_tile_map_margin(layer, margin)`
- `set_tile_map_mode(layer, mode)`
- `set_tile_map_color(layer, color)`
- `set_tile_debug(layer, grid, ids)`
- `set_tile_map_iso_compression(layer, compression)`
- `set_tile_map_tile(layer, x, y, tile, solid)`  
 
- `get_tile_map_tile(layer, x, y) -> tile`
- `has_tile_map(layer) -> bool`
- `import_tilemap(filename) -> bool`

### Time

- `delta() -> seconds`
- `time() -> seconds`

### Math 

- `get_distx(angle, distance) -> number`
- `get_disty(angle, distance) -> number`
- `get_angle(x1, y1, x2, y2) -> number`
- `get_dist(x1, y1, x2, y2) -> number`
- `near_angle(current, target, step) -> number`
- `normalize_angle(angle) -> number`

## Input Functions

- `key_down(keyCode) -> bool`
- `key_pressed(keyCode) -> bool`
- `key_released(keyCode) -> bool`
- `key_up(keyCode) -> bool`
- `get_key_pressed() -> int`
- `get_char_pressed() -> int`

- `mouse_pressed(button) -> bool`
- `mouse_down(button) -> bool`
- `mouse_released(button) -> bool`
- `mouse_up(button) -> bool`

- `get_mouse_x() -> number`
- `get_mouse_y() -> number`
- `get_mouse_position() -> (x, y)`
- `get_mouse_delta() -> (dx, dy)`
- `set_mouse_position(x, y)`
- `set_mouse_offset(offsetX, offsetY)`
- `set_mouse_scale(scaleX, scaleY)`

## Draw Functions

- `draw_line(x1, y1, x2, y2)`
- `draw_circle(centerX, centerY, radius, fill)`
- `draw_point(x, y)`
- `draw_text(text, x, y, size)`
- `draw_rectangle(x, y, width, height, fill)`
- `load_font(path) -> fontId`
- `set_font(fontId)` (`-1` volta para a fonte default)
- `reset_font()`
- `set_color(red, green, blue)`
- `set_alpha(alpha)`
 

- `start_fade(targetAlpha, speed)`
- `is_fade_complete() -> bool`
- `get_fade_progress() -> number`
- `fade_in(speed)`
- `fade_out(speed)`

## Box2D Physics API

This API provides bindings for the Box2D physics engine. Note that coordinates and sizes are in pixels and are automatically converted to meters for Box2D (30 pixels = 1 meter).

### Global Functions

- `create_physics([gravityX=0, gravityY=9.8])`: Initializes the physics world.
- `update_physics(deltaTime, [velocityIterations=8], [positionIterations=3])`: Steps the physics simulation.
- `destroy_physics()`: Destroys the physics world.
- `set_physics_debug(enabled)`: Enables or disables debug rendering.
- `set_physics_debug_flags(flags)`: Sets the debug draw flags (e.g., `b2Draw.e_shapeBit`).
- `get_body_count() -> int`: Returns the number of bodies in the world.
- `physics_collide(idA, idB) -> bool`: Checks if two processes are currently in contact.
- `physics_collide_with(processType) -> processId`: Returns the ID of the first process of the given type that is colliding with the current process.
- `physics_collision() -> idA, idB`: Pops and returns the IDs of two processes from the collision event queue.
- `physics_contact_count() -> int`: Returns the number of collision events in the queue.
- `physics_contact_at(index) -> idA, idB, x, y`: Returns contact information at a specific index in the event queue.
- `physics_contact_clear()`: Clears the collision event queue.
- `physics_raycast(x1, y1, x2, y2, [type], [ignoreSelf]) -> hitId, hitX, hitY, normalX, normalY`: Performs a raycast.
- `physics_overlap_point(x, y, [type], [ignoreSelf]) -> hitId, bodyInstance`: Checks for bodies at a point.
- `physics_overlap_rect(x, y, w, h, [type], [ignoreSelf]) -> hitId`: Checks for bodies in a rectangle.
- `physics_overlap_circle(x, y, radius, [type], [ignoreSelf]) -> hitId`: Checks for bodies in a circle.
- `create_bodydef([type]) -> BodyDef`: Creates a new body definition.
- `create_fixture_def() -> FixtureDef`: Creates a new fixture definition.

### Process-Native Functions

- `create_body(bodyDef) -> Body`: Creates a physics body from a `BodyDef` and attaches it to the current process.

### Native Class: `BodyDef`

Used to define the properties of a new `Body`.

- `set_type(type)`: Sets the body type (`BODY_STATIC`, `BODY_KINEMATIC`, `BODY_DYNAMIC`).
- `set_position(x, y)`: Sets the initial position in pixels.
- `set_angle(degrees)`: Sets the initial angle in degrees.
- `set_linear_velocity(vx, vy)`
- `set_angular_velocity(vel)`
- `set_linear_damping(damping)`
- `set_angular_damping(damping)`
- `set_gravity_scale(scale)`
- `set_allow_sleep(allow)`
- `set_awake(isAwake)`
- `set_fixed_rotation(isFixed)`
- `set_bullet(isBullet)`
- `set_enabled(isEnabled)`

### Native Class: `FixtureDef`

Used to define the properties of a new `Fixture`.

- `set_density(density)`
- `set_friction(friction)`
- `set_restitution(restitution)`
- `set_sensor(isSensor)`
- `set_filter(categoryBits, maskBits, groupIndex)`
- `set_category_bits(bits)`
- `set_mask_bits(bits)`
- `add_mask_bits(bits)`
- `remove_mask_bits(bits)`
- `set_group_index(index)`
- `set_circle_shape(radius, [centerX, centerY])`: All values in pixels.
- `set_box_shape(halfWidth, halfHeight, [centerX, centerY, angleDegrees])`: All values in pixels/degrees.
- `set_edge_shape(x1, y1, x2, y2)`
- `set_chain_shape(pointsArray, [loop=false])`
- `set_polygon_shape(pointsArray)`: For convex polygons only.
- `clear_shape()`

### Native Class: `Body`

Represents a rigid body in the physics world.

- `remove()`: Destroys the body.
- `set_transform(x, y, angleDegrees)`
- `get_position() -> x, y`
- `set_linear_velocity(vx, vy)`
- `get_linear_velocity() -> vx, vy`
- `set_angular_velocity(vel)`
- `get_angular_velocity() -> vel`
- `apply_force(forceX, forceY)`
- `apply_impulse(impulseX, impulseY)`
- `set_gravity_scale(scale)`
- `get_gravity_scale() -> scale`
- `set_awake(isAwake)`
- `is_awake() -> bool`
- `set_fixed_rotation(isFixed)`
- `is_fixed_rotation() -> bool`
- `set_bullet(isBullet)`
- `is_bullet() -> bool`
- `get_mass() -> mass`
- `get_inertia() -> inertia`
- `get_angle() -> degrees`
- `set_angle(degrees)`
- `get_type() -> bodyType`
- `set_filter(category, mask, group)`
- `set_category_bits(bits)`
- `set_mask_bits(bits)`
- `add_mask_bits(bits)`
- `remove_mask_bits(bits)`
- `set_group_index(index)`
- `add_box(halfWidth, halfHeight, [fixtureDef])`: Adds a box fixture.
- `add_circle(radius, [fixtureDef])`: Adds a circle fixture.
- `add_edge(x1, y1, x2, y2, [fixtureDef])`: Adds an edge fixture.
- `add_chain(points, [loop], [fixtureDef])`: Adds a chain shape fixture.
- `add_polygon(points, [fixtureDef]) -> int`: Adds a (possibly concave) polygon, triangulating it. Returns number of fixtures created.
- `add_fixture(fixtureDef) -> Fixture`: Adds a pre-configured fixture.

### Native Class: `Fixture`

Represents a shape attached to a body.

- `set_sensor(isSensor)`
- `set_filter(category, mask, group)`
- `set_category_bits(bits)`
- `set_mask_bits(bits)`
- `add_mask_bits(bits)`
- `remove_mask_bits(bits)`
- `set_group_index(index)`

### Joints API

Joints are created by instantiating a `JointDef` class, configuring it, and then passing it to the constructor of the corresponding `Joint` class.

**Common `JointDef` Methods:**
- `set_body_a(body)`
- `set_body_b(body)`
- `set_collide_connected(bool)`

#### Revolute Joint
- `RevoluteJointDef()`: Methods: `initialize(bodyA, bodyB, anchorX, anchorY)`, `set_local_anchor_a/b`, `set_reference_angle`, `set_enable_limit`, `set_limits(lowerDeg, upperDeg)`, `set_enable_motor`, `set_motor_speed(deg/s)`, `set_max_motor_torque`.
- `RevoluteJoint(def)`: Methods: `enable_limit`, `set_limits`, `enable_motor`, `set_motor_speed`, `set_max_motor_torque`, `get_joint_angle`, `get_joint_speed`, `get_motor_torque`, `get_anchor_a/b`, `destroy`, `exists`.

#### Prismatic Joint
- `PrismaticJointDef()`: Methods: `initialize(bodyA, bodyB, anchorX, anchorY, axisX, axisY)`, `set_local_anchor_a/b`, `set_local_axis_a`, `set_reference_angle`, `set_enable_limit`, `set_limits(lowerPx, upperPx)`, `set_enable_motor`, `set_motor_speed(px/s)`, `set_max_motor_force`.
- `PrismaticJoint(def)`: Methods: `enable_limit`, `set_limits`, `enable_motor`, `set_motor_speed`, `set_max_motor_force`, `get_joint_translation`, `get_joint_speed`, `get_motor_force`, `get_anchor_a/b`, `destroy`, `exists`.

#### Distance Joint
- `DistanceJointDef()`: Methods: `initialize(bodyA, bodyB, anchorAx, anchorAy, anchorBx, anchorBy)`, `set_local_anchor_a/b`, `set_length(px)`, `set_min_length(px)`, `set_max_length(px)`, `set_stiffness`, `set_damping`.
- `DistanceJoint(def)`: Methods: `set_length`, `set_min_length`, `set_max_length`, `set_stiffness`, `set_damping`, `get_length`, `get_current_length`, `get_anchor_a/b`, `destroy`, `exists`.

#### Pulley Joint
- `PulleyJointDef()`: Methods: `initialize(bodyA, bodyB, groundA_x, groundA_y, groundB_x, groundB_y, anchorA_x, anchorA_y, anchorB_x, anchorB_y, ratio)`, `set_ground_anchor_a/b`, `set_local_anchor_a/b`, `set_length_a/b`, `set_ratio`.
- `PulleyJoint(def)`: Methods: `get_ratio`, `get_length_a/b`, `get_current_length_a/b`, `get_anchor_a/b`, `get_ground_anchor_a/b`, `destroy`, `exists`.

#### Mouse Joint
- `MouseJointDef()`: Methods: `initialize(bodyA, bodyB, targetX, targetY)`, `set_target`, `set_max_force`, `set_stiffness`, `set_damping`.
- `MouseJoint(def)`: Methods: `set_target`, `set_max_force`, `set_stiffness`, `set_damping`, `get_target`, `destroy`, `exists`.

#### Gear Joint
- `GearJointDef()`: Methods: `set_joint1(revoluteOrPrismaticJoint)`, `set_joint2(revoluteOrPrismaticJoint)`, `set_ratio`.
- `GearJoint(def)`: Methods: `set_ratio`, `get_ratio`, `get_anchor_a/b`, `destroy`, `exists`.

#### Wheel Joint
- `WheelJointDef()`: Methods: `initialize(bodyA, bodyB, anchorX, anchorY, axisX, axisY)`, `set_local_anchor_a/b`, `set_local_axis_a`, `set_enable_motor`, `set_max_motor_torque`, `set_motor_speed(deg/s)`, `set_stiffness`, `set_damping`.
- `WheelJoint(def)`: Methods: `enable_motor`, `set_max_motor_torque`, `set_motor_speed`, `set_stiffness`, `set_damping`, `get_motor_speed`, `get_joint_translation`, `get_joint_linear_speed`, `get_motor_torque`, `get_anchor_a/b`, `destroy`, `exists`.

#### Motor Joint
- `MotorJointDef()`: Methods: `initialize(bodyA, bodyB)`, `set_linear_offset(x,y)`, `set_angular_offset(deg)`, `set_max_force`, `set_max_torque`, `set_correction_factor`.
- `MotorJoint(def)`: Methods: `set/get_linear_offset`, `set/get_angular_offset`, `set/get_max_force`, `set/get_max_torque`, `set/get_correction_factor`, `get_anchor_a/b`, `destroy`, `exists`.

#### Friction Joint
- `FrictionJointDef()`: Methods: `initialize(bodyA, bodyB, anchorX, anchorY)`, `set_local_anchor_a/b`, `set_max_force`, `set_max_torque`.
- `FrictionJoint(def)`: Methods: `set/get_max_force`, `set/get_max_torque`, `get_anchor_a/b`, `destroy`, `exists`.

### Rope API

Provides access to the `b2Rope` simulation for realistic ropes.

#### Native Class: `b2RopeTuning`

- `b2RopeTuning()`: Constructor.
- Methods: `set_stretching_model`, `set_bending_model`, `set_damping`, `set_stretch_stiffness`, `set_stretch_hertz`, `set_stretch_damping`, `set_bend_stiffness`, `set_bend_hertz`, `set_bend_damping`, `set_isometric`, `set_fixed_effective_mass`, `set_warm_start`.

#### Native Class: `b2RopeDef`

- `b2RopeDef(count)`: Constructor, takes the number of vertices.
- Methods: `set_position`, `set_gravity`, `set_tuning(b2RopeTuning)`, `set_vertices(pointsArray, massesArray)`, `clear_vertices`. Also includes direct setters for all tuning parameters.

#### Native Class: `b2Rope`

- `b2Rope()`: Constructor.
- `create(b2RopeDef)`: Initializes the rope simulation from a definition.
- `set_tuning(b2RopeTuning)`: Updates the rope's tuning parameters.
- `step(deltaTime, iterations, positionX, positionY)`: Advances the simulation.
- `reset(x, y)`: Resets the rope to a specific position.
- `get_count() -> int`: Returns the number of vertices.
- `get_point(index) -> x, y`: Returns the position of a vertex.

## Simple Physics & Process Functions

These are process-native functions from the engine's simpler, built-in physics and helper system. They can be used alongside or independently of the Box2D API.

### Movement & Collision
- `advance(speed)`: Moves the process forward based on its angle.
- `xadvance(speed, angle)`: Moves the process in a specific direction.
- `place_free(x, y) -> bool`: Checks if the process would be free of collisions at a new position.
- `place_meeting(x, y) -> processId|-1`: Checks for a collision at a new position and returns the colliding process.
- `collision(typeName, x, y) -> processId|-1`: A more specific version of `place_meeting`.
- `move_and_collide(vx, vy) -> bool`: Moves the process and stops on collision.
- `move_and_slide(vx, vy, [up_x=0, up_y=-1]) -> bool`: Moves with sliding behavior.

### Shape & Properties
- `set_rect_shape(x, y, w, h)`: Sets a simple rectangle collision shape.
- `set_circle_shape(radius)`: Sets a simple circle collision shape.
- `set_collision_layer(layer)`
- `set_collision_mask(mask)`
- `add_collision_mask(layer)`
- `remove_collision_mask(layer)`
- `set_static()`
- `enable_collision()`
- `disable_collision()`

### Hierarchy & Helpers
- `atach(childProcID, front)`: Attaches another process as a child.
- `out_screen() -> bool`: Checks if the process is outside the camera view.
- `set_layer(layer)`
- `get_layer() -> int`
- `flip_vertical(enabled)`
- `flip_horizontal(enabled)`
- `set_visible(enabled)`
- `flip(flipX, flipY)`

### Math & Targeting
- `get_point(pointIndex) -> x, y`: Gets a point from the process's graphic.
- `get_real_point(pointIndex) -> x, y`: Gets a point from the graphic transformed into world space.
- `get_local_point(x, y) -> localX, localY`: Converts world coordinates to the process's local space.
- `get_world_point(x, y) -> worldX, worldY`: Converts the process's local coordinates to world space.
- `fget_angle(processID) -> number`: Gets the angle to another process.
- `fget_dist(processID) -> number`: Gets the distance to another process.
- `turn_to(processID, step)`: Turns the process towards another process by a given step.

## Triangulation API (Poly2Tri)

Provides bindings to the Poly2Tri library for constrained Delaunay triangulation. This is useful for converting complex polygons into a set of triangles for rendering or physics.

- `triangulate(points) -> triangle_points`
  - **points**: A flat array of vertex coordinates defining the polygon outline, e.g., `[x1, y1, x2, y2, ...]`.
  - **Returns**: A flat array of vertex coordinates for the resulting triangles, e.g., `[t1_x1, t1_y1, t1_x2, t1_y2, t1_x3, t1_y3, t2_x1, ...]`.

## Messaging API

A simple queue-based system for communication between processes. This is more flexible than `signal()` as it allows passing arbitrary data.

### Global Functions
- `send_message(recipient_id, type, [data1, data2, ...])`: Sends a message with a custom integer `type` and optional data arguments to a specific process ID.
- `broadcast(process_type, message_type, [data1, ...])`: Sends a message to all living processes of a given blueprint type.

### Process-Native Functions
- `has_message() -> bool`: Returns `true` if the current process has an unread message in its queue.
- `get_message() -> sender_id, type, data_array`: Retrieves the next message from the queue. Returns the sender's process ID, the message type, and an array containing all additional data arguments. Returns `nil` if no message is available.
- `clear_messages()`: Clears all pending messages for the current process.

**Note:** The global message queue is cleared at the end of each frame.

## Image API

Provides functions for creating, manipulating, and loading images in memory. These can then be converted to graphics for drawing.

### Native Class: `Image`

An in-memory representation of an image.

**Constructor:**
- `Image(width, height)`: Creates a new blank 32-bit (RGBA) image.

**Methods:**
- `get_width() -> int`
- `get_height() -> int`
- `get_bpp() -> int`: Returns bytes per pixel (1, 2, 3, or 4).
- `set_pixel(x, y, color)` or `set_pixel(x, y, r, g, b, [a])`: Sets the color of a pixel.
- `get_pixel(x, y) -> r, g, b, a`: Returns the color components of a pixel.
- `fill(color)` or `fill(r, g, b, [a])`: Fills the entire image with a color.
- `resize(width, height)`: Resizes the image using bicubic scaling.
- `resize_nn(width, height)`: Resizes the image using nearest-neighbor scaling.
- `flip_horizontal()`
- `flip_vertical()`
- `rotate(degrees)`: Rotates the image clockwise.
- `rotate_cw()`: Rotates the image 90 degrees clockwise.
- `rotate_ccw()`: Rotates the image 90 degrees counter-clockwise.
- `load(path) -> bool`: Loads image data from a file into this instance.
- `save(path) -> bool`: Saves the image data to a file (e.g., ".png", ".bmp").
- `to_graph([name]) -> graphId`: Creates a new drawable graphic from the image and returns its ID.
- `update_graph(graphId) -> bool`: Updates the texture of an existing graphic with this image's data.
- `blit(srcImage, dstX, dstY)` or `blit(srcImage, dstX, dstY, srcX, srcY, srcW, srcH)`: Copies pixels from another `Image`.
- `crop(x, y, w, h)`: Crops this image in-place to the given rectangle.
- `draw_rect(x, y, w, h, r, g, b[, a], [fill=true])`: Draws a rectangle into the image.
- `draw_circle(x, y, radius, r, g, b[, a], [fill=true])`: Draws a circle into the image.
- `draw_line(x1, y1, x2, y2, r, g, b[, a])`: Draws a line into the image.

### Global Image Functions

- `create_image(width, height, [bpp=4]) -> Image`: Creates a new blank image instance.
- `create_image_from_file(path) -> Image|nil`: Loads an image from a file into a new `Image` instance.
- `get_image_info(path) -> width, height, bpp`: Returns image dimensions and bytes per pixel without loading the full image.

## Drawing API

Drawing functions can operate in world space (affected by camera and scroll) or screen space.

### Drawing State
- `set_color(r, g, b)`: Sets the drawing color for subsequent draw calls.
- `set_alpha(alpha)`: Sets the alpha component of the drawing color.
- `set_draw_layer(layer)`: Sets the target layer for world-space drawing.
- `set_draw_screen(isScreenSpace)`: If `true`, subsequent draw calls are in screen space (ignoring camera). If `false`, they are in world space.

### Primitives
- `draw_point(x, y)`
- `draw_line(x1, y1, x2, y2)`
- `draw_line_ex(x1, y1, x2, y2, thickness)`
- `draw_circle(centerX, centerY, radius, fill)`
- `draw_ellipse(centerX, centerY, radiusX, radiusY, fill)`
- `draw_ring(centerX, centerY, innerRadius, outerRadius, startAngle, endAngle, fill)`
- `draw_rectangle(x, y, width, height, fill)`
- `draw_rotated_rectangle(x, y, width, height, rotation, fill)`
- `draw_rotated_rectangle_ex(x, y, width, height, rotation, fill, originX, originY)`
- `draw_triangle(x1, y1, x2, y2, x3, y3, fill)`

### Text & Fonts
- `load_font(path) -> fontId`
- `draw_text(text, x, y, size)`
- `draw_font(text, x, y, size, spacing, fontId)`
- `draw_font_rotate(text, x, y, size, rotation, spacing, pivotX, pivotY, fontId)`
- `get_text_width(text, size) -> width`
- `get_font_text_width(text, size, spacing, fontId) -> width`
- `draw_fps(x, y)`

### Graphics
- `draw_graph(graphId, x, y)`
- `draw_graph_ex(graphId, x, y, angle, sizeX, sizeY, flipX, flipY)`
- `draw_graph_part(graphId, srcX, srcY, srcW, srcH, x, y)`
- `draw_graph_part_ex(graphId, srcX, srcY, srcW, srcH, x, y, angle, sizeX, sizeY, flipX, flipY)`
- `get_graph_width(graphId) -> width`
- `get_graph_height(graphId) -> height`

### Clipping
- `clip_begin(x, y, width, height)` (alias: `set_clip_rect`)
- `clip_end()` (alias: `clear_clip_rect`)

## Window, Camera & Screen Effects

### Window
- `set_window_size(width, height)`
- `set_window_title(title)`
- `set_fullscreen(enabled)`
- `set_window_resizable(enabled)`
- `set_log_level(level)`

### Camera
- `set_camera_zoom(zoom)`
- `set_camera_rotation(rotation)`
- `set_camera_target(x, y)`
- `set_camera_offset(x, y)`
- `get_camera_zoom() -> zoom`
- `get_camera_rotation() -> rotation`
- `get_camera_target() -> x, y`
- `get_camera_offset() -> x, y`
- `get_camera_x() -> x`
- `get_camera_y() -> y`
- `start_camera_shake(amplitudeX, amplitudeY, frequency, durationCycles)`
- `stop_camera_shake()`

### Screen Scaling & Viewport
- `set_design_resolution(width, height)`
- `set_virtual_screen_enabled(enabled)`
- `set_screen_scale_mode(mode)`
- `get_viewport() -> x, y, width, height`
- `get_fit_scale() -> scale`

### Screen Effects
- `start_fade(targetAlpha, speed)`
- `is_fade_complete() -> bool`
- `get_fade_progress() -> number`
- `fade_in(speed)`
- `fade_out(speed)`

### Post-Processing
- `enable_post_processing(enabled)`: Enables or disables the post-processing system.
- `add_post_processing_pass(pass)`: Adds a configured `RenderPass` object to the post-processing pipeline.
- `clear_post_processing_passes()`: Removes all shaders from the post-processing pipeline.

**Note:** The post-processing pipeline (which involves rendering to an off-screen texture) is only activated if `enable_post_processing` is `true`. Otherwise, the engine renders directly to the screen with no performance overhead.

### Native Class: `RenderPass`

Represents a single step in the post-processing pipeline.

**Constructor:**
- `RenderPass(shaderId)`: Creates a new pass with the given shader.

**Methods:**
- `set_clear(enabled, [color])` or `set_clear(enabled, [r, g, b, a])`: Configures if the render target should be cleared before this pass.
- `set_size(width, height)`: Sets a custom resolution for this pass. Use 0 for default screen size. **Note: This feature is not fully implemented and will be ignored for now.**

## Shaders & Blending

### Blending
- `set_blend_mode(mode)` (alias: `set_blend`)
- `reset_blend_mode()` (alias: `reset_blend`)

### Shaders
- `load_shader(vertexPath, fragmentPath) -> shaderId`
- `load_shader_file(fragmentPath) -> shaderId`
- `load_shader_auto(basePath) -> shaderId`
- `unload_shader(shaderId)`
- `set_shader(shaderId)` (alias: `set_material_shader`)
- `reset_shader()` (alias: `reset_material_shader`)

### Shader Uniforms
- `set_shader_uniform_float(shaderId, name, value)`
- `set_shader_uniform_int(shaderId, name, value)`
- `set_shader_uniform_vec2(shaderId, name, x, y)`
- `set_shader_uniform_vec3(shaderId, name, x, y, z)`
- `set_shader_uniform_vec4(shaderId, name, x, y, z, w)`
