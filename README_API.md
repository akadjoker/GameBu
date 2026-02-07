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

### Collision / Process Helpers

- `init_collision(x, y, width, height)`
- `proc(processId) -> processHandle|nil`
- `type(processId) -> string`
- `signal(processId, signalType)`
- `exists(processId) -> bool`
- `get_id(typeName) -> processHandle|-1`

### Sound

- `play_sound(soundId, volume, pitch)`
- `stop_sound(soundId)`
- `is_sound_playing(soundId) -> bool`
- `pause_sound(soundId)`
- `resume_sound(soundId)`

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
- `set_tile_map_tile(layer, x, y, tile,solid)`  
 
- `get_tile_map_tile(layer, x, y) -> tile`
- `has_tile_map(layer) -> bool`
- `import_tilemap(filename) -> bool`

### Time

- `delta() -> seconds`
- `time() -> seconds`

### Math (DIV-style)

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
- `set_color(red, green, blue)`
- `set_alpha(alpha)`
- `set_screen_space(enabled)`

- `start_fade(targetAlpha, speed)`
- `is_fade_complete() -> bool`
- `get_fade_progress() -> number`
- `fade_in(speed)`
- `fade_out(speed)`

## Process-Native Functions



- `advance(speed)`
- `xadvance(speed, angle)`
- `get_point(pointIndex) -> (x, y)`
- `get_real_point(pointIndex) -> (x, y)`
- `set_rect_shape(x, y, w, h)`
- `set_circle_shape(radius)`
- `get_local_point(x, y) -> (x, y)`
- `get_world_point(x, y) -> (x, y)`

- `set_collision_layer(layer)`
- `set_collision_mask(mask)`
- `add_collision_mask(layer)`
- `remove_collision_mask(layer)`
- `set_static()`
- `enable_collision()`
- `disable_collision()`

- `place_free(x, y) -> bool`
- `place_meeting(x, y) -> processId|-1`
- `collision(typeName, x, y) -> processId|-1`
- `atach(childProcID, front)`
- `out_screen() -> bool`

- `set_layer(layer)`
- `get_layer() -> int`
- `flip_vertical(enabled)`
- `flip_horizontal(enabled)`
- `set_visible(enabled)`
- `flip(flipX, flipY)`

- `fget_angle(processID) -> number`
- `fget_dist(processID) -> number`
- `turn_to(processID, step)`

## Window / Camera / Log

- `set_window_size(width, height)`
- `set_window_title(title)`
- `set_fullscreen(enabled)`
- `set_window_resizable(enabled)`
- `set_camera_zoom(zoom)`
- `set_camera_rotation(rotation)`
- `set_camera_target(x, y)`
- `set_camera_offset(x, y)`
- `set_log_level(level)`
