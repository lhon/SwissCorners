#include <pebble.h>
#include "main.h"

// Number of updates per second for stop2go, must be < 1000
#define STOP2GO_TICK_RESOLUTION 4

// Hand path definitions
const GPathInfo MINUTE_HAND_POINTS = {
  4,
  (GPoint []) {
    { -4, 16 },
    { 6, 16 },
    { 4, -66 },
    { -2, -66 }
  }
};

const GPathInfo HOUR_HAND_POINTS = {
  4,
  (GPoint []) {
    { -5, 16 },
    { 7, 16 },
    { 5, -45 },
    { -3, -45 }
  }
};

// Scaled hand path definitions for round mode (round face on square displays)
// SVG viewBox is 105x105 with center at (52.5, 52.5)
// Hour hand: base at y=64.5 (12 below center), tip at y=20.5 (32 above center)
// Minute hand: base at y=64.5 (12 below center), tip at y=6.5 (46 above center)
const GPathInfo MINUTE_HAND_POINTS_ROUND = {
  4,
  (GPoint []) {
    { -4, 16 },   // left base
    { 4, 16 },    // right base
    { 3, -63 },   // right tip
    { -3, -63 }   // left tip
  }
};

const GPathInfo HOUR_HAND_POINTS_ROUND = {
  4,
  (GPoint []) {
    { -4, 16 },   // left base
    { 4, 16 },    // right base
    { 4, -44 },   // right tip
    { -4, -44 }   // left tip
  }
};

// Scaled hand path definitions for Chalk (180x180 round)
// Scale factor: ~1.25x relative to 144x168
const GPathInfo MINUTE_HAND_POINTS_CHALK = {
  4,
  (GPoint []) {
    { -5, 20 },    // left base (scaled)
    { 5, 20 },     // right base (scaled)
    { 4, -79 },    // right tip (scaled for 180x180)
    { -4, -79 }    // left tip (scaled)
  }
};

const GPathInfo HOUR_HAND_POINTS_CHALK = {
  4,
  (GPoint []) {
    { -5, 20 },    // left base (scaled)
    { 5, 20 },     // right base (scaled)
    { 5, -55 },    // right tip (scaled for 180x180)
    { -5, -55 }    // left tip (scaled)
  }
};

// Scaled hand path definitions for Emery (200x228 rectangular)
// Scale factor: ~1.4x relative to 144x168
const GPathInfo MINUTE_HAND_POINTS_EMERY = {
  4,
  (GPoint []) {
    { -5, 20 },    // left base (scaled)
    { 5, 20 },     // right base (scaled)
    { 4, -88 },    // right tip (shorter for 200x228)
    { -4, -88 }    // left tip (shorter for 200x228)
  }
};

const GPathInfo HOUR_HAND_POINTS_EMERY = {
  4,
  (GPoint []) {
    { -6, 20 },    // left base (scaled)
    { 6, 20 },     // right base (scaled)
    { 5, -60 },    // right tip (shorter for 200x228)
    { -5, -60 }    // left tip (shorter for 200x228)
  }
};

// Scaled hand path definitions for Gabbro (260x260 round)
// Scale factor: ~1.8x relative to 144x168
const GPathInfo MINUTE_HAND_POINTS_GABBRO = {
  4,
  (GPoint []) {
    { -6, 24 },    // left base (scaled)
    { 6, 24 },     // right base (scaled)
    { 5, -115 },   // right tip (scaled to reach near edge of 260x260)
    { -5, -115 }   // left tip (scaled)
  }
};

const GPathInfo HOUR_HAND_POINTS_GABBRO = {
  4,
  (GPoint []) {
    { -7, 24 },    // left base (scaled)
    { 7, 24 },     // right base (scaled)
    { 6, -78 },    // right tip (scaled to reach near edge of 260x260)
    { -6, -78 }    // left tip (scaled)
  }
};

// Persistent storage key (bumped for complications settings)
#define SETTINGS_KEY 2

// Message keys from package.json
#define MESSAGE_KEY_DialColor 0
#define MESSAGE_KEY_SecondHandOption 1
#define MESSAGE_KEY_DateOption 2
#define MESSAGE_KEY_HourlyVibration 3
#define MESSAGE_KEY_BluetoothStatusDetection 4
#define MESSAGE_KEY_RoundBackgroundColor 5
#define MESSAGE_KEY_CompTL 6
#define MESSAGE_KEY_CompTLColor 7
#define MESSAGE_KEY_CompTR 8
#define MESSAGE_KEY_CompTRColor 9
#define MESSAGE_KEY_CompBL 10
#define MESSAGE_KEY_CompBLColor 11
#define MESSAGE_KEY_CompBR 12
#define MESSAGE_KEY_CompBRColor 13
#define MESSAGE_KEY_TempUnit 14
#define MESSAGE_KEY_Temperature 15
#define MESSAGE_KEY_WeatherCode 16
#define MESSAGE_KEY_Sunrise 17
#define MESSAGE_KEY_Sunset 18
#define MESSAGE_KEY_RequestWeather 19

typedef enum {
  CORNER_TL = 0,
  CORNER_TR = 1,
  CORNER_BL = 2,
  CORNER_BR = 3,
  CORNER_COUNT = 4
} Corner;

typedef struct ClaySettings {
  char dialcolor[12];
  char secondhandoption[10];
  char dateoption[10];
  char hourlyvibration[10];
  uint8_t bluetoothstatusdetection;
  char roundbackgroundcolor[7];  // Hex color string (e.g., "FF0000")
  char comp_type[CORNER_COUNT][12];
  char comp_color[CORNER_COUNT][7];
  char tempunit[2];  // "c" or "f"
} ClaySettings;

static ClaySettings settings;

// Weather data from phone (Celsius; converted on display)
static int s_temperature_c = 0;
static bool s_temperature_valid = false;
static int s_weather_code = -1;
static int s_sunrise_mins = -1;
static int s_sunset_mins = -1;

static Window *s_window;
static Layer *s_hands_layer;
static Layer *s_complications_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static Layer *s_bg_color_layer;  // Background color layer for round face on square Pebble

static GPath *s_minute_arrow, *s_hour_arrow;
static GPath *s_minute_arrow_round, *s_hour_arrow_round;
static GPath *s_minute_arrow_chalk, *s_hour_arrow_chalk;
static GPath *s_minute_arrow_emery, *s_hour_arrow_emery;
static GPath *s_minute_arrow_gabbro, *s_hour_arrow_gabbro;

