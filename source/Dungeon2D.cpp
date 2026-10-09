#include "Dungeon2D.hpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace {

constexpr int MIN_ROOM = 5;
constexpr int CORRIDOR_HALF = 1;

int regionWidth(const Rect& r) { return r.right - r.left; }
int regionHeight(const Rect& r) { return r.bottom - r.top; }

bool canSplitX(const Rect& r) { return regionWidth(r) >= MIN_ROOM * 2; }
bool canSplitY(const Rect& r) { return regionHeight(r) >= MIN_ROOM * 2; }

Rect carveRoom(const Rect& region, std::mt19937& rng) {
    std::uniform_int_distribution<int> wDist(MIN_ROOM, regionWidth(region));
    std::uniform_int_distribution<int> hDist(MIN_ROOM, regionHeight(region));
    int roomW = wDist(rng);
    int roomH = hDist(rng);

    std::uniform_int_distribution<int> xDist(region.left, region.right - roomW);
    std::uniform_int_distribution<int> yDist(region.top, region.bottom - roomH);
    int left = xDist(rng);
    int top = yDist(rng);

    return Rect{ left, top, left + roomW, top + roomH };
}

std::unique_ptr<BSPNode> buildTree(const Rect& region, std::mt19937& rng) {
    auto node = std::make_unique<BSPNode>();
    node->region = region;

    const bool splitX = canSplitX(region);
    const bool splitY = canSplitY(region);

    if (!splitX && !splitY) {
        node->room = carveRoom(region, rng);
        return node;
    }

    const bool cutAlongX = splitX && (!splitY || regionWidth(region) >= regionHeight(region));

    if (cutAlongX) {
        std::uniform_int_distribution<int> cutDist(region.left + MIN_ROOM, region.right - MIN_ROOM);
        const int cut = cutDist(rng);
        node->left = buildTree(Rect{ region.left, region.top, cut, region.bottom }, rng);
        node->right = buildTree(Rect{ cut, region.top, region.right, region.bottom }, rng);
    } else {
        std::uniform_int_distribution<int> cutDist(region.top + MIN_ROOM, region.bottom - MIN_ROOM);
        const int cut = cutDist(rng);
        node->left = buildTree(Rect{ region.left, region.top, region.right, cut }, rng);
        node->right = buildTree(Rect{ region.left, cut, region.right, region.bottom }, rng);
    }

    return node;
}

void representativeRoomCentre(const BSPNode& node, int& x, int& y) {
    if (node.isLeaf()) {
        x = (node.room.left + node.room.right) / 2;
        y = (node.room.top + node.room.bottom) / 2;
        return;
    }
    representativeRoomCentre(*node.left, x, y);
}

void connect(int ax, int ay, int bx, int by, std::vector<Corridor>& corridors) {
    const int x1 = std::min(ax, bx);
    const int x2 = std::max(ax, bx);
    const int y1 = std::min(ay, by);
    const int y2 = std::max(ay, by);

    corridors.push_back(Corridor{ x1, ay - CORRIDOR_HALF, x2, ay + CORRIDOR_HALF });
    corridors.push_back(Corridor{ bx - CORRIDOR_HALF, y1, bx + CORRIDOR_HALF, y2 });
}

void connectSubtrees(const BSPNode& node, std::vector<Corridor>& corridors) {
    if (node.isLeaf()) {
        return;
    }

    int ax, ay, bx, by;
    representativeRoomCentre(*node.left, ax, ay);
    representativeRoomCentre(*node.right, bx, by);
    connect(ax, ay, bx, by, corridors);

    connectSubtrees(*node.left, corridors);
    connectSubtrees(*node.right, corridors);
}

}

Dungeon2D::Dungeon2D(int widthTiles, int heightTiles)
    : width(widthTiles), height(heightTiles) {
    assert(widthTiles > 0 && heightTiles > 0);
}

void Dungeon2D::generate(unsigned seed) {
    assert(width >= MIN_ROOM && height >= MIN_ROOM);

    std::mt19937 rng(seed);
    root = buildTree(Rect{ 0, 0, width, height }, rng);

    corridors.clear();
    connectSubtrees(*root, corridors);
}
