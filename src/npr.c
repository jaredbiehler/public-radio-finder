#include <pebble.h>
#include "network.h"
#include "npr.h"
#include "debug.h"

static Layer *npr_layer;

const int NPR_ANIMATION_REFRESH = 1000; // 1 second animation 

// Initial animation dots
static AppTimer *npr_animation_timer;
static bool animation_timer_enabled = true;
static int  animation_step = 0;


static void npr_animate_update(Layer *me, GContext *ctx) 
{
  int dots = 3; 
  int spacer = 12;
  
  graphics_context_set_fill_color(ctx, GColorWhite);

  for (int i=1; i<=dots; i++) {
    if (i == animation_step) {
      graphics_fill_circle(ctx, GPoint((i*spacer), 4), 4);
    } else {
      graphics_fill_circle(ctx, GPoint((i*spacer), 4), 2);
    }
  } 
}

void npr_animate(void *context)
{
  NprData *npr_data = (NprData*) context;
  NprLayerData *nld = layer_get_data(npr_layer);

  if (npr_data->updated == 0 && npr_data->error == ERROR_OK) {    
    animation_step = (animation_step % 3) + 1;
    layer_mark_dirty(nld->loading_layer);
    npr_animation_timer = app_timer_register(NPR_ANIMATION_REFRESH, npr_animate, npr_data);
  } 
}

static void draw_signal_strength(GContext *ctx, int strength)
{
  int width = 4; // pixels
  int bars = 5;

  if (strength <= 0) {
    return;
  }
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  for (int i=1; i<=bars; i++) {
    if (i > strength) {
      graphics_draw_rect(ctx, GRect((i*width)+i, 50-(width*i), width, width*i));
    } else {
      graphics_fill_rect(ctx, GRect((i*width)+i, 50-(width*i), width, width*i), 0, GCornerNone);
    }
  } 
}

static void primary_strength_layer_update(Layer *me, GContext *ctx) 
{
  NprLayerData *nld = layer_get_data(npr_layer);
  draw_signal_strength(ctx, nld->primary_strength);
}

static void secondary_strength_layer_update(Layer *me, GContext *ctx) 
{
  NprLayerData *nld = layer_get_data(npr_layer);
  draw_signal_strength(ctx, nld->secondary_strength);
}

