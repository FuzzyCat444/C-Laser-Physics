#include "Raster.h"

Raster* Raster_Create(int width, int height) {
    Raster* raster = (Raster*) malloc(sizeof(Raster));
    
    raster->width = width;
    raster->height = height;
    raster->pixels = (uint8_t*) malloc(3 * width * height * sizeof(uint8_t));
    Raster_Clear(raster, (Color) {0});
    
    return raster;
}

void Raster_Clear(Raster* raster, Color col) {
    int size = raster->width * raster->height;
    uint8_t* pixels = raster->pixels;
    
    int idx = 0;
    for (int i = 0; i < size; i++) {
        pixels[idx + 0] = col.r;
        pixels[idx + 1] = col.g;
        pixels[idx + 2] = col.b;
        idx += 3;
    }
}

void Raster_Destroy(Raster* raster) {
    if (raster == NULL)
        return;
    
    free(raster->pixels);
    
    free(raster);
}