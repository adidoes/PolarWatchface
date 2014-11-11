#include <pebble.h>
#include "main.h"

#define HOUR_RADIUS 38
#define MINUTE_RADIUS 53
#define SECOND_RADIUS 68
#define INFO_TIMEOUT 10000

static Window *s_main_window;

Layer *second_display_layer;
Layer *minute_display_layer;
Layer *hour_display_layer;

// static AppTimer *info_timer;

// static GFont *font_inconsolata_10;

static const GPathInfo SECOND_SEGMENT_PATH_POINTS = {
    .num_points = 3,
    .points = (GPoint []) {
        {0, 0},
        {-7, -70}, // 70 = radius + fudge; 7 = 70*tan(6 degrees)
        {7,  -70},
    }
};

static const GPathInfo MINUTE_SEGMENT_PATH_POINTS = {
    .num_points = 3,
    .points = (GPoint []) {
        {0, 0},
        {-6, -58}, // 58 = radius + fudge; 6 = 58*tan(6 degrees)
        {6,  -58},
    }
};

static const GPathInfo HOUR_SEGMENT_PATH_POINTS = {
    .num_points = 3,
    .points = (GPoint []) {
        {0, 0},
        {-5, -44}, // 48 = radius + fudge; 5 = 48*tan(6 degrees)
        {5,  -44},
    }
};

static GPath *second_segment_path = NULL;
static GPath *minute_segment_path = NULL;
static GPath *hour_segment_path = NULL;

static void second_display_callback(Layer *layer, GContext *ctx) {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    unsigned int angle = current_time->tm_sec * 6;

    GRect bounds = layer_get_bounds(layer);
    GPoint centre = grect_center_point(&bounds);

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, centre, SECOND_RADIUS);
    graphics_context_set_fill_color(ctx, GColorBlack);

    for (; angle < 355; angle += 6) {
        gpath_rotate_to(second_segment_path, (TRIG_MAX_ANGLE / 360) * angle);
        gpath_draw_filled(ctx, second_segment_path);
    }

    graphics_fill_circle(ctx, centre, SECOND_RADIUS - 9);
}

static void minute_display_callback(Layer *layer, GContext *ctx) {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    unsigned int angle = current_time->tm_min * 6;

    GRect bounds = layer_get_bounds(layer);
    GPoint centre = grect_center_point(&bounds);

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, centre, MINUTE_RADIUS);
    graphics_context_set_fill_color(ctx, GColorBlack);

    for (; angle < 355; angle += 6) {
        gpath_rotate_to(minute_segment_path, (TRIG_MAX_ANGLE / 360) * angle);
        gpath_draw_filled(ctx, minute_segment_path);
    }

    graphics_fill_circle(ctx, centre, MINUTE_RADIUS - 9);
}

static void hour_display_callback(Layer *layer, GContext *ctx) {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    unsigned int angle;

    if(clock_is_24h_style()) {
        angle = (current_time->tm_hour * 15) + (current_time->tm_min / 4);
    } else {
        angle = (( current_time->tm_hour % 12 ) * 30) + (current_time->tm_min / 2);
    }
    angle = angle - (angle % 6);

    GRect bounds = layer_get_bounds(layer);
    GPoint centre = grect_center_point(&bounds);

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, centre, HOUR_RADIUS);
    graphics_context_set_fill_color(ctx, GColorBlack);


    for (; angle < 355; angle += 6) {
        gpath_rotate_to(hour_segment_path, (TRIG_MAX_ANGLE / 360) * angle);
        gpath_draw_filled(ctx, hour_segment_path);
    }

    graphics_fill_circle(ctx, centre, HOUR_RADIUS - 9);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_change) {
    //app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "units_change %i", (int)units_change);
    if (units_change & SECOND_UNIT) {
        //app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dirtying seconds layer");
        layer_mark_dirty(second_display_layer);
    }

    if (units_change & MINUTE_UNIT) {
        //app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dirtying minutes layer");
        layer_mark_dirty(minute_display_layer);
    }

    if (units_change & HOUR_UNIT) {
        //app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dirtying hours layer");
        layer_mark_dirty(hour_display_layer);
    }
}

// static void info_timeout_handler() {
// }

// static void tap_handler(AccelAxisType axis, int32_t direction) {
//     info_timer = app_timer_register(INFO_TIMEOUT, info_timeout_handler, NULL);;
// }

static void bt_handler(bool connected) {
    // Show current connection state
    if (connected) {
        // text_layer_set_text(s_output_layer, "Connected");
    } else {
        // text_layer_set_text(s_output_layer, "Disconnected");
    }
}

static void handle_tick_second(struct tm *tick_time, TimeUnits units_change) {
    handle_tick(tick_time, units_change);
}

// static void handle_tick_minute(struct tm *tick_time, TimeUnits units_change) {
//     handle_tick(tick_time, units_change);
// }

// static void handle_tick_hour(struct tm *tick_time, TimeUnits units_change) {
//     handle_tick(tick_time, units_change);
// }

static void window_load(Window *s_main_window) {
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_bounds(window_layer);

//     font_inconsolata_10 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_REGULAR_10));

    // Set background to black
    window_set_background_color(s_main_window, GColorBlack);
    GPoint centre = grect_center_point(&bounds);

    // Seconds display layer
    second_segment_path = gpath_create(&SECOND_SEGMENT_PATH_POINTS);
    second_display_layer = layer_create(bounds);
    gpath_move_to(second_segment_path, centre);
    layer_set_update_proc(second_display_layer, &second_display_callback);

    // Minutes display setup
    minute_segment_path = gpath_create(&MINUTE_SEGMENT_PATH_POINTS);
    minute_display_layer = layer_create(bounds);
    gpath_move_to(minute_segment_path, centre);
    layer_set_update_proc(minute_display_layer, &minute_display_callback);

    // Hours display setup
    hour_segment_path = gpath_create(&HOUR_SEGMENT_PATH_POINTS);
    hour_display_layer = layer_create(bounds);
    gpath_move_to(hour_segment_path, centre);
    layer_set_update_proc(hour_display_layer, &hour_display_callback);

    // This order is important!
    layer_add_child(window_layer, second_display_layer);
    layer_add_child(window_layer, minute_display_layer);
    layer_add_child(window_layer, hour_display_layer);

    tick_timer_service_subscribe(SECOND_UNIT, handle_tick_second);
//     accel_tap_service_subscribe(tap_handler);
//     bluetooth_connection_service_subscribe(bt_handler);
}

static void window_unload(Window *s_main_window) {
    layer_destroy(second_display_layer);
    layer_destroy(minute_display_layer);
    layer_destroy(hour_display_layer);

//     fonts_unload_custom_font(font_inconsolata_10);

//     bluetooth_connection_service_unsubscribe();
}

static void init(void) {
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    const bool animated = true;
    window_stack_push(s_main_window, animated);
}

static void deinit(void) {
    window_destroy(s_main_window);
}


int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_main_window);

    app_event_loop();
    deinit();
}
