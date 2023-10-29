#ifndef PARALLAX_H
#define PARALLAX_H

#include "Raster.h"
#include "Polygon.h"
#include "View.h"

void renderParallax(Raster* screen, const Polygon* const* polygons,
    RenderPolygon* const* renderPolygons, int polygonsSize, const View* view,
    int numSlices, double z1, Color col1, double z2, Color col2, int doCap, 
    Color capCol);

#endif