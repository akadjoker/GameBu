# BuGL Game Engine — Script API Reference

> Complete API reference for the BuGL game engine runtime.  
> Language syntax is documented separately in `BULANG_SYNTAX_REFERENCE.md`.

---

## Table of Contents

1. [Process Model](#1-process-model)
2. [Global Constants](#2-global-constants)
3. [Core Functions](#3-core-functions)
4. [Drawing](#4-drawing)
5. [Text & Fonts](#5-text--fonts)
6. [Graphics & Sprites](#6-graphics--sprites)
7. [Input](#7-input)
8. [Camera & Viewport](#8-camera--viewport)
9. [Layers, Scroll & Tilemaps](#9-layers-scroll--tilemaps)
10. [Process-Native Functions](#10-process-native-functions)
11. [Particles](#11-particles)
12. [Sound & Music](#12-sound--music)
13. [Image API](#13-image-api)
14. [Mesh (PolyMesh)](#14-mesh-polymesh)
15. [Pathfinding](#15-pathfinding)
16. [Messaging](#16-messaging)
17. [Box2D Physics](#17-box2d-physics)
18. [Shaders & Blending](#18-shaders--blending)
19. [Screen Effects](#19-screen-effects)
20. [Easing Functions](#20-easing-functions)
21. [Triangulation](#21-triangulation)
22. [Window Management](#22-window-management)
23. [Debug](#23-debug)

---

## 1. Process Model

A `process` is the fundamental game entity. Each process has **native properties** (privates) that are automatically synced to the rendering engine. These properties do not need `var` — they are pre-declared inside any `process` block.

### Native Properties

| Property | Index | Default | Description |
|----------|-------|---------|-------------|
| `x` | 0 | 0 | X position in world (pixels) |
| `y` | 1 | 0 | Y position in world (pixels) |
| `z` | 2 | 0 | Draw depth (higher = in front) |
| `graph` | 3 | 0 | Sprite/graphic ID |
| `angle` | 4 | 0 | Rotation in degrees |
| `size` | 5 | 100 | *Legacy — no longer affects rendering.* Use `sizex`/`sizey`. |
| `flags` | 6 | 0 | Internal flags |
| `id` | 7 | (auto) | Unique process ID (read-only) |
| `father` | 8 | 0 | Parent process ID |
| `red` | 9 | 255 | Tint red (0–255) |
| `green` | 10 | 255 | Tint green (0–255) |
| `blue` | 11 | 255 | Tint blue (0–255) |
| `alpha` | 12 | 255 | Opacity (0–255) |
| `tag` | 13 | 0 | User tag |
| `state` | 14 | 0 | User state (for state machines) |
| `speed` | 15 | 0 | User speed |
| `group` | 16 | 0 | User group |
| `velx` | 17 | 0 | Velocity X |
| `vely` | 18 | 0 | Velocity Y |
| `hp` | 19 | 0 | Hit points |
| `progress` | 20 | 0 | User progress |
| `life` | 21 | 100 | Life/timer |
| `active` | 22 | 1 | Active flag |
| `show` | 23 | 1 | Show flag |
| `xold` | 24 | 0 | Previous X |
| `yold` | 25 | 0 | Previous Y |
| `sizex` | 26 | 100 | Horizontal scale (percentage, 100 = original size) |
| `sizey` | 27 | 100 | Vertical scale (percentage, 100 = original size) |

### Process Control Keywords

| Keyword | Description |
|---------|-------------|
| `frame;` | Yields execution until the next game frame |
| `loop { ... }` | Infinite loop (use with `frame;` inside) |
| `break;` | Exit current loop — kills the process if inside the main loop |

### Process Helper Functions (Global)

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `proc(processId)` | 1 | process handle or `nil` | Get a handle to a living process by ID |
| `type(processId)` | 1 | string | Get the type name of a process |
| `signal(processId, signalType)` | 2 | — | Send a signal to a process |
| `exists(processId)` | 1 | bool | Check if a process is alive |
| `get_id(typeName)` | 1 | process handle or -1 | Get first process of a given type |
| `count_processes(typeName)` | 1 | int | Count living processes of a type |
| `get_ids(typeName)` | 1 | array | Get array of all process IDs of a type |

### Signal Types

| Constant | Value | Description |
|----------|-------|-------------|
| `SKILL` | 0 | Kill the process |
| `SFREEZE` | 1 | Freeze the process |
| `SHIDE` | 2 | Hide the process |
| `SSHOW` | 3 | Show the process |

### Scaling

The `sizex` and `sizey` properties control entity scaling as a **percentage**:
- `sizex = 100; sizey = 100;` — original size (default)
- `sizex = 200; sizey = 200;` — 2× scale
- `sizex = 50; sizey = 100;` — half width, original height

These affect rendering, collision bounds, and all transform calculations.

---

## 2. Global Constants

### Blend Modes

| Constant | Description |
|----------|-------------|
| `BLEND_ALPHA` | Standard alpha blending |
| `BLEND_ADDITIVE` | Additive blending |
| `BLEND_MULTIPLIED` | Multiply blending |
| `BLEND_ADD_COLORS` | Add colors |
| `BLEND_SUBTRACT_COLORS` | Subtract colors |
| `BLEND_ALPHA_PREMULTIPLY` | Pre-multiplied alpha |
| `SHADER_NONE` | No shader (-1) |

### Physics Body Types

| Constant | Value |
|----------|-------|
| `BODY_DYNAMIC` | 0 |
| `BODY_STATIC` | 1 |
| `BODY_KINEMATIC` | 2 |

### Pathfinding

| Constant | Description |
|----------|-------------|
| `PATH_ASTAR` | A* algorithm |
| `PATH_DIJKSTRA` | Dijkstra algorithm |
| `PF_MANHATTAN` | Manhattan heuristic |
| `PF_EUCLIDEAN` | Euclidean heuristic |
| `PF_OCTILE` | Octile heuristic |
| `PF_CHEBYSHEV` | Chebyshev heuristic |

### Key Codes

Key codes are **not** built-in constants. Define them in your script:

```bu
var KEY_UP    = 265;
var KEY_DOWN  = 264;
var KEY_LEFT  = 263;
var KEY_RIGHT = 262;
var KEY_SPACE = 32;
var KEY_ENTER = 257;
var KEY_ESCAPE = 256;
var KEY_W = 87;
var KEY_A = 65;
var KEY_S = 83;
var KEY_D = 68;
var KEY_R = 82;
var KEY_P = 80;
var KEY_F = 70;
// Letters: A=65, B=66, ... Z=90
// Numbers: 0=48, 1=49, ... 9=57
// F-keys: F1=290, F2=291, ... F12=301
```

---

## 3. Core Functions

### Time

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `delta()` | 0 | number | Seconds since last frame |
| `time()` | 0 | number | Total elapsed seconds |
| `get_fps()` | 0 | int | Current frames per second |

### Math Helpers

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `get_distx(angle, distance)` | 2 | number | X component of a vector |
| `get_disty(angle, distance)` | 2 | number | Y component of a vector |
| `get_angle(x1, y1, x2, y2)` | 4 | number | Angle between two points (degrees) |
| `get_dist(x1, y1, x2, y2)` | 4 | number | Distance between two points |
| `near_angle(current, target, step)` | 3 | number | Step an angle toward a target |
| `normalize_angle(angle)` | 1 | number | Normalize angle to 0–360 |
| `angle_delta(a, b)` | 2 | number | Shortest delta between two angles |
| `slerp_angle(a, b, t)` | 3 | number | Spherical interpolation between angles |
| `slerp(a, b, t)` | 3 | number | Alias for `slerp_angle` |

> **Note:** Trigonometric functions `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `sqrt`, `pow`, `abs`, `floor`, `ceil`, `log`, `exp`, `deg`, `rad` are **language built-ins** available globally. For the `math` module (`math.rand`, `math.pi`, etc.), see `BULANG_SYNTAX_REFERENCE.md`.

---

## 4. Drawing

Drawing functions can operate in **world space** (affected by camera/scroll) or **screen space**.

### Drawing State

| Function | Args | Description |
|----------|------|-------------|
| `set_color(r, g, b)` | 3 | Set draw color (0–255 each) |
| `set_alpha(alpha)` | 1 | Set draw alpha (0–255) |
| `set_draw_layer(layer)` | 1 | Set target layer for world-space drawing |
| `set_draw_screen(isScreenSpace)` | 1 | `true` = screen space, `false` = world space |

### Primitives

| Function | Args | Description |
|----------|------|-------------|
| `draw_point(x, y)` | 2 | Draw a single pixel |
| `draw_line(x1, y1, x2, y2)` | 4 | Draw a line |
| `draw_line_ex(x1, y1, x2, y2, thickness)` | 5 | Draw a thick line |
| `draw_circle(cx, cy, radius, fill)` | 4 | Draw circle (`fill`: bool) |
| `draw_ellipse(cx, cy, rx, ry, fill)` | 5 | Draw ellipse |
| `draw_ring(cx, cy, innerR, outerR, startAngle, endAngle, fill)` | 7 | Draw ring/arc |
| `draw_rectangle(x, y, w, h, fill)` | 5 | Draw rectangle |
| `draw_rotated_rectangle(x, y, w, h, rotation, fill)` | 6 | Draw rotated rectangle |
| `draw_rotated_rectangle_ex(x, y, w, h, rotation, fill, originX, originY)` | 8 | Draw rotated rect with custom origin |
| `draw_triangle(x1, y1, x2, y2, x3, y3, fill)` | 7 | Draw triangle |
| `draw_rounded_rect(x, y, w, h, roundness, fill)` | 6 | Draw rounded rectangle |
| `draw_arc(cx, cy, radius, startAngle, endAngle, segments)` | 6 | Draw arc |
| `draw_bezier(x1, y1, x2, y2, thickness)` | 5 | Draw quadratic bezier |
| `draw_poly(cx, cy, sides, radius, rotation, fill)` | 6 | Draw regular polygon |
| `draw_gradient_rect(x, y, w, h, r1, g1, b1, r2, g2, b2)` | Varies | Draw gradient rectangle |

### Clipping

| Function | Args | Description |
|----------|------|-------------|
| `clip_begin(x, y, w, h)` | 4 | Begin scissor clip region |
| `clip_end()` | 0 | End scissor clip region |

Aliases: `set_clip_rect` = `clip_begin`, `clear_clip_rect` = `clip_end`

---

## 5. Text & Fonts

| Function | Args | Description |
|----------|------|-------------|
| `draw_text(text, x, y, size)` | 4 | Draw text using current color/font |
| `draw_font(text, x, y, size, spacing, fontId)` | 6 | Draw text with specific font |
| `draw_font_rotate(text, x, y, size, rotation, spacing, pivotX, pivotY, fontId)` | 9 | Draw rotated text |
| `draw_fps(x, y)` | 2 | Draw FPS counter |
| `load_font(path)` | 1 | Load font file → `fontId` |
| `get_text_width(text, size)` | 2 | Get text width in pixels → `number` |
| `get_font_text_width(text, size, spacing, fontId)` | 4 | Get text width with specific font → `number` |

---

## 6. Graphics & Sprites

### Loading

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `load_graph(path)` | 1 | graphId | Load image as a graph |
| `load_atlas(texturePath, countX, countY)` | 3 | graphId | Load sprite atlas (first frame ID) |
| `load_subgraph(parentId, name, x, y, w, h)` | 6 | graphId | Extract sub-region from a graph |
| `save_graphics(filename)` | 1 | — | Save all graphics to file |
| `load_graphics(filename)` | 1 | — | Load graphics from file |
| `set_graphics_point(graphId, x, y)` | 3 | — | Set a control point on a graph |

### Drawing Graphs

| Function | Args | Description |
|----------|------|-------------|
| `draw_graph(graphId, x, y)` | 3 | Draw a graph at position |
| `draw_graph_ex(graphId, x, y, angle, sizeX, sizeY, flipX, flipY)` | 8 | Draw graph with transform |
| `draw_graph_part(graphId, srcX, srcY, srcW, srcH, x, y)` | 7 | Draw a sub-region of a graph |
| `draw_graph_part_ex(graphId, srcX, srcY, srcW, srcH, x, y, angle, sizeX, sizeY, flipX, flipY)` | 12 | Draw sub-region with transform |

### Querying

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `get_graph_width(graphId)` | 1 | int | Width of a graph in pixels |
| `get_graph_height(graphId)` | 1 | int | Height of a graph in pixels |

---

## 7. Input

### Keyboard

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `key_down(keyCode)` | 1 | bool | Key is currently held |
| `key_pressed(keyCode)` | 1 | bool | Key was just pressed this frame |
| `key_released(keyCode)` | 1 | bool | Key was just released this frame |
| `key_up(keyCode)` | 1 | bool | Key is not held |
| `get_key_pressed()` | 0 | int | Get last key pressed (0 if none) |
| `get_char_pressed()` | 0 | int | Get last char pressed (0 if none) |

### Mouse

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `mouse_pressed(button)` | 1 | bool | Button pressed this frame |
| `mouse_down(button)` | 1 | bool | Button currently held |
| `mouse_released(button)` | 1 | bool | Button released this frame |
| `mouse_up(button)` | 1 | bool | Button not held |
| `get_mouse_x()` | 0 | number | Mouse X in world space |
| `get_mouse_y()` | 0 | number | Mouse Y in world space |
| `get_mouse_position()` | 0 | (x, y) | Mouse position in world space |
| `get_mouse_screen_x()` | 0 | number | Mouse X in screen space |
| `get_mouse_screen_y()` | 0 | number | Mouse Y in screen space |
| `get_mouse_screen_position()` | 0 | (x, y) | Mouse position in screen space |
| `get_mouse_delta()` | 0 | (dx, dy) | Mouse movement since last frame |
| `get_mouse_wheel()` | 0 | number | Mouse wheel vertical movement |
| `get_mouse_wheel_x()` | 0 | number | Mouse wheel horizontal movement |
| `get_mouse_wheel_y()` | 0 | number | Mouse wheel vertical movement |
| `set_mouse_position(x, y)` | 2 | — | Set mouse position |
| `set_mouse_offset(offsetX, offsetY)` | 2 | — | Set mouse offset |
| `set_mouse_scale(scaleX, scaleY)` | 2 | — | Set mouse scale |
| `hide_cursor()` | 0 | — | Hide mouse cursor |
| `show_cursor()` | 0 | — | Show mouse cursor |

### Touch

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `touch_count()` | 0 | int | Number of active touch points |
| `touch_down(id)` | 1 | bool | Touch point is active |
| `touch_pressed_any()` | 0 | bool | Any touch started this frame |
| `touch_released_any()` | 0 | bool | Any touch ended this frame |
| `get_touch_id(index)` | 1 | int | Get touch ID by index |
| `get_touch_x(index)` | 1 | number | Touch X in world space |
| `get_touch_y(index)` | 1 | number | Touch Y in world space |
| `get_touch_position(index)` | 1 | (x, y) | Touch position in world space |
| `get_touch_screen_x(index)` | 1 | number | Touch X in screen space |
| `get_touch_screen_y(index)` | 1 | number | Touch Y in screen space |
| `get_touch_screen_position(index)` | 1 | (x, y) | Touch position in screen space |

### Gestures

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `get_gesture()` | 0 | int | Current detected gesture |
| `gesture_detected(gestureType)` | 1 | bool | Check for specific gesture |

### Virtual Keys

| Function | Args | Description |
|----------|------|-------------|
| `vkey_add(keyCode, x, y, w, h)` | 5 | Add a virtual on-screen key |
| `vkey_clear()` | 0 | Remove all virtual keys |
| `vkey_remove(keyCode)` | 1 | Remove a specific virtual key |
| `vkey_count()` | 0 | Get virtual key count |
| `vkey_set_visible(visible)` | 1 | Show/hide all virtual keys |

---

## 8. Camera & Viewport

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `set_camera_target(x, y)` | 2 | — | Set camera look-at position |
| `set_camera_offset(x, y)` | 2 | — | Set camera offset (e.g., screen center) |
| `set_camera_zoom(zoom)` | 1 | — | Set camera zoom level |
| `set_camera_rotation(degrees)` | 1 | — | Set camera rotation |
| `get_camera_target()` | 0 | (x, y) | Get camera target |
| `get_camera_offset()` | 0 | (x, y) | Get camera offset |
| `get_camera_x()` | 0 | number | Get camera X position |
| `get_camera_y()` | 0 | number | Get camera Y position |
| `get_camera_zoom()` | 0 | number | Get camera zoom |
| `get_camera_rotation()` | 0 | number | Get camera rotation |
| `start_camera_shake(ampX, ampY, freq, durationCycles)` | 4 | — | Start screen shake |
| `stop_camera_shake()` | 0 | — | Stop screen shake |
| `set_design_resolution(w, h)` | 2 | — | Set virtual resolution |
| `set_virtual_screen_enabled(enabled)` | 1 | — | Enable/disable virtual screen |
| `set_screen_scale_mode(mode)` | 1 | — | Set scaling mode |
| `get_viewport()` | 0 | (x, y, w, h) | Get current viewport rect |
| `get_fit_scale()` | 0 | number | Get current fit scale factor |

---

## 9. Layers, Scroll & Tilemaps

### Layers & Scroll

| Function | Args | Description |
|----------|------|-------------|
| `set_scroll(x, y)` | 2 | Set global scroll position |
| `set_layer_mode(layer, mode)` | 2 | Set layer rendering mode |
| `set_layer_scroll_factor(layer, x, y)` | 3 | Set parallax scroll factor |
| `set_layer_size(layer, x, y, w, h)` | 5 | Set layer bounds |
| `set_layer_back_graph(layer, graphId)` | 2 | Set layer background graphic |
| `set_layer_front_graph(layer, graphId)` | 2 | Set layer foreground graphic |
| `set_layer_visible(layer, visible)` | 2 | Show/hide a layer |
| `set_layer_clip(layer)` | 1 | Set layer clipping |

### Tilemaps

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `set_tile_map(layer, mapW, mapH, tileW, tileH, columns, graphId)` | 7 | — | Create a tilemap on a layer |
| `set_tile_map_tile(layer, x, y, tileId, solid)` | 5 | — | Set a specific tile |
| `get_tile_map_tile(layer, x, y)` | 3 | tileId | Get tile at position |
| `has_tile_map(layer)` | 1 | bool | Check if layer has a tilemap |
| `import_tilemap(filename)` | 1 | bool | Import TMX tilemap |
| `set_tile_map_spacing(layer, spacing)` | 2 | — | Set tile spacing |
| `set_tile_map_free(layer, tileId)` | 2 | — | Mark tile as non-solid |
| `set_tile_map_solid(layer, tileId)` | 2 | — | Mark tile as solid |
| `set_tile_map_visible(layer, visible)` | 2 | — | Show/hide tilemap layer |
| `set_tile_map_margin(layer, margin)` | 2 | — | Set tilemap margin |
| `set_tile_map_mode(layer, mode)` | 2 | — | Set tilemap mode (orthogonal/isometric) |
| `set_tile_map_color(layer, color)` | 2 | — | Set tilemap tint color |
| `set_tile_debug(layer, grid, ids)` | 3 | — | Enable/disable tile debug view |
| `set_tile_map_iso_compression(layer, compression)` | 2 | — | Set isometric compression factor |

---

## 10. Process-Native Functions

These functions can only be called from within a `process` block.

### Movement

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `advance(speed)` | 1 | — | Move forward along current `angle` |
| `xadvance(speed, angle)` | 2 | — | Move in a specific direction |

### Collision & Shape

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `set_rect_shape(x, y, w, h)` | 4 | — | Set rectangular collision shape |
| `set_circle_shape(radius)` | 1 | — | Set circular collision shape |
| `set_collision_layer(layer)` | 1 | — | Set which collision layer this entity is on |
| `set_collision_mask(mask)` | 1 | — | Set collision mask (bitmask) |
| `add_collision_mask(layer)` | 1 | — | Add a layer to collision mask |
| `remove_collision_mask(layer)` | 1 | — | Remove a layer from collision mask |
| `set_static()` | 0 | — | Mark entity as static (for quadtree) |
| `enable_collision()` | 0 | — | Enable collision detection |
| `disable_collision()` | 0 | — | Disable collision detection |
| `place_free(x, y)` | 2 | bool | Check if position is free of collisions |
| `place_meeting(x, y)` | 2 | process or false | Check collision at position |
| `collision(typeName, x, y)` | 3 | process or false | Check collision with specific type at position |

### Layer & Rendering

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `set_layer(layer)` | 1 | — | Move entity to a render layer |
| `get_layer()` | 0 | int | Get current render layer |
| `update_order()` | 0 | — | Sort current layer by `z` (call after changing `z`) |
| `set_visible(enabled)` | 1 | — | Show/hide entity |
| `flip_horizontal(enabled)` | 1 | — | Mirror entity horizontally |
| `flip_vertical(enabled)` | 1 | — | Mirror entity vertically |
| `flip(flipX, flipY)` | 2 | — | Set both flip states |
| `set_entity_blend(blendMode)` | 1 | — | Set per-entity blend mode |

### Hierarchy

| Function | Args | Description |
|----------|------|-------------|
| `atach(childProcess, front)` | 2 | Attach a child process (front=true for in-front) |

### Spatial Queries

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `out_screen()` | 0 | bool | Is entity outside camera view? |
| `get_point(index)` | 1 | (x, y) | Get control point from graph (local) |
| `get_real_point(index)` | 1 | (x, y) | Get control point in world space |
| `get_local_point(x, y)` | 2 | (lx, ly) | World → local coordinate transform |
| `get_world_point(x, y)` | 2 | (wx, wy) | Local → world coordinate transform |

### Targeting & AI

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `get_nearest(typeName)` | 1 | process or false | Find nearest process of a type |
| `fget_angle(target)` | 1 | number | Angle to a process (degrees) |
| `fget_dist(target)` | 1 | number | Distance to a process |
| `turn_to(target, step)` | 2 | — | Rotate `angle` toward a target by `step` degrees |
| `let_me_alone()` | 0 | — | Kill all other processes |

---

## 11. Particles

### Factory Functions

All return an `Emitter` instance.

| Function | Args | Description |
|----------|------|-------------|
| `create_emitter(persistent, graphId, maxParticles)` | 3 | Generic emitter |
| `create_fire(x, y, graphId)` | 3 | Fire effect |
| `create_smoke(x, y, graphId)` | 3 | Smoke effect |
| `create_explosion(x, y, graphId, color)` | 4 | Explosion effect |
| `create_sparks(x, y, graphId, color)` | 4 | Sparks effect |
| `create_landing_dust(x, y, graphId, facingRight)` | 4 | Landing dust |
| `create_wall_impact(x, y, graphId, hitFromLeft, sizeStart, sizeEnd)` | 6 | Wall impact |
| `create_water_splash(x, y, graphId)` | 3 | Water splash |
| `create_run_trail(x, y, graphId, sizeMin, sizeMax)` | 5 | Running trail |
| `create_speed_lines(x, y, graphId, velX, velY)` | 5 | Speed lines |
| `create_collect_effect(x, y, graphId, itemColor)` | 4 | Item collect |
| `create_power_up_aura(x, y, graphId, auraColor)` | 4 | Power-up aura |
| `create_sparkle(x, y, graphId)` | 3 | Sparkle effect |
| `create_blood_splatter(x, y, graphId, dirX, dirY)` | 5 | Blood splatter |
| `create_rain(x, y, graphId, width)` | 4 | Rain effect |
| `create_shell_ejection(x, y, graphId, facingRight)` | 4 | Shell ejection |
| `create_muzzle_flash(x, y, graphId, shootDirection)` | 4 | Muzzle flash |

### Emitter Methods

| Method | Args | Description |
|--------|------|-------------|
| `.set_position(x, y)` | 2 | Set emitter position |
| `.set_direction(x, y)` | 2 | Set emission direction |
| `.set_emission_rate(rate)` | 1 | Particles per second |
| `.set_life(life)` | 1 | Particle lifetime |
| `.set_speed_range(min, max)` | 2 | Particle speed range |
| `.set_spread(radians)` | 1 | Emission spread angle |
| `.set_color_curve(startColor, endColor)` | 2 | Color transition |
| `.set_size_curve(startSize, endSize)` | 2 | Size transition |
| `.set_spawn_zone(x, y, w, h)` | 4 | Spawn area |
| `.set_lifetime(time)` | 1 | Emitter lifetime |
| `.set_gravity(x, y)` | 2 | Gravity on particles |
| `.set_drag(drag)` | 1 | Particle drag |
| `.set_rotation_range(min, max)` | 2 | Initial rotation range |
| `.set_angular_vel_range(min, max)` | 2 | Angular velocity range |
| `.set_blend_mode(blendMode)` | 1 | Particle blend mode |
| `.set_layer(layer)` | 1 | Render layer |
| `.stop()` | 0 | Stop emitting |

---

## 12. Sound & Music

### Sound Effects

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `load_sound(path)` | 1 | soundId | Load sound file |
| `play_sound(soundId, [volume], [pitch])` | 1–3 | — | Play a sound |
| `stop_sound(soundId)` | 1 | — | Stop a sound |
| `pause_sound(soundId)` | 1 | — | Pause a sound |
| `resume_sound(soundId)` | 1 | — | Resume a paused sound |
| `is_sound_playing(soundId)` | 1 | bool | Check if sound is playing |
| `set_sound_volume(soundId, volume)` | 2 | — | Set sound volume |
| `set_sound_pitch(soundId, pitch)` | 2 | — | Set sound pitch |
| `set_sound_pan(soundId, pan)` | 2 | — | Set sound panning |
| `remove_sound(soundId)` | 1 | — | Unload a sound |
| `stop_all_sounds()` | 0 | — | Stop all playing sounds |

### Music (Streaming)

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `load_music(path)` | 1 | musicId | Load music file |
| `play_music(musicId)` | 1 | — | Play music |
| `stop_music(musicId)` | 1 | — | Stop music |
| `pause_music(musicId)` | 1 | — | Pause music |
| `resume_music(musicId)` | 1 | — | Resume music |
| `set_music_volume(musicId)` | 1 | — | Set music volume |
| `is_music_playing()` | 0 | bool | Check if music is playing |

### Volume Control

| Function | Args | Description |
|----------|------|-------------|
| `set_master_volume(volume)` | 1 | Set master volume (0.0–1.0) |
| `set_sfx_volume(volume)` | 1 | Set SFX volume (0.0–1.0) |

### Procedural Audio

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `create_waveform(type, frequency, duration)` | 3 | soundId | Create procedural waveform |
| `create_noise(...)` | varies | soundId | Create noise sound |
| `enable_sfx_delay(...)` | varies | — | Enable delay effect on SFX |
| `enable_music_lowpass(...)` | varies | — | Enable low-pass filter on music |

---

## 13. Image API

### Class: `Image`

In-memory image for procedural content creation.

**Constructor:** `Image(width, height)`

| Method | Args | Returns | Description |
|--------|------|---------|-------------|
| `.get_width()` | 0 | int | Image width |
| `.get_height()` | 0 | int | Image height |
| `.get_bpp()` | 0 | int | Bytes per pixel |
| `.set_pixel(x, y, color)` | 3 | — | Set pixel (Color struct) |
| `.set_pixel(x, y, r, g, b, [a])` | 5–6 | — | Set pixel (components) |
| `.get_pixel(x, y)` | 2 | (r,g,b,a) | Get pixel color components |
| `.fill(color)` | 1 | — | Fill with Color |
| `.fill(r, g, b, [a])` | 3–4 | — | Fill with color components |
| `.draw_rect(x, y, w, h, r, g, b, [a], [fill])` | 8–9 | — | Draw rectangle into image |
| `.draw_circle(x, y, radius, r, g, b, [a], [fill])` | 7–8 | — | Draw circle into image |
| `.draw_line(x1, y1, x2, y2, r, g, b, [a])` | 7–8 | — | Draw line into image |
| `.resize(w, h)` | 2 | — | Resize (bicubic) |
| `.resize_nn(w, h)` | 2 | — | Resize (nearest-neighbor) |
| `.flip_horizontal()` | 0 | — | Mirror horizontally |
| `.flip_vertical()` | 0 | — | Mirror vertically |
| `.rotate(degrees)` | 1 | — | Rotate image |
| `.rotate_cw()` | 0 | — | Rotate 90° clockwise |
| `.rotate_ccw()` | 0 | — | Rotate 90° counter-clockwise |
| `.load(path)` | 1 | bool | Load image from file |
| `.save(path)` | 1 | bool | Save image to file |
| `.to_graph([name])` | 0–1 | graphId | Convert to drawable graph |
| `.update_graph(graphId)` | 1 | bool | Update existing graph texture |
| `.blit(srcImage, dstX, dstY)` | 3 | — | Copy from another Image |
| `.blit(srcImage, dstX, dstY, srcX, srcY, srcW, srcH)` | 7 | — | Copy sub-region |
| `.crop(x, y, w, h)` | 4 | — | Crop image in-place |

Aliases: `.draw_pixel` = `.set_pixel`, `.clear` = `.fill`, `.flip_x` = `.flip_horizontal`, `.flip_y` = `.flip_vertical`

### Global Image Functions

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `create_image(w, h, [bpp])` | 2–3 | Image | Create blank image |
| `create_image_from_file(path)` | 1 | Image or nil | Load image from file |
| `image_from_file(path)` | 1 | Image or nil | Alias for above |
| `load_image(...)` | varies | Image | Load image |
| `get_image_info(path)` | 1 | (w, h, bpp) | Get image dimensions without loading |

---

## 14. Mesh (PolyMesh)

Functions for creating and drawing textured polygon meshes and tracks.

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `create_mesh()` | 0 | meshId | Create new empty mesh |
| `mesh_clear(meshId)` | 1 | — | Clear all points |
| `mesh_add_point(meshId, x, y)` | 3 | — | Add a control point |
| `mesh_build_track(meshId, depth)` | 2+ | — | Build a track mesh |
| `mesh_build_polygon(meshId, [uvScale])` | 1–2 | — | Build polygon mesh |
| `mesh_set_texture(meshId, graphId)` | 2 | — | Set mesh texture |
| `mesh_set_body_texture(meshId, graphId)` | 2 | — | Set track body texture |
| `mesh_set_edge_texture(meshId, graphId)` | 2 | — | Set track edge texture |
| `mesh_set_scale_top(meshId, scaleX, scaleY)` | 3 | — | Set top edge scale |
| `mesh_set_scale_bottom(meshId, scaleX, scaleY)` | 3 | — | Set bottom edge scale |
| `mesh_draw(meshId, x, y, rotation, scale, [screenSpace])` | 5–6 | — | Draw the mesh |

Aliases: `set_scale_top`, `set_scale_bottom`

---

## 15. Pathfinding

### Class: `Path`

**Constructor:** `Path(width, height, resolution)`

| Method | Args | Returns | Description |
|--------|------|---------|-------------|
| `.set_occupied(x, y)` | 2 | — | Mark cell as occupied |
| `.set_free(x, y)` | 2 | — | Mark cell as free |
| `.clear_all()` | 0 | — | Clear all occupancy |
| `.is_occupied(x, y)` | 2 | bool | Check if cell is occupied |
| `.is_walkable(x, y)` | 2 | bool | Check if cell is walkable |
| `.load_from_image(path, [threshold])` | 1–2 | — | Load occupancy from image |
| `.get_width()` | 0 | int | Grid width |
| `.get_height()` | 0 | int | Grid height |
| `.get_resolution()` | 0 | int | Cell resolution |
| `.world_to_grid(x, y)` | 2 | (gx, gy) | World → grid coords |
| `.grid_to_world(x, y)` | 2 | (wx, wy) | Grid → world coords |
| `.find(sx, sy, ex, ey, [diag], [algo], [heur])` | 4–7 | x1,y1,... | Find path (flat coord pairs) |
| `.fill_from_layer(layer, [useSolid], [clearFirst])` | 1–3 | int | Fill from tilemap layer |
| `.find_ex(sx, sy, ex, ey, ...)` | 4+ | — | Extended path find |
| `.get_result_count()` | 0 | int | Number of results |
| `.get_result(index)` | 1 | (x, y) | Get result point |

---

## 16. Messaging

A queue-based system for inter-process communication.

### Process-Native Functions

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `send(recipientId, type, data)` | 3 | — | Send message to a process |
| `has_message(type)` | 1 | bool | Check for messages of a type |
| `pop_message()` | 0 | message or nil | Get next message |
| `pop_ex_message()` | 0 | message or nil | Get next message (extended) |
| `peek_message(index)` | 1 | message or nil | Peek at message without removing |
| `count_messages()` | 0 | int | Number of pending messages |
| `clean_messages()` | 0 | — | Clear message queue |
| `clear_messages()` | 0 | — | Alias for clean_messages |

---

## 17. Box2D Physics

### World Management

| Function | Args | Description |
|----------|------|-------------|
| `create_physics([gravityX, gravityY])` | 0–2 | Initialize physics world |
| `update_physics(dt, [velIter], [posIter])` | 1–3 | Step physics simulation |
| `destroy_physics()` | 0 | Destroy physics world |
| `set_physics_debug(enabled)` | 1 | Enable/disable debug draw |
| `get_body_count()` | 0 | Get number of bodies |

Aliases: `create_world`, `update_world`, `clean_world`

### Collision Queries

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `physics_collide(idA, idB)` | 2 | bool | Two processes in contact? |
| `physics_collide_with(processType)` | 1 | processId | First collision with type |
| `physics_collision()` | 0 | (idA, idB) | Pop collision event |
| `physics_contact_count()` | 0 | int | Collision event count |
| `physics_contact_at(index)` | 1 | (idA, idB, x, y) | Get contact info |
| `physics_contact_clear()` | 0 | — | Clear collision events |
| `physics_raycast(x1, y1, x2, y2, [type], [ignoreSelf])` | 4–6 | (hitId, x, y, nx, ny) | Raycast |
| `physics_overlap_point(x, y, [type], [ignoreSelf])` | 2–4 | (hitId, body) | Point overlap query |
| `physics_overlap_rect(x, y, w, h, [type], [ignoreSelf])` | 4–6 | hitId | Rectangle overlap query |
| `physics_overlap_circle(x, y, r, [type], [ignoreSelf])` | 3–5 | hitId | Circle overlap query |

### Class: `BodyDef`

**Constructor:** `BodyDef([type])`

| Method | Args | Description |
|--------|------|-------------|
| `.set_type(type)` | 1 | BODY_STATIC/DYNAMIC/KINEMATIC |
| `.set_position(x, y)` | 2 | Initial position (pixels) |
| `.set_angle(degrees)` | 1 | Initial angle |
| `.set_linear_velocity(vx, vy)` | 2 | Initial velocity |
| `.set_angular_velocity(vel)` | 1 | Initial angular velocity |
| `.set_linear_damping(d)` | 1 | Linear damping |
| `.set_angular_damping(d)` | 1 | Angular damping |
| `.set_gravity_scale(s)` | 1 | Gravity multiplier |
| `.set_allow_sleep(allow)` | 1 | Allow sleeping |
| `.set_awake(isAwake)` | 1 | Active on creation |
| `.set_fixed_rotation(fixed)` | 1 | Prevent rotation |
| `.set_bullet(isBullet)` | 1 | CCD for fast objects |
| `.set_enabled(enabled)` | 1 | Active state |

### Class: `FixtureDef`

**Constructor:** `FixtureDef()`

| Method | Args | Description |
|--------|------|-------------|
| `.set_density(density)` | 1 | Mass density |
| `.set_friction(friction)` | 1 | Surface friction |
| `.set_restitution(restitution)` | 1 | Bounciness |
| `.set_sensor(isSensor)` | 1 | Sensor (no physical response) |
| `.set_filter(category, mask, group)` | 3 | Collision filtering |
| `.set_category_bits(bits)` | 1 | Category bits |
| `.set_mask_bits(bits)` | 1 | Mask bits |
| `.set_group_index(index)` | 1 | Group index |
| `.set_circle_shape(radius, [cx, cy])` | 1–3 | Circle fixture shape |
| `.set_box_shape(hw, hh, [cx, cy, angle])` | 2–5 | Box fixture shape |
| `.set_edge_shape(x1, y1, x2, y2)` | 4 | Edge fixture shape |
| `.set_chain_shape(points, [loop])` | 1–2 | Chain fixture shape |
| `.set_polygon_shape(points)` | 1 | Convex polygon shape |
| `.clear_shape()` | 0 | Remove shape |

### Class: `Body`

**Constructor:** `Body(bodyDef)` — process-native via `create_body(bodyDef)`

| Method | Args | Returns | Description |
|--------|------|---------|-------------|
| `.remove()` | 0 | — | Destroy body |
| `.set_transform(x, y, angle)` | 3 | — | Set position & angle |
| `.get_position()` | 0 | (x, y) | Get position |
| `.set_linear_velocity(vx, vy)` | 2 | — | Set velocity |
| `.get_linear_velocity()` | 0 | (vx, vy) | Get velocity |
| `.set_angular_velocity(vel)` | 1 | — | Set angular velocity |
| `.get_angular_velocity()` | 0 | number | Get angular velocity |
| `.apply_force(fx, fy)` | 2 | — | Apply force |
| `.apply_impulse(ix, iy)` | 2 | — | Apply impulse |
| `.set_gravity_scale(s)` | 1 | — | Set gravity scale |
| `.get_gravity_scale()` | 0 | number | Get gravity scale |
| `.set_awake(awake)` | 1 | — | Wake up / sleep |
| `.is_awake()` | 0 | bool | Is awake? |
| `.set_fixed_rotation(fixed)` | 1 | — | Lock rotation |
| `.is_fixed_rotation()` | 0 | bool | Is rotation fixed? |
| `.set_bullet(bullet)` | 1 | — | Enable CCD |
| `.is_bullet()` | 0 | bool | Is bullet? |
| `.get_mass()` | 0 | number | Total mass |
| `.get_inertia()` | 0 | number | Rotational inertia |
| `.get_angle()` | 0 | number | Angle in degrees |
| `.set_angle(degrees)` | 1 | — | Set angle |
| `.get_type()` | 0 | int | Body type |
| `.add_box(hw, hh, [fixDef])` | 2–3 | — | Add box fixture |
| `.add_circle(radius, [fixDef])` | 1–2 | — | Add circle fixture |
| `.add_edge(x1, y1, x2, y2, [fixDef])` | 4–5 | — | Add edge fixture |
| `.add_chain(points, [loop], [fixDef])` | 1–3 | — | Add chain fixture |
| `.add_polygon(points, [fixDef])` | 1–2 | int | Add polygon (auto-triangulates) |
| `.add_fixture(fixDef)` | 1 | Fixture | Add fixture from def |

### Joints

Each joint type has a corresponding `Def` class and `Joint` class.

**Joint types:** `RevoluteJoint`, `PrismaticJoint`, `DistanceJoint`, `PulleyJoint`, `MouseJoint`, `GearJoint`, `WheelJoint`, `MotorJoint`, `FrictionJoint`

Common pattern:
```bu
var def = RevoluteJointDef();
def.set_body_a(bodyA);
def.set_body_b(bodyB);
def.initialize(bodyA, bodyB, anchorX, anchorY);
var joint = RevoluteJoint(def);
```

### Rope Simulation

| Class | Description |
|-------|-------------|
| `b2RopeTuning()` | Configure rope physics parameters |
| `b2RopeDef(count)` | Define rope with N vertices |
| `b2Rope()` | Rope simulation instance |

```bu
var tuning = b2RopeTuning();
var def = b2RopeDef(10);
def.set_position(100, 100);
def.set_gravity(0, 9.8);
def.set_tuning(tuning);
var rope = b2Rope();
rope.create(def);
rope.step(delta(), 8, mouseX, mouseY);
```

---

## 18. Shaders & Blending

### Blending

| Function | Args | Description |
|----------|------|-------------|
| `set_blend_mode(mode)` | 1 | Set blend mode |
| `reset_blend_mode()` | 0 | Reset to default alpha blending |

Aliases: `set_blend`, `reset_blend`

### Shaders

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `load_shader(vertexPath, fragmentPath)` | 2 | shaderId | Load shader from files |
| `load_shader_file(fragmentPath)` | 1 | shaderId | Load fragment shader (default vertex) |
| `load_shader_auto(basePath)` | 1 | shaderId | Auto-detect shader files |
| `unload_shader(shaderId)` | 1 | — | Unload shader |
| `set_shader(shaderId)` | 1 | — | Activate shader for drawing |
| `reset_shader()` | 0 | — | Deactivate shader |

Aliases: `set_material_shader`, `reset_material_shader`

### Shader Uniforms

| Function | Args | Description |
|----------|------|-------------|
| `set_shader_uniform_float(shaderId, name, value)` | 3 | Set float uniform |
| `set_shader_uniform_int(shaderId, name, value)` | 3 | Set int uniform |
| `set_shader_uniform_vec2(shaderId, name, x, y)` | 4 | Set vec2 uniform |
| `set_shader_uniform_vec3(shaderId, name, x, y, z)` | 5 | Set vec3 uniform |
| `set_shader_uniform_vec4(shaderId, name, x, y, z, w)` | 6 | Set vec4 uniform |

---

## 19. Screen Effects

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `start_fade(targetAlpha, speed)` | 2 | — | Start fade to target alpha |
| `fade_in(speed)` | 1 | — | Fade from black |
| `fade_out(speed)` | 1 | — | Fade to black |
| `is_fade_complete()` | 0 | bool | Fade finished? |
| `get_fade_progress()` | 0 | number | Current fade progress (0–1) |

---

## 20. Easing Functions

All easing functions take a single `t` parameter (0.0–1.0) and return the eased value.

| Function | Curve |
|----------|-------|
| `ease_linear(t)` | Linear |
| `ease_sine_in(t)` | Sine ease-in |
| `ease_sine_out(t)` | Sine ease-out |
| `ease_sine_in_out(t)` | Sine ease-in-out |
| `ease_quad_in(t)` | Quadratic ease-in |
| `ease_quad_out(t)` | Quadratic ease-out |
| `ease_quad_in_out(t)` | Quadratic ease-in-out |
| `ease_cubic_in(t)` | Cubic ease-in |
| `ease_cubic_out(t)` | Cubic ease-out |
| `ease_cubic_in_out(t)` | Cubic ease-in-out |
| `ease_quart_in(t)` | Quartic ease-in |
| `ease_quart_out(t)` | Quartic ease-out |
| `ease_quart_in_out(t)` | Quartic ease-in-out |
| `ease_quint_in(t)` | Quintic ease-in |
| `ease_quint_out(t)` | Quintic ease-out |
| `ease_quint_in_out(t)` | Quintic ease-in-out |
| `ease_expo_in(t)` | Exponential ease-in |
| `ease_expo_out(t)` | Exponential ease-out |
| `ease_expo_in_out(t)` | Exponential ease-in-out |
| `ease_circ_in(t)` | Circular ease-in |
| `ease_circ_out(t)` | Circular ease-out |
| `ease_circ_in_out(t)` | Circular ease-in-out |
| `ease_back_in(t)` | Back ease-in |
| `ease_back_out(t)` | Back ease-out |
| `ease_back_in_out(t)` | Back ease-in-out |
| `ease_elastic_in(t)` | Elastic ease-in |
| `ease_elastic_out(t)` | Elastic ease-out |
| `ease_elastic_in_out(t)` | Elastic ease-in-out |
| `ease_bounce_in(t)` | Bounce ease-in |
| `ease_bounce_out(t)` | Bounce ease-out |
| `ease_bounce_in_out(t)` | Bounce ease-in-out |

---

## 21. Triangulation

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `triangulate(points)` | 1 | flat array | Triangulate polygon → triangle vertices |

### Class: `CDT`

**Constructor:** `CDT(points)` — constrained Delaunay triangulation.

| Method | Args | Returns | Description |
|--------|------|---------|-------------|
| `.add_hole(points)` | 1 | — | Add a hole polygon |
| `.add_point(x, y)` | 2 | — | Add a Steiner point |
| `.triangulate()` | 0 | — | Perform triangulation |
| `.get_triangles()` | 0 | array | Get resulting triangles |

---

## 22. Window Management

| Function | Args | Description |
|----------|------|-------------|
| `set_window_size(w, h)` | 2 | Set window dimensions |
| `set_window_title(title)` | 1 | Set window title |
| `set_fullscreen(enabled)` | 1 | Toggle fullscreen |
| `set_window_resizable(enabled)` | 1 | Allow window resizing |
| `close_window()` | 0 | Close the window/exit |
| `set_log_level(level)` | 1 | Set engine log verbosity |

---

## 23. Debug

| Function | Args | Description |
|----------|------|-------------|
| `debug_stack(...)` | varies | Print VM stack |
| `debug_locals(...)` | varies | Print local variables |
| `debug_frames(...)` | varies | Print call frames |
| `debug_processes()` | 0 | Print all living processes |
| `init_collision(x, y, w, h)` | 4 | Initialize collision system |

---

## Native Structs

### `Color(r, g, b, a)`

Fields: `r` (byte), `g` (byte), `b` (byte), `a` (byte)

```bu
var red = Color(255, 0, 0, 255);
```

### `Vec2(x, y)`

Fields: `x` (float), `y` (float)

```bu
var pos = Vec2(100.0, 200.0);
```
