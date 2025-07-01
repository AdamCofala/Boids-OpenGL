#pragma once
#include "Boid.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>


class Simulation {
public:
	Simulation() = default;
	Simulation(unsigned int N, float aspect): aspect(aspect){
		setupSimulation(N);
	};

	std::vector<Boid> Boids;
	float aspect;

	void setupSimulation(unsigned int N) {
		
		std::random_device rd;
		std::uniform_real_distribution<float> posY(-1.0f, 1.0f);
		std::uniform_real_distribution<float> posX(-aspect, aspect);
		std::uniform_real_distribution<float> dir(-0.0001f, 0.0001f);
		std::uniform_real_distribution<float> color(0.5f, 1.0f);
	//	std::uniform_real_distribution<float> gradient(0.0f, 0.5f);

		std::mt19937 gen(rd());

		for (int i = 0; i < N; i++) {

			glm::vec2 posVec = { posX(gen), posY(gen) };
			glm::vec2 dirVec = { dir(gen), dir(gen) };
			glm::vec3 colorVec = { color(gen), color(gen), color(gen) };

		/*	if (colorVec.x > 0.8f) {
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
			} */



			Boid b(posVec, dirVec, colorVec);
			Boids.push_back(b);
		}
	
	}

	void update() {

		for (auto& boid : Boids)
		{
			boid.update(aspect);
		}
	}

	void updateAspect(float aspectNew) {
		aspect = aspectNew;
	}

};
