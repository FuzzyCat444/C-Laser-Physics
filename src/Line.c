#include "Line.h"

void renderLine(Raster* screen, Vector p1, Vector p2, Color col,
    Vector offset, Vector scale) {
        
    int width = screen->width;
    int height = screen->height;
    uint8_t* pixels = screen->pixels;
    
    p1.x = offset.x + scale.x * p1.x;
    p1.y = offset.y + scale.y * p1.y;
    p2.x = offset.x + scale.x * p2.x;
    p2.y = offset.y + scale.y * p2.y;
    
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dydx = dy / dx;
    double dxdy = dx / dy;
    
    if (p2.x < p1.x) {
        Vector tmp = p1;
        p1 = p2;
        p2 = tmp;
    }
    double edge = 0.1;
    double leftEdge = edge;
    double rightEdge = width - edge;
    double topEdge = edge;
    double bottomEdge = height - edge;
    if (p2.x < leftEdge) {
        return;
    } else if (p1.x < leftEdge) {
        double x = leftEdge;
        double y = p1.y + dydx * (leftEdge - p1.x);
        p1.x = x;
        p1.y = y;
    }
    if (p1.x > rightEdge) {
        return;
    } else if (p2.x > rightEdge) {
        double x = rightEdge;
        double y = p1.y + dydx * (rightEdge - p1.x);
        p2.x = x;
        p2.y = y;
    }
    
    if (p2.y < p1.y) {
        Vector tmp = p1;
        p1 = p2;
        p2 = tmp;
    }
    if (p2.y < topEdge) {
        return;
    } else if (p1.y < topEdge) {
        double x = p1.x + dxdy * (topEdge - p1.y);
        double y = topEdge;
        p1.x = x;
        p1.y = y;
    }
    if (p1.y > bottomEdge) {
        return;
    } else if (p2.y > bottomEdge) {
        double x = p1.x + dxdy * (bottomEdge - p1.y);
        double y = bottomEdge;
        p2.x = x;
        p2.y = y;
    }
    
    if (abs(dx) >= abs(dy)) {
        if (p2.x < p1.x) {
            Vector tmp = p1;
            p1 = p2;
            p2 = tmp;
        }
        
        int startX = (int) (p1.x + 0.5);
        int endX = (int) (p2.x + 0.5);
        double startYExact = p1.y + dydx * (startX + 0.5 - p1.x);
        double startY = (int) startYExact;
        int yInc;
        double error;
        double errorInc;
        if (dydx > 0.0) {
            yInc = 1;
            error = startYExact - startY;
            errorInc = dydx;
        } else {
            yInc = -1;
            error = startY + 1 - startYExact;
            errorInc = -dydx;
        }
        
        int idx = 3 * (startX + startY * width);
        int rowInc = 3 * width * yInc;
        for (int xi = startX; xi < endX; xi++) {
            pixels[idx + 0] = col.r;
            pixels[idx + 1] = col.g;
            pixels[idx + 2] = col.b;
            
            idx += 3;
            error += errorInc;
            if (error > 1.0) {
                error -= 1.0;
                idx += rowInc;
            }
        }
    } else {
        if (p2.y < p1.y) {
            Vector tmp = p1;
            p1 = p2;
            p2 = tmp;
        }
        
        int startY = (int) (p1.y + 0.5);
        int endY = (int) (p2.y + 0.5);
        double startXExact = p1.x + dxdy * (startY + 0.5 - p1.y);
        double startX = (int) startXExact;
        int xInc;
        double error;
        double errorInc;
        if (dxdy > 0.0) {
            xInc = 1;
            error = startXExact - startX;
            errorInc = dxdy;
        } else {
            xInc = -1;
            error = startX + 1 - startXExact;
            errorInc = -dxdy;
        }
        
        int idx = 3 * (startX + startY * width);
        int rowInc = 3 * width;
        int colInc = 3 * xInc;
        for (int yi = startY; yi < endY; yi++) {
            pixels[idx + 0] = col.r;
            pixels[idx + 1] = col.g;
            pixels[idx + 2] = col.b;
            
            idx += rowInc;
            error += errorInc;
            if (error > 1.0) {
                error -= 1.0;
                idx += colInc;
            }
        }
    }
}

