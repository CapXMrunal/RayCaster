#include "game/Renderer.hpp"

#include <algorithm>

namespace game {

Renderer::Renderer(SDL_Renderer* renderer, int windowWidth, int windowHeight)
    : renderer_(renderer), windowWidth_(windowWidth), windowHeight_(windowHeight) {}

void Renderer::drawMinimap(const core::Map& map) {
    const int cellSize = std::min(windowWidth_ / map.width(), windowHeight_ / map.height());

    SDL_SetRenderDrawColor(renderer_, 30, 30, 40, 255);
    SDL_RenderClear(renderer_);

    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            SDL_Rect cellRect{x * cellSize, y * cellSize, cellSize, cellSize};
            if (map.isWall(x, y)) {
                SDL_SetRenderDrawColor(renderer_, 90, 90, 100, 255);
            } else {
                SDL_SetRenderDrawColor(renderer_, 200, 200, 210, 255);
            }
            SDL_RenderFillRect(renderer_, &cellRect);
        }
    }

    SDL_RenderPresent(renderer_);
}

} // namespace game
