#include <pebble.h>

// 24-hour one-hand watchface for Pebble Time 2 (emery, 200x228).
// Clone of WatchPebble by cy8aer (https://github.com/cy8aer/WatchPebble),
// without the minute hand. Midnight (24) at the top, noon (12) at the bottom.

#define PKEY_USE_12H 1

static Window *s_window;
static Layer  *s_canvas_layer;
static bool    s_use_12h = false;

// Midnight = top = 0; noon = bottom = TRIG_MAX_ANGLE/2.
static int32_t minutes_to_angle(int local_min) {
  return (int32_t)((int64_t)TRIG_MAX_ANGLE * (local_min % (24 * 60)) / (24 * 60));
}

static int32_t hour_to_angle(int h) {
  return (int32_t)((int64_t)TRIG_MAX_ANGLE * (h % 24) / 24);
}

static int32_t minute_to_angle(int m) {
  return (int32_t)((int64_t)TRIG_MAX_ANGLE * (m % 60) / 60);
}

static GPoint polar(GPoint center, int r, int32_t angle) {
  return GPoint(
    center.x + (r * sin_lookup(angle)) / TRIG_MAX_RATIO,
    center.y - (r * cos_lookup(angle)) / TRIG_MAX_RATIO
  );
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect  bounds = layer_get_bounds(layer);
  int    size   = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  int    radius = size / 2;
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  time_t     now   = time(NULL);
  struct tm *local = localtime(&now);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Minute dots between the tick marks.
  graphics_context_set_fill_color(ctx, GColorWhite);
  for (int i = 1; i < 60; i++) {
    if (i % 5 == 0) continue;
    graphics_fill_circle(ctx, polar(center, radius - 6, minute_to_angle(i)), 1);
  }

  // Tick marks every 5 minutes (= every 2 hours), longer at the quarters.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  for (int i = 0; i < 60; i += 5) {
    int32_t angle    = minute_to_angle(i);
    int     tick_len = (i % 30 == 0) ? 12 : (i % 15 == 0) ? 8 : 6;
    graphics_draw_line(ctx, polar(center, radius - 1, angle),
                            polar(center, radius - 1 - tick_len, angle));
  }

  // Numerals 1-24, 24 at the top. Larger at 24, 6, 12 and 18.
  GFont font_major = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont font_minor = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  graphics_context_set_text_color(ctx, GColorWhite);
  for (int i = 0; i < 24; i++) {
    bool   is_major = (i % 6 == 0);
    GPoint pos      = polar(center, radius - 22, hour_to_angle(i));

    int display_h = s_use_12h ? (i % 12 == 0 ? 12 : i % 12) : i;
    if (display_h == 0) display_h = 24;

    char num_str[4];
    snprintf(num_str, sizeof(num_str), "%d", display_h);
    GFont font      = is_major ? font_major : font_minor;
    GRect text_rect = is_major ? GRect(pos.x - 12, pos.y - 12, 24, 20)
                               : GRect(pos.x - 10, pos.y - 9, 20, 16);
    graphics_draw_text(ctx, num_str, font, text_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  // Single 24h hand: thin red line with a short tail, one rotation per day.
  int32_t hand_angle = minutes_to_angle(local->tm_hour * 60 + local->tm_min);
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, polar(center, -12, hand_angle),
                          polar(center, radius * 2 / 3, hand_angle));

  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_circle(ctx, center, 6);
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_find(iter, MESSAGE_KEY_USE_12H);
  if (t) {
    s_use_12h = (t->value->int32 != 0);
    persist_write_bool(PKEY_USE_12H, s_use_12h);
    layer_mark_dirty(s_canvas_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_canvas_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root, s_canvas_layer);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void prv_init(void) {
  if (persist_exists(PKEY_USE_12H)) s_use_12h = persist_read_bool(PKEY_USE_12H);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load   = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  app_message_register_inbox_received(inbox_received);
  app_message_open(64, 32);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