void renderLineDoubleBuffer(double* screen, int width, int height, 
    Vector p1, Vector p2, Vector offset, Vector scale) {
    
    p1.x = offset.x + scale.x * p1.x;
    p1.y = offset.y + scale.y * p1.y;
    p2.x = offset.x + scale.x * p2.x;
    p2.y = offset.y + scale.y * p2.y;
    
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dydx = dy / dx;
    double dxdy = dx / dy;
    
    if (p2.x < p1.x) {
        Vector tmp = p1;
        p1 = p2;
        p2 = tmp;
    }
    double edge = 0.1;
    double leftEdge = edge;
    double rightEdge = width - edge;
    double topEdge = edge;
    double bottomEdge = height - edge;
    if (p2.x < leftEdge) {
        return;
    } else if (p1.x < leftEdge) {
        double x = leftEdge;
        double y = p1.y + dydx * (leftEdge - p1.x);
        p1.x = x;
        p1.y = y;
    }
    if (p1.x > rightEdge) {
        return;
    } else if (p2.x > rightEdge) {
        double x = rightEdge;
        double y = p1.y + dydx * (rightEdge - p1.x);
        p2.x = x;
        p2.y = y;
    }
    
    if (p2.y < p1.y) {
        Vector tmp = p1;
        p1 = p2;
        p2 = tmp;
    }
    if (p2.y < topEdge) {
        return;
    } else if (p1.y < topEdge) {
        double x = p1.x + dxdy * (topEdge - p1.y);
        double y = topEdge;
        p1.x = x;
        p1.y = y;
    }
    if (p1.y > bottomEdge) {
        return;
    } else if (p2.y > bottomEdge) {
        double x = p1.x + dxdy * (bottomEdge - p1.y);
        double y = bottomEdge;
        p2.x = x;
        p2.y = y;
    }
    
    if (abs(dx) >= abs(dy)) {
        if (p2.x < p1.x) {
            Vector tmp = p1;
            p1 = p2;
            p2 = tmp;
        }
        
        int startX = (int) (p1.x + 0.5);
        int endX = (int) (p2.x + 0.5);
        double startYExact = p1.y + dydx * (startX + 0.5 - p1.x);
        double startY = (int) startYExact;
        int yInc;
        double error;
        double errorInc;
        if (dydx > 0.0) {
            yInc = 1;
            error = startYExact - startY;
            errorInc = dydx;
        } else {
            yInc = -1;
            error = startY + 1 - startYExact;
            errorInc = -dydx;
        }
        
        int idx = startX + startY * width;
        int rowInc = width * yInc;
        for (int xi = startX; xi < endX; xi++) {
            screen[idx] = 1.0;
            
            idx++;
            error += errorInc;
            if (error > 1.0) {
                error -= 1.0;
                idx += rowInc;
            }
        }
    } else {
        if (p2.y < p1.y) {
            Vector tmp = p1;
            p1 = p2;
            p2 = tmp;
        }
        
        int startY = (int) (p1.y + 0.5);
        int endY = (int) (p2.y + 0.5);
        double startXExact = p1.x + dxdy * (startY + 0.5 - p1.y);
        double startX = (int) startXExact;
        int xInc;
        double error;
        double errorInc;
        if (dxdy > 0.0) {
            xInc = 1;
            error = startXExact - startX;
            errorInc = dxdy;
        } else {
            xInc = -1;
            error = startX + 1 - startXExact;
            errorInc = -dxdy;
        }
        
        int idx = startX + startY * width;
        int rowInc = width;
        int colInc = xInc;
        for (int yi = startY; yi < endY; yi++) {
            screen[idx] = 1.0;
            
            idx += rowInc;
            error += errorInc;
            if (error > 1.0) {
                error -= 1.0;
                idx += colInc;
            }
        }
    }
}
