#include "Ray.h"

RayHit castRayPolygons(Vector pos, Vector dir, ObjectID avoidID,
    const Polygon* const* polygons, int polygonsSize) {
    
    dir = normalized(dir);
    
    double closestT = 1.0e100;
    int closestPolygonIndex = -1;
    int closestLineIndex1 = -1;
    int closestLineIndex2 = -1;
    ObjectID id = (ObjectID) { -1, -1 };
    for (int i = 0; i < polygonsSize; i++) {
        const Polygon* polygon = polygons[i];
        
        int p1i = 0;
        int p2i = 1;
        for (int j = 0; j < polygon->pointsSize; j++) {
            Vector p1 = polygon->points[p1i];
            Vector p2 = polygon->points[p2i];
            
            double axbx = pos.x - p1.x;
            double ayby = pos.y - p1.y;
            Vector u = {
                dir.x,
                dir.y
            };
            Vector v = {
                p2.x - p1.x,
                p2.y - p1.y
            };
            double uxvy_uyvx = u.x * v.y - u.y * v.x;
            double s = (u.x * ayby - u.y * axbx) / uxvy_uyvx;
            if (s >= 0.0 && s <= 1.0) {
                double t = (v.x * ayby - v.y * axbx) / uxvy_uyvx;
                ObjectID newID = (ObjectID) { i, p1i };
                if (!sameID(newID, avoidID) && t > 0.0 && t < closestT) {
                    closestT = t;
                    closestPolygonIndex = i;
                    closestLineIndex1 = p1i;
                    closestLineIndex2 = p2i;
                    id = newID;
                }
            }
            
            p1i++;
            p2i++;
            if (p2i == polygon->pointsSize) {
                p2i = 0;
            }
        }
    }
    
    RayHit hit;
    hit.id = id;
    if (closestPolygonIndex == -1) {
        hit.validHit = 0;
        hit.reflectedRay = dir;
        return hit;
    }
    
    hit.validHit = 1;
    hit.t = closestT;
    
    hit.point = (Vector) {
        pos.x + hit.t * dir.x,
        pos.y + hit.t * dir.y
    };
    
    const Polygon* hitPolygon = polygons[closestPolygonIndex];
    Vector p1 = hitPolygon->points[closestLineIndex1];
    Vector p2 = hitPolygon->points[closestLineIndex2];
    Vector p1p2 = (Vector) {
        p2.x - p1.x,
        p2.y - p1.y
    };
    Vector normal = normalized(
        (Vector) {
            p1p2.y,
            -p1p2.x
        }
    );
    double _2dot = 2.0 * dotProduct(dir, normal);
    hit.reflectedRay = (Vector) {
        dir.x - _2dot * normal.x,
        dir.y - _2dot * normal.y
    };
    
    return hit;
}