static GFont s_res_gothic_18_bold;
static GFont s_comp_font;
static TextLayer *s_textlayer_date;

static Window *bluetooth_connected_splash_window;
static BitmapLayer *s_bluetoothconnected_layer;
static GBitmap *s_bluetoothconnected_bitmap;

static Window *bluetooth_disconnected_splash_window;
static BitmapLayer *s_bluetoothdisconnected_layer;
static GBitmap *s_bluetoothdisconnected_bitmap;

// Helper function to convert hex char to value
static uint8_t hex_char_to_val(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

// Helper function to parse hex string to RGB values
static void parse_hex_color(const char *hex, uint8_t *r, uint8_t *g, uint8_t *b) {
  // Default to black if parsing fails
  *r = 0; *g = 0; *b = 0;
    
  if (hex == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "parse_hex_color: hex is NULL");
    return;
  }
  
  size_t len = strlen(hex);
  if (len < 6) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "parse_hex_color: string too short");
    return;
  }
  
  // Skip optional "0x" or "#" prefix
  const char *ptr = hex;
  if (ptr[0] == '#') {
    ptr++;
    len--;
  } else if (len >= 8 && ptr[0] == '0' && ptr[1] == 'x') {
    ptr += 2;
    len -= 2;
  }
  
  if (len < 6) return;
  
  // Parse hex values manually
  *r = (hex_char_to_val(ptr[0]) << 4) | hex_char_to_val(ptr[1]);
  *g = (hex_char_to_val(ptr[2]) << 4) | hex_char_to_val(ptr[3]);
  *b = (hex_char_to_val(ptr[4]) << 4) | hex_char_to_val(ptr[5]);  
}

// Convert hex color string to GColor (B&W falls back by brightness)
static GColor color_from_hex_str(const char *hex) {
  uint8_t r, g, b;
  parse_hex_color(hex, &r, &g, &b);
  #ifdef PBL_COLOR
    return GColorFromRGB(r, g, b);
  #else
    return ((r + g + b) / 3) > 127 ? GColorWhite : GColorBlack;
  #endif
}

static void format_steps(char *buf, size_t n, int steps) {
  if (steps < 0) {
    snprintf(buf, n, "--");
  } else if (steps < 1000) {
    snprintf(buf, n, "%d", steps);
  } else if (steps < 10000) {
    // e.g. 1.2k
    snprintf(buf, n, "%d.%dk", steps / 1000, (steps % 1000) / 100);
  } else {
    snprintf(buf, n, "%dk", steps / 1000);
  }
}

static void format_clock_mins(char *buf, size_t n, int mins) {
  if (mins < 0) {
    snprintf(buf, n, "--");
    return;
  }
  int h = (mins / 60) % 24;
  int m = mins % 60;
  if (clock_is_24h_style()) {
    snprintf(buf, n, "%d:%02d", h, m);
  } else {
    int h12 = h % 12;
    if (h12 == 0) h12 = 12;
    snprintf(buf, n, "%d:%02d", h12, m);
  }
}

static int get_steps_today(void) {
  #if defined(PBL_HEALTH)
    if (health_service_metric_accessible(HealthMetricStepCount,
          time_start_of_today(), time(NULL)) & HealthServiceAccessibilityMaskAvailable) {
      return (int)health_service_sum_today(HealthMetricStepCount);
    }
  #endif
  return -1;
}

static int get_heart_rate(void) {
  #if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (health_service_metric_accessible(HealthMetricHeartRateBPM, now - 900, now)
          & HealthServiceAccessibilityMaskAvailable) {
      HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
      if (hr > 0) {
        return (int)hr;
      }
    }
  #endif
  return -1;
}

static void request_weather(void) {
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return;
  }
  dict_write_uint8(iter, MESSAGE_KEY_RequestWeather, 1);
  app_message_outbox_send();
}

static void draw_weather_icon(GContext *ctx, GPoint center, int code, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 1);
  #endif

  if (code < 0) {
    graphics_draw_circle(ctx, center, 5);
    return;
  }

  // WMO weather interpretation codes
  if (code == 0) {
    // Clear — sun disc
    graphics_fill_circle(ctx, center, 5);
  } else if (code <= 3) {
    // Cloudy
    graphics_fill_circle(ctx, GPoint(center.x - 3, center.y), 4);
    graphics_fill_circle(ctx, GPoint(center.x + 3, center.y + 1), 5);
  } else if (code <= 48) {
    // Fog — horizontal lines
    for (int i = -4; i <= 4; i += 3) {
      graphics_draw_line(ctx, GPoint(center.x - 7, center.y + i), GPoint(center.x + 7, center.y + i));
    }
  } else if (code <= 67 || (code >= 80 && code <= 82)) {
    // Rain / showers — cloud + drops
    graphics_fill_circle(ctx, GPoint(center.x - 2, center.y - 2), 4);
    graphics_fill_circle(ctx, GPoint(center.x + 3, center.y - 1), 4);
    graphics_draw_line(ctx, GPoint(center.x - 3, center.y + 3), GPoint(center.x - 4, center.y + 7));
    graphics_draw_line(ctx, GPoint(center.x + 1, center.y + 3), GPoint(center.x, center.y + 7));
    graphics_draw_line(ctx, GPoint(center.x + 5, center.y + 3), GPoint(center.x + 4, center.y + 7));
  } else if (code <= 77 || (code >= 85 && code <= 86)) {
    // Snow
    graphics_fill_circle(ctx, GPoint(center.x - 2, center.y - 2), 4);
    graphics_fill_circle(ctx, GPoint(center.x + 3, center.y - 1), 4);
    graphics_draw_pixel(ctx, GPoint(center.x - 3, center.y + 5));
    graphics_draw_pixel(ctx, GPoint(center.x + 1, center.y + 6));
    graphics_draw_pixel(ctx, GPoint(center.x + 5, center.y + 5));
  } else {
    // Thunderstorm — zigzag
    graphics_draw_line(ctx, GPoint(center.x + 2, center.y - 7), GPoint(center.x - 2, center.y));
    graphics_draw_line(ctx, GPoint(center.x - 2, center.y), GPoint(center.x + 3, center.y));
    graphics_draw_line(ctx, GPoint(center.x + 3, center.y), GPoint(center.x - 1, center.y + 7));
  }
}

