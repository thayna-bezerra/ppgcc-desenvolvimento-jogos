#include "TileMap.hpp"
#include "Dungeon2D.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>

namespace {

void carveRect(std::vector<int>& cells, int width, int height, const Rect& rect) {
    const int rowStart = std::max(0, rect.top);
    const int rowEnd = std::min(height, rect.bottom);
    const int colStart = std::max(0, rect.left);
    const int colEnd = std::min(width, rect.right);

    for (int row = rowStart; row < rowEnd; ++row) {
        for (int col = colStart; col < colEnd; ++col) {
            cells[row * width + col] = EMPTY_TILE;
        }
    }
}

void carveLeaves(const BSPNode& node, std::vector<int>& cells, int width, int height) {
    if (node.isLeaf()) {
        carveRect(cells, width, height, node.room);
        return;
    }
    carveLeaves(*node.left, cells, width, height);
    carveLeaves(*node.right, cells, width, height);
}

} 

void TileMap::loadFromDungeon(const Dungeon2D& dungeon) {
    m_width = dungeon.width;
    m_height = dungeon.height;
    m_cells.assign(static_cast<std::size_t>(m_width) * m_height, WALL_TILE);

    if (dungeon.root) {
        carveLeaves(*dungeon.root, m_cells, m_width, m_height);
    }

    for (const auto& corridor : dungeon.corridors) {
        carveRect(m_cells, m_width, m_height, corridor);
    }
}

bool TileMap::saveCSV(const std::string& path) const {
    std::ofstream file(path);
    if (!file) {
        return false;
    }

    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) {
            if (col > 0) {
                file << ',';
            }
            file << m_cells[row * m_width + col];
        }
        file << '\n';
    }

    return static_cast<bool>(file);
}

bool TileMap::loadCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }

    std::vector<int> cells;
    int width = -1;
    int height = 0;
    std::string line;

    try {
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            std::stringstream rowStream(line);
            std::string token;
            int columns = 0;
            while (std::getline(rowStream, token, ',')) {
                cells.push_back(std::stoi(token));
                ++columns;
            }

            if (width == -1) {
                width = columns;
            } else if (columns != width) {
                return false;
            }
            ++height;
        }
    } catch (const std::exception&) {
        return false;
    }

    if (width <= 0 || height <= 0) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_cells = std::move(cells);
    return true;
}

int TileMap::width() const noexcept {
    return m_width;
}

int TileMap::height() const noexcept {
    return m_height;
}

int TileMap::at(int col, int row) const {
    assert(col >= 0 && col < m_width && row >= 0 && row < m_height);
    return m_cells[row * m_width + col];
}

bool TileMap::isWall(int col, int row) const {
    return at(col, row) == WALL_TILE;
}

const int* TileMap::data() const noexcept {
    return m_cells.data();
}

void TileMap::toIndices(const Vector2D& world, int& col, int& row) const noexcept {
    col = static_cast<int>(std::floor(world.x / TILE_SIZE));
    row = static_cast<int>(std::floor(world.y / TILE_SIZE));
}

Vector2D TileMap::toWorld(int col, int row) const noexcept {
    return Vector2D{ (col + 0.5f) * TILE_SIZE, (row + 0.5f) * TILE_SIZE };
}

void TileMap::randomEmptyCell(int& col, int& row) const {
    std::vector<int> emptyIndices;
    for (std::size_t i = 0; i < m_cells.size(); ++i) {
        if (m_cells[i] == EMPTY_TILE) {
            emptyIndices.push_back(static_cast<int>(i));
        }
    }
    assert(!emptyIndices.empty());

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, emptyIndices.size() - 1);
    const int index = emptyIndices[dist(rng)];

    col = index % m_width;
    row = index / m_width;
}
