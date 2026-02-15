# Bu Physics API (Box2D)

This file documents the current physics API exposed by Bu from:
- `main/src/box2d.cpp`
- `main/src/box2d_joints.cpp`

## Units and conventions

- Position/size arguments are in **pixels** unless explicitly noted.
- Internally physics runs in Box2D world units with `30 px = 1 meter`.
- Angles exposed to Bu are mostly in **degrees** (except body angular velocity, which is raw Box2D units).
- Boolean args accept `bool` or numeric (`0`/`!=0`) in many APIs.
- `Body`, `Fixture`, and all `*Joint` classes are native class instances.

## Global functions

- `create_physics()`
- `create_physics(gx, gy)`
- Alias: `create_world(...)`
  - Creates/resets world.

- `update_physics(dt)`
- `update_physics(dt, velocityIterations)`
- `update_physics(dt, velocityIterations, positionIterations)`
- Alias: `update_world(...)`

- `destroy_physics()`
- Alias: `clean_world()`

- `set_physics_debug(enabled)`
- Alias: `debug_physics(enabled)`

- `set_physics_debug_flags(bitmask)`

- `get_body_count() -> int`
- Alias: `body_count()`

- `physics_collide(idA, idB) -> bool`
- Alias: `body_collide(idA, idB)`

- `physics_collide_with(typeOrProcess) -> int`
- Alias: `body_collide_with(typeOrProcess)`
  - Must be called inside a process.
  - Returns collided process id or `-1`.

- `physics_collision() -> (idA, idB)`
  - Pops one collision event from internal queue.
  - Returns `(-1, -1)` if none.

- `physics_raycast(x1, y1, x2, y2) -> (id, hitX, hitY, normalX, normalY)`
- `physics_raycast(x1, y1, x2, y2, typeOrIgnoreSelf) -> (...)`
- `physics_raycast(x1, y1, x2, y2, type, ignoreSelf) -> (...)`
- Alias: `body_raycast(...)`
  - Returns `id = -1` when no hit.

- `physics_overlap_point(x, y) -> (id, body)`
- `physics_overlap_point(x, y, typeOrIgnoreSelf) -> (id, body)`
- `physics_overlap_point(x, y, type, ignoreSelf) -> (id, body)`
- Alias: `body_overlap_point(...)`
  - `body` is `Body` or `nil`.

- `physics_overlap_rect(x, y, w, h) -> id`
- `physics_overlap_rect(x, y, w, h, typeOrIgnoreSelf) -> id`
- `physics_overlap_rect(x, y, w, h, type, ignoreSelf) -> id`
- Alias: `body_overlap_rect(...)`

- `physics_overlap_circle(x, y, radius) -> id`
- `physics_overlap_circle(x, y, radius, typeOrIgnoreSelf) -> id`
- `physics_overlap_circle(x, y, radius, type, ignoreSelf) -> id`
- Alias: `body_overlap_circle(...)`

- `create_fixture_def() -> FixtureDef`
- `create_bodydef([type]) -> BodyDef`
- `create_body(bodyDef) -> Body` (process native function)

## Global constants

- `BODY_DYNAMIC`
- `BODY_STATIC`
- `BODY_KINEMATIC`
- `SHAPE_BOX`
- `SHAPE_CIRCLE`
- `BODY_SYNC_AUTO`
- `BODY_SYNC_PROCESS_TO_BODY`
- `BODY_SYNC_BODY_TO_PROCESS`
- `b2_pbdStretchingModel`
- `b2_xpbdStretchingModel`
- `b2_springAngleBendingModel`
- `b2_pbdAngleBendingModel`
- `b2_xpbdAngleBendingModel`
- `b2_pbdDistanceBendingModel`
- `b2_pbdHeightBendingModel`
- `b2_pbdTriangleBendingModel`

## Class: BodyDef

Constructor:
- `BodyDef()`
- `BodyDef(type)`

Methods:
- `set_type(type)`
- `set_position(x, y)` (pixels)
- `set_linear_velocity(x, y)` (raw Box2D units)
- `set_angle(degrees)`
- `set_angular_velocity(v)` (raw Box2D units)
- `set_linear_damping(v)`
- `set_angular_damping(v)`
- `set_gravity_scale(v)`
- `set_allow_sleep(enabled)`
- `set_awake(enabled)`
- `set_fixed_rotation(enabled)`
- `set_bullet(enabled)`
- `set_enabled(enabled)`

## Class: FixtureDef

Constructor:
- `FixtureDef()`

