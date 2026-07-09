#include "core/Map.hpp"

#include <cassert>
#include <iostream>

int main() {
    core::Map map("maps/demo1.txt");

    assert(map.width() == 10);
    assert(map.height() == 8);

    assert(map.isWall(0, 0));   // top-left corner, part of the border
    assert(map.isWall(3, 3));   // inside the interior obstacle
    assert(!map.isWall(1, 1));  // just inside the border, open floor
    assert(!map.isWall(5, 5));  // open floor near the bottom

    assert(map.isWall(-1, 0));   // off the left edge
    assert(map.isWall(0, -1));   // off the top edge
    assert(map.isWall(100, 0));  // off the right edge
    assert(map.isWall(0, 100));  // off the bottom edge

    std::cout << "test_map: all assertions passed\n";
    return 0;
}
