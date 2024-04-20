# Physics

## `bool raycast()`

- By default, the ray is pointed straight up
- raycast() returns true if it hits something
- nullptr can be passed to hit_pos if it's unnecessary

## `bool safe_air_check()`

- Safely checks if there's air at the position provided in ray_pos
- Returns true if air is at position or is outside bounds
