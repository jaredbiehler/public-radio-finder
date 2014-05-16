#include <pebble.h>
#include "network.h"
#include "npr.h"
#include "debug.h"
#include "main.h"

const  int MAX_RETRY = 2;
static int retry_count = 0;

static void appmsg_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In received.");

  NprData *npr_data = (NprData*) context;

  Tuple *primary_call_tuple      = dict_find(received, KEY_PRIMARY_CALL);
  Tuple *primary_frequency_tuple = dict_find(received, KEY_PRIMARY_FREQUENCY);
  Tuple *primary_band_tuple      = dict_find(received, KEY_PRIMARY_BAND);
  Tuple *primary_strength_tuple  = dict_find(received, KEY_PRIMARY_STRENGTH);
  Tuple *primary_program_tuple   = dict_find(received, KEY_PRIMARY_PROGRAM);

  Tuple *secondary_call_tuple      = dict_find(received, KEY_SECONDARY_CALL);
  Tuple *secondary_frequency_tuple = dict_find(received, KEY_SECONDARY_FREQUENCY);
  Tuple *secondary_band_tuple      = dict_find(received, KEY_SECONDARY_BAND);
  Tuple *secondary_strength_tuple  = dict_find(received, KEY_SECONDARY_STRENGTH);
  Tuple *secondary_program_tuple   = dict_find(received, KEY_SECONDARY_PROGRAM);

  Tuple *error_tuple    = dict_find(received, KEY_ERROR);
  Tuple *js_ready_tuple = dict_find(received, KEY_JS_READY);


  // look to see if this was a npr data update first
  if (primary_call_tuple) {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "IN: Pri Call Tuple.");

    strncpy(npr_data->primary_call,  primary_call_tuple->value->cstring, 5);
    strncpy(npr_data->primary_frequency,  primary_frequency_tuple->value->cstring, 6);
    strncpy(npr_data->primary_band,  primary_band_tuple->value->cstring, 3);
    npr_data->primary_strength  = primary_strength_tuple->value->int32;

    /*
    npr_data->primary_call      = primary_call_tuple->value->cstring;
    npr_data->primary_frequency = primary_frequency_tuple->value->cstring;
    npr_data->primary_band      = primary_band_tuple->value->cstring;
    */

    if (secondary_call_tuple) {
      npr_data->secondary_available = true;
      strncpy(npr_data->secondary_call,  secondary_call_tuple->value->cstring, 5);
      strncpy(npr_data->secondary_frequency,  secondary_frequency_tuple->value->cstring, 6);
      strncpy(npr_data->secondary_band,  secondary_band_tuple->value->cstring, 3);
      npr_data->secondary_strength  = secondary_strength_tuple->value->int32;
      /*
      npr_data->secondary_call      = secondary_call_tuple->value->cstring;
      npr_data->secondary_frequency = secondary_frequency_tuple->value->cstring;
      npr_data->secondary_band      = secondary_band_tuple->value->cstring;
      */
    } else {
      npr_data->secondary_available = false;
    }

    strncpy(npr_data->primary_program,  "", 255);
    strncpy(npr_data->secondary_program,  "", 255);

    npr_data->error       = ERROR_OK;
    npr_data->updated     = time(NULL);
    debug_update_npr(npr_data);

  }
  // Separate update for program names
  else if (primary_program_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "IN: Pri Program Tuple.");
    strncpy(npr_data->primary_program,  primary_program_tuple->value->cstring, 255);
  }
  else if (secondary_program_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "IN: Sec Program Tuple.");
    strncpy(npr_data->secondary_program,  secondary_program_tuple->value->cstring, 255);
  }
  // Initial Javascript Ready message
  else if (js_ready_tuple) {
    npr_data->js_ready = true;
    npr_data->error    = ERROR_OK;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Javascript reports that it is ready");
    debug_update_message("JS ready");
    initial_jsready_callback();
  }
  else if (error_tuple) {
    npr_data->error   = ERROR_NETWORK;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Error: %s", error_tuple->value->cstring);
  }
  else {
    npr_data->error = ERROR_PHONE;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got message with unknown keys...");
  }

  npr_layer_update(npr_data);

  // Succes! reset the retry count...
  retry_count = 0;
}

static char *translate_error(AppMessageResult result) 
{
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void appmsg_in_dropped(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %s", translate_error(reason));
}

static void appmsg_out_sent(DictionaryIterator *sent, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out sent.");
}

static void appmsg_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) 
{
  NprData *npr_data = (NprData*) context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out failed: %s", translate_error(reason));

  retry_count++;
  
  switch (reason) {
    case APP_MSG_NOT_CONNECTED:
      npr_data->error = ERROR_DISCONNECTED;
      request_npr(npr_data);
      break;
    case APP_MSG_SEND_REJECTED:
    case APP_MSG_SEND_TIMEOUT:
    default:
      npr_data->error = ERROR_PHONE;
      request_npr(npr_data);
      break;
  }
}

void init_network(NprData *npr_data)
{
  int max_in  = app_message_inbox_size_maximum();
  int max_out = app_message_outbox_size_maximum();

  app_message_register_inbox_received(appmsg_in_received);
  app_message_register_inbox_dropped(appmsg_in_dropped);
  app_message_register_outbox_sent(appmsg_out_sent);
  app_message_register_outbox_failed(appmsg_out_failed);
  app_message_set_context(npr_data);
  app_message_open(max_in, max_out);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage max_IN: %i, max_OUT: %i", max_in, max_out);

  npr_data->error    = ERROR_OK;
  npr_data->updated  = 0;
  npr_data->js_ready = false;
  
  npr_data->secondary_available = false;

  retry_count = 0;
}

void close_network()
{
  app_message_deregister_callbacks();
}

void request_npr(NprData *npr_data)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Request npr called, retry count: %i", retry_count);

  if (retry_count > MAX_RETRY) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Too many retries, let's wait for the next main loop request to try again.");
    retry_count = 0;
    return;
  }

  if (!bluetooth_connection_service_peek()) {
    npr_data->error = ERROR_DISCONNECTED;
    return;
  }

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "null iter");
    return;
  }

  /*
  dict_write_cstring(iter, KEY_SERVICE, npr_data->service);
  dict_write_cstring(iter, KEY_SCALE, npr_data->scale);
  dict_write_uint8(iter, KEY_DEBUG, (uint8_t)npr_data->debug);
  dict_write_uint8(iter, KEY_BATTERY, (uint8_t)npr_data->battery);
  */

  dict_write_end(iter);

  app_message_outbox_send();
}
