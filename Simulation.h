#pragma once
#include "Boid.h"
#include <vector>
#include <random>

class Simulation {
public:
	Simulation() = default;
	Simulation(unsigned int N) {
		setupSimulation(N);
	};

	std::vector<Boid> Boids;

	void setupSimulation(unsigned int N) {
		
		std::random_device rd;
		std::uniform_real_distribution<float> pos(-2.0f, 2.0f);
		std::uniform_real_distribution<float> dir(-0.0001f, 0.0001f);
		std::mt19937 gen(rd());

		for (int i = 0; i < N; i++) {
			Boid b({ pos(gen), pos(gen) }, {dir(gen) , dir(gen)});
			Boids.push_back(b);
		}
	
	}

	void update() {

		for (auto& boid : Boids)
		{
			boid.update();
		}

	}

};
