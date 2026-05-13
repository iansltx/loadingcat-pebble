#include <pebble.h>

#ifdef PBL_COLOR
  #define COLOR_SEG_GHOST  GColorLightGray
  #define COLOR_SEG_MINUTE GColorDukeBlue
  #define COLOR_SEG_HOUR   GColorJazzberryJam
  #define COLOR_SEG_CORE   GColorBlack
#else
  #define COLOR_SEG_GHOST  GColorWhite
  #define COLOR_SEG_MINUTE GColorWhite
  #define COLOR_SEG_HOUR   GColorWhite
  #define COLOR_SEG_CORE   GColorBlack
#endif

#define BG_ORIGIN_X 0
#define BG_ORIGIN_Y 0
#define SPINNER_X PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 66, 66, 82, 66, 90, 66, 119)
#define SPINNER_Y PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 44, 44, 49, 44, 59, 44, 71)
#define SPINNER_BASE_INNER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 12, 12, 14, 12, 16, 12, 20)
#define SPINNER_BASE_OUTER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 28, 28, 32, 28, 36, 28, 47)
#define SPINNER_MINUTE_INNER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 14, 14, 16, 14, 18, 14, 22)
#define SPINNER_MINUTE_OUTER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 30, 30, 35, 30, 40, 30, 51)
#define SPINNER_HOUR_INNER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 12, 12, 14, 12, 16, 12, 20)
#define SPINNER_HOUR_OUTER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 28, 28, 32, 28, 36, 28, 47)
#define SPINNER_CENTER_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 4, 4, 5, 4, 6, 4, 7)
#define SPINNER_BASE_WIDTH PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 2, 2, 2, 2, 3, 2, 4)
#define SPINNER_MINUTE_WIDTH PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 3, 3, 3, 3, 4, 3, 5)
#define SPINNER_HOUR_WIDTH PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 4, 4, 5, 4, 6, 4, 7)

static Window *s_main_window;
static Layer *s_canvas_layer;
static GBitmap *s_background_bitmap;

static GPoint point_from_polar(GPoint center, int32_t angle, int16_t radius) {
  return GPoint(
    center.x + (int16_t)((sin_lookup(angle) * radius) / TRIG_MAX_RATIO),
    center.y - (int16_t)((cos_lookup(angle) * radius) / TRIG_MAX_RATIO)
  );
}

static void draw_radial_dash(GContext *ctx, GPoint center, int32_t angle,
                             int16_t inner_radius, int16_t outer_radius,
                             uint8_t stroke_width, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, stroke_width);
  graphics_draw_line(ctx,
                     point_from_polar(center, angle, inner_radius),
                     point_from_polar(center, angle, outer_radius));
}

static void draw_spinner_base(GContext *ctx, GPoint center) {
  for (int i = 0; i < 12; ++i) {
    draw_radial_dash(ctx, center, (i * TRIG_MAX_ANGLE) / 12,
                     SPINNER_BASE_INNER_RADIUS, SPINNER_BASE_OUTER_RADIUS,
                     SPINNER_BASE_WIDTH, COLOR_SEG_GHOST);
  }
}

static void draw_spinner_time(GContext *ctx, GPoint center, const struct tm *tick_time) {
  const int32_t minute_angle = (tick_time->tm_min * TRIG_MAX_ANGLE) / 60;
  const int32_t hour_angle =
      (((tick_time->tm_hour % 12) * 60 + tick_time->tm_min) * TRIG_MAX_ANGLE) / (12 * 60);

  draw_radial_dash(ctx, center, hour_angle,
                   SPINNER_HOUR_INNER_RADIUS, SPINNER_HOUR_OUTER_RADIUS,
                   SPINNER_HOUR_WIDTH, COLOR_SEG_HOUR);
  draw_radial_dash(ctx, center, minute_angle,
                   SPINNER_MINUTE_INNER_RADIUS, SPINNER_MINUTE_OUTER_RADIUS,
                   SPINNER_MINUTE_WIDTH, COLOR_SEG_MINUTE);

  graphics_context_set_fill_color(ctx, COLOR_SEG_CORE);
  graphics_fill_circle(ctx, center, SPINNER_CENTER_RADIUS);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint spinner_center = GPoint(SPINNER_X, SPINNER_Y);

  if (s_background_bitmap) {
    graphics_draw_bitmap_in_rect(ctx, s_background_bitmap,
                                 GRect(BG_ORIGIN_X, BG_ORIGIN_Y, bounds.size.w, bounds.size.h));
  } else {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  }

  graphics_context_set_antialiased(ctx, true);
  draw_spinner_base(ctx, spinner_center);

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  if (tick_time) {
    draw_spinner_time(ctx, spinner_center, tick_time);
  }
}

static void update_time(void) {
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  (void)tick_time;
  (void)units_changed;
  update_time();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  update_time();
}

static void main_window_unload(Window *window) {
  (void)window;

  if (s_background_bitmap) {
    gbitmap_destroy(s_background_bitmap);
    s_background_bitmap = NULL;
  }

  if (s_canvas_layer) {
    layer_destroy(s_canvas_layer);
    s_canvas_layer = NULL;
  }
}

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
