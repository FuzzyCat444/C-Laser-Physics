#include "Collision.h"

Vector collideCirclePolygons(Vector center, double radius, 
    const Polygon* const* polygons, int polygonsSize) {

    CirclePolygonsCollision check1 = collideCirclePolygonsHelper(
        center, radius, polygons, polygonsSize
    );
    
    switch (check1.pushVecsSize) {
    case 0:
        return center;
        break;
    case 1: {
        Vector newCenter = center;
        newCenter.x += check1.pushVecs[0].x;
        newCenter.y += check1.pushVecs[0].y;
        CirclePolygonsCollision check2 = collideCirclePolygonsHelper(
            newCenter, radius, polygons, polygonsSize
        );
        
        if (check2.pushVecsSize == 0 ||
            (check2.pushVecsSize == 1 &&
            sameID(check1.ids[0], check2.ids[0]))) {
            return newCenter;
        }
        
        int indexOpposite;
        if (check2.pushVecsSize == 1) {
            indexOpposite = 0;
        } else {
            if (sameID(check2.ids[0], check1.ids[0])) {
                indexOpposite = 1;
            } else {
                indexOpposite = 0;
            }
        }
        Vector nudge = {
            0.5 * check2.pushVecs[indexOpposite].x,
            0.5 * check2.pushVecs[indexOpposite].y
        };
        newCenter.x += nudge.x;
        newCenter.y += nudge.y;
        check2 = collideCirclePolygonsHelper(
            newCenter, radius, polygons, polygonsSize
        );
        if (check2.pushVecsSize == 0) {
            return newCenter;
        } else if (check2.pushVecsSize == 1) {
            newCenter.x += nudge.x;
            newCenter.y += nudge.y;
            return newCenter;
        } else {
            Vector push = getMutualProjectionVector(
                check2.pushVecs[0], check2.pushVecs[1]
            );
            newCenter.x += push.x;
            newCenter.y += push.y;
            return newCenter;
        }
        break;
    }
    case 2: {
        Vector newCenter = center;
        newCenter.x += check1.pushVecs[0].x;
        newCenter.y += check1.pushVecs[0].y;
        CirclePolygonsCollision check2 = collideCirclePolygonsHelper(
            newCenter, radius, polygons, polygonsSize
        );
        
        if (check2.pushVecsSize == 0 ||
            (check2.pushVecsSize == 1 &&
            sameID(check1.ids[0], check2.ids[0]))) {
            return newCenter;
        }
        
        newCenter = center;
        newCenter.x += check1.pushVecs[1].x;
        newCenter.y += check1.pushVecs[1].y;
        check2 = collideCirclePolygonsHelper(
            newCenter, radius, polygons, polygonsSize
        );
        
        if (check2.pushVecsSize == 0 ||
            (check2.pushVecsSize == 1 &&
            sameID(check1.ids[1], check2.ids[0]))) {
            return newCenter;
        }
        
        Vector push = getMutualProjectionVector(
            check1.pushVecs[0], check1.pushVecs[1]
        );
        newCenter = center;
        newCenter.x += push.x;
        newCenter.y += push.y;
        return newCenter;
        break;
    }
    }
    return center;
}

CirclePolygonsCollision collideCirclePolygonsHelper(
    Vector center, double radius,
    const Polygon* const* polygons, int polygonsSize) {
        
    double r2 = radius * radius;
        
    CirclePolygonsCollision res;
    for (int i = 0; i < 2; i++) {
        ObjectID nullID = (ObjectID) {
            -1, -1
        };
        res.ids[i] = nullID;
    }
    res.pushVecsSize = 0;
        
    for (int i = 0; i < polygonsSize; i++) {
        const Polygon* polygon = polygons[i];
        int pointsSize = polygon->pointsSize;
        
        int p1i = 0;
        int p2i = 1;
        int p3i = 2;
        for (int j = 0; j < pointsSize; j++) {
            Vector p1 = polygon->points[p1i];
            Vector p2 = polygon->points[p2i];
            Vector p3 = polygon->points[p3i];
            
            Vector p1p2 = (Vector) {
                p2.x - p1.x,
                p2.y - p1.y
            };
            Vector p3p2 = (Vector) {
                p2.x - p3.x,
                p2.y - p3.y
            };
            Vector p1c = (Vector) {
                center.x - p1.x,
                center.y - p1.y
            };
            Vector p2c = (Vector) {
                center.x - p2.x,
                center.y - p2.y
            };
            // Side
            if (hasProjectionOnVector(p1c, p1p2)) {
                double a2 = magnitudeSquared(p1c);
                double b2 = magnitudeSquared(p1p2);
                double b2Inv = 1.0 / b2;
                double dot = dotProduct(p1c, p1p2);
                double dot2 = dot * dot;
                double d2 = a2 - dot2 * b2Inv;
                if (d2 < r2) {
                    double d = sqrt(d2);
                    double pen = radius - d;
                    double scl = pen / d;
                    double projScl = dot * b2Inv;
                    Vector proj = (Vector) {
                        p1.x + projScl * p1p2.x,
                        p1.y + projScl * p1p2.y
                    };
                    Vector push = (Vector) {
                        center.x - proj.x,
                        center.y - proj.y
                    };
                    push.x *= scl;
                    push.y *= scl;
                    res.ids[res.pushVecsSize] = (ObjectID) {
                        i, p1i
                    };
                    res.pushVecs[res.pushVecsSize] = push;
                    res.pushVecsSize++;
                }
            }
            if (res.pushVecsSize == 2) break;
            // Vertex
            if (dotProduct(p2c, p1p2) >= 0.0 && dotProduct(p2c, p3p2) >= 0.0) {
                double d2 = magnitudeSquared(p2c);
                if (d2 < r2) {
                    double d = sqrt(d2);
                    double pen = radius - d;
                    double scl = pen / d;
                    Vector push = p2c;
                    push.x *= scl;
                    push.y *= scl;
                    res.ids[res.pushVecsSize] = (ObjectID) {
                        i, -p2i - 2
                    };
                    res.pushVecs[res.pushVecsSize] = push;
                    res.pushVecsSize++;
                }
            }
            if (res.pushVecsSize == 2) break;
            
            p1i++;
            p2i++;
            p3i++;
            if (p3i == pointsSize)
                p3i = 0;
            else if (p2i == pointsSize)
                p2i = 0;
        }
        if (res.pushVecsSize == 2) break;
    }
    
    return res;
}
