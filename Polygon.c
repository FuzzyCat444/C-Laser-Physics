#include "Polygon.h"

Polygon* Polygon_Create() {
    Polygon* polygon = (Polygon*) malloc(sizeof(Polygon));
    polygon->points = (Vector*) malloc(sizeof(Vector));
    polygon->pointsSize = 0;
    polygon->pointsCapacity = 1;
    return polygon;
}

void Polygon_PushPoint(Polygon* polygon, Vector p) {
    int size = polygon->pointsSize;
    int capacity = polygon->pointsCapacity;
    polygon->points[size++] = p;
    if (size == capacity) {
        capacity *= 2;
        polygon->points = (Vector*) realloc(polygon->points,
            capacity * sizeof(Vector));
    }
    polygon->pointsSize = size;
    polygon->pointsCapacity = capacity;
}

void Polygon_SetPoint(Polygon* polygon, int index, Vector p) {
    if (index < 0 || index >= polygon->pointsSize)
        return;
    
    polygon->points[index] = p;
}

void Polygon_PopPoint(Polygon* polygon) {
    int size = polygon->pointsSize;
    int capacity = polygon->pointsCapacity;
    if (--size * 2 < capacity) {
        capacity /= 2;
        polygon->points = (Vector*) realloc(polygon->points,
            capacity * sizeof(Vector));
    }
    polygon->pointsSize = size;
    polygon->pointsCapacity = capacity;
}

void Polygon_RemovePoint(Polygon* polygon, int index) {
    if (index < 0 || index >= polygon->pointsSize)
        return;
    
    for (int i = index + 1; i < polygon->pointsSize; i++) {
        polygon->points[i - 1] = polygon->points[i];
    }
    
    Polygon_PopPoint(polygon);
}

Vector Polygon_GetPoint(Polygon* polygon, int index) {
    if (index < 0 || index >= polygon->pointsSize)
        return (Vector) {0.0, 0.0 };
    
    return polygon->points[index];
}

void Polygon_Destroy(Polygon* polygon) {
    if (polygon == NULL)
        return;
    
    free(polygon->points);
    
    free(polygon);
}

