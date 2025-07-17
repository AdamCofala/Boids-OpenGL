# 🕊️ Instanced 2D Boids Simulation with OpenGL + ImGui

A high-performance, visually interactive 2D boids (flocking) simulation written in modern C++ using OpenGL, GLFW, and ImGui. Features GPU instancing, dynamic behavior control, and intuitive mouse interaction.

## 📸 Preview

| Speed-Based Coloring | Friend Influence Visualization |
|----------------------|-------------------------------|
| ![image](https://github.com/user-attachments/assets/1e6e520e-e93b-43ff-b383-6b9a6e410dd5) | ![image](https://github.com/user-attachments/assets/869cdfb4-1624-4ee5-8137-ae9fb94a84f0) |


## ✨ Features

- 🚀 **Instanced rendering** of hundreds/thousands of boids using a single draw call
- ⚙️ Real-time control over:
  - Separation, alignment, cohesion weights
  - Speed limit
  - Edge behavior (wrap/bounce)
  - Color: speed-based or friend-averaged
- 🖱️ Mouse interaction:
  - **Left click** — Attract boids
  - **Right click** — Repel boids
  - **Middle click** — Spawn 100 boids at mouse location
- 🎮 UI built with **ImGui** (custom sci-fi styled)
- 📐 Dynamic viewport resizing and zooming support

## 🧠 Boid Behavior Overview

Each boid:
- Aligns with neighbors' direction
- Moves toward average neighbor position (cohesion)
- Avoids crowding (separation)
- Interacts with mouse position (attraction/repulsion)
- Can bounce or wrap off screen edges
- Can be colorized based on velocity or group blending
