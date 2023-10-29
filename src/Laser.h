#ifndef LASER_H
#define LASER_H

#include <stdlib.h>

#include "Vector.h"
#include "Polygon.h"
#include "Ray.h"
#include "Raster.h"
#include "Line.h"
#include "Collision.h"

typedef struct Laser {
    Vector* points;
    int pointsCapacity;
    double totalSegmentsLength;
    double tailSegmentLength;
    
    int headIndex;
    int tailIndex;
    
    double head;
    double tailLength;
    int emitting;
    Vector nextDir;
    
    ObjectID lastObjectHitID;
} Laser;

Laser* Laser_Create(Vector pos, Vector dir, 
    const Polygon* const* polygons, int polygonsSize);
    
void Laser_Update(Laser* laser, double speed, double delta,
    const Polygon* const* polygons, int polygonsSize);

void Laser_Bounce(Laser* laser,
    const Polygon* const* polygons, int polygonsSize);
    
void Laser_GrowCapacity(Laser* laser);

void Laser_ShrinkCapacity(Laser* laser);
    
void Laser_CleanTail(Laser* laser);

void Laser_PushBack(Laser* laser);

void Laser_Render(Laser* laser, Raster* screen, double* glowScreen, 
    Vector offset, Vector scale);

void Laser_Destroy(Laser* laser);

#endif