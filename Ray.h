#ifndef RAY_H
#define RAY_H

#include "Vector.h"
#include "Polygon.h"
#include "ObjectID.h"

typedef struct RayHit {
    int validHit;
    ObjectID id;
    double t;
    Vector point;
    Vector reflectedRay;
} RayHit;

RayHit castRayPolygons(Vector pos, Vector dir, ObjectID avoidID,
    const Polygon* const* polygons, int polygonsSize);

#endif