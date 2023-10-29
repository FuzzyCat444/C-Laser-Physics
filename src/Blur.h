#ifndef BLUR_H
#define BLUR_H

#include <math.h>

#include "Raster.h"

typedef enum BlurType {
    BOX_BLUR,
    BOX_GAUSSIAN_BLUR,
    GAUSSIAN_BLUR
} BlurType;

void blur(double* rasterA, double* rasterB, int width, int height, 
    int size, BlurType type);

#endif