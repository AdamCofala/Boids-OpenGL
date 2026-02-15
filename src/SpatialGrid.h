#include <unordered_map>
#include <vector>
#include <functional>
#include "Boid.h"

// Hash function for std::pair
struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

class SpatialGrid {
    /*
    A spatial grid data structure for efficient collision detection and spatial queries.
    This class divides 2D space into a grid of cells and allows fast retrieval of objects
    in nearby cells, which is useful for broad-phase collision detection.
    */
    float cell_size;
    std::unordered_map<std::pair<int, int>, std::vector<int>, PairHash> grid;

public:
    SpatialGrid(float cell_size) : cell_size(cell_size) {}

    std::pair<int, int> getCell(float x, float y) const {
        // Calculate the grid cell coordinates for a given position.
        return {static_cast<int>(x / cell_size),
                static_cast<int>(y / cell_size) };
    }

    void clear() {
        // Clear the spatial grid.
        grid.clear();
    }

    void insert(const Boid& boid, int id) {
        // Insert a body into the appropriate cell in the grid.
        auto cell = getCell(boid.pos.x, boid.pos.y);
        grid[cell].push_back(id);
    }

    void get_nearby(const Boid& boid, std::vector<int>& nearby) const {
        // Retrieve bodies in the same and neighboring cells for potential collision checks.
        auto cell = getCell(boid.pos.x, boid.pos.y);
        nearby.clear();

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                auto neighbor = std::make_pair(cell.first + dx, cell.second + dy);
                auto it = grid.find(neighbor);
                if (it != grid.end()) {
                    nearby.insert(nearby.end(), it->second.begin(), it->second.end());
                }
            }
        }
    }
};
