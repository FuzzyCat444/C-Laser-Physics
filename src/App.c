#include "App.h"

App* App_Create(int width, int height, Raster*(*loadRaster)(const char*)) {
    App* app = (App*) malloc(sizeof(App));
    
    app->screen = Raster_Create(width, height);
    app->blurBuffer1 = (double*) malloc(width * height * sizeof(double));
    app->blurBuffer2 = (double*) malloc(width * height * sizeof(double));
    
    app->leftPressed = 0;
    app->rightPressed = 0;
    app->downPressed = 0;
    app->upPressed = 0;
    app->spacePressed = 0;
    app->mousePressed = 0;
    app->mouseMoving = 0;
    app->mousePos = (Vector) { 0.0, 0.0 };
    
    app->polygonsSize = 0;
    app->polygonsCapacity = 1;
    app->polygons = (Polygon**) malloc(sizeof(Polygon*));
    app->renderPolygons = (RenderPolygon**) malloc(sizeof(RenderPolygon*));
    
    app->playerPos = (Vector) { 300.0, 100.0 };
    app->playerSpeed = 250.0;
    app->playerRadius = 15.0;
    app->shootTime = 0.0;
    app->shootTimeLimit = 0.03;
    app->playerCol = (Color) { 180, 0, 0 };
    
    App_GenTestLevel(app);
    
    app->lasers = (Laser**) malloc(sizeof(Laser*));
    app->lasersSize = 0;
    app->lasersCapacity = 1;
    app->shootingLaser = NULL;
    app->laserSpeed = 1000.0;
    
    app->glowColor = (Color) { 255, 40, 20 };
    app->glowSize = 25;
    app->glowIntensity = 5.5;
    
    app->bottomCol = (Color) { 0, 22 * 2, 17 * 2 };
    app->topCol = (Color) { 37 / 2, (22 * 2 + 220) / 2, (174 + 17 * 2) / 2 };
    app->ceilCol = (Color) { 37, 220, 174 };
    app->floorCol = (Color) { 4, 143, 120 };
    
    app->ceilHeight = 0.2;
    app->numSlices = 90;
    
    app->quit = 0;
    
    return app;
}

void App_Destroy(App* app) {
    if (app == NULL)
        return;
    
    for (int i = 0; i < app->lasersSize; i++) {
        Laser_Destroy(app->lasers[i]);
    }
    free(app->lasers);
    
    Raster_Destroy(app->screen);
    
    free(app->blurBuffer1);
    free(app->blurBuffer2);
    
    for (int i = 0; i < app->polygonsSize; i++) {
        Polygon_Destroy(app->polygons[i]);
        RenderPolygon_Destroy(app->renderPolygons[i]);
    }
    free(app->polygons);
    free(app->renderPolygons);
    
    free(app);
}

void App_AddPolygon(App* app, Polygon* polygon) {
    int size = app->polygonsSize;
    int capacity = app->polygonsCapacity;
    
    int index = size++;
    app->polygons[index] = polygon;
    app->renderPolygons[index] = RenderPolygon_Create(polygon);
    
    if (size == capacity) {
        capacity *= 2;
        app->polygons = (Polygon**) realloc(app->polygons, 
            capacity * sizeof(Polygon*));
        app->renderPolygons = (RenderPolygon**) realloc(app->renderPolygons, 
            capacity * sizeof(RenderPolygon*));
    }
    
    app->polygonsSize = size;
    app->polygonsCapacity = capacity;
}

void App_AddLaser(App* app, Laser* laser) {
    int size = app->lasersSize;
    int capacity = app->lasersCapacity;
    app->lasers[size++] = laser;
    if (size == capacity) {
        capacity *= 2;
        app->lasers = (Laser**) realloc(app->lasers,
            capacity * sizeof(Laser*));
    }
    app->lasersSize = size;
    app->lasersCapacity = capacity;
}

void App_HandleEvent(App* app, AppEventType type, AppEvent event) {
    switch (type) {
    case APPEVENT_KEYPRESSED:
        if (event.key == APPEVENT_KEY_ESCAPE) {
            app->quit = 1;
        } else if (event.key == APPEVENT_KEY_LEFT) {
            app->leftPressed = 1;
        } else if (event.key == APPEVENT_KEY_RIGHT) {
            app->rightPressed = 1;
        } else if (event.key == APPEVENT_KEY_DOWN) {
            app->downPressed = 1;
        } else if (event.key == APPEVENT_KEY_UP) {
            app->upPressed = 1;
        } else if (event.key == APPEVENT_KEY_SPACE) {
            app->spacePressed = 1;
        }
        break;
    case APPEVENT_KEYRELEASED:
        if (event.key == APPEVENT_KEY_LEFT) {
            app->leftPressed = 0;
        } else if (event.key == APPEVENT_KEY_RIGHT) {
            app->rightPressed = 0;
        } else if (event.key == APPEVENT_KEY_DOWN) {
            app->downPressed = 0;
        } else if (event.key == APPEVENT_KEY_UP) {
            app->upPressed = 0;
        } else if (event.key == APPEVENT_KEY_SPACE) {
            app->spacePressed = 0;
        }
        break;
    case APPEVENT_MOUSEPRESSED:
        app->mousePressed = 1;
        app->mousePos.x = event.mouse.x;
        app->mousePos.y = event.mouse.y;
        break;
    case APPEVENT_MOUSERELEASED:
        app->mousePressed = 0;
        app->mousePos.x = event.mouse.x;
        app->mousePos.y = event.mouse.y;
        break;
    case APPEVENT_MOUSEMOVED:
        app->mouseMoving = 1;
        app->mousePos.x = event.mouse.x;
        app->mousePos.y = event.mouse.y;
        break;
    }
}

