#include <pebble.h>

#ifdef PBL_COLOR
  #define COLOR_BG            GColorRajah
  #define COLOR_CAT           GColorBlack
  #define COLOR_FACE_DETAIL   GColorDarkGray
  #define COLOR_EYE           GColorWhite
  #define COLOR_TEXT          GColorBlack
  #define COLOR_SEG_BASE      GColorDarkGray
  #define COLOR_SEG_MID       GColorLightGray
  #define COLOR_SEG_ACTIVE    GColorWhite
  #define COLOR_BATTERY_FILL  GColorBlack
#else
  #define COLOR_BG            GColorWhite
  #define COLOR_CAT           GColorBlack
  #define COLOR_FACE_DETAIL   GColorBlack
  #define COLOR_EYE           GColorWhite
  #define COLOR_TEXT          GColorBlack
  #define COLOR_SEG_BASE      GColorWhite
  #define COLOR_SEG_MID       GColorWhite
  #define COLOR_SEG_ACTIVE    GColorWhite
  #define COLOR_BATTERY_FILL  GColorBlack
#endif

#define FACE_RADIUS             72
#define EAR_HEIGHT              56
#define BATTERY_BAR_WIDTH      112
#define BATTERY_BAR_HEIGHT       8
#define BATTERY_BAR_Y           12
#define DATE_BOX_WIDTH         112
#define DATE_BOX_HEIGHT         24
#define DATE_BOX_Y             192

static Window *s_main_window;
static Layer *s_canvas_layer;

static int s_battery_level = 100;

static GPoint s_face_center;
static GPoint s_spinner_center;
static GPoint s_left_eye_center;
static GPoint s_right_eye_center;

static GPath *s_left_ear_path;
static GPath *s_right_ear_path;
static GPath *s_nose_path;

static GPoint s_left_ear_points[3];
static GPoint s_right_ear_points[3];
static GPoint s_nose_points[3];

static GPathInfo s_left_ear_info = {
  .num_points = 3,
  .points = s_left_ear_points,
};

static GPathInfo s_right_ear_info = {
  .num_points = 3,
  .points = s_right_ear_points,
};

static GPathInfo s_nose_info = {
  .num_points = 3,
  .points = s_nose_points,
};

static char s_date_buffer[16];

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

  GPoint inner = point_from_polar(center, angle, inner_radius);
  GPoint outer = point_from_polar(center, angle, outer_radius);
  graphics_draw_line(ctx, inner, outer);
}

static void draw_spinner_base(GContext *ctx) {
  const int16_t inner_radius = 13;
  const int16_t outer_radius = 27;

  for (int i = 0; i < 12; ++i) {
    int32_t angle = (i * TRIG_MAX_ANGLE) / 12;
    draw_radial_dash(ctx, s_spinner_center, angle, inner_radius, outer_radius, 3,
                     COLOR_SEG_BASE);
  }
}

static GColor color_for_trail_step(int step) {
  if (step == 0) {
    return COLOR_SEG_ACTIVE;
  }
  if (step <= 2) {
    return COLOR_SEG_MID;
  }
  return COLOR_SEG_BASE;
}

static void draw_spinner_time(GContext *ctx, struct tm *tick_time) {
  int32_t minute_angle = (tick_time->tm_min * TRIG_MAX_ANGLE) / 60;
  int32_t hour_angle =
      (((tick_time->tm_hour % 12) * 60 + tick_time->tm_min) * TRIG_MAX_ANGLE) / (12 * 60);

  const int32_t minute_step = TRIG_MAX_ANGLE / 60;
  const int32_t hour_step = TRIG_MAX_ANGLE / 24;

  for (int i = 5; i >= 0; --i) {
    draw_radial_dash(ctx, s_spinner_center, minute_angle - (i * minute_step),
                     18, 31, 2, color_for_trail_step(i));
  }

  for (int i = 3; i >= 0; --i) {
    draw_radial_dash(ctx, s_spinner_center, hour_angle - (i * hour_step),
                     8, 20, 4, color_for_trail_step(i));
  }

  graphics_context_set_fill_color(ctx, COLOR_CAT);
  graphics_fill_circle(ctx, s_spinner_center, 6);
}

static void draw_whiskers(GContext *ctx) {
  graphics_context_set_stroke_color(ctx, COLOR_EYE);
  graphics_context_set_stroke_width(ctx, 2);

  graphics_draw_line(ctx, GPoint(s_face_center.x - 22, s_face_center.y + 22),
                     GPoint(s_face_center.x - 68, s_face_center.y + 14));
  graphics_draw_line(ctx, GPoint(s_face_center.x - 24, s_face_center.y + 28),
                     GPoint(s_face_center.x - 70, s_face_center.y + 31));
  graphics_draw_line(ctx, GPoint(s_face_center.x - 22, s_face_center.y + 34),
                     GPoint(s_face_center.x - 68, s_face_center.y + 48));

  graphics_draw_line(ctx, GPoint(s_face_center.x + 22, s_face_center.y + 22),
                     GPoint(s_face_center.x + 68, s_face_center.y + 14));
  graphics_draw_line(ctx, GPoint(s_face_center.x + 24, s_face_center.y + 28),
                     GPoint(s_face_center.x + 70, s_face_center.y + 31));
  graphics_draw_line(ctx, GPoint(s_face_center.x + 22, s_face_center.y + 34),
                     GPoint(s_face_center.x + 68, s_face_center.y + 48));
}

