#include "Laser.h"

Laser* Laser_Create(Vector pos, Vector dir, 
    const Polygon* const* polygons, int polygonsSize) {
        
    Laser* laser = (Laser*) malloc(sizeof(Laser));
    
    laser->points = (Vector*) malloc(8 * sizeof(Vector));
    laser->pointsCapacity = 8;
    
    laser->headIndex = 0;
    laser->tailIndex = 0;
    
    laser->head = 0.0;
    laser->tailLength = 0.0;
    laser->emitting = 0;
    laser->totalSegmentsLength = 0.0;
    laser->points[0] = pos;
    laser->nextDir = dir;
    
    laser->lastObjectHitID = (ObjectID) { -1, -1 };
    
    Laser_Bounce(laser, polygons, polygonsSize);
    
    laser->tailSegmentLength = laser->totalSegmentsLength;
    
    return laser;
}
    
void Laser_Update(Laser* laser, double speed, double delta,
    const Polygon* const* polygons, int polygonsSize) {
    
    laser->head += speed * delta;
    if (laser->emitting) {
        laser->tailLength = laser->head;
    }
    while (laser->head > laser->totalSegmentsLength) {
        Laser_Bounce(laser, polygons, polygonsSize);
    }
    
    Laser_CleanTail(laser);
    Laser_ShrinkCapacity(laser);
}

void Laser_Bounce(Laser* laser,
    const Polygon* const* polygons, int polygonsSize) {
        
    int headIndex = laser->headIndex;
    Vector pos = laser->points[headIndex];
    Vector dir = laser->nextDir;
    RayHit hit = castRayPolygons(pos, dir, laser->lastObjectHitID,
        polygons, polygonsSize);
    
    int newHeadIndex = headIndex + 1;
    if (hit.validHit) {
        laser->lastObjectHitID = hit.id;
        laser->points[newHeadIndex] = hit.point;
        laser->nextDir = hit.reflectedRay;
        Vector lastHead = laser->points[headIndex];
        laser->totalSegmentsLength += magnitude((Vector) {
            hit.point.x - lastHead.x,
            hit.point.y - lastHead.y
        });
    } else {
        Vector infinity = (Vector) {
            pos.x + dir.x * 1.0e6,
            pos.y + dir.y * 1.0e6
        };
        laser->points[newHeadIndex] = infinity;
        laser->nextDir = hit.reflectedRay;
        Vector lastHead = laser->points[headIndex];
        laser->totalSegmentsLength += magnitude((Vector) {
            infinity.x - lastHead.x,
            infinity.y - lastHead.y
        });
    }
    laser->headIndex = newHeadIndex;
        
    Laser_GrowCapacity(laser);
}

void Laser_GrowCapacity(Laser* laser) {
    int headIndex = laser->headIndex;
    int tailIndex = laser->tailIndex;
    int pointsCapacity = laser->pointsCapacity;
    int numPoints = headIndex - tailIndex + 1;
    int twiceNumPoints = 2 * numPoints;
    if (twiceNumPoints >= pointsCapacity) {
        pointsCapacity *= 2;
        laser->points = (Vector*) realloc(laser->points, 
            pointsCapacity * sizeof(Vector));
    } else if (headIndex >= pointsCapacity - 1) {
        Laser_PushBack(laser);
    }
    laser->pointsCapacity = pointsCapacity;
}

void Laser_ShrinkCapacity(Laser* laser) {
    Laser_PushBack(laser);
    
    int numPoints = laser->headIndex - laser->tailIndex + 1;
    int fourTimesNumPoints = 4 * numPoints;
    int pointsCapacity = laser->pointsCapacity;
    while (fourTimesNumPoints < pointsCapacity) {
        pointsCapacity /= 2;
    }
    laser->points = (Vector*) realloc(laser->points,
        pointsCapacity * sizeof(Vector));
    laser->pointsCapacity = pointsCapacity;
}

void Laser_CleanTail(Laser* laser) {
    double head = laser->head;
    double totalSegmentsLength = laser->totalSegmentsLength;
    double tailSegmentLength = laser->tailSegmentLength;
    double tailLength = laser->tailLength;
    while (head - tailLength > tailSegmentLength) {
        int tailIndex = ++laser->tailIndex;
        head -= tailSegmentLength;
        totalSegmentsLength -= tailSegmentLength;
        Vector p1 = laser->points[tailIndex];
        Vector p2 = laser->points[tailIndex + 1];
        tailSegmentLength = magnitude((Vector) {
            p2.x - p1.x,
            p2.y - p1.y
        });
    }
    laser->totalSegmentsLength = totalSegmentsLength;
    laser->tailSegmentLength = tailSegmentLength;
    laser->head = head;
}