Filter/material:
- `set_density(v)`
- `set_friction(v)`
- `set_restitution(v)`
- `set_sensor(enabled)`
- `set_filter(categoryBits, maskBits, groupIndex)`
- `set_category_bits(bits)`
- `set_mask_bits(bits)`
- `add_mask_bits(bits)`
- `remove_mask_bits(bits)`
- `set_group_index(group)`

Aliases:
- `set_category(bits)`
- `set_mask(bits)`
- `set_group(group)`

Shape assignment:
- `set_circle_shape(radius)`
- `set_circle_shape(radius, centerX, centerY)`
- `set_box_shape(halfW, halfH)`
- `set_box_shape(halfW, halfH, centerX, centerY, angleDegrees)`
- `set_edge_shape(x1, y1, x2, y2)`
- `set_chain_shape(points)`
- `set_chain_shape(points, loop)`
- `set_polygon_shape(points)` (convex only)
- `clear_shape()`

## Class: Fixture

Constructor:
- Not for direct creation (`Body.add_fixture` returns it).

Methods:
- `set_sensor(enabled)`
- `set_filter(categoryBits, maskBits, groupIndex)`
- `set_category_bits(bits)`
- `set_mask_bits(bits)`
- `add_mask_bits(bits)`
- `remove_mask_bits(bits)`
- `set_group_index(group)`

Aliases:
- `set_category(bits)`
- `set_mask(bits)`
- `set_group(group)`

## Class: Body

Constructor:
- `Body(bodyDef)` (requires world created first)

Lifecycle:
- `remove()`

Transform/state:
- `set_transform(x, y, angleDegrees)` (`x,y` in pixels)
- `get_position() -> (x, y)` (pixels)
- `set_angle(degrees)`
- `get_angle() -> degrees`
- `set_linear_velocity(x, y)` (raw Box2D units)
- `get_linear_velocity() -> (x, y)` (raw Box2D units)
- `set_angular_velocity(v)` (raw Box2D units)
- `get_angular_velocity() -> number` (raw Box2D units)
- `set_gravity_scale(v)`
- `get_gravity_scale() -> number`
- `set_awake(enabled)`
- `is_awake() -> bool`
- `set_fixed_rotation(enabled)`
- `is_fixed_rotation() -> bool`
- `set_bullet(enabled)`
- `is_bullet() -> bool`
- `get_type() -> BODY_*`
- `get_mass() -> number`
- `get_inertia() -> number`

Forces:
- `apply_force(x, y)`
- `apply_impulse(x, y)`

Filter (applies to body fixtures):
- `set_filter(categoryBits, maskBits, groupIndex)`
- `set_category_bits(bits)`
- `set_mask_bits(bits)`
- `add_mask_bits(bits)`
- `remove_mask_bits(bits)`
- `set_group_index(group)`

Aliases:
- `set_category(bits)`
- `set_mask(bits)`
- `set_group(group)`

Shape/fixture creation:
- `add_box(halfW, halfH)`
- `add_box(halfW, halfH, fixtureDef)`
- `add_circle(radius)`
- `add_circle(radius, fixtureDef)`
- `add_edge(x1, y1, x2, y2)`
- `add_edge(x1, y1, x2, y2, fixtureDef)`
- `add_chain(points)`
- `add_chain(points, loop)`
- `add_chain(points, fixtureDef)`
- `add_chain(points, loop, fixtureDef)`
- `add_fixture(fixtureDef) -> Fixture`
- `add_polygon(points)`
- `add_polygon(points, fixtureDef)`
  - Returns created fixture count (`int`).
  - Handles concave polygons by triangulating.

## Shape points format

`points` for chain/polygon APIs must be a flat numeric array:
- `[x0, y0, x1, y1, x2, y2, ...]`

## Class: MouseJointDef

Constructor:
- `MouseJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, x, y)` (`x,y` pixels)
- `set_target(x, y)` (pixels)
- `set_max_force(force)`
- `set_force(force)` (alias)
- `set_stiffness(v)`
- `set_damping(v)`
- `set_collide_connected(enabled)`

## Class: MouseJoint

Constructor:
- `MouseJoint(mouseJointDef)`
- `MouseJoint(bodyB, mouseJointDef)`

Methods:
- `set_target(x, y)` (pixels)
- `target(x, y)` (alias)
- `set_max_force(force)`
- `set_force(force)` (alias)
- `set_stiffness(v)`
- `set_damping(v)`
- `get_target() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: RevoluteJointDef

Constructor:
- `RevoluteJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, x, y)` (`x,y` pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_reference_angle(degrees)`
- `set_enable_limit(enabled)`
- `set_limits(lowerDeg, upperDeg)`
- `set_enable_motor(enabled)`
- `set_motor_speed(degreesPerSec)`
- `set_max_motor_torque(v)`
- `set_collide_connected(enabled)`