void App_Render(App* app, double delta) {
    Color floorCol = app->floorCol;
    Color ceilCol = app->ceilCol;
    Color bottomCol = app->bottomCol;
    Color topCol = app->topCol;
    Color avgBottomTopCol = (Color) {
        (uint8_t) (0.5 * (bottomCol.r + topCol.r)),
        (uint8_t) (0.5 * (bottomCol.g + topCol.g)),
        (uint8_t) (0.5 * (bottomCol.b + topCol.b))
    };
    Color playerCol = app->playerCol;
    
    Raster* screen = app->screen;
    uint8_t* pixels = screen->pixels;
    int width = screen->width;
    int height = screen->height;
    Raster_Clear(screen, floorCol);
    
    double* blurBuffer1 = app->blurBuffer1;
    double* blurBuffer2 = app->blurBuffer2;
    int blurBufferSize = width * height;
    for (int i = 0; i < blurBufferSize; i++) {
        blurBuffer1[i] = 0.0;
    }
    
    Polygon** polygons = app->polygons;
    RenderPolygon** renderPolygons = app->renderPolygons;
    int polygonsSize = app->polygonsSize;
    
    // Update
    
    double speed = app->playerSpeed;
    double playerRadius = app->playerRadius;
    int leftPressed = app->leftPressed;
    int rightPressed = app->rightPressed;
    int downPressed = app->downPressed;
    int upPressed = app->upPressed;
    int spacePressed = app->spacePressed;
    int moving = leftPressed != rightPressed || downPressed != upPressed;
    int mouseMoving = app->mouseMoving;
    int mousePressed = app->mousePressed;
    Vector move = {
        speed * (rightPressed - leftPressed),
        speed * (downPressed - upPressed)
    };
    Vector playerPos = app->playerPos;
    
    const int stepsPerSec = 400;
    int numSteps = (int) (delta * stepsPerSec) + 1;
    double delta2 = delta / numSteps;
    for (int i = 0; i < numSteps; i++) {
        playerPos.x += move.x * delta2;
        playerPos.y += move.y * delta2;
        Vector newPlayerPos = collideCirclePolygons(playerPos, playerRadius, 
            polygons, polygonsSize);
        playerPos = newPlayerPos;
    }
    app->playerPos = playerPos;
    
    View playerView = View_Create(
        (Vector) { width, height },
        playerPos, 0.0
    );
    Vector playerScreenPos = {
        playerView.offset.x + playerView.scale.x * playerPos.x,
        playerView.offset.y + playerView.scale.y * playerPos.y
    };
    Vector mouse = app->mousePos;
    Vector mouseDir = normalized((Vector) {
        mouse.x - playerScreenPos.x,
        mouse.y - playerScreenPos.y
    });
    
    double shootTime = app->shootTime;
    if (mousePressed) {
        Laser* shootingLaser = app->shootingLaser;
        if (shootingLaser == NULL) {
            shootingLaser = Laser_Create((Vector) {
                    playerPos.x + (playerRadius - 3.0) * mouseDir.x,
                    playerPos.y + (playerRadius - 3.0) * mouseDir.y,
                }, mouseDir, polygons, polygonsSize);
                
            App_AddLaser(app, shootingLaser);
            app->shootingLaser = shootingLaser;
        } 
        shootingLaser->emitting = 1;
        
        shootTime += delta;
        if ((moving || mouseMoving) && shootTime > app->shootTimeLimit) {
            shootingLaser->emitting = 0;
            app->shootingLaser = NULL;
            shootTime = 0.0;
        }
    } else {
        Laser* shootingLaser = app->shootingLaser;
        if (shootingLaser != NULL)
            shootingLaser->emitting = 0;
        app->shootingLaser = NULL;
        shootTime = 0.0;
    }
    app->shootTime = shootTime;
    
    for (int i = 0; i < app->lasersSize; i++) {
        Laser* laser = app->lasers[i];
        Laser_Update(laser, app->laserSpeed, delta, polygons, polygonsSize);
    }
    
    if (spacePressed) {
        for (int i = 0; i < app->lasersSize; i++) {
            Laser* laser = app->lasers[i];
            Laser_Destroy(laser);
        }
        app->lasersSize = 0;
        app->lasersCapacity = 1;
        app->lasers = (Laser**) realloc(app->lasers, sizeof(Laser*));
    }
    
    // Render
    
    double ch2 = app->ceilHeight / 2.0;
    int numSlices = app->numSlices;
    
    renderParallax(screen, polygons, renderPolygons, polygonsSize, 
        &playerView,
        numSlices / 2, 1.0 + ch2, bottomCol, 1.0, avgBottomTopCol, 0,
        ceilCol);
        
    for (int i = 0; i < app->lasersSize; i++) {
        Laser* laser = app->lasers[i];
        Laser_Render(laser, screen, app->blurBuffer1,
            playerView.offset, playerView.scale);
    }
    
    blur(blurBuffer1, blurBuffer2, width, height, 
        app->glowSize, GAUSSIAN_BLUR);
    
    App_ApplyGlowEffect(app);
    
    renderLine(screen, playerPos, (Vector) {
            playerPos.x + (playerRadius + 5.0) * mouseDir.x,
            playerPos.y + (playerRadius + 5.0) * mouseDir.y,
        }, (Color) {0}, playerView.offset, playerView.scale);
    renderCircle(screen, playerPos, playerRadius, (Color) {0}, playerCol,
        playerView.offset, playerView.scale);
    
    renderParallax(screen, polygons, renderPolygons, polygonsSize, 
        &playerView,
        numSlices / 2, 1.0, avgBottomTopCol, 1.0 - ch2, topCol, 1,
        ceilCol);
    
    app->mouseMoving = 0;
}