void npr_layer_create(GRect frame, Window *window)
{
  npr_layer = layer_create_with_data(frame, sizeof(NprLayerData));
  NprLayerData *nld = layer_get_data(npr_layer);

  nld->loading_layer = layer_create(GRect(47, 60, 50, 20));
  layer_set_update_proc(nld->loading_layer, npr_animate_update);
  layer_add_child(npr_layer, nld->loading_layer);

  nld->primary_call_layer = text_layer_create(GRect(0, 0, 44, 20));
  text_layer_set_text_color(nld->primary_call_layer, GColorWhite);
  text_layer_set_background_color(nld->primary_call_layer, GColorClear);
  text_layer_set_font(nld->primary_call_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(nld->primary_call_layer, GTextAlignmentLeft);
  layer_add_child(npr_layer, text_layer_get_layer(nld->primary_call_layer));

  nld->primary_frequency_layer = text_layer_create(GRect(0, 9, 110, 45));
  text_layer_set_text_color(nld->primary_frequency_layer, GColorWhite);
  text_layer_set_background_color(nld->primary_frequency_layer, GColorClear);
  text_layer_set_font(nld->primary_frequency_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(nld->primary_frequency_layer, GTextAlignmentLeft);
  layer_add_child(npr_layer, text_layer_get_layer(nld->primary_frequency_layer));

  nld->primary_strength_layer = layer_create(GRect(104, 1, 44, 50));
  layer_set_update_proc(nld->primary_strength_layer, primary_strength_layer_update);
  layer_add_child(npr_layer, nld->primary_strength_layer);

  nld->primary_program_layer = text_layer_create(GRect(0, 48, 144, 22));
  text_layer_set_text_color(nld->primary_program_layer, GColorWhite);
  text_layer_set_background_color(nld->primary_program_layer, GColorClear);
  text_layer_set_font(nld->primary_program_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(nld->primary_program_layer, GTextAlignmentRight);
  layer_add_child(npr_layer, text_layer_get_layer(nld->primary_program_layer));

  nld->secondary_call_layer = text_layer_create(GRect(0, 75, 44, 20));
  text_layer_set_text_color(nld->secondary_call_layer, GColorWhite);
  text_layer_set_background_color(nld->secondary_call_layer, GColorClear);
  text_layer_set_font(nld->secondary_call_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(nld->secondary_call_layer, GTextAlignmentLeft);
  layer_add_child(npr_layer, text_layer_get_layer(nld->secondary_call_layer));

  nld->secondary_frequency_layer = text_layer_create(GRect(0, 84, 110, 45));
  text_layer_set_text_color(nld->secondary_frequency_layer, GColorWhite);
  text_layer_set_background_color(nld->secondary_frequency_layer, GColorClear);
  text_layer_set_font(nld->secondary_frequency_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(nld->secondary_frequency_layer, GTextAlignmentLeft);
  layer_add_child(npr_layer, text_layer_get_layer(nld->secondary_frequency_layer));

  nld->secondary_strength_layer = layer_create(GRect(104, 76, 44, 50));
  layer_set_update_proc(nld->secondary_strength_layer, secondary_strength_layer_update);
  layer_add_child(npr_layer, nld->secondary_strength_layer);

  nld->secondary_program_layer = text_layer_create(GRect(0, 124, 144, 22));
  text_layer_set_text_color(nld->secondary_program_layer, GColorWhite);
  text_layer_set_background_color(nld->secondary_program_layer, GColorClear);
  text_layer_set_font(nld->secondary_program_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(nld->secondary_program_layer, GTextAlignmentRight);
  layer_add_child(npr_layer, text_layer_get_layer(nld->secondary_program_layer));

  layer_add_child(window_get_root_layer(window), npr_layer);
}


// Update the bottom half of the screen: icon and temperature
void npr_layer_update(NprData *npr_data) 
{
  // We have no npr data yet... don't update until we do
  if (npr_data->updated == 0) {
    return;
  }

  NprLayerData *nld = layer_get_data(npr_layer);

  if (npr_animation_timer && animation_timer_enabled) {
    app_timer_cancel(npr_animation_timer);
    // this is only needed to stop the error message when cancelling an already cancelled timer... 
    animation_timer_enabled = false;
    layer_set_hidden(nld->loading_layer, true);
  }

  time_t current_time = time(NULL);
  bool stale = false;

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "ct:%i wup:%i, stale:%i", 
  //   (int)current_time, (int)npr_data->updated, (int)NPR_STALE_TIMEOUT);

  // Update the npr icon and temperature
  if (npr_data->error) {
    // Only update the error icon if the npr data is stale
    if (stale) {
      switch (npr_data->error) {
        case ERROR_NETWORK:
          //npr_layer_set_icon(NPR_ICON_CLOUD_ERROR);
          debug_update_message("Network error");
          break;
        case ERROR_DISCONNECTED:
        case ERROR_PHONE:
        default:
          //npr_layer_set_icon(NPR_ICON_PHONE_ERROR);
          debug_update_message("Phone disco / error");
          break;
      }
    }
  } else {

    text_layer_set_text(nld->primary_frequency_layer, npr_data->primary_frequency);
    text_layer_set_text(nld->primary_call_layer, npr_data->primary_call);
    //text_layer_set_text(nld->primary_band_layer, npr_data->primary_band);

    nld->primary_strength = npr_data->primary_strength;
    layer_mark_dirty(nld->primary_strength_layer);

    if (npr_data->secondary_available) {

      text_layer_set_text(nld->secondary_frequency_layer, npr_data->secondary_frequency);
      text_layer_set_text(nld->secondary_call_layer, npr_data->secondary_call);
      //text_layer_set_text(nld->secondary_band_layer, npr_data->secondary_band);

      nld->secondary_strength = npr_data->secondary_strength;
      layer_mark_dirty(nld->secondary_strength_layer);
    }

    text_layer_set_text(nld->primary_program_layer, npr_data->primary_program);
    text_layer_set_text(nld->secondary_program_layer, npr_data->secondary_program);
  }
}

void npr_layer_destroy() 
{
  NprLayerData *nld = layer_get_data(npr_layer);

  layer_destroy(nld->loading_layer);

  text_layer_destroy(nld->primary_frequency_layer);
  text_layer_destroy(nld->primary_band_layer);
  text_layer_destroy(nld->primary_call_layer);
  layer_destroy(nld->primary_strength_layer);
  text_layer_destroy(nld->primary_program_layer);

  text_layer_destroy(nld->secondary_frequency_layer);
  text_layer_destroy(nld->secondary_band_layer);
  text_layer_destroy(nld->secondary_call_layer);
  layer_destroy(nld->secondary_strength_layer);
  text_layer_destroy(nld->secondary_program_layer);

  layer_destroy(npr_layer);
}