## Class: RevoluteJoint

Constructor:
- `RevoluteJoint(revoluteJointDef)`
- `RevoluteJoint(bodyA, bodyB, revoluteJointDef)`

Methods:
- `enable_limit(enabled)`
- `set_limits(lowerDeg, upperDeg)`
- `enable_motor(enabled)`
- `set_motor_speed(degreesPerSec)`
- `set_max_motor_torque(v)`
- `get_joint_angle() -> degrees`
- `get_joint_speed() -> degreesPerSec`
- `get_motor_torque(inv_dt) -> number`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: WheelJointDef

Constructor:
- `WheelJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, anchorX, anchorY, axisX, axisY)` (`anchor` in pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_local_axis_a(x, y)` (raw axis vector)
- `set_enable_motor(enabled)`
- `set_max_motor_torque(v)`
- `set_motor_speed(degreesPerSec)`
- `set_stiffness(v)`
- `set_damping(v)`
- `set_collide_connected(enabled)`

## Class: WheelJoint

Constructor:
- `WheelJoint(wheelJointDef)`
- `WheelJoint(bodyA, bodyB, wheelJointDef)`

Methods:
- `enable_motor(enabled)`
- `set_max_motor_torque(v)`
- `set_motor_speed(degreesPerSec)`
- `set_stiffness(v)`
- `set_damping(v)`
- `get_motor_speed() -> degreesPerSec`
- `get_joint_translation() -> pixels`
- `get_joint_linear_speed() -> pixelsPerSec`
- `get_motor_torque(inv_dt) -> number`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: MotorJointDef

Constructor:
- `MotorJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB)`
- `set_linear_offset(x, y)` (pixels)
- `set_angular_offset(degrees)`
- `set_max_force(v)`
- `set_max_torque(v)`
- `set_correction_factor(v)`
- `set_collide_connected(enabled)`

## Class: MotorJoint

Constructor:
- `MotorJoint(motorJointDef)`
- `MotorJoint(bodyA, bodyB, motorJointDef)`

Methods:
- `set_linear_offset(x, y)` (pixels)
- `get_linear_offset() -> (x, y)` (pixels)
- `set_angular_offset(degrees)`
- `get_angular_offset() -> degrees`
- `set_max_force(v)`
- `get_max_force() -> number`
- `set_max_torque(v)`
- `get_max_torque() -> number`
- `set_correction_factor(v)`
- `get_correction_factor() -> number`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: RopeJointDef

Constructor:
- `RopeJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, ax, ay, bx, by)` (pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_max_length(pixels)`
- `set_collide_connected(enabled)`

## Class: RopeJoint

Constructor:
- `RopeJoint(ropeJointDef)`
- `RopeJoint(bodyA, bodyB, ropeJointDef)`

Methods:
- `set_max_length(pixels)`
- `get_max_length() -> pixels`
- `get_current_length() -> pixels`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: b2RopeTuning

Constructor:
- `b2RopeTuning()`

Methods:
- `set_stretching_model(model)`
- `set_bending_model(model)`
- `set_damping(v)`
- `set_stretch_stiffness(v)`
- `set_stretch_hertz(v)`
- `set_stretch_damping(v)`
- `set_bend_stiffness(v)`
- `set_bend_hertz(v)`
- `set_bend_damping(v)`
- `set_isometric(enabled)`
- `set_fixed_effective_mass(enabled)`
- `set_warm_start(enabled)`

## Class: b2RopeDef

Constructor:
- `b2RopeDef()`

Methods:
- `set_position(x, y)` (pixels)
- `set_gravity(gx, gy)`
- `set_tuning(b2RopeTuning)`
- `set_vertices(points, masses)`  
`points`: `[x0, y0, x1, y1, ...]` in pixels (min 3 points)  
`masses`: `[m0, m1, ...]` same point count
- `clear_vertices()`

## Class: b2Rope

Constructor:
- `b2Rope()`

Methods:
- `create(b2RopeDef)`
- `set_tuning(b2RopeTuning)`
- `step(dt, iterations, x, y)` (`x,y` in pixels)
- `reset(x, y)` (pixels)

## Class: DistanceJointDef

