# Loading Cat Pebble Watchface

Pebble watchface based on the loading-cat meme.

## Platforms

- `emery` - Pebble Time 2, 200x228
- `flint` - Pebble 2 Duo, 144x168

## Concept

- Beige background with a close-up black cat face
- White loading spinner on the forehead
- Spinner acts as the time display
- Emery and flint use different background crops and spinner geometry
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

## CloudPebble

Import this repository into CloudPebble, choose either `emery` or `flint`, then build and run there.
