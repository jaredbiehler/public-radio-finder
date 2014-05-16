#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

void debug_layer_create(GRect frame, Window *window);
void debug_enable_display();
void debug_disable_display();
void debug_update_message(char *message);
void debug_update_npr(NprData *npr_data);
void debug_layer_destroy();

#endif