static bool complication_text(Corner corner, char *buf, size_t n) {
  const char *type = settings.comp_type[corner];
  buf[0] = '\0';

  if (strcmp(type, "off") == 0) {
    return false;
  }
  if (strcmp(type, "weather") == 0) {
    return false;  // drawn as icon
  }

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  if (strcmp(type, "temp") == 0) {
    if (!s_temperature_valid) {
      snprintf(buf, n, "--");
    } else {
      int temp = s_temperature_c;
      if (strcmp(settings.tempunit, "f") == 0) {
        temp = (s_temperature_c * 9) / 5 + 32;
      }
      snprintf(buf, n, "%d\xC2\xB0", temp);
    }
  } else if (strcmp(type, "steps") == 0) {
    format_steps(buf, n, get_steps_today());
  } else if (strcmp(type, "day") == 0) {
    strftime(buf, n, "%a", t);
  } else if (strcmp(type, "date") == 0) {
    snprintf(buf, n, "%d", t->tm_mday);
  } else if (strcmp(type, "hr") == 0) {
    int hr = get_heart_rate();
    if (hr <= 0) {
      snprintf(buf, n, "--");
    } else {
      snprintf(buf, n, "%d", hr);
    }
  } else if (strcmp(type, "sunrise") == 0) {
    format_clock_mins(buf, n, s_sunrise_mins);
  } else if (strcmp(type, "sunset") == 0) {
    format_clock_mins(buf, n, s_sunset_mins);
  } else {
    return false;
  }
  return true;
}

static GRect complication_rect(Corner corner, GRect bounds) {
  const int16_t box_w = 52;
  const int16_t box_h = 22;
  #if defined(PBL_ROUND)
    #if PBL_DISPLAY_WIDTH >= 260
      const int16_t pad_x = 28;
      const int16_t pad_y = 28;
    #else
      const int16_t pad_x = 18;
      const int16_t pad_y = 18;
    #endif
  #else
    #if PBL_DISPLAY_WIDTH >= 200
      const int16_t pad_x = 6;
      const int16_t pad_y = 4;
    #else
      const int16_t pad_x = 2;
      const int16_t pad_y = 0;
    #endif
  #endif

  switch (corner) {
    case CORNER_TL:
      return GRect(pad_x, pad_y, box_w, box_h);
    case CORNER_TR:
      return GRect(bounds.size.w - pad_x - box_w, pad_y, box_w, box_h);
    case CORNER_BL:
      return GRect(pad_x, bounds.size.h - pad_y - box_h, box_w, box_h);
    case CORNER_BR:
    default:
      return GRect(bounds.size.w - pad_x - box_w, bounds.size.h - pad_y - box_h, box_w, box_h);
  }
}

static void complications_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  static char buf[12];

  for (int i = 0; i < CORNER_COUNT; i++) {
    Corner corner = (Corner)i;
    if (strcmp(settings.comp_type[corner], "off") == 0) {
      continue;
    }

    GColor color = color_from_hex_str(settings.comp_color[corner]);
    GRect rect = complication_rect(corner, bounds);
    GTextAlignment align = (corner == CORNER_TL || corner == CORNER_BL)
      ? GTextAlignmentLeft : GTextAlignmentRight;

    if (strcmp(settings.comp_type[corner], "weather") == 0) {
      int16_t cx = (align == GTextAlignmentLeft) ? rect.origin.x + 8 : rect.origin.x + rect.size.w - 8;
      int16_t cy = rect.origin.y + rect.size.h / 2;
      draw_weather_icon(ctx, GPoint(cx, cy), s_weather_code, color);
      continue;
    }

    if (!complication_text(corner, buf, sizeof(buf))) {
      continue;
    }

    graphics_context_set_text_color(ctx, color);
    graphics_draw_text(ctx, buf, s_comp_font, rect, GTextOverflowModeTrailingEllipsis, align, NULL);
  }
}

// Default settings
static void prv_default_settings() {
  strcpy(settings.dialcolor, "round");
  strcpy(settings.secondhandoption, "quartz");
  strcpy(settings.dateoption, "nodate");
  strcpy(settings.hourlyvibration, "off");
  settings.bluetoothstatusdetection = 0;
  strcpy(settings.roundbackgroundcolor, "FFFFFF");  // Default to white
  strcpy(settings.comp_type[CORNER_TL], "temp");
  strcpy(settings.comp_type[CORNER_TR], "day");
  strcpy(settings.comp_type[CORNER_BL], "steps");
  strcpy(settings.comp_type[CORNER_BR], "date");
  for (int i = 0; i < CORNER_COUNT; i++) {
    strcpy(settings.comp_color[i], "000000");
  }
  strcpy(settings.tempunit, "c");
}

// Save settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Helper to check if string contains only valid hex chars
static bool is_valid_hex_string(const char *str, size_t expected_len) {
  if (str == NULL) return false;
  size_t len = strlen(str);
  if (len != expected_len) return false;
  for (size_t i = 0; i < len; i++) {
    char c = str[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
      return false;
    }
  }
  return true;
}

// Load settings from persistent storage
static void prv_load_settings() {
  prv_default_settings();
  if (persist_exists(SETTINGS_KEY)) {
    ClaySettings saved_settings;
    int bytes_read = persist_read_data(SETTINGS_KEY, &saved_settings, sizeof(saved_settings));

    if (bytes_read >= (int)sizeof(ClaySettings)) {
      settings = saved_settings;
      // Ensure null termination for safety
      settings.dialcolor[sizeof(settings.dialcolor) - 1] = '\0';
      settings.secondhandoption[sizeof(settings.secondhandoption) - 1] = '\0';
      settings.dateoption[sizeof(settings.dateoption) - 1] = '\0';
      settings.hourlyvibration[sizeof(settings.hourlyvibration) - 1] = '\0';
      settings.roundbackgroundcolor[sizeof(settings.roundbackgroundcolor) - 1] = '\0';
      settings.tempunit[sizeof(settings.tempunit) - 1] = '\0';
      for (int i = 0; i < CORNER_COUNT; i++) {
        settings.comp_type[i][sizeof(settings.comp_type[i]) - 1] = '\0';
        settings.comp_color[i][sizeof(settings.comp_color[i]) - 1] = '\0';
        if (!is_valid_hex_string(settings.comp_color[i], 6)) {
          strcpy(settings.comp_color[i], "000000");
        }
      }

      if (!is_valid_hex_string(settings.roundbackgroundcolor, 6)) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "prv_load_settings: invalid color '%s', resetting", settings.roundbackgroundcolor);
        strcpy(settings.roundbackgroundcolor, "000000");
      }
    }
  }
}

