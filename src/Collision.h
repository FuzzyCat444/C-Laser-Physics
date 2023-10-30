#ifndef COLLISION_H
#define COLLISION_H

#include "Vector.h"
#include "Polygon.h"
#include "ObjectID.h"

typedef struct CirclePolygonsCollision {
    ObjectID ids[2];
    Vector pushVecs[2];
    int pushVecsSize;
} CirclePolygonsCollision;

Vector collideCirclePolygons(Vector center, double radius, 
    const Polygon* const* polygons, int polygonsSize);

CirclePolygonsCollision collideCirclePolygonsHelper(
    Vector center, double radius, 
    const Polygon* const* polygons, int polygonsSize);

#endif