Constructor:
- `DistanceJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, ax, ay, bx, by)` (pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_length(pixels)`
- `set_min_length(pixels)`
- `set_max_length(pixels)`
- `set_stiffness(v)`
- `set_damping(v)`
- `set_collide_connected(enabled)`

## Class: DistanceJoint

Constructor:
- `DistanceJoint(distanceJointDef)`
- `DistanceJoint(bodyA, bodyB, distanceJointDef)`

Methods:
- `set_length(pixels)`
- `set_min_length(pixels)`
- `set_max_length(pixels)`
- `set_stiffness(v)`
- `set_damping(v)`
- `get_length() -> pixels`
- `get_current_length() -> pixels`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: PrismaticJointDef

Constructor:
- `PrismaticJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, anchorX, anchorY, axisX, axisY)` (`anchor` in pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_local_axis_a(x, y)` (raw axis vector)
- `set_reference_angle(degrees)`
- `set_enable_limit(enabled)`
- `set_limits(lowerPixels, upperPixels)`
- `set_enable_motor(enabled)`
- `set_motor_speed(pixelsPerSec)`
- `set_max_motor_force(v)`
- `set_collide_connected(enabled)`

## Class: PrismaticJoint

Constructor:
- `PrismaticJoint(prismaticJointDef)`
- `PrismaticJoint(bodyA, bodyB, prismaticJointDef)`

Methods:
- `enable_limit(enabled)`
- `set_limits(lowerPixels, upperPixels)`
- `enable_motor(enabled)`
- `set_motor_speed(pixelsPerSec)`
- `set_max_motor_force(v)`
- `get_joint_translation() -> pixels`
- `get_joint_speed() -> pixelsPerSec`
- `get_motor_force(inv_dt) -> number`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: PulleyJointDef

Constructor:
- `PulleyJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, gax, gay, gbx, gby, ax, ay, bx, by, ratio)` (`x,y` in pixels)
- `set_ground_anchor_a(x, y)` (pixels)
- `set_ground_anchor_b(x, y)` (pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_length_a(pixels)`
- `set_length_b(pixels)`
- `set_ratio(v)` (`v > 0`)
- `set_collide_connected(enabled)`

## Class: PulleyJoint

Constructor:
- `PulleyJoint(pulleyJointDef)`
- `PulleyJoint(bodyA, bodyB, pulleyJointDef)`

Methods:
- `get_ratio() -> number`
- `get_length_a() -> pixels`
- `get_length_b() -> pixels`
- `get_current_length_a() -> pixels`
- `get_current_length_b() -> pixels`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `get_ground_anchor_a() -> (x, y)` (pixels)
- `get_ground_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: FrictionJointDef

Constructor:
- `FrictionJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `initialize(bodyA, bodyB, x, y)` (`x,y` in pixels)
- `set_local_anchor_a(x, y)` (pixels)
- `set_local_anchor_b(x, y)` (pixels)
- `set_max_force(v)`
- `set_max_torque(v)`
- `set_collide_connected(enabled)`

## Class: FrictionJoint

Constructor:
- `FrictionJoint(frictionJointDef)`
- `FrictionJoint(bodyA, bodyB, frictionJointDef)`

Methods:
- `set_max_force(v)`
- `get_max_force() -> number`
- `set_max_torque(v)`
- `get_max_torque() -> number`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Class: GearJointDef

Constructor:
- `GearJointDef()`

Methods:
- `set_body_a(body)`
- `set_body_b(body)`
- `set_joint1(joint)` (`RevoluteJoint` or `PrismaticJoint`)
- `set_joint2(joint)` (`RevoluteJoint` or `PrismaticJoint`)
- `set_ratio(v)`
- `set_collide_connected(enabled)`

## Class: GearJoint

Constructor:
- `GearJoint(gearJointDef)`
- `GearJoint(bodyA, bodyB, gearJointDef)`

Methods:
- `set_ratio(v)`
- `get_ratio() -> number`
- `get_anchor_a() -> (x, y)` (pixels)
- `get_anchor_b() -> (x, y)` (pixels)
- `destroy()`
- `exists() -> bool`

## Notes

- Joint/body creation requires a valid world (`create_physics` / `create_world` first).
- Joint creation can fail while world is locked (during physics step).
- `RopeJoint` here is implemented as a compatibility layer over `b2DistanceJoint` (`minLength=0`), because this Box2D version does not expose `b2RopeJoint`.
- `add_polygon` accepts concave input but creates multiple fixtures via triangulation.
- `set_polygon_shape` on `FixtureDef` requires convex polygon.
