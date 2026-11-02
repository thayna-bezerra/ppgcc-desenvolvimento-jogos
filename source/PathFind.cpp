#include "PathFind.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <queue>

namespace {

constexpr double ORTHOGONAL_COST = 1.0;
constexpr double DIAGONAL_COST = 1.4142135623730951;

struct OpenCell {
    double f;
    double g;
    int index;
};

struct LowerPriority {
    bool operator()(const OpenCell& a, const OpenCell& b) const {
        return a.f > b.f || (a.f == b.f && a.g < b.g);
    }
};

double heuristic(int col, int row, int goalCol, int goalRow) {
    const double dx = std::abs(col - goalCol);
    const double dy = std::abs(row - goalRow);
    return (dx + dy) - (2.0 * ORTHOGONAL_COST - DIAGONAL_COST) * std::min(dx, dy);
}

bool toCell(const Vector2D& world, int width, int height, int& col, int& row) {
    const float fx = std::floor(world.x / TILE_SIZE);
    const float fy = std::floor(world.y / TILE_SIZE);
    if (!(fx >= 0.0f && fx < static_cast<float>(width) && fy >= 0.0f && fy < static_cast<float>(height))) {
        return false;
    }
    col = static_cast<int>(fx);
    row = static_cast<int>(fy);
    return true;
}

Vector2D cellCentre(int col, int row) {
    return Vector2D{ (col + 0.5f) * TILE_SIZE, (row + 0.5f) * TILE_SIZE };
}

}

PathFind::PathFind(const int* grid, int width, int height)
    : m_grid(grid), m_width(width), m_height(height) {}

std::vector<Vector2D> PathFind::findPath(const Vector2D& startWorld, const Vector2D& goalWorld) const {
    if (!m_grid || m_width <= 0 || m_height <= 0) {
        return {};
    }

    int startCol = 0, startRow = 0, goalCol = 0, goalRow = 0;
    if (!toCell(startWorld, m_width, m_height, startCol, startRow) ||
        !toCell(goalWorld, m_width, m_height, goalCol, goalRow)) {
        return {};
    }

    const auto walkable = [this](int col, int row) {
        return m_grid[row * m_width + col] == EMPTY_TILE;
    };
    if (!walkable(startCol, startRow) || !walkable(goalCol, goalRow)) {
        return {};
    }

    const int cellCount = m_width * m_height;
    const int startIndex = startRow * m_width + startCol;
    const int goalIndex = goalRow * m_width + goalCol;

    std::vector<double> cost(cellCount, std::numeric_limits<double>::infinity());
    std::vector<int> parent(cellCount, -1);
    std::priority_queue<OpenCell, std::vector<OpenCell>, LowerPriority> open;

    cost[startIndex] = 0.0;
    open.push(OpenCell{ heuristic(startCol, startRow, goalCol, goalRow), 0.0, startIndex });

    while (!open.empty()) {
        const OpenCell current = open.top();
        open.pop();

        if (current.g > cost[current.index]) {
            continue;
        }
        if (current.index == goalIndex) {
            break;
        }

        const int col = current.index % m_width;
        const int row = current.index / m_width;

        for (int dRow = -1; dRow <= 1; ++dRow) {
            for (int dCol = -1; dCol <= 1; ++dCol) {
                if (dCol == 0 && dRow == 0) {
                    continue;
                }

                const int nextCol = col + dCol;
                const int nextRow = row + dRow;
                if (nextCol < 0 || nextCol >= m_width || nextRow < 0 || nextRow >= m_height) {
                    continue;
                }
                if (!walkable(nextCol, nextRow)) {
                    continue;
                }

                const bool diagonal = (dCol != 0 && dRow != 0);
                if (diagonal && !(walkable(col + dCol, row) && walkable(col, row + dRow))) {
                    continue;
                }

                const double nextCost = current.g + (diagonal ? DIAGONAL_COST : ORTHOGONAL_COST);
                const int nextIndex = nextRow * m_width + nextCol;
                if (nextCost < cost[nextIndex]) {
                    cost[nextIndex] = nextCost;
                    parent[nextIndex] = current.index;
                    open.push(OpenCell{ nextCost + heuristic(nextCol, nextRow, goalCol, goalRow), nextCost, nextIndex });
                }
            }
        }
    }

    if (cost[goalIndex] == std::numeric_limits<double>::infinity()) {
        return {};
    }

    std::vector<Vector2D> path;
    for (int index = goalIndex; index != -1; index = parent[index]) {
        path.push_back(cellCentre(index % m_width, index / m_width));
    }
    std::reverse(path.begin(), path.end());
    return path;
}
