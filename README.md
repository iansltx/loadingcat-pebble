# Loading Cat Pebble Watchface

Pebble Time 2 (`emery`) watchface based on the loading-cat meme.

## Concept

- Beige background with a close-up black cat face
- White loading spinner on the forehead
- Spinner acts as the time display
- Minute hand: longer, thinner rotating spinner trail
- Hour hand: shorter, thicker inner spinner trail

## Files

- `src/c/main.c`: watchface drawing and time logic
- `src/pkjs/index.js`: minimal PebbleKit JS entry
- `package.json`: Pebble app manifest
- `wscript`: Pebble build config
- `tools/convert_pebble_bitmap.py`: converts a source image to a Pebble 64-color bitmap asset

## Build

```bash
pebble build
```

## Regenerate Bitmap Asset

```bash
python3 tools/convert_pebble_bitmap.py \
  ../loading_cat_pebble_time2_200x228.png \
  resources/images/loadingcat_bg.png
```

## CloudPebble

Import this repository into CloudPebble, target `emery`, then build and run in the emulator there.
