#include "core/Map.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace core {

Map::Map(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Map: failed to open " << path << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        rows_.push_back(line);
    }

    height_ = static_cast<int>(rows_.size());
    for (const auto& row : rows_) {
        width_ = std::max(width_, static_cast<int>(row.size()));
    }
}

bool Map::isWall(int x, int y) const {
    if (y < 0 || y >= height_ || x < 0) {
        return true;
    }
    const std::string& row = rows_[y];
    if (x >= static_cast<int>(row.size())) {
        return true;
    }
    return row[x] == '#';
}

} // namespace core
