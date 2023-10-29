#ifndef POLYGON_H
#define POLYGON_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "Raster.h"
#include "Vector.h"

typedef struct Polygon {
    Vector* points;
    int pointsSize;
    int pointsCapacity;
} Polygon;

Polygon* Polygon_Create();

void Polygon_PushPoint(Polygon* polygon, Vector p);

void Polygon_SetPoint(Polygon* polygon, int index, Vector p);

void Polygon_PopPoint(Polygon* polygon);

void Polygon_RemovePoint(Polygon* polygon, int index);

Vector Polygon_GetPoint(Polygon* polygon, int index);

void Polygon_Destroy(Polygon* polygon);

typedef struct SlopeScanner {
    double y1;
    double y2;
    double* xms;
    int xmsSize;
    int xmsCapacity;
} SlopeScanner;

SlopeScanner* SlopeScanner_Create(double y1, double y2);

void SlopeScanner_InsertLine(SlopeScanner* slopeScanner, Vector p1, Vector p2);

void SlopeScanner_Render(SlopeScanner* slopeScanner, Raster* screen,
    Color col, Vector offset, Vector scale);

void SlopeScanner_Destroy(SlopeScanner* slopeScanner);

typedef struct RenderPolygon {
    SlopeScanner** slopeScanners;
    int slopeScannersSize;
    double xmin;
    double ymin;
    double xmax;
    double ymax;
} RenderPolygon;

RenderPolygon* RenderPolygon_Create(Polygon* polygon);

void RenderPolygon_Render(RenderPolygon* renderPolygon, Raster* screen,
    Color col, Vector offset, Vector scale);

void RenderPolygon_Destroy(RenderPolygon* renderPolygon);

#endif