#include <pebble.h>
#include "main.h"
#include "network.h"
//#include "persist.h"
#include "npr.h"
#include "debug.h"
#include "datetime.h"

#define TIME_FRAME      (GRect(0, 0, 144, 15))
#define DATE_FRAME      (GRect(0, 15, 144, 15))
#define NPR_FRAME       (GRect(0, 15, 144, 168-18)) // 130 Veritcal space
#define DEBUG_FRAME     (GRect(0, 168-15, 144, 15))

/* Keep a pointer to the current npr data as a global variable */
static NprData *npr_data;

/* Global variables to keep track of the UI elements */
static Window *window;

/* Need to wait for JS to be ready */
const  int  MAX_JS_READY_WAIT = 5000; // 5s
static bool initial_request = true;
static AppTimer *initial_jsready_timer;

static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
  if (units_changed & MINUTE_UNIT) {
    time_layer_update();
    if (!initial_request) {
      debug_update_npr(npr_data);
      npr_layer_update(npr_data);
    }
  }

  /*
  if (units_changed & DAY_UNIT) {
    date_layer_update(tick_time);
  }
  */

  // Refresh the npr info every 15 mins, targeting 18 mins after the hour (Yahoo updates around then)
  if ((units_changed & MINUTE_UNIT) && 
      (tick_time->tm_min % 15 == 3) &&
      !initial_request) {
    request_npr(npr_data);
  }
} 

/**
 * Wait for an official 'ready' from javascript or MAX_JS_READY_WAIT, whichever happens sooner 
 */
void initial_jsready_callback()
{
  initial_request = false;

  if (initial_jsready_timer) {
    app_timer_cancel(initial_jsready_timer);
  }

  request_npr(npr_data); 
}

static void init(void) 
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init started");

  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  npr_data = malloc(sizeof(NprData));
  init_network(npr_data);

  // Setup our layers
  time_layer_create(TIME_FRAME, window);
  date_layer_create(DATE_FRAME, window);
  npr_layer_create(NPR_FRAME, window);
  debug_layer_create(DEBUG_FRAME, window);

  //load_persisted_values(npr_data);
  debug_update_message("Initializing...");

  // Kickoff our npr loading 'dot' animation
  npr_animate(npr_data);

  // Setup a timer incase we miss or don't receive js_ready to manually try ourselves
  initial_jsready_timer = app_timer_register(MAX_JS_READY_WAIT, initial_jsready_callback, NULL);

  // Update the screen right away
  time_t now = time(NULL);
  handle_tick(localtime(&now), MINUTE_UNIT | DAY_UNIT );

  // And then every minute
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

static void deinit(void) 
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit started");

  tick_timer_service_unsubscribe();

  window_destroy(window);

  time_layer_destroy();
  date_layer_destroy();
  npr_layer_destroy();
  debug_layer_destroy();

  free(npr_data);

  close_network();

  //store_persisted_values(npr_data);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