void Laser_PushBack(Laser* laser) {
    int tailIndex = laser->tailIndex;
    int headIndex = laser->headIndex;
    int newTailIndex = 0;
    int newHeadIndex = headIndex - tailIndex;
    int j = tailIndex;
    Vector* points = laser->points;
    for (int i = newTailIndex; i <= newHeadIndex; i++) {
        points[i] = points[j];
        j++;
    }
    laser->tailIndex = newTailIndex;
    laser->headIndex = newHeadIndex;
}

static void renderMultipleLines(Raster* screen, double* glowScreen, 
    Vector p1, Vector p2, Vector p1p2, Vector offset, Vector scale) {
    
    const int numLines = 3;
    const double width = 1.0;
    const double step = width / (numLines - 1);
    const double width2 = 0.5 * width;
    Vector a = {
        width2 * -p1p2.y,
        width2 * p1p2.x
    };
    Vector b = {
        step * p1p2.y,
        step * -p1p2.x
    };
    
    for (int i = 0; i < numLines; i++) {
        Vector o = {
            a.x + b.x * i,
            a.y + b.y * i
        };
        Vector p1_ = {
            p1.x + o.x,
            p1.y + o.y
        };
        Vector p2_ = {
            p2.x + o.x,
            p2.y + o.y
        };
        renderLine(screen, p1_, p2_, (Color) { 255, 255, 255 }, 
            offset, scale);
        renderLineDoubleBuffer(glowScreen, screen->width, screen->height, 
            p1_, p2_, offset, scale);
    }
}

void Laser_Render(Laser* laser, Raster* screen, double* glowScreen, 
    Vector offset, Vector scale) {
        
    Vector* points = laser->points;
    int tailIndex = laser->tailIndex;
    int headIndex = laser->headIndex;
    double head = laser->head;
    double tailLength = laser->tailLength;
    double tail = head - tailLength;
    double tailSegmentLength = laser->tailSegmentLength;
    double totalSegmentsLength = laser->totalSegmentsLength;
    
    for (int i = tailIndex + 1; i < headIndex - 1; i++) {
        Vector p1 = points[i];
        Vector p2 = points[i + 1];
        Vector p1p2 = (Vector) {
            p2.x - p1.x,
            p2.y - p1.y
        };
        double mag = magnitude(p1p2);
        double oneOverMagnitude = 1.0 / mag;
        p1p2.x *= oneOverMagnitude;
        p1p2.y *= oneOverMagnitude;
        renderMultipleLines(screen, glowScreen, 
            p1, p2, p1p2, offset, scale);
    }
    
    if (tailLength > tailSegmentLength - tail) {
        Vector p1 = points[tailIndex];
        Vector p2 = points[tailIndex + 1];
        Vector p1p2 = (Vector) {
            p2.x - p1.x,
            p2.y - p1.y
        };
        double oneOverMagnitude = 1.0 / tailSegmentLength;
        p1p2.x *= oneOverMagnitude;
        p1p2.y *= oneOverMagnitude;
        Vector p1Start = (Vector) {
            p1.x + tail * p1p2.x,
            p1.y + tail * p1p2.y
        };
        renderMultipleLines(screen, glowScreen, 
            p1Start, p2, p1p2, offset, scale);
        
        p1 = points[headIndex - 1];
        p2 = points[headIndex];
        p1p2 = (Vector) {
            p2.x - p1.x,
            p2.y - p1.y
        };
        double mag = magnitude(p1p2);
        oneOverMagnitude = 1.0 / mag;
        double lenFront = mag - (totalSegmentsLength - head);
        p1p2.x *= oneOverMagnitude;
        p1p2.y *= oneOverMagnitude;
        Vector p2End = (Vector) {
            p1.x + lenFront * p1p2.x,
            p1.y + lenFront * p1p2.y
        };
        renderMultipleLines(screen, glowScreen, 
            p1, p2End, p1p2, offset, scale);
    } else {
        Vector p1 = points[tailIndex];
        Vector p2 = points[tailIndex + 1];
        Vector p1p2 = (Vector) {
            p2.x - p1.x,
            p2.y - p1.y
        };
        double oneOverMagnitude = 1.0 / tailSegmentLength;
        p1p2.x *= oneOverMagnitude;
        p1p2.y *= oneOverMagnitude;
        Vector p1Start = (Vector) {
            p1.x + tail * p1p2.x,
            p1.y + tail * p1p2.y
        };
        Vector p2End = (Vector) {
            p1.x + head * p1p2.x,
            p1.y + head * p1p2.y
        };
        renderMultipleLines(screen, glowScreen, 
            p1Start, p2End, p1p2, offset, scale);
    }
}

void Laser_Destroy(Laser* laser) {
    if (laser == NULL)
        return;
    
    free(laser->points);
    
    free(laser);
}