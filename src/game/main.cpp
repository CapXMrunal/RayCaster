#include <SDL.h>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr int kFramesToRun = 60;

// Clears the screen to a solid color so we have something to look at before
// any real rendering exists. This will be replaced in later iterations.
void renderFrame(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

// Dumps the current back buffer to a BMP file so we can inspect a headless
// run's output. SDL_SaveBMP needs no extra dependency beyond SDL2 itself.
bool saveScreenshot(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, kWindowWidth, kWindowHeight, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        std::fprintf(stderr, "SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
        return false;
    }
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32,
                              surface->pixels, surface->pitch) != 0) {
        std::fprintf(stderr, "SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    bool ok = SDL_SaveBMP(surface, path) == 0;
    if (!ok) {
        std::fprintf(stderr, "SDL_SaveBMP failed: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return ok;
}

} // namespace

int main(int argc, char** argv) {
    const char* screenshotPath = argc > 1 ? argv[1] : nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "RayCaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWindowWidth, kWindowHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    for (int frame = 0; frame < kFramesToRun && !quit; ++frame) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        renderFrame(renderer);
    }

    if (screenshotPath) {
        saveScreenshot(renderer, screenshotPath);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
