#include "Circle.h"

void renderCircle(Raster* screen, Vector center, double radius, 
    Color col1, Color col2, Vector offset, Vector scale) {
    
    int width = screen->width;
    int height = screen->height;
    uint8_t* pixels = screen->pixels;
    
    center.x = offset.x + scale.x * center.x;
    center.y = offset.y + scale.y * center.y;
    radius *= scale.x;
    
    double xmin = center.x - radius;
    double ymin = center.y - radius;
    double xmax = center.x + radius;
    double ymax = center.y + radius;
    
    double edge = 0.1;
    double leftEdge = edge;
    double rightEdge = width - edge;
    double topEdge = edge;
    double bottomEdge = height - edge;
    if (xmax < leftEdge) return;
    if (ymax < topEdge) return;
    if (xmin > rightEdge) return;
    if (ymin > bottomEdge) return;
    if (xmin < leftEdge) xmin = leftEdge;
    if (ymin < topEdge) ymin = topEdge;
    if (xmax > rightEdge) xmax = rightEdge;
    if (ymax > bottomEdge) ymax = bottomEdge;
    
    double r2 = radius * radius;
    double oneOverR = 1.0 / radius;
    
    int startX = (int) (xmin + 0.5);
    int endX = (int) (xmax + 0.5);
    int startY = (int) (ymin + 0.5);
    int endY = (int) (ymax + 0.5);
    int idx = 3 * (startX + startY * width);
    int rowInc = 3 * (width - (endX - startX));
    for (int yi = startY; yi < endY; yi++) {
        double y = yi + 0.5;
        for (int xi = startX; xi < endX; xi++) {
            double x = xi + 0.5;
            double xDiff = x - center.x;
            double yDiff = y - center.y;
            double h = sqrt(r2 - (xDiff * xDiff + yDiff * yDiff));
            h *= oneOverR;
            double hInv = 1.0 - h;
            
            if (h < 1.0) {
                pixels[idx + 0] = (uint8_t) (hInv * col1.r + h * col2.r);
                pixels[idx + 1] = (uint8_t) (hInv * col1.g + h * col2.g);
                pixels[idx + 2] = (uint8_t) (hInv * col1.b + h * col2.b);
            }
            idx += 3;
        }
        idx += rowInc;
    }
}