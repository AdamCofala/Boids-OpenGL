#pragma once
#include "Boid.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <omp.h>
#include "SpatialGrid.h"
#include <unordered_set>


class Simulation {
public:
	Simulation() = default;
	Simulation(unsigned int N, float aspect) : aspect(aspect) {
		setupSimulation(N);
	};

	std::vector<Boid> Boids;
	float aspect;

	float fov          = 0.5f;
	float fovRadius    = 0.1f;
	int   frameCount   = 0;

	float alignment    = 2.0f;
	float cohesion	   = 3.0f;
	float separation   = 1.0f;
	float maxSpeed     = 0.5f; 
    float minSpeed     = 0.2f;

	bool  atract       = false;
	bool  repel        = false; 
	bool  bounce       = true;
	bool  friendVisual = false;
	bool  speedCol     = false;

	glm::vec2 mousePoint;
	SpatialGrid grid{ fovRadius };
	
	void setupSimulation(unsigned int N) {

		std::uniform_real_distribution<float> posY(-1.0f, 1.0f);
		std::uniform_real_distribution<float> posX(-aspect, aspect);
		std::random_device rd;
		std::mt19937 gen(rd());

	
		for (int i = 0; i < N; i++) {
			glm::vec2 pos = { posX(gen), posY(gen) };
			Boids.push_back(generateBoid(pos));
		}
	}

	Boid generateBoid(glm::vec2 &pos, bool predators = false) {

    	std::uniform_real_distribution<float> dir(-0.3f, 0.3f);
		std::uniform_real_distribution<float> color(0.0f, 1.0f);
		std::uniform_real_distribution<float> gradient(0.0f, 0.5f);
		std::random_device rd;
		std::mt19937 gen(rd());

		glm::vec2 posVec = pos;
		glm::vec2 dirVec = { dir(gen), dir(gen) };
		glm::vec3 colorVec = { color(gen), color(gen), color(gen) };


		if (glm::length(dirVec) < 0.2f) {
			dirVec = glm::normalize(dirVec) * 0.2f;
		}

		if (colorVec.x > 0.8f) {
				colorVec.y = gradient(gen);
				colorVec.z = gradient(gen);
		}

		if (colorVec.y > 0.8f) {
				colorVec.x = gradient(gen);
				colorVec.z = gradient(gen);
		}

		if (colorVec.z > 0.8f) {
				colorVec.y = gradient(gen);
				colorVec.x = gradient(gen);
		} 

		Boid b(posVec, dirVec, colorVec, predators);
		return b;

	}

	void update(float dt) {

		optimizedMadeFriends();

		int numBoids = static_cast<int>(Boids.size());

		#pragma omp parallel for schedule(static)
		for (int i = 0; i < numBoids; i++) {
			Boids[i].update(alignment, cohesion, separation, aspect, dt, minSpeed, maxSpeed,
				mousePoint, atract, repel, bounce, speedCol);
		}

		if (friendVisual) showFriends();
	}

	void updateAspect(float aspectNew) {
		aspect = aspectNew;
	}

	void madeFriends(float dt) {
		// Fucking pointer typeshit friends making, breaking the KISS
		auto boid = Boids.begin();
		while (boid < Boids.end()) {
			(*boid).friends.clear();
			(*boid).predators.clear();
			auto potentialFriend = boid + 1;

			while (potentialFriend < Boids.end()) {

				(*boid).getFriend(&(*potentialFriend), fov, fovRadius);
				++potentialFriend;
			}
			++boid;
		}
	}

	void optimizedMadeFriends() {

		int numBoids = static_cast<int>(Boids.size());

		// Phase 1: Clear lists and build grid (sequential - grid is not thread-safe for writes)
		for (int id = 0; id < numBoids; id++) {
			Boids[id].friends.clear();
			Boids[id].predators.clear();
			grid.insert(Boids[id], id);
		}

		// Phase 2: Query neighbors and build friend lists (parallel)
		// Each thread gets its own 'nearby' vector. Each boid writes only to its own lists.
		#pragma omp parallel
		{
			std::vector<int> nearby;
			nearby.reserve(64);

			#pragma omp for schedule(dynamic)
			for (int x = 0; x < numBoids; x++) {
				Boid& boid = Boids[x];
				grid.get_nearby(boid, nearby);

				for (int neighbor_id : nearby) {
					if (x >= neighbor_id) continue;

					Boid& other = Boids[neighbor_id];
					boid.getFriend(&other, fov, fovRadius);
				}
			}
		}

		grid.clear();
	}

	void showFriends() {

		auto boid = Boids.begin();

		while (boid != Boids.begin()+1) {

			(*boid).friends.clear();

			auto potentialFriend = boid + 1;
			
			while (potentialFriend != Boids.end()) {

				if ((*boid).getFriend(&(*potentialFriend), fov, fovRadius)) {
					(*boid).friends.push_back(&(*potentialFriend));
					(*potentialFriend).visColor = { 0,0,1 };
					(*boid).visColor = { 1,0,0 };
				}
				else {
					(*potentialFriend).visColor = { 0.2,0.2,0.2 };
				}
				potentialFriend++;

			}
			boid++;
		}
	}
};

