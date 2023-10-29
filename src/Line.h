#ifndef LINE_H
#define LINE_H

#include "Vector.h"
#include "Raster.h"

void renderLine(Raster* screen, Vector p1, Vector p2, Color col,
    Vector offset, Vector scale);
    
void renderLineDoubleBuffer(double* screen, int width, int height, 
    Vector p1, Vector p2, Vector offset, Vector scale);

#endif