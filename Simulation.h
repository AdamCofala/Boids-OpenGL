#pragma once
#include "Boid.h"
#include <vector>
#include <random>

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
		std::mt19937 gen(rd());

		for (int i = 0; i < N; i++) {
			Boid b({ posX(gen), posY(gen) }, { dir(gen) , dir(gen) }, { 1,0,1 });
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