void App_ApplyGlowEffect(App* app) {
    Raster* screen = app->screen;
    uint8_t* pixels = screen->pixels;
    int width = screen->width;
    int height = screen->height;
    
    double* blurBuffer1 = app->blurBuffer1;
    
    double glowIntensity = app->glowIntensity;
    Color glowColor = app->glowColor;
    int idx = 0;
    int blurIdx = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx0 = idx + 0;
            int idx1 = idx + 1;
            int idx2 = idx + 2;
            int rScreen = pixels[idx0];
            int gScreen = pixels[idx1];
            int bScreen = pixels[idx2];
            if (!(rScreen == 255 && gScreen == 255 && bScreen == 255)) {
                double glowValue = blurBuffer1[blurIdx];
                double glow = glowIntensity * glowValue;
                double glow_ = 1.0 - glow;
                if (glow_ < 0.0) glow_ = 0.0;
                int r = (int) (glow_ * rScreen + glow * glowColor.r);
                int g = (int) (glow_ * gScreen + glow * glowColor.g);
                int b = (int) (glow_ * bScreen + glow * glowColor.b);
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                pixels[idx0] = (uint8_t) r;
                pixels[idx1] = (uint8_t) g;
                pixels[idx2] = (uint8_t) b;
            }
            
            idx += 3;
            blurIdx++;
        }
    }
}

int App_Continue(App* app) {
    return !app->quit;
}

