#ifndef RASTER_H
#define RASTER_H

#include <stdlib.h>
#include <stdint.h>

// Array of pixels with format R, G, B, R, G, B...
// Each value is 8 bits.
typedef struct Raster {
    int width;
    int height;
    uint8_t* pixels;
} Raster;

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

Raster* Raster_Create(int width, int height);

void Raster_Clear(Raster* raster, Color col);

void Raster_Destroy(Raster* raster);

#endif