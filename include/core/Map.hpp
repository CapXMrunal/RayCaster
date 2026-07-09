#pragma once

#include <string>
#include <vector>

namespace core {

// A grid loaded from a text file: '#' is a wall, anything else is floor.
// Coordinates outside the grid always read as walls, so raycasting and
// collision code never has to special-case going off the edge of the map.
class Map {
public:
    explicit Map(const std::string& path);

    int width() const { return width_; }
    int height() const { return height_; }
    bool isWall(int x, int y) const;

private:
    std::vector<std::string> rows_;
    int width_ = 0;
    int height_ = 0;
};

} // namespace core
