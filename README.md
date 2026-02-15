# Boids Simulation

Real-time 2D flocking simulation implementing Craig Reynolds' Boids algorithm with spatial grid optimization. Built with C++17, OpenGL 4.6, GLFW, GLM, and ImGui.

**Key Features:**
- Spatial grid partitioning for O(n) neighbor queries
- GPU instanced rendering for thousands of boids
- Real-time parameter tuning via ImGui
- Mouse interaction (attract/repel)
- Dynamic coloring modes (speed-based & friend visualization)
- Configurable edge behaviors (bounce/wrap)

| ![image](placeholder1.png) | ![image](placeholder2.png) | ![image](placeholder3.png) |
|---|---|---|
| Velocity-based color mapping | Friend influence visualization | High-density flocking behavior |

---

## 1. Architecture

### 1.1. `Boid` Class (`Boid.h`)
Represents individual boid agents with position, velocity, and behavior rules. Implements Reynolds' three core behaviors:
- **Separation**: Avoid crowding neighbors
- **Alignment**: Steer toward average heading of neighbors
- **Cohesion**: Move toward average position of neighbors

### 1.2. `SpatialGrid` Class (`SpatialGrid.h`)
Implements spatial hash grid for efficient neighbor finding. Divides 2D space into uniform cells, reducing collision checks from O(n²) to O(n).

**Key methods:**
```cpp
void insert(Boid boid, int id);              // Add boid to grid
std::vector<int> get_nearby(const Boid& boid); // Get neighbors in 3×3 cell region
void clear();                                 // Reset grid each frame
```

### 1.3. `Simulation` Class (`Simulation.h`)
Core simulation manager that:
- Initializes boid population with random positions/velocities
- Builds spatial grid each frame
- Computes neighbor relationships using optimized pair checking
- Updates all boid behaviors with configurable weights
- Handles mouse interaction (attraction/repulsion)
- Manages edge behavior (bounce/wrap)

### 1.4. Rendering System (`main.cpp`)
OpenGL 4.6 instanced rendering pipeline:
- **Vertex shader**: Transforms boid triangles via model-view-projection matrices
- **Fragment shader**: Applies per-boid coloring (velocity-based or friend-based)
- **Instancing**: Renders all boids in a single draw call using instance matrices
- **ImGui overlay**: Real-time parameter control and statistics display

---

## 2. Building

### 2.1. Requirements
- CMake 3.16+
- C++17 compiler (MSVC/GCC/Clang)
- OpenGL 4.6 capable GPU
- vcpkg (for dependency management)

### 2.2. Dependencies
```bash
vcpkg install glfw3:x64-windows glm:x64-windows glad:x64-windows imgui[opengl3-binding,glfw-binding]:x64-windows
```

### 2.3. Build Instructions

#### Visual Studio
1. Open project folder in Visual Studio
2. CMake configuration auto-detects
3. Build and run the `Boids` target on release configuration

#### Command Line
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
./build/Release/Boids.exe
```

---

## 3. Controls

### 3.1. Mouse
| Action          | Effect                    |
|-----------------|---------------------------|
| **Left Click**  | Attract boids to cursor   |
| **Right Click** | Repel boids from cursor   |
| **Scroll**      | Zoom in/out (planned)     |

### 3.2. Keyboard
| Key   | Action                          |
|-------|---------------------------------|
| `ESC` | Exit application                |
| `V`   | Toggle vsync on/off             |
| `F`   | Toggle friend visualization     |
| `C`   | Toggle speed-based coloring     |

### 3.3. ImGui Panel
Real-time adjustable parameters:
- **Behavior weights**: Alignment, Cohesion, Separation
- **Speed limits**: Min/Max velocity
- **Field of view**: Angle and radius for neighbor detection
- **Edge behavior**: Bounce vs wrap-around
- **Visual modes**: Friend visualization, speed coloring
- **Spawn controls**: Add boids at runtime

---

## 4. Configuration

### 4.1. Simulation Parameters (`Simulation.h`)
```cpp
// Behavior weights
float alignment  = 2.0f;   // Steer toward average heading
float cohesion   = 3.0f;   // Move toward flock center
float separation = 1.0f;   // Avoid crowding

// Speed constraints
float maxSpeed = 0.5f;
float minSpeed = 0.2f;

// Perception
float fov       = 0.5f;    // Field of view angle (radians)
float fovRadius = 0.3f;    // Detection radius

// Edge behavior
bool bounce = true;        // Bounce off edges vs wrap-around
```

### 4.2. Initial Setup (`main.cpp`)
```cpp
const int N_BOIDS = 1000;           // Starting population
const float ASPECT_RATIO = 16.0f/9.0f;
```

### 4.3. Spatial Grid (`SpatialGrid.h`)
```cpp
// Cell size should be >= fovRadius for correctness
SpatialGrid grid{ fovRadius * 1.5f };
```

---

## 5. Performance

### 5.1. Optimizations Implemented
1. **Spatial Grid Partitioning**: Reduces neighbor search from O(n²) to O(n)
2. **Pair Deduplication**: Uses `unordered_set` to avoid checking (A,B) and (B,A)
3. **GPU Instancing**: Single draw call for all boids
4. **Reference Passing**: Avoids unnecessary boid copies in hot loops

### 5.2. Known Issues
- Cell size must be ≥ `fovRadius` or neighbor detection fails
- Very high boid counts (10k+) may cause frame drops during grid rebuild
- Friend visualization mode has performance cost (full N^2 check for one boid)
- Attracting boids by LPM might cause frame drops due to a lot of objects in neighbor cells

---

## 6. References

### Core Algorithm
- [Boids - Craig Reynolds (1987)](https://www.red3d.com/cwr/boids/)
- [Flocks, Herds, and Schools: A Distributed Behavioral Model](https://dl.acm.org/doi/10.1145/37402.37406)

### Implementation Guides
- [Boids Algorithm Explanation - Conrad Parker](http://www.kfish.org/boids/pseudocode.html)
- [Learn OpenGL - Instancing](https://learnopengl.com/Advanced-OpenGL/Instancing)
- [Spatial Partitioning - Game Programming Patterns](https://gameprogrammingpatterns.com/spatial-partition.html)

### Tools & Libraries
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- [GLM Documentation](https://github.com/g-truc/glm)
- [Dear ImGui](https://github.com/ocornut/imgui)