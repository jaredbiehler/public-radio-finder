#include <pebble.h>
#include "network.h"
#include "datetime.h"

static TextLayer *time_layer;
static TextLayer *date_layer;

static char date_text[] = "XXX 00";
static char time_text[] = "00:00";

void time_layer_create(GRect frame, Window *window)
{
  time_layer = text_layer_create(frame);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
}

void date_layer_create(GRect frame, Window *window)
{
  date_layer = text_layer_create(frame);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
}

void time_layer_update()
{
  time_t currentTime = time(NULL);
  // Update the time - Fix to deal with 12 / 24 centering bug
  struct tm *currentLocalTime = localtime(&currentTime);

  // Manually format the time as 12 / 24 hour, as specified
  strftime(   time_text, 
              sizeof(time_text), 
              clock_is_24h_style() ? "%R" : "%I:%M", 
              currentLocalTime);

  // Drop the first char of time_text if needed
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(time_layer, time_text);
}

void date_layer_update(struct tm *tick_time)
{
  // Update the date - Without a leading 0 on the day of the month
  char day_text[4];
  strftime(day_text, sizeof(day_text), "%a", tick_time);
  snprintf(date_text, sizeof(date_text), "%s %i", day_text, tick_time->tm_mday);
  text_layer_set_text(date_layer, date_text);
}

void time_layer_destroy() 
{
  text_layer_destroy(time_layer);
}

void date_layer_destroy() 
{
  text_layer_destroy(date_layer);
}

