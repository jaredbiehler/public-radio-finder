#include <pebble.h>
#include "network.h"
#include "debug.h"

static TextLayer *debug_layer;

static char last_update_text[] = "00:00";
static char debug_msg[200];

static bool is_enabled = false;

void debug_layer_create(GRect frame, Window *window)
{
  debug_layer = text_layer_create(frame);
  text_layer_set_text_color(debug_layer, GColorWhite);
  text_layer_set_background_color(debug_layer, GColorClear);
  text_layer_set_font(debug_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(debug_layer, GTextAlignmentRight);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(debug_layer));
}

void debug_enable_display() 
{
  if (is_enabled) {
    return;
  }

  is_enabled = true;
  layer_set_hidden(text_layer_get_layer(debug_layer), false);
}

void debug_disable_display() 
{
  if (!is_enabled) {
    return;
  }

  is_enabled = false;
  layer_set_hidden(text_layer_get_layer(debug_layer), true);
}

void debug_update_message(char *message) 
{
  if (!is_enabled) {
    return;
  }
  
  strcpy(debug_msg, message);
  text_layer_set_text(debug_layer, debug_msg);
}

void debug_update_npr(NprData *npr_data)
{
  if (!is_enabled) {
    return;
  }

  if (npr_data->updated != 0) {

    time_t last_updated = npr_data->updated;
    struct tm *updated_time = localtime(&last_updated);
    strftime(last_update_text, sizeof(last_update_text), "%R", updated_time);
    snprintf(debug_msg, sizeof(debug_msg), 
      "%s, %s, %s, S%i", 
      last_update_text, npr_data->primary_call, npr_data->primary_frequency, npr_data->primary_strength);

    // reset localtime, critical as localtime modifies a shared object!
    time_t currentTime = time(NULL);
    localtime(&currentTime);

    text_layer_set_text(debug_layer, debug_msg);
  } 
}

void debug_layer_destroy() 
{
  text_layer_destroy(debug_layer);
}


