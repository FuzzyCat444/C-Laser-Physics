#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "App.h"
#include "Vector.h"

Raster* loadBitmap(const char* file);

int main(int argc, char* argv[]) {
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("%s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    const int appWidth = 500;
    const int appHeight = 400;
    const int scale = 1;
    const int width = scale * appWidth;
    const int height = scale * appHeight;
    SDL_Window* win = SDL_CreateWindow(
        "FuzzyCat - Laser Game", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        width, height, 
        SDL_WINDOW_SHOWN
    );
    
    if (win == NULL) {
        printf("%s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    SDL_Surface* winSurface = SDL_GetWindowSurface(win);
    
    App* app = App_Create(appWidth, appHeight, loadBitmap);
    SDL_Surface* screenSurface = SDL_CreateRGBSurfaceWithFormat(
        0, width, height, 0, SDL_PIXELFORMAT_RGB24
    );
    
    int running = 1;
    Uint32 lastRender = SDL_GetTicks();
    Uint32 lastSecond = SDL_GetTicks();
    int frames = 0;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN ||
                       event.type == SDL_KEYUP) {
                AppEventType type = 
                    event.type == SDL_KEYDOWN ? 
                    APPEVENT_KEYPRESSED : 
                    APPEVENT_KEYRELEASED;
                AppEvent appEvent;
                int handled = 1;
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    appEvent.key = APPEVENT_KEY_ESCAPE;
                } else if (event.key.keysym.sym == SDLK_a) {
                    appEvent.key = APPEVENT_KEY_LEFT;
                } else if (event.key.keysym.sym == SDLK_d) {
                    appEvent.key = APPEVENT_KEY_RIGHT;
                } else if (event.key.keysym.sym == SDLK_s) {
                    appEvent.key = APPEVENT_KEY_DOWN;
                } else if (event.key.keysym.sym == SDLK_w) {
                    appEvent.key = APPEVENT_KEY_UP;
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    appEvent.key = APPEVENT_KEY_SPACE;
                } else {
                    handled = 0;
                }
                if (handled)
                    App_HandleEvent(app, type, appEvent);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                AppEventType type = APPEVENT_MOUSEPRESSED;
                AppEvent appEvent;
                appEvent.mouse.x = event.button.x;
                appEvent.mouse.y = event.button.y;
                App_HandleEvent(app, type, appEvent);
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                AppEventType type = APPEVENT_MOUSERELEASED;
                AppEvent appEvent;
                appEvent.mouse.x = event.button.x;
                appEvent.mouse.y = event.button.y;
                App_HandleEvent(app, type, appEvent);
            } else if (event.type == SDL_MOUSEMOTION) {
                AppEventType type = APPEVENT_MOUSEMOVED;
                AppEvent appEvent;
                appEvent.mouse.x = event.motion.x;
                appEvent.mouse.y = event.motion.y;
                App_HandleEvent(app, type, appEvent);
            }
        }
        Uint32 time = SDL_GetTicks();
        Uint32 timeDiff = time - lastRender;
        if (timeDiff > 0) {
            
            App_Render(app, timeDiff / 1000.0);
            
            // Lock the surface so we can copy the App's raster to the screen.
            SDL_LockSurface(screenSurface);
            uint8_t* screenPixels = (uint8_t*) screenSurface->pixels;
            uint8_t* appPixels = app->screen->pixels;
            int pitch = screenSurface->pitch;
            int screenI = 0;
            int appI = 0;
            int appPitch = 3 * appWidth;
            int ay = 0;
            for (int y = 0; y < height; y++) {
                int screenI_ = screenI;
                int ax = 0;
                for (int x = 0; x < width; x++) {
                    for (int i = 0; i < 3; i++)
                        screenPixels[screenI_ + i] = appPixels[appI + i];
                    ax++;
                    if (ax == scale) {
                        ax = 0;
                        appI += 3;
                    }
                    screenI_ += 3;
                }
                ay++;
                if (ay == scale) {
                    ay = 0;
                } else {
                    appI -= appPitch;
                }
                screenI += pitch;
            }
            SDL_UnlockSurface(screenSurface);
            
            SDL_BlitSurface(screenSurface, NULL, winSurface, NULL);
            
            SDL_UpdateWindowSurface(win);
            
            lastRender = time;
            frames++;
        }
        if (time - lastSecond >= 1000) {
            printf("FPS: %d\n", frames);
            
            lastSecond = time;
            frames = 0;
        }
        
        if (!App_Continue(app)) {
            running = 0;
        }
    }
    
    App_Destroy(app);

    SDL_FreeSurface(screenSurface);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}

Raster* loadBitmap(const char* file) {
    SDL_Surface* loaded = SDL_LoadBMP(file);
    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);
    SDL_Surface* converted = SDL_ConvertSurface(
        loaded, format, 0
    );
    Raster* raster = Raster_Create(converted->w, converted->h);
    uint8_t* cpixels = (uint8_t*) converted->pixels;
    uint8_t* rasterPixels = raster->pixels;
    int pitch = converted->pitch;
    int pi = 0;
    int ci = 0;
    int width = converted->w;
    int height = converted->h;
    for (int y = 0; y < height; y++) {
        int ci_ = ci;
        for (int x = 0; x < width; x++) {
            rasterPixels[pi] = cpixels[ci_];
            rasterPixels[pi + 1] = cpixels[ci_ + 1];
            rasterPixels[pi + 2] = cpixels[ci_ + 2];
            pi += 3;
            ci_ += 3;
        }
        ci += pitch;
    }
    
    SDL_FreeSurface(converted);
    SDL_FreeFormat(format);
    SDL_FreeSurface(loaded);
    return raster;
}