// Background color layer update proc for round face on square Pebble
static void bg_color_update_proc(Layer *layer, GContext *ctx) {
  #if !defined(PBL_ROUND)
    // Only apply background color on square Pebble when using round face
    if (strcmp(settings.dialcolor, "round") == 0) {
      // Parse hex color string to RGB
      uint8_t r, g, b;
      parse_hex_color(settings.roundbackgroundcolor, &r, &g, &b);
      
      #ifdef PBL_COLOR
        // On color Pebble, use the actual color
        graphics_context_set_fill_color(ctx, GColorFromRGB(r, g, b));
      #else
        // On black and white Pebble, use black or white based on brightness
        uint8_t brightness = (r + g + b) / 3;
        if (brightness > 127) {
          graphics_context_set_fill_color(ctx, GColorWhite);
        } else {
          graphics_context_set_fill_color(ctx, GColorBlack);
        }
      #endif
      graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
    }
  #endif
}

// Update the display based on current settings
static void prv_update_display() {
  // Reload background image
  gbitmap_destroy(s_background_bitmap);
  
  if (strcmp(settings.dialcolor, "white") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_WHITEBG);
  } else if (strcmp(settings.dialcolor, "white_nl") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_WHITENLBG);
  } else if (strcmp(settings.dialcolor, "black") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_BLACKBG);
  } else if (strcmp(settings.dialcolor, "black_nl") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_BLACKNLBG);
  } else if (strcmp(settings.dialcolor, "round") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SBB_FACE_ROUND);
  } else {
    // Default to white
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_WHITEBG);
  }
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_background_color(s_background_layer, GColorClear);
  
  // Update window background color for round face on square Pebble
  #if !defined(PBL_ROUND)
    APP_LOG(APP_LOG_LEVEL_INFO, "prv_update_display: square Pebble, dialcolor='%s'", settings.dialcolor);
    if (strcmp(settings.dialcolor, "round") == 0) {
      uint8_t r, g, b;
      parse_hex_color(settings.roundbackgroundcolor, &r, &g, &b);
      APP_LOG(APP_LOG_LEVEL_INFO, "prv_update_display: setting window bg to r=%d g=%d b=%d", r, g, b);
      #ifdef PBL_COLOR
        window_set_background_color(s_window, GColorFromRGB(r, g, b));
      #else
        uint8_t brightness = (r + g + b) / 3;
        window_set_background_color(s_window, brightness > 127 ? GColorWhite : GColorBlack);
      #endif
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "prv_update_display: setting window bg to black");
      window_set_background_color(s_window, GColorBlack);
    }
  #endif
  
  // Mark background color layer dirty to redraw
  if (s_bg_color_layer) {
    layer_mark_dirty(s_bg_color_layer);
  }
  
  // Update date visibility
  if (strcmp(settings.dateoption, "nodate") == 0) {
    layer_set_hidden((Layer *)s_textlayer_date, true);
  } else {
    layer_set_hidden((Layer *)s_textlayer_date, false);
  }
  
  // Update date text color based on dial color
  #ifdef PBL_COLOR
    if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0) {
      text_layer_set_text_color(s_textlayer_date, GColorDarkGray);
    } else if (strcmp(settings.dialcolor, "round") == 0) {
      text_layer_set_text_color(s_textlayer_date, GColorBlack);
    } else {
      text_layer_set_text_color(s_textlayer_date, GColorWhite);
    }
  #else
    if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0 || strcmp(settings.dialcolor, "round") == 0) {
      text_layer_set_text_color(s_textlayer_date, GColorBlack);
    } else {
      text_layer_set_text_color(s_textlayer_date, GColorWhite);
    }
  #endif
  
  // Mark hands layer dirty to redraw with new colors
  layer_mark_dirty(s_hands_layer);
  if (s_complications_layer) {
    layer_mark_dirty(s_complications_layer);
  }
}

static void copy_cstring_setting(char *dest, size_t dest_size, Tuple *t) {
  if (!t || !dest || dest_size == 0) return;
  strncpy(dest, t->value->cstring, dest_size - 1);
  dest[dest_size - 1] = '\0';
}

static void copy_color_setting(char *dest, size_t dest_size, Tuple *t) {
  if (!t || !dest || dest_size < 7) return;
  uint32_t value = (uint32_t)t->value->int32;
  snprintf(dest, dest_size, "%06lX", (unsigned long)value);
}