void App_GenTestLevel(App* app) {
    Polygon* leftBound = Polygon_Create();
    Polygon_PushPoint(leftBound, (Vector) { 0.0, -1000.0 });
    Polygon_PushPoint(leftBound, (Vector) { 0.0, 2000.0 });
    Polygon_PushPoint(leftBound, (Vector) { -1000.0, 2000.0 });
    Polygon_PushPoint(leftBound, (Vector) { -1000.0, -1000.0 });
    App_AddPolygon(app, leftBound);
    Polygon* rightBound = Polygon_Create();
    Polygon_PushPoint(rightBound, (Vector) { 2000.0, -1000.0 });
    Polygon_PushPoint(rightBound, (Vector) { 2000.0, 2000.0 });
    Polygon_PushPoint(rightBound, (Vector) { 1000.0, 2000.0 });
    Polygon_PushPoint(rightBound, (Vector) { 1000.0, -1000.0 });
    App_AddPolygon(app, rightBound);
    Polygon* topBound = Polygon_Create();
    Polygon_PushPoint(topBound, (Vector) { -1000.0, -1000.0 });
    Polygon_PushPoint(topBound, (Vector) { 2000.0, -1000.0 });
    Polygon_PushPoint(topBound, (Vector) { 2000.0, 0.0 });
    Polygon_PushPoint(topBound, (Vector) { -1000.0, 0.0 });
    App_AddPolygon(app, topBound);
    Polygon* bottomBound = Polygon_Create();
    Polygon_PushPoint(bottomBound, (Vector) { -1000.0, 1000.0 });
    Polygon_PushPoint(bottomBound, (Vector) { 2000.0, 1000.0 });
    Polygon_PushPoint(bottomBound, (Vector) { 2000.0, 2000.0 });
    Polygon_PushPoint(bottomBound, (Vector) { -1000.0, 2000.0 });
    App_AddPolygon(app, bottomBound);
    
    Polygon* poly1 = Polygon_Create();
    Polygon_PushPoint(poly1, (Vector) { 98.301545, 225.72413 });
    Polygon_PushPoint(poly1, (Vector) { 110.23251, 168.72318 });
    Polygon_PushPoint(poly1, (Vector) { 55.124594, 187.5533 });
    Polygon_PushPoint(poly1, (Vector) { 95.978605, 146.05142 });
    Polygon_PushPoint(poly1, (Vector) { 39.438544, 132.09874 });
    Polygon_PushPoint(poly1, (Vector) { 96.244735, 119.27248 });
    Polygon_PushPoint(poly1, (Vector) { 56.223594, 76.966868 });
    Polygon_PushPoint(poly1, (Vector) { 110.9464, 96.888478 });
    Polygon_PushPoint(poly1, (Vector) { 100.15061, 39.661668 });
    Polygon_PushPoint(poly1, (Vector) { 135.41593, 86.006178 });
    Polygon_PushPoint(poly1, (Vector) { 157.27308, 32.027288 });
    Polygon_PushPoint(poly1, (Vector) { 161.88442, 90.080638 });
    Polygon_PushPoint(poly1, (Vector) { 209.45503, 56.487578 });
    Polygon_PushPoint(poly1, (Vector) { 181.94832, 107.81824 });
    Polygon_PushPoint(poly1, (Vector) { 240.12904, 105.27657 });
    Polygon_PushPoint(poly1, (Vector) { 189.23748, 133.58742 });
    Polygon_PushPoint(poly1, (Vector) { 239.55635, 162.9041 });
    Polygon_PushPoint(poly1, (Vector) { 181.43763, 159.20665 });
    Polygon_PushPoint(poly1, (Vector) { 207.91877, 211.07385 });
    Polygon_PushPoint(poly1, (Vector) { 161.02518, 176.542 });
    Polygon_PushPoint(poly1, (Vector) { 155.26101, 234.49225 });
    Polygon_PushPoint(poly1, (Vector) { 134.48094, 180.08963 });
    App_AddPolygon(app, poly1);
    Polygon* poly2 = Polygon_Create();
    Polygon_PushPoint(poly2, (Vector) { 788.68568, 942.38177 });
    Polygon_PushPoint(poly2, (Vector) { 824.30641, 967.79494 });
    Polygon_PushPoint(poly2, (Vector) { 866.94057, 977.64093 });
    Polygon_PushPoint(poly2, (Vector) { 910.09677, 970.42232 });
    Polygon_PushPoint(poly2, (Vector) { 947.20622, 947.23863 });
    Polygon_PushPoint(poly2, (Vector) { 972.61831, 911.6175 });
    Polygon_PushPoint(poly2, (Vector) { 982.46538, 868.98374 });
    Polygon_PushPoint(poly2, (Vector) { 975.24752, 825.82684 });
    Polygon_PushPoint(poly2, (Vector) { 952.06238, 788.71735 });
    Polygon_PushPoint(poly2, (Vector) { 916.44271, 763.3053 });
    Polygon_PushPoint(poly2, (Vector) { 873.80819, 753.45894 });
    Polygon_PushPoint(poly2, (Vector) { 830.65127, 760.67751 });
    Polygon_PushPoint(poly2, (Vector) { 810.94635, 772.98795 });
    Polygon_PushPoint(poly2, (Vector) { 824.82537, 787.74418 });
    Polygon_PushPoint(poly2, (Vector) { 837.82668, 779.62191 });
    Polygon_PushPoint(poly2, (Vector) { 873.18789, 773.70764 });
    Polygon_PushPoint(poly2, (Vector) { 908.12042, 781.77495 });
    Polygon_PushPoint(poly2, (Vector) { 937.30577, 802.59672 });
    Polygon_PushPoint(poly2, (Vector) { 956.30274, 833.00261 });
    Polygon_PushPoint(poly2, (Vector) { 962.21668, 868.36344 });
    Polygon_PushPoint(poly2, (Vector) { 954.14829, 903.29557 });
    Polygon_PushPoint(poly2, (Vector) { 933.32684, 932.48202 });
    Polygon_PushPoint(poly2, (Vector) { 902.921, 951.47754 });
    Polygon_PushPoint(poly2, (Vector) { 867.56088, 957.39223 });
    Polygon_PushPoint(poly2, (Vector) { 832.62835, 949.32492 });
    Polygon_PushPoint(poly2, (Vector) { 803.4423, 928.50239 });
    Polygon_PushPoint(poly2, (Vector) { 784.44602, 898.09726 });
    Polygon_PushPoint(poly2, (Vector) { 778.53209, 862.73642 });
    Polygon_PushPoint(poly2, (Vector) { 786.59939, 827.80389 });
    Polygon_PushPoint(poly2, (Vector) { 795.71005, 815.03291 });
    Polygon_PushPoint(poly2, (Vector) { 781.83067, 800.2763 });
    Polygon_PushPoint(poly2, (Vector) { 768.12973, 819.48233 });
    Polygon_PushPoint(poly2, (Vector) { 758.28339, 862.11612 });
    Polygon_PushPoint(poly2, (Vector) { 765.50159, 905.27339 });
    App_AddPolygon(app, poly2);
    Polygon* poly3 = Polygon_Create();
    Polygon_PushPoint(poly3, (Vector) { 626.19516, 270.12339 });
    Polygon_PushPoint(poly3, (Vector) { 532.06123, 180.08227 });
    Polygon_PushPoint(poly3, (Vector) { 575.03541, 102.31947 });
    Polygon_PushPoint(poly3, (Vector) { 722.37546, 87.994743 });
    Polygon_PushPoint(poly3, (Vector) { 814.46298, 137.10809 });
    Polygon_PushPoint(poly3, (Vector) { 697.81879, 161.66476 });
    Polygon_PushPoint(poly3, (Vector) { 832.88049, 196.45338 });
    Polygon_PushPoint(poly3, (Vector) { 871.76188, 163.71115 });
    Polygon_PushPoint(poly3, (Vector) { 898.36494, 266.03063 });
    Polygon_PushPoint(poly3, (Vector) { 775.58158, 313.09757 });
    Polygon_PushPoint(poly3, (Vector) { 663.03017, 292.63368 });
    Polygon_PushPoint(poly3, (Vector) { 738.74658, 351.97899 });
    Polygon_PushPoint(poly3, (Vector) { 871.76188, 327.42229 });
    Polygon_PushPoint(poly3, (Vector) { 939.29273, 261.93784 });
    Polygon_PushPoint(poly3, (Vector) { 898.36494, 112.55142 });
    Polygon_PushPoint(poly3, (Vector) { 699.86516, 49.113344 });
    Polygon_PushPoint(poly3, (Vector) { 507.50456, 87.994743 });
    Polygon_PushPoint(poly3, (Vector) { 478.85511, 225.10283 });
    App_AddPolygon(app, poly3);
    Polygon* poly4 = Polygon_Create();
    Polygon_PushPoint(poly4, (Vector) { 32.959047, 548.6492 });
    Polygon_PushPoint(poly4, (Vector) { 100.15169, 508.64057 });
    Polygon_PushPoint(poly4, (Vector) { 264.79059, 778.43437 });
    Polygon_PushPoint(poly4, (Vector) { 192.99357, 815.37341 });
    App_AddPolygon(app, poly4);
    Polygon* poly5 = Polygon_Create();
    Polygon_PushPoint(poly5, (Vector) { 166.46262, 472.86284 });
    Polygon_PushPoint(poly5, (Vector) { 235.94461, 436.43274 });
    Polygon_PushPoint(poly5, (Vector) { 379.61861, 712.31431 });
    Polygon_PushPoint(poly5, (Vector) { 315.76419, 749.256 });
    App_AddPolygon(app, poly5);
    Polygon* poly6 = Polygon_Create();
    Polygon_PushPoint(poly6, (Vector) { 496.03773, 437.57105 });
    Polygon_PushPoint(poly6, (Vector) { 530.92836, 499.38091 });
    Polygon_PushPoint(poly6, (Vector) { 459.95414, 498.69215 });
    App_AddPolygon(app, poly6);
    Polygon* poly7 = Polygon_Create();
    Polygon_PushPoint(poly7, (Vector) { 584.40466, 438.4286 });
    Polygon_PushPoint(poly7, (Vector) { 619.29529, 500.23845 });
    Polygon_PushPoint(poly7, (Vector) { 548.32107, 499.54969 });
    App_AddPolygon(app, poly7);
    Polygon* poly8 = Polygon_Create();
    Polygon_PushPoint(poly8, (Vector) { 672.24206, 439.281 });
    Polygon_PushPoint(poly8, (Vector) { 707.13268, 501.09086 });
    Polygon_PushPoint(poly8, (Vector) { 636.15846, 500.4021 });
    App_AddPolygon(app, poly8);
    Polygon* poly9 = Polygon_Create();
    Polygon_PushPoint(poly9, (Vector) { 760.60898, 440.13855 });
    Polygon_PushPoint(poly9, (Vector) { 795.49961, 501.9484 });
    Polygon_PushPoint(poly9, (Vector) { 724.52539, 501.25964 });
    App_AddPolygon(app, poly9);
    Polygon* poly10 = Polygon_Create();
    Polygon_PushPoint(poly10, (Vector) { 717.3493, 480.89001 });
    Polygon_PushPoint(poly10, (Vector) { 682.45867, 419.08015 });
    Polygon_PushPoint(poly10, (Vector) { 753.43289, 419.76891 });
    App_AddPolygon(app, poly10);
    Polygon* poly11 = Polygon_Create();
    Polygon_PushPoint(poly11, (Vector) { 628.98237, 480.03246 });
    Polygon_PushPoint(poly11, (Vector) { 594.09174, 418.22261 });
    Polygon_PushPoint(poly11, (Vector) { 665.06596, 418.91137 });
    App_AddPolygon(app, poly11);
    Polygon* poly12 = Polygon_Create();
    Polygon_PushPoint(poly12, (Vector) { 541.14494, 479.18006 });
    Polygon_PushPoint(poly12, (Vector) { 506.25439, 417.3702 });
    Polygon_PushPoint(poly12, (Vector) { 577.22855, 418.05896 });
    App_AddPolygon(app, poly12);
    Polygon* poly13 = Polygon_Create();
    Polygon_PushPoint(poly13, (Vector) { 452.778, 478.32251 });
    Polygon_PushPoint(poly13, (Vector) { 417.88745, 416.51266 });
    Polygon_PushPoint(poly13, (Vector) { 488.86161, 417.20141 });
    App_AddPolygon(app, poly13);
    Polygon* poly14 = Polygon_Create();
    Polygon_PushPoint(poly14, (Vector) { 496.15734, 530.68732 });
    Polygon_PushPoint(poly14, (Vector) { 531.04796, 592.49717 });
    Polygon_PushPoint(poly14, (Vector) { 460.07374, 591.80841 });
    App_AddPolygon(app, poly14);
    Polygon* poly15 = Polygon_Create();
    Polygon_PushPoint(poly15, (Vector) { 584.52426, 531.54486 });
    Polygon_PushPoint(poly15, (Vector) { 619.41489, 593.35472 });
    Polygon_PushPoint(poly15, (Vector) { 548.44067, 592.66596 });
    App_AddPolygon(app, poly15);
    Polygon* poly16 = Polygon_Create();
    Polygon_PushPoint(poly16, (Vector) { 672.36166, 532.39727 });
    Polygon_PushPoint(poly16, (Vector) { 707.25228, 594.20712 });
    Polygon_PushPoint(poly16, (Vector) { 636.27807, 593.51836 });
    App_AddPolygon(app, poly16);
    Polygon* poly17 = Polygon_Create();
    Polygon_PushPoint(poly17, (Vector) { 760.72859, 533.25481 });
    Polygon_PushPoint(poly17, (Vector) { 795.61921, 595.06467 });
    Polygon_PushPoint(poly17, (Vector) { 724.645, 594.37591 });
    App_AddPolygon(app, poly17);
    Polygon* poly18 = Polygon_Create();
    Polygon_PushPoint(poly18, (Vector) { 717.4689, 574.00627 });
    Polygon_PushPoint(poly18, (Vector) { 682.57828, 512.19642 });
    Polygon_PushPoint(poly18, (Vector) { 753.5525, 512.88518 });
    App_AddPolygon(app, poly18);
    Polygon* poly19 = Polygon_Create();
    Polygon_PushPoint(poly19, (Vector) { 629.10197, 573.14873 });
    Polygon_PushPoint(poly19, (Vector) { 594.21135, 511.33887 });
    Polygon_PushPoint(poly19, (Vector) { 665.18557, 512.02763 });
    App_AddPolygon(app, poly19);
    Polygon* poly20 = Polygon_Create();
    Polygon_PushPoint(poly20, (Vector) { 541.26454, 572.29632 });
    Polygon_PushPoint(poly20, (Vector) { 506.37399, 510.48647 });
    Polygon_PushPoint(poly20, (Vector) { 577.34815, 511.17523 });
    App_AddPolygon(app, poly20);
    Polygon* poly21 = Polygon_Create();
    Polygon_PushPoint(poly21, (Vector) { 452.8976, 571.43878 });
    Polygon_PushPoint(poly21, (Vector) { 418.00705, 509.62892 });
    Polygon_PushPoint(poly21, (Vector) { 488.98121, 510.31768 });
    App_AddPolygon(app, poly21);
    Polygon* poly22 = Polygon_Create();
    Polygon_PushPoint(poly22, (Vector) { 495.79377, 624.83171 });
    Polygon_PushPoint(poly22, (Vector) { 530.68439, 686.64157 });
    Polygon_PushPoint(poly22, (Vector) { 459.71018, 685.95281 });
    App_AddPolygon(app, poly22);
    Polygon* poly23 = Polygon_Create();
    Polygon_PushPoint(poly23, (Vector) { 584.1607, 625.68926 });
    Polygon_PushPoint(poly23, (Vector) { 619.05132, 687.49911 });
    Polygon_PushPoint(poly23, (Vector) { 548.07711, 686.81035 });
    App_AddPolygon(app, poly23);
    Polygon* poly24 = Polygon_Create();
    Polygon_PushPoint(poly24, (Vector) { 671.99809, 626.54166 });
    Polygon_PushPoint(poly24, (Vector) { 706.88872, 688.35152 });
    Polygon_PushPoint(poly24, (Vector) { 635.9145, 687.66276 });
    App_AddPolygon(app, poly24);
    Polygon* poly25 = Polygon_Create();
    Polygon_PushPoint(poly25, (Vector) { 760.36502, 627.39921 });
    Polygon_PushPoint(poly25, (Vector) { 795.25567, 689.20906 });
    Polygon_PushPoint(poly25, (Vector) { 724.28143, 688.5203 });
    App_AddPolygon(app, poly25);
    Polygon* poly26 = Polygon_Create();
    Polygon_PushPoint(poly26, (Vector) { 717.10534, 668.15067 });
    Polygon_PushPoint(poly26, (Vector) { 682.21471, 606.34081 });
    Polygon_PushPoint(poly26, (Vector) { 753.18895, 607.02957 });
    App_AddPolygon(app, poly26);
    Polygon* poly27 = Polygon_Create();
    Polygon_PushPoint(poly27, (Vector) { 628.73841, 667.29312 });
    Polygon_PushPoint(poly27, (Vector) { 593.84778, 605.48327 });
    Polygon_PushPoint(poly27, (Vector) { 664.822, 606.17203 });
    App_AddPolygon(app, poly27);
    Polygon* poly28 = Polygon_Create();
    Polygon_PushPoint(poly28, (Vector) { 540.90097, 666.44072 });
    Polygon_PushPoint(poly28, (Vector) { 506.01043, 604.63086 });
    Polygon_PushPoint(poly28, (Vector) { 576.98459, 605.31962 });
    App_AddPolygon(app, poly28);
    Polygon* poly29 = Polygon_Create();
    Polygon_PushPoint(poly29, (Vector) { 452.53403, 665.58317 });
    Polygon_PushPoint(poly29, (Vector) { 417.64349, 603.77332 });
    Polygon_PushPoint(poly29, (Vector) { 488.61765, 604.46207 });
    App_AddPolygon(app, poly29);
    Polygon* poly30 = Polygon_Create();
    Polygon_PushPoint(poly30, (Vector) { 422.52864, 920.30208 });
    Polygon_PushPoint(poly30, (Vector) { 447.85141, 897.14982 });
    Polygon_PushPoint(poly30, (Vector) { 461.59805, 914.514 });
    Polygon_PushPoint(poly30, (Vector) { 478.23872, 910.17295 });
    Polygon_PushPoint(poly30, (Vector) { 504.28502, 916.68453 });
    Polygon_PushPoint(poly30, (Vector) { 506.45556, 902.93787 });
    Polygon_PushPoint(poly30, (Vector) { 528.16078, 917.40804 });
    Polygon_PushPoint(poly30, (Vector) { 542.63095, 902.93787 });
    Polygon_PushPoint(poly30, (Vector) { 565.05968, 913.06699 });
    Polygon_PushPoint(poly30, (Vector) { 591.82948, 902.93787 });
    Polygon_PushPoint(poly30, (Vector) { 629.45188, 925.36662 });
    Polygon_PushPoint(poly30, (Vector) { 641.02801, 905.1084 });
    Polygon_PushPoint(poly30, (Vector) { 669.24482, 921.02558 });
    Polygon_PushPoint(poly30, (Vector) { 685.16199, 905.83191 });
    Polygon_PushPoint(poly30, (Vector) { 710.48478, 921.02558 });
    Polygon_PushPoint(poly30, (Vector) { 711.20829, 910.17295 });
    Polygon_PushPoint(poly30, (Vector) { 727.12546, 919.57855 });
    Polygon_PushPoint(poly30, (Vector) { 735.13544, 882.00757 });
    Polygon_PushPoint(poly30, (Vector) { 421.80511, 876.8916 });
    App_AddPolygon(app, poly30);
    Polygon* poly31 = Polygon_Create();
    Polygon_PushPoint(poly31, (Vector) { 727.20235, 952.60585 });
    Polygon_PushPoint(poly31, (Vector) { 701.87958, 975.75811 });
    Polygon_PushPoint(poly31, (Vector) { 688.13294, 958.39393 });
    Polygon_PushPoint(poly31, (Vector) { 671.49227, 962.73498 });
    Polygon_PushPoint(poly31, (Vector) { 645.44597, 956.2234 });
    Polygon_PushPoint(poly31, (Vector) { 643.27543, 969.97006 });
    Polygon_PushPoint(poly31, (Vector) { 621.57021, 955.49989 });
    Polygon_PushPoint(poly31, (Vector) { 607.10004, 969.97006 });
    Polygon_PushPoint(poly31, (Vector) { 584.67131, 959.84094 });
    Polygon_PushPoint(poly31, (Vector) { 557.90151, 969.97006 });
    Polygon_PushPoint(poly31, (Vector) { 520.27911, 947.54131 });
    Polygon_PushPoint(poly31, (Vector) { 508.70298, 967.79953 });
    Polygon_PushPoint(poly31, (Vector) { 480.48617, 951.88235 });
    Polygon_PushPoint(poly31, (Vector) { 464.569, 967.07602 });
    Polygon_PushPoint(poly31, (Vector) { 439.24621, 951.88235 });
    Polygon_PushPoint(poly31, (Vector) { 438.5227, 962.73498 });
    Polygon_PushPoint(poly31, (Vector) { 422.60553, 953.32938 });
    Polygon_PushPoint(poly31, (Vector) { 419.71152, 994.99314 });
    Polygon_PushPoint(poly31, (Vector) { 727.92588, 996.01633 });
    App_AddPolygon(app, poly31);
    Polygon* poly32 = Polygon_Create();
    Polygon_PushPoint(poly32, (Vector) { 163.95035, 920.68216 });
    Polygon_PushPoint(poly32, (Vector) { 189.27312, 897.5299 });
    Polygon_PushPoint(poly32, (Vector) { 203.01976, 914.89408 });
    Polygon_PushPoint(poly32, (Vector) { 219.66043, 910.55303 });
    Polygon_PushPoint(poly32, (Vector) { 245.70673, 917.06461 });
    Polygon_PushPoint(poly32, (Vector) { 247.87727, 903.31795 });
    Polygon_PushPoint(poly32, (Vector) { 269.58249, 917.78812 });
    Polygon_PushPoint(poly32, (Vector) { 284.05266, 903.31795 });
    Polygon_PushPoint(poly32, (Vector) { 306.48139, 913.44707 });
    Polygon_PushPoint(poly32, (Vector) { 333.25119, 903.31795 });
    Polygon_PushPoint(poly32, (Vector) { 370.87359, 925.7467 });
    Polygon_PushPoint(poly32, (Vector) { 382.44972, 905.48848 });
    Polygon_PushPoint(poly32, (Vector) { 410.66653, 921.40566 });
    Polygon_PushPoint(poly32, (Vector) { 426.5837, 906.21199 });
    Polygon_PushPoint(poly32, (Vector) { 451.90649, 921.40566 });
    Polygon_PushPoint(poly32, (Vector) { 452.63, 910.55303 });
    Polygon_PushPoint(poly32, (Vector) { 468.54717, 919.95863 });
    Polygon_PushPoint(poly32, (Vector) { 471.44118, 877.27168 });
    Polygon_PushPoint(poly32, (Vector) { 162.20363, 879.31807 });
    App_AddPolygon(app, poly32);
    Polygon* poly33 = Polygon_Create();
    Polygon_PushPoint(poly33, (Vector) { 468.62406, 952.98593 });
    Polygon_PushPoint(poly33, (Vector) { 443.30129, 976.13819 });
    Polygon_PushPoint(poly33, (Vector) { 429.55465, 958.77401 });
    Polygon_PushPoint(poly33, (Vector) { 412.91398, 963.11506 });
    Polygon_PushPoint(poly33, (Vector) { 386.86768, 956.60348 });
    Polygon_PushPoint(poly33, (Vector) { 384.69714, 970.35014 });
    Polygon_PushPoint(poly33, (Vector) { 362.99192, 955.87997 });
    Polygon_PushPoint(poly33, (Vector) { 348.52175, 970.35014 });
    Polygon_PushPoint(poly33, (Vector) { 326.09302, 960.22102 });
    Polygon_PushPoint(poly33, (Vector) { 299.32322, 970.35014 });
    Polygon_PushPoint(poly33, (Vector) { 261.70082, 947.92139 });
    Polygon_PushPoint(poly33, (Vector) { 250.12469, 968.17961 });
    Polygon_PushPoint(poly33, (Vector) { 221.90788, 952.26243 });
    Polygon_PushPoint(poly33, (Vector) { 205.99071, 967.4561 });
    Polygon_PushPoint(poly33, (Vector) { 180.66792, 952.26243 });
    Polygon_PushPoint(poly33, (Vector) { 179.94441, 963.11506 });
    Polygon_PushPoint(poly33, (Vector) { 164.02724, 953.70946 });
    Polygon_PushPoint(poly33, (Vector) { 161.64483, 994.86165 });
    Polygon_PushPoint(poly33, (Vector) { 469.34759, 996.39644 });
    App_AddPolygon(app, poly33);
    Polygon* poly34 = Polygon_Create();
    Polygon_PushPoint(poly34, (Vector) { 313.10353, 354.3702 });
    Polygon_PushPoint(poly34, (Vector) { 270.67713, 321.06763 });
    Polygon_PushPoint(poly34, (Vector) { 278.30479, 267.67401 });
    Polygon_PushPoint(poly34, (Vector) { 328.35885, 247.58296 });
    Polygon_PushPoint(poly34, (Vector) { 370.78525, 280.88551 });
    Polygon_PushPoint(poly34, (Vector) { 363.15758, 334.27913 });
    App_AddPolygon(app, poly34);
}