#include <pebble.h>

#define KEY_ERROR 0
#define KEY_JS_READY 1
#define KEY_PRIMARY_CALL 2
#define KEY_PRIMARY_FREQUENCY 3
#define KEY_PRIMARY_BAND 4
#define KEY_PRIMARY_STRENGTH 5
#define KEY_PRIMARY_PROGRAM 6
#define KEY_SECONDARY_CALL 7
#define KEY_SECONDARY_FREQUENCY 8
#define KEY_SECONDARY_BAND 9
#define KEY_SECONDARY_STRENGTH 10
#define KEY_SECONDARY_PROGRAM 11
#define KEY_ZIP_CODE 12

#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H


typedef enum {
  ERROR_OK = 0,
  ERROR_DISCONNECTED,
  ERROR_PHONE,
  ERROR_NETWORK
} Error;

typedef struct {
  bool js_ready;
  time_t updated;
  char primary_call[5];
  char primary_frequency[6];
  char primary_band[3] ;
  int  primary_strength;
  char primary_program[255];
  bool secondary_available;
  char secondary_call[5];
  char secondary_frequency[6];
  char secondary_band[3];
  int  secondary_strength;
  char secondary_program[255];
  char zip_code[6];
  Error error;
} NprData;

void init_network(NprData *npr_data);
void close_network();
void request_npr();

#endif