// Handle incoming AppMessage
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received: START");
  bool settings_changed = false;
  bool weather_changed = false;

  // Dial Color
  Tuple *dial_color_t = dict_find(iterator, MESSAGE_KEY_DialColor);
  if (dial_color_t) {
    copy_cstring_setting(settings.dialcolor, sizeof(settings.dialcolor), dial_color_t);
    settings_changed = true;
  }

  // Second Hand Option
  Tuple *second_hand_t = dict_find(iterator, MESSAGE_KEY_SecondHandOption);
  if (second_hand_t) {
    copy_cstring_setting(settings.secondhandoption, sizeof(settings.secondhandoption), second_hand_t);

    tick_timer_service_unsubscribe();
    if (strcmp(settings.secondhandoption, "off") == 0) {
      tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    } else {
      tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    }
    settings_changed = true;
  }

  // Date Option
  Tuple *date_option_t = dict_find(iterator, MESSAGE_KEY_DateOption);
  if (date_option_t) {
    copy_cstring_setting(settings.dateoption, sizeof(settings.dateoption), date_option_t);
    settings_changed = true;
  }

  // Hourly Vibration
  Tuple *hourly_vib_t = dict_find(iterator, MESSAGE_KEY_HourlyVibration);
  if (hourly_vib_t) {
    copy_cstring_setting(settings.hourlyvibration, sizeof(settings.hourlyvibration), hourly_vib_t);
    settings_changed = true;
  }

  // Bluetooth Status Detection
  Tuple *bt_status_t = dict_find(iterator, MESSAGE_KEY_BluetoothStatusDetection);
  if (bt_status_t) {
    settings.bluetoothstatusdetection = bt_status_t->value->uint8;
    settings_changed = true;
  }

  // Round Background Color
  Tuple *round_bg_color_t = dict_find(iterator, MESSAGE_KEY_RoundBackgroundColor);
  if (round_bg_color_t) {
    copy_color_setting(settings.roundbackgroundcolor, sizeof(settings.roundbackgroundcolor), round_bg_color_t);
    settings_changed = true;
  }

  // Complications
  Tuple *comp_tl_t = dict_find(iterator, MESSAGE_KEY_CompTL);
  if (comp_tl_t) {
    copy_cstring_setting(settings.comp_type[CORNER_TL], sizeof(settings.comp_type[CORNER_TL]), comp_tl_t);
    settings_changed = true;
  }
  Tuple *comp_tl_c_t = dict_find(iterator, MESSAGE_KEY_CompTLColor);
  if (comp_tl_c_t) {
    copy_color_setting(settings.comp_color[CORNER_TL], sizeof(settings.comp_color[CORNER_TL]), comp_tl_c_t);
    settings_changed = true;
  }
  Tuple *comp_tr_t = dict_find(iterator, MESSAGE_KEY_CompTR);
  if (comp_tr_t) {
    copy_cstring_setting(settings.comp_type[CORNER_TR], sizeof(settings.comp_type[CORNER_TR]), comp_tr_t);
    settings_changed = true;
  }
  Tuple *comp_tr_c_t = dict_find(iterator, MESSAGE_KEY_CompTRColor);
  if (comp_tr_c_t) {
    copy_color_setting(settings.comp_color[CORNER_TR], sizeof(settings.comp_color[CORNER_TR]), comp_tr_c_t);
    settings_changed = true;
  }
  Tuple *comp_bl_t = dict_find(iterator, MESSAGE_KEY_CompBL);
  if (comp_bl_t) {
    copy_cstring_setting(settings.comp_type[CORNER_BL], sizeof(settings.comp_type[CORNER_BL]), comp_bl_t);
    settings_changed = true;
  }
  Tuple *comp_bl_c_t = dict_find(iterator, MESSAGE_KEY_CompBLColor);
  if (comp_bl_c_t) {
    copy_color_setting(settings.comp_color[CORNER_BL], sizeof(settings.comp_color[CORNER_BL]), comp_bl_c_t);
    settings_changed = true;
  }
  Tuple *comp_br_t = dict_find(iterator, MESSAGE_KEY_CompBR);
  if (comp_br_t) {
    copy_cstring_setting(settings.comp_type[CORNER_BR], sizeof(settings.comp_type[CORNER_BR]), comp_br_t);
    settings_changed = true;
  }
  Tuple *comp_br_c_t = dict_find(iterator, MESSAGE_KEY_CompBRColor);
  if (comp_br_c_t) {
    copy_color_setting(settings.comp_color[CORNER_BR], sizeof(settings.comp_color[CORNER_BR]), comp_br_c_t);
    settings_changed = true;
  }

  Tuple *temp_unit_t = dict_find(iterator, MESSAGE_KEY_TempUnit);
  if (temp_unit_t) {
    copy_cstring_setting(settings.tempunit, sizeof(settings.tempunit), temp_unit_t);
    settings_changed = true;
  }

  // Weather data from phone
  Tuple *temp_t = dict_find(iterator, MESSAGE_KEY_Temperature);
  if (temp_t) {
    s_temperature_c = (int)temp_t->value->int32;
    s_temperature_valid = true;
    weather_changed = true;
  }
  Tuple *weather_code_t = dict_find(iterator, MESSAGE_KEY_WeatherCode);
  if (weather_code_t) {
    s_weather_code = (int)weather_code_t->value->int32;
    weather_changed = true;
  }
  Tuple *sunrise_t = dict_find(iterator, MESSAGE_KEY_Sunrise);
  if (sunrise_t) {
    s_sunrise_mins = (int)sunrise_t->value->int32;
    weather_changed = true;
  }
  Tuple *sunset_t = dict_find(iterator, MESSAGE_KEY_Sunset);
  if (sunset_t) {
    s_sunset_mins = (int)sunset_t->value->int32;
    weather_changed = true;
  }

  if (settings_changed) {
    prv_save_settings();
    prv_update_display();
    request_weather();
  } else if (weather_changed && s_complications_layer) {
    layer_mark_dirty(s_complications_layer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! Reason: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // Check if in round mode for hand and date positioning
  bool is_round_mode = (strcmp(settings.dialcolor, "round") == 0);
  
  // Date
  if (strcmp(settings.dateoption, "nodate") != 0) {
    static char date[3];
    strftime(date, sizeof(date), "%d", t);
    text_layer_set_text(s_textlayer_date, date);
  }
  
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 2);
    if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0) {
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_context_set_stroke_color(ctx, GColorDarkGray);
    }
    else if (strcmp(settings.dialcolor, "black") == 0 || strcmp(settings.dialcolor, "black_nl") == 0) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_context_set_stroke_color(ctx, GColorWhite);
    }
    else if (strcmp(settings.dialcolor, "round") == 0) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_context_set_stroke_color(ctx, GColorBlack);
    }
  #else
    if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0 || strcmp(settings.dialcolor, "round") == 0) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_context_set_stroke_color(ctx, GColorBlack);
    }
    else if (strcmp(settings.dialcolor, "black") == 0 || strcmp(settings.dialcolor, "black_nl") == 0) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_context_set_stroke_color(ctx, GColorWhite);
    }
  #endif
    
  // Date box - position based on watchface mode
  if (strcmp(settings.dateoption, "nodate") != 0) {
    if (is_round_mode) {
      graphics_draw_rect(ctx, GRect(bounds.size.w / 2 + 25, bounds.size.h / 2 - 8, 22, 20));
    } else {
      graphics_draw_rect(ctx, GRect(102, 74, 22, 20));
    }
  }

  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 1);
  #endif
  
  // Hour hand
  if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0 || strcmp(settings.dialcolor, "round") == 0) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorBlack);
  }
  else if (strcmp(settings.dialcolor, "black") == 0 || strcmp(settings.dialcolor, "black_nl") == 0) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
  }

  // Select appropriate hand paths based on display size and mode
  GPath *hour_path;
  GPath *minute_path;
  #if PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260
    // Gabbro (260x260 round)
    hour_path = s_hour_arrow_gabbro;
    minute_path = s_minute_arrow_gabbro;
  #elif PBL_DISPLAY_WIDTH == 200 && PBL_DISPLAY_HEIGHT == 228
    // Emery (200x228 rectangular)
    hour_path = s_hour_arrow_emery;
    minute_path = s_minute_arrow_emery;
  #elif PBL_DISPLAY_WIDTH == 180 && PBL_DISPLAY_HEIGHT == 180
    // Chalk (180x180 round) - medium size hands
    hour_path = s_hour_arrow_chalk;
    minute_path = s_minute_arrow_chalk;
  #else
    // Standard displays (144x168)
    hour_path = is_round_mode ? s_hour_arrow_round : s_hour_arrow;
    minute_path = is_round_mode ? s_minute_arrow_round : s_minute_arrow;
  #endif

  gpath_rotate_to(hour_path, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, hour_path);
  gpath_draw_outline(ctx, hour_path);
  
  // Minute hand
  if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0 || strcmp(settings.dialcolor, "round") == 0) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorBlack);
  }
  else if (strcmp(settings.dialcolor, "black") == 0 || strcmp(settings.dialcolor, "black_nl") == 0) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
  }
  
  gpath_rotate_to(minute_path, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, minute_path);
  gpath_draw_outline(ctx, minute_path);
  
  // Second hand
  if (strcmp(settings.secondhandoption, "quartz") == 0 || strcmp(settings.secondhandoption, "stop2go") == 0) {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorRed);
      graphics_context_set_stroke_color(ctx, GColorRed);
    #else
      if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0 || strcmp(settings.dialcolor, "round") == 0) {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_context_set_stroke_color(ctx, GColorBlack);
      }
      else if (strcmp(settings.dialcolor, "black") == 0 || strcmp(settings.dialcolor, "black_nl") == 0) {
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_context_set_stroke_color(ctx, GColorWhite);
      }
    #endif

    // Scale stroke width based on display size for proper proportions
    #ifdef PBL_COLOR
      #if PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260
        graphics_context_set_stroke_width(ctx, 5);  // Thicker for Gabbro
      #elif PBL_DISPLAY_WIDTH == 200 && PBL_DISPLAY_HEIGHT == 228
        graphics_context_set_stroke_width(ctx, 4);  // Thicker for Emery
      #elif PBL_DISPLAY_WIDTH == 180 && PBL_DISPLAY_HEIGHT == 180
        graphics_context_set_stroke_width(ctx, 4);  // Thicker for Chalk
      #else
        graphics_context_set_stroke_width(ctx, 3);  // Standard for 144x168
      #endif
    #endif

    // Scale second hand based on display size
    // Basalt (144x168) uses (144/2)-20 = 52px as reference - ends before dial numbers
    int16_t second_hand_length;
    int16_t second_hand_opp_length;
    #if PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260
      // Gabbro (260x260 round) - scale proportionally to basalt
      second_hand_length = 90;   // Shorter to not overreach dial numbers
      second_hand_opp_length = 28;
    #elif PBL_DISPLAY_WIDTH == 200 && PBL_DISPLAY_HEIGHT == 228
      // Emery (200x228 rectangular) - scale proportionally to basalt
      second_hand_length = 70;   // Shorter to not overreach dial numbers
      second_hand_opp_length = 22;
    #elif PBL_DISPLAY_WIDTH == 180 && PBL_DISPLAY_HEIGHT == 180
      // Chalk (180x180 round)
      second_hand_length = 62;   // Scaled proportionally
      second_hand_opp_length = 20;
    #else
      // Standard displays (144x168)
      bool is_round_mode = (strcmp(settings.dialcolor, "round") == 0);
      second_hand_length = is_round_mode ? 42 : (bounds.size.w / 2) - 20;
      second_hand_opp_length = is_round_mode ? 16 : 23;
    #endif
     
    double second_angle = 0;
    
    if (strcmp(settings.secondhandoption, "quartz") == 0) {
      second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    }
    // Stop2go
    else if (strcmp(settings.secondhandoption, "stop2go") == 0) {
      // Move the second hand around the watch in 58 seconds
      // Get current time with milliseconds for smooth animation
      time_t sec;
      uint16_t ms;
      time_ms(&sec, &ms);
      struct tm *now = localtime(&sec);
      
      if (now->tm_sec < 58) {
        // Moving phase: hand completes full circle in 58 seconds
        double fractional_sec = now->tm_sec + ms / 1000.0;
        second_angle = TRIG_MAX_ANGLE * fractional_sec / 58.0;
      } else {
        // Pause at 12 o'clock for 2 seconds
        second_angle = 0;
      }
    }
    
    // Cast angle to int32_t for trig functions (TRIG_MAX_ANGLE = 65536)
    int32_t angle = (int32_t)second_angle;
    
    GPoint second_hand = {
      .x = (int16_t)(sin_lookup(angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
    };
    
    graphics_draw_line(ctx, second_hand, center);
    
    GPoint second_hand_opp = {
      .x = (int16_t)(-sin_lookup(angle) * (int32_t)second_hand_opp_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(cos_lookup(angle) * (int32_t)second_hand_opp_length / TRIG_MAX_RATIO) + center.y,
    };
    
    graphics_draw_line(ctx, second_hand_opp, center);

    // Second hand circle - scale based on display size
    #if PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260
      graphics_fill_circle(ctx, second_hand, 12);  // Largest circle for Gabbro
    #elif PBL_DISPLAY_WIDTH == 200 && PBL_DISPLAY_HEIGHT == 228
      graphics_fill_circle(ctx, second_hand, 10);  // Medium-large circle for Emery
    #elif PBL_DISPLAY_WIDTH == 180 && PBL_DISPLAY_HEIGHT == 180
      graphics_fill_circle(ctx, second_hand, 9);   // Medium circle for Chalk
    #else
      graphics_fill_circle(ctx, second_hand, 7);   // Standard circle
    #endif
    
    // Dot in the middle
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorRed);
      graphics_context_set_stroke_color(ctx, GColorRed);
    #else
      if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0) {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_context_set_stroke_color(ctx, GColorBlack);
      }
      else if (strcmp(settings.dialcolor, "black") == 0 || strcmp(settings.dialcolor, "black_nl") == 0) {
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_context_set_stroke_color(ctx, GColorWhite);
      }
    #endif
    
    // Center dot - scale based on display size
    #if PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260
      graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 7);  // Largest for Gabbro
    #elif PBL_DISPLAY_WIDTH == 200 && PBL_DISPLAY_HEIGHT == 228
      graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 6);  // Medium-large for Emery
    #elif PBL_DISPLAY_WIDTH == 180 && PBL_DISPLAY_HEIGHT == 180
      graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 5);   // Medium for Chalk
    #else
      graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 4);   // Standard
    #endif
  }
}

