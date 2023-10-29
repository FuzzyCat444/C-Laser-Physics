#ifndef CIRCLE_H
#define CIRCLE_H

#include <math.h>

#include "Vector.h"
#include "Raster.h"

void renderCircle(Raster* screen, Vector center, double radius, 
    Color col1, Color col2, Vector offset, Vector scale);

#endif