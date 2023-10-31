#include "Parallax.h"

#include "Line.h"

void renderParallax(Raster* screen, const Polygon* const* polygons,
    RenderPolygon* const* renderPolygons, int polygonsSize, const View* view,
    int numSlices, double z1, Color col1, double z2, Color col2, int doCap,
    Color capCol) {
        
    int width = screen->width;
    int height = screen->height;
    double width2 = width / 2.0;
    double height2 = height / 2.0;
    
    double z = z1;
    double zInc = (z2 - z1) / numSlices;
    double colorT = 0.0;
    double colorTInc = 1.0 / numSlices;
    double oneOverLog2 = 1.0 / log(2.0);
    View combined;
    Color col;
    int iterations = doCap ? numSlices - 1 : numSlices;
    for (int i = 0; i < iterations; i++) {
        View parallaxView = View_Create(
            (Vector) {width, height },
            (Vector) {width2, height2 },
            -log(z) * oneOverLog2);
        double colorTInv = 1.0 - colorT;
        col = (Color) {
            (uint8_t) (col1.r * colorTInv + col2.r * colorT),
            (uint8_t) (col1.g * colorTInv + col2.g * colorT),
            (uint8_t) (col1.b * colorTInv + col2.b * colorT)
        };
        combined = View_Combine(view, &parallaxView);
        for (int j = 0; j < polygonsSize; j++) {
            const Polygon* polygon = polygons[j];
            int p1i = 0;
            int p2i = 1;
            for (int k = 0; k < polygon->pointsSize; k++) {
                Vector p1 = polygon->points[p1i];
                Vector p2 = polygon->points[p2i];
                renderLine(screen, p1, p2, col,
                    combined.offset, combined.scale);
                
                p1i++;
                p2i++;
                if (p2i == polygon->pointsSize) p2i = 0;
            }
        }
        z += zInc;
        colorT += colorTInc;
    }
    if (doCap) {
        for (int j = 0; j < polygonsSize; j++) { 
            // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
            RenderPolygon_Render(renderPolygons[j], screen,
                capCol, combined.offset, combined.scale);
        }
    }
}