void handle_tick(struct tm *t, TimeUnits units_changed) {
  (void)units_changed;
  if (strcmp(settings.secondhandoption, "stop2go") == 0) {
    for (int i = 0; i < STOP2GO_TICK_RESOLUTION; i++) {
      app_timer_register(1000 / STOP2GO_TICK_RESOLUTION * i, (void*)layer_mark_dirty, window_get_root_layer(s_window));
    }
  } else {
    layer_mark_dirty(window_get_root_layer(s_window));
  }

  // Refresh weather every 30 minutes
  if (t->tm_min % 30 == 0 && t->tm_sec == 0) {
    request_weather();
  }

  // Hourly vibration
  if (t->tm_min == 0 && t->tm_sec == 0) {
    if (strcmp(settings.hourlyvibration, "short") == 0) {
      vibes_short_pulse();
    } else if (strcmp(settings.hourlyvibration, "long") == 0) {
      vibes_long_pulse();
    } else if (strcmp(settings.hourlyvibration, "double") == 0) {
      vibes_double_pulse();
    }
  }
}

static void bluetooth_connected_splash_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  // Load bluetooth connected image
  s_bluetoothconnected_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  s_bluetoothconnected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHCONNECTED);
  bitmap_layer_set_bitmap(s_bluetoothconnected_layer, s_bluetoothconnected_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetoothconnected_layer));
}

