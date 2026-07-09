#include <SDL.h>
#include <iostream>

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

} // namespace

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "RayCaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWindowWidth, kWindowHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
