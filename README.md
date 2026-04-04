# Loading Cat Pebble Watchface

Pebble watchface based on the loading-cat meme.

## Platforms

- `aplite` - Pebble Classic, 144x168
- `basalt` - Pebble Time, 144x168
- `chalk` - Pebble Time Round, 180x180
- `diorite` - Pebble 2, 144x168
- `emery` - Pebble Time 2, 200x228
- `flint` - Pebble 2 Duo, 144x168
- `gabbro` - Pebble Round 2, 260x260

## Concept

- Beige background with a close-up black cat face
- White loading spinner on the forehead
- Spinner acts as the time display
- Each platform uses its own background crop and spinner geometry
- Spinner uses distinct hour/minute strokes without trailing ghost segments

## Files

- `src/c/main.c`: watchface drawing and time logic
- `src/pkjs/index.js`: minimal PebbleKit JS entry
- `package.json`: Pebble app manifest
- `wscript`: Pebble build config

## Build

```bash
pebble build
```

## Regenerate Platform Backgrounds

```bash
python3 tools/generate_platform_assets.py
```

## CloudPebble

Import this repository into CloudPebble, choose any supported platform, then build and run there.
