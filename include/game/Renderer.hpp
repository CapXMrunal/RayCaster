#pragma once

#include <SDL.h>

#include "core/Map.hpp"

namespace game {

class Renderer {
public:
    Renderer(SDL_Renderer* renderer, int windowWidth, int windowHeight);

    void drawMinimap(const core::Map& map);

private:
    SDL_Renderer* renderer_;
    int windowWidth_;
    int windowHeight_;
};

} // namespace game
