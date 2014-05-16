#ifndef DATETIME_LAYER_H
#define DATETIME_LAYER_H

void date_layer_create(GRect frame, Window *window);
void time_layer_create(GRect frame, Window *window);
void date_layer_update();
void time_layer_update();
void date_layer_destroy();
void time_layer_destroy();

#endif