static void draw_cat_background(GContext *ctx, GRect bounds) {
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_fill_color(ctx, COLOR_CAT);
  graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h + 8), 86);
  graphics_fill_rect(ctx, GRect(0, bounds.size.h - 58, bounds.size.w, 58), 0, GCornerNone);

  if (s_left_ear_path) {
    graphics_context_set_fill_color(ctx, COLOR_CAT);
    gpath_draw_filled(ctx, s_left_ear_path);
  }
  if (s_right_ear_path) {
    graphics_context_set_fill_color(ctx, COLOR_CAT);
    gpath_draw_filled(ctx, s_right_ear_path);
  }

  graphics_fill_circle(ctx, s_face_center, FACE_RADIUS);

  graphics_context_set_fill_color(ctx, COLOR_FACE_DETAIL);
  graphics_fill_circle(ctx, GPoint(s_face_center.x, s_face_center.y + 18), 18);

  graphics_context_set_fill_color(ctx, COLOR_EYE);
  graphics_fill_circle(ctx, s_left_eye_center, 14);
  graphics_fill_circle(ctx, s_right_eye_center, 14);

  graphics_context_set_fill_color(ctx, COLOR_CAT);
  graphics_fill_circle(ctx, GPoint(s_left_eye_center.x + 2, s_left_eye_center.y + 1), 4);
  graphics_fill_circle(ctx, GPoint(s_right_eye_center.x + 1, s_right_eye_center.y + 1), 4);

  if (s_nose_path) {
    graphics_context_set_fill_color(ctx, COLOR_EYE);
    gpath_draw_filled(ctx, s_nose_path);
  }

  draw_whiskers(ctx);
}

static void draw_battery_bar(GContext *ctx, GRect bounds) {
  GRect outline = GRect((bounds.size.w - BATTERY_BAR_WIDTH) / 2,
                        BATTERY_BAR_Y,
                        BATTERY_BAR_WIDTH,
                        BATTERY_BAR_HEIGHT);
  GRect fill = outline;
  fill.size.w = (s_battery_level * outline.size.w) / 100;

  graphics_context_set_stroke_color(ctx, COLOR_TEXT);
  graphics_draw_round_rect(ctx, outline, 4);

  graphics_context_set_fill_color(ctx, COLOR_BATTERY_FILL);
  graphics_fill_rect(ctx, fill, 3, GCornersAll);
}

static void draw_date_badge(GContext *ctx, GRect bounds) {
  GRect box = GRect((bounds.size.w - DATE_BOX_WIDTH) / 2,
                    DATE_BOX_Y,
                    DATE_BOX_WIDTH,
                    DATE_BOX_HEIGHT);

  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, box, 12, GCornersAll);

  graphics_context_set_stroke_color(ctx, COLOR_TEXT);
  graphics_draw_round_rect(ctx, box, 12);

  graphics_context_set_text_color(ctx, COLOR_TEXT);
  graphics_draw_text(ctx, s_date_buffer,
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                     box,
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter,
                     NULL);
}

static void update_time(void) {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  if (!tick_time) {
    return;
  }

  strftime(s_date_buffer, sizeof(s_date_buffer), "%b %d", tick_time);

  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_antialiased(ctx, true);
  draw_cat_background(ctx, bounds);
  draw_spinner_base(ctx);

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  if (tick_time) {
    draw_spinner_time(ctx, tick_time);
  }

  draw_battery_bar(ctx, bounds);
  draw_date_badge(ctx, bounds);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  (void)tick_time;
  (void)units_changed;
  update_time();
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void setup_layout(GRect bounds) {
  s_face_center = GPoint(bounds.size.w / 2, 118);
  s_spinner_center = GPoint(bounds.size.w / 2, 73);
  s_left_eye_center = GPoint(bounds.size.w / 2 - 30, 112);
  s_right_eye_center = GPoint(bounds.size.w / 2 + 30, 112);

  s_left_ear_points[0] = GPoint(s_face_center.x - 54, s_face_center.y - 36);
  s_left_ear_points[1] = GPoint(s_face_center.x - 34, s_face_center.y - EAR_HEIGHT - 4);
  s_left_ear_points[2] = GPoint(s_face_center.x - 6, s_face_center.y - 30);

  s_right_ear_points[0] = GPoint(s_face_center.x + 6, s_face_center.y - 30);
  s_right_ear_points[1] = GPoint(s_face_center.x + 36, s_face_center.y - EAR_HEIGHT - 2);
  s_right_ear_points[2] = GPoint(s_face_center.x + 56, s_face_center.y - 34);

  s_nose_points[0] = GPoint(s_face_center.x, s_face_center.y + 25);
  s_nose_points[1] = GPoint(s_face_center.x - 8, s_face_center.y + 18);
  s_nose_points[2] = GPoint(s_face_center.x + 8, s_face_center.y + 18);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  setup_layout(bounds);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  s_left_ear_path = gpath_create(&s_left_ear_info);
  s_right_ear_path = gpath_create(&s_right_ear_info);
  s_nose_path = gpath_create(&s_nose_info);

  s_battery_level = battery_state_service_peek().charge_percent;
  update_time();
}

static void main_window_unload(Window *window) {
  (void)window;

  if (s_left_ear_path) {
    gpath_destroy(s_left_ear_path);
    s_left_ear_path = NULL;
  }
  if (s_right_ear_path) {
    gpath_destroy(s_right_ear_path);
    s_right_ear_path = NULL;
  }
  if (s_nose_path) {
    gpath_destroy(s_nose_path);
    s_nose_path = NULL;
  }
  if (s_canvas_layer) {
    layer_destroy(s_canvas_layer);
    s_canvas_layer = NULL;
  }
}

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, COLOR_BG);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
}

static void deinit(void) {
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
