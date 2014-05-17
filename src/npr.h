#ifndef NPR_LAYER_H
#define NPR_LAYER_H

typedef struct {
	Layer     *loading_layer;
    TextLayer *primary_frequency_layer;
	TextLayer *primary_band_layer;
	TextLayer *primary_call_layer;
    Layer     *primary_strength_layer;
    TextLayer *primary_program_layer;
    int primary_strength;
    TextLayer *secondary_frequency_layer;
	TextLayer *secondary_band_layer;
	TextLayer *secondary_call_layer;
    Layer     *secondary_strength_layer;
    TextLayer *secondary_program_layer;
    int secondary_strength;
} NprLayerData;


void npr_layer_create(GRect frame, Window *window);
void npr_animate(void *context);
void npr_layer_update(NprData *npr_data);
void npr_layer_destroy();

#endif