static void bluetooth_connected_window_unload(Window *window) {
  gbitmap_destroy(s_bluetoothconnected_bitmap);
  bitmap_layer_destroy(s_bluetoothconnected_layer);
}
      
static void bluetooth_disconnected_splash_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  // Load bluetooth disconnected image
  s_bluetoothdisconnected_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  s_bluetoothdisconnected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHDISCONNECTED);
  bitmap_layer_set_bitmap(s_bluetoothdisconnected_layer, s_bluetoothdisconnected_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetoothdisconnected_layer));
}

static void bluetooth_disconnected_window_unload(Window *window) {
  gbitmap_destroy(s_bluetoothdisconnected_bitmap);
  bitmap_layer_destroy(s_bluetoothdisconnected_layer);
}

static void hide_bluetooth_connected_splash_window() {
  window_stack_remove(bluetooth_connected_splash_window, true);
}

static void hide_bluetooth_disconnected_splash_window() {
  window_stack_remove(bluetooth_disconnected_splash_window, true);
}

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
  (void)event;
  (void)context;
  if (s_complications_layer) {
    layer_mark_dirty(s_complications_layer);
  }
}
#endif

static void request_weather_timer(void *data) {
  (void)data;
  request_weather();
}

static void handle_bluetooth_connected() {
  window_stack_remove(bluetooth_disconnected_splash_window, true);
  window_stack_push(bluetooth_connected_splash_window, true);

  vibes_long_pulse();
  // Hide splash screen
  app_timer_register(2000, (void*)hide_bluetooth_connected_splash_window, NULL);
  light_enable_interaction();
}

static void handle_bluetooth_disconnected() {
  window_stack_remove(bluetooth_connected_splash_window, true);
  window_stack_push(bluetooth_disconnected_splash_window, true);
      
  uint32_t vibes_pulse_segments[] = { 200, 100, 200, 100, 200, 100, 200, 100, 200 };
  VibePattern vibes_pulse_pattern = {
    .durations = vibes_pulse_segments,
    .num_segments = ARRAY_LENGTH(vibes_pulse_segments),
  };

  vibes_enqueue_custom_pattern(vibes_pulse_pattern);
  light_enable_interaction();
  // Hide splash screen
  app_timer_register(5000, (void*)hide_bluetooth_disconnected_splash_window, NULL);
}

static void bluetooth_connection_callback(bool is_connected) {
  if (settings.bluetoothstatusdetection == 1) {
    if (is_connected) {
      handle_bluetooth_connected();
    } else {
      handle_bluetooth_disconnected();
    }
  }
}

static void startup_bluetooth_disconnection_callback(bool is_connected) {
  if (settings.bluetoothstatusdetection == 1) {
    if (!is_connected) {
      handle_bluetooth_disconnected();
    }
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "window_load: dialcolor='%s' roundbackgroundcolor='%s'", settings.dialcolor, settings.roundbackgroundcolor);
  
  // Set window background color (for round face on square Pebble)
  #if !defined(PBL_ROUND)
    APP_LOG(APP_LOG_LEVEL_INFO, "window_load: square Pebble detected");
    if (strcmp(settings.dialcolor, "round") == 0) {
      uint8_t r, g, b;
      parse_hex_color(settings.roundbackgroundcolor, &r, &g, &b);
      APP_LOG(APP_LOG_LEVEL_INFO, "window_load: setting bg color r=%d g=%d b=%d", r, g, b);
      #ifdef PBL_COLOR
        window_set_background_color(s_window, GColorFromRGB(r, g, b));
      #else
        uint8_t brightness = (r + g + b) / 3;
        window_set_background_color(s_window, brightness > 127 ? GColorWhite : GColorBlack);
      #endif
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "window_load: not round face, setting black bg");
      window_set_background_color(s_window, GColorBlack);
    }
  #else
    APP_LOG(APP_LOG_LEVEL_INFO, "window_load: round Pebble detected, setting black bg");
    window_set_background_color(s_window, GColorBlack);
  #endif
  
  // Create background color layer (behind the bitmap layer for round face on square Pebble)
  s_bg_color_layer = layer_create(bounds);
  layer_set_update_proc(s_bg_color_layer, bg_color_update_proc);
  layer_add_child(window_layer, s_bg_color_layer);
  
  // Load Mondaine background image
  s_background_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  
  if (strcmp(settings.dialcolor, "white") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_WHITEBG);
  } else if (strcmp(settings.dialcolor, "white_nl") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_WHITENLBG);
  } else if (strcmp(settings.dialcolor, "black") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_BLACKBG);
  } else if (strcmp(settings.dialcolor, "black_nl") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_BLACKNLBG);
  } else if (strcmp(settings.dialcolor, "round") == 0) {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SBB_FACE_ROUND);
  } else {
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONDAINE_WHITEBG);
  }

  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_background_color(s_background_layer, GColorClear);
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Initialize date layer
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  static char date[3];
  strftime(date, sizeof(date), "%d", t);

  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_comp_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  // Position date layer - adjust for round watchface mode
  if (strcmp(settings.dialcolor, "round") == 0) {
    s_textlayer_date = text_layer_create(GRect(bounds.size.w / 2 + 25, bounds.size.h / 2 - 10, 22, 20));
  } else {
    s_textlayer_date = text_layer_create(GRect(102, 72, 22, 20));
  }

  #ifdef PBL_COLOR
    if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0) {
      text_layer_set_text_color(s_textlayer_date, GColorDarkGray);
    } else if (strcmp(settings.dialcolor, "round") == 0) {
      text_layer_set_text_color(s_textlayer_date, GColorBlack);
    } else {
      text_layer_set_text_color(s_textlayer_date, GColorWhite);
    }
  #else
    if (strcmp(settings.dialcolor, "white") == 0 || strcmp(settings.dialcolor, "white_nl") == 0 || strcmp(settings.dialcolor, "round") == 0) {
      text_layer_set_text_color(s_textlayer_date, GColorBlack);
    } else {
      text_layer_set_text_color(s_textlayer_date, GColorWhite);
    }
  #endif

  text_layer_set_background_color(s_textlayer_date, GColorClear);
  text_layer_set_text(s_textlayer_date, date);
  text_layer_set_text_alignment(s_textlayer_date, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_date, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_date);
  
  if (strcmp(settings.dateoption, "nodate") == 0) {
    layer_set_hidden((Layer *)s_textlayer_date, true);
  }

  // Corner complications (below hands so hands draw on top)
  s_complications_layer = layer_create(bounds);
  layer_set_update_proc(s_complications_layer, complications_update_proc);
  layer_add_child(window_layer, s_complications_layer);

  // Initialize hands layer
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  layer_destroy(s_bg_color_layer);

  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);
  gpath_destroy(s_minute_arrow_round);
  gpath_destroy(s_hour_arrow_round);
  gpath_destroy(s_minute_arrow_chalk);
  gpath_destroy(s_hour_arrow_chalk);
  gpath_destroy(s_minute_arrow_emery);
  gpath_destroy(s_hour_arrow_emery);
  gpath_destroy(s_minute_arrow_gabbro);
  gpath_destroy(s_hour_arrow_gabbro);

  layer_destroy(s_hands_layer);
  if (s_complications_layer) {
    layer_destroy(s_complications_layer);
    s_complications_layer = NULL;
  }
  text_layer_destroy(s_textlayer_date);
}

static void init() {
  // Load settings first
  prv_load_settings();

  // Set up AppMessage with larger buffers for Clay + weather
  const int inbox_size = 1024;
  const int outbox_size = 256;
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(inbox_size, outbox_size);

  // Create main window
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  // Init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  s_minute_arrow_round = gpath_create(&MINUTE_HAND_POINTS_ROUND);
  s_hour_arrow_round = gpath_create(&HOUR_HAND_POINTS_ROUND);
  s_minute_arrow_chalk = gpath_create(&MINUTE_HAND_POINTS_CHALK);
  s_hour_arrow_chalk = gpath_create(&HOUR_HAND_POINTS_CHALK);
  s_minute_arrow_emery = gpath_create(&MINUTE_HAND_POINTS_EMERY);
  s_hour_arrow_emery = gpath_create(&HOUR_HAND_POINTS_EMERY);
  s_minute_arrow_gabbro = gpath_create(&MINUTE_HAND_POINTS_GABBRO);
  s_hour_arrow_gabbro = gpath_create(&HOUR_HAND_POINTS_GABBRO);

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);

  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);
  gpath_move_to(s_minute_arrow_round, center);
  gpath_move_to(s_hour_arrow_round, center);
  gpath_move_to(s_minute_arrow_chalk, center);
  gpath_move_to(s_hour_arrow_chalk, center);
  gpath_move_to(s_minute_arrow_emery, center);
  gpath_move_to(s_hour_arrow_emery, center);
  gpath_move_to(s_minute_arrow_gabbro, center);
  gpath_move_to(s_hour_arrow_gabbro, center);

  // Subscribe to tick timer
  if (strcmp(settings.secondhandoption, "off") == 0) {
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  } else {
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  }

  #if defined(PBL_HEALTH)
  health_service_events_subscribe(health_handler, NULL);
  #endif

  // Bluetooth status detection
  bluetooth_connected_splash_window = window_create();
  window_set_window_handlers(bluetooth_connected_splash_window, (WindowHandlers) {
    .load = bluetooth_connected_splash_window_load,
    .unload = bluetooth_connected_window_unload,
  });

  bluetooth_disconnected_splash_window = window_create();
  window_set_window_handlers(bluetooth_disconnected_splash_window, (WindowHandlers) {
    .load = bluetooth_disconnected_splash_window_load,
    .unload = bluetooth_disconnected_window_unload,
  });

  // Check bluetooth connection on startup
  startup_bluetooth_disconnection_callback(bluetooth_connection_service_peek());

  bluetooth_connection_service_subscribe(bluetooth_connection_callback);

  // Ask phone for weather once AppMessage is open
  app_timer_register(500, request_weather_timer, NULL);
}

static void deinit() {
  prv_save_settings();

  #if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
  #endif

  bluetooth_connection_service_unsubscribe();
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();

  window_destroy(s_window);
  window_destroy(bluetooth_connected_splash_window);
  window_destroy(bluetooth_disconnected_splash_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
  return 0;
}