SlopeScanner* SlopeScanner_Create(double y1, double y2) {
    SlopeScanner* slopeScanner = (SlopeScanner*) malloc(sizeof(SlopeScanner));
    
    if (y2 < y1) {
        double tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    slopeScanner->y1 = y1;
    slopeScanner->y2 = y2;
    slopeScanner->xms = (double*) malloc(4 * sizeof(double));
    slopeScanner->xmsSize = 0;
    slopeScanner->xmsCapacity = 1;
    
    return slopeScanner;
}

void SlopeScanner_InsertLine(SlopeScanner* slopeScanner, 
    Vector p1, Vector p2) {
        
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    
    int size = slopeScanner->xmsSize;
    int capacity = slopeScanner->xmsCapacity;
    size++;
    if (size == capacity) {
        capacity *= 2;
        slopeScanner->xms = (double*) realloc(slopeScanner->xms,
            capacity * 4 * sizeof(double));
    }
    slopeScanner->xmsSize = size;
    slopeScanner->xmsCapacity = capacity;
    
    double* xms = slopeScanner->xms;
    double t = (slopeScanner->y1 - p1.y) / dy;
    double x = p1.x + t * dx;
    double m = dx / dy;
    double scannerDy = slopeScanner->y2 - slopeScanner->y1;
    double xsort = x + m * 0.5 * scannerDy;
    
    int i = 0;
    int j = size - 1;
    while (i != j) {
        int k = (i + j) / 2;
        
        int idx = 2 * k;
        double xk = xms[idx];
        double mk = xms[idx + 1];
        double xsortk = xk + mk * 0.5 * scannerDy;
        
        if (xsort > xsortk) {
            i = k + 1;
        } else {
            j = k;
        }
    }
    
    for (int k = size - 2; k >= i; k--) {
        int idx = 2 * k;
        xms[idx + 2] = xms[idx];
        xms[idx + 3] = xms[idx + 1];
    }
    int idx = 2 * i;
    xms[idx] = x;
    xms[idx + 1] = m;
    
}

void SlopeScanner_Render(SlopeScanner* slopeScanner, Raster* screen,
    Color col, Vector offset, Vector scale) {
    
    uint8_t* pixels = screen->pixels;
    int width = screen->width;
    int height = screen->height;
    
    double* xms = slopeScanner->xms;
    int size = slopeScanner->xmsSize;
    
    const int copyIndex = 2 * size;
    int idx = copyIndex;
    double slopeScale = scale.x / scale.y;
    for (int i = 0; i < size; i++) {
        double cx = xms[idx - copyIndex];
        double cm = xms[idx - copyIndex + 1];
        cx = offset.x + scale.x * cx;
        cm = slopeScale * cm;
        xms[idx] = cx;
        xms[idx + 1] = cm;
        idx += 2;
    }
    
    double y1 = offset.y + scale.y * (slopeScanner->y1);
    double y2 = offset.y + scale.y * (slopeScanner->y2);
    int yi1 = (int) (y1 + 0.5);
    int yi2 = (int) (y2 + 0.5);
    if (yi1 < 0) yi1 = 0;
    if (yi2 > height) yi2 = height;
    double startDy = yi1 + 0.5 - y1;
    idx = copyIndex;
    for (int i = 0; i < size; i++) {
        xms[idx] += startDy * xms[idx + 1];
        idx += 2;
    }
    for (int yi = yi1; yi < yi2; yi++) {
        int idx1 = copyIndex;
        int idx2 = copyIndex + 2;
        for (int i = 0; i < size / 2; i++) {
            int xi1 = (int) (xms[idx1] + 0.5);
            int xi2 = (int) (xms[idx2] + 0.5);
            if (xi2 < xi1) {
                int tmp = xi1;
                xi1 = xi2;
                xi2 = tmp;
            }
            if (xi1 < 0) xi1 = 0;
            if (xi2 > width) xi2 = width;
            int pixelI = 3 * (xi1 + yi * width);
            for (int xi = xi1; xi < xi2; xi++) {
                pixels[pixelI + 0] = col.r;
                pixels[pixelI + 1] = col.g;
                pixels[pixelI + 2] = col.b;
                pixelI += 3;
            }
            idx1 += 4;
            idx2 += 4;
        }
        
        idx = copyIndex;
        for (int i = 0; i < size; i++) {
            xms[idx] += xms[idx + 1];
            idx += 2;
        }
    }
}

void SlopeScanner_Destroy(SlopeScanner* slopeScanner) {
    if (slopeScanner == NULL)
        return;
    
    free(slopeScanner->xms);
    
    free(slopeScanner);
}

RenderPolygon* RenderPolygon_Create(Polygon* polygon) {
    RenderPolygon* renderPolygon = 
        (RenderPolygon*) malloc(sizeof(RenderPolygon));
    
    Vector* points = polygon->points;
    int numPoints = polygon->pointsSize;
    int numScanners = numPoints - 1;
    double xmin = INFINITY, ymin = INFINITY,
           xmax = -INFINITY, ymax = -INFINITY;
    for (int i = 0; i < numPoints; i++) {
        Vector p = points[i];
        if (p.x < xmin) xmin = p.x;
        if (p.y < ymin) ymin = p.y;
        if (p.x > xmax) xmax = p.x;
        if (p.y > ymax) ymax = p.y;
    }
    renderPolygon->xmin = xmin;
    renderPolygon->ymin = ymin;
    renderPolygon->xmax = xmax;
    renderPolygon->ymax = ymax;
    
    renderPolygon->slopeScanners = 
        (SlopeScanner**) malloc(numScanners * sizeof(SlopeScanner*));
    renderPolygon->slopeScannersSize = numScanners;
    SlopeScanner** scanners = renderPolygon->slopeScanners;
    
    Vector* pointsSorted = (Vector*) malloc(numPoints * sizeof(Vector));
    int* indicesSorted = (int*) malloc(numPoints * sizeof(int));
    for (int i = 0; i < numPoints; i++) {
        pointsSorted[i] = points[i];
        indicesSorted[i] = i;
    }
    sortVectorsByY(pointsSorted, indicesSorted, numPoints);
    
    for (int i = 0; i < numScanners; i++) {
        Vector p1 = pointsSorted[i];
        Vector p2 = pointsSorted[i + 1];
        scanners[i] = SlopeScanner_Create(p1.y, p2.y);
    }
    
    for (int i = 0; i < numPoints; i++) {
        int p1i = i;
        int p2i = i + 1;
        if (p2i == numPoints) p2i = 0;
        int startIndex = -1;
        int endIndex = -1;
        for (int j = 0; j < numPoints; j++) {
            if (p1i == indicesSorted[j] || p2i == indicesSorted[j]) {
                if (startIndex == -1) {
                    startIndex = j;
                } else {
                    endIndex = j;
                    break;
                }
            }
        }
        
        Vector p1 = points[p1i];
        Vector p2 = points[p2i];
        for (int j = startIndex; j < endIndex; j++) {
            SlopeScanner_InsertLine(scanners[j], p1, p2);
        }
    }
    
    for (int i = 0; i < numScanners; i++) {
        SlopeScanner* scanner = scanners[i];
        if (scanner->xmsSize % 2 == 1) {
            SlopeScanner_Destroy(scanner);
            scanners[i] = NULL;
        }
    }
    
    free(pointsSorted);
    free(indicesSorted);
        
    return renderPolygon;
}

void RenderPolygon_Render(RenderPolygon* renderPolygon, Raster* screen,
    Color col, Vector offset, Vector scale) {
    
    double xmin = offset.x + scale.x * renderPolygon->xmin;
    double ymin = offset.y + scale.y * renderPolygon->ymin;
    double xmax = offset.x + scale.x * renderPolygon->xmax;
    double ymax = offset.y + scale.y * renderPolygon->ymax;
    if (xmax < 0.0 || ymax < 0.0 || 
        xmin >= screen->width || ymin >= screen->height) {
        return;    
    }
    
    for (int i = 0; i < renderPolygon->slopeScannersSize; i++) {
        SlopeScanner* scanner = renderPolygon->slopeScanners[i];
        if (scanner != NULL)
            SlopeScanner_Render(scanner, screen, col, offset, scale);
    }
}

void RenderPolygon_Destroy(RenderPolygon* renderPolygon) {
    if (renderPolygon == NULL)
        return;
    
    for (int i = 0; i < renderPolygon->slopeScannersSize; i++) {
        SlopeScanner_Destroy(renderPolygon->slopeScanners[i]);
    }
    free(renderPolygon->slopeScanners);
    
    free(renderPolygon);
}