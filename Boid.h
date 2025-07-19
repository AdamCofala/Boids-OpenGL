#pragma once
#include <glm/glm.hpp>
#include <vector>

class Boid {
public:
	Boid(glm::vec2 p, glm::vec2 d, glm::vec3 c = { 0,0,0 }, bool ispred = false) : pos(p), dir(d), color(c), isPredator(ispred) {};
	glm::vec2 pos;
	glm::vec2 dir;
	glm::vec3 color;
	glm::vec3 blendedColor = { 0,0,0 };
	glm::vec3 visColor;

	std::vector<Boid*> friends;
	std::vector<Boid*> predators;
	bool isPredator;
	bool isPanicked;

	void update(float aligmentStength, float cohesionStrength, float seperationStrength, float aspect,float deltaTime, float minSpeed, float maxSpeed, glm::vec2 mousePoint, bool atract, bool repel, bool bounce, bool speedBasedColor);

	void handleBoundaries(float aspect);

	void addForce(float strength, glm::vec2 p, float deltaTime);

	void bounceBoundaries(float aspect);

	void limitSpeed(float& minSpeed, float& maxSpeed);

	glm::vec3 getSpeedColor(float speed, float minSpeed, float maxSpeed);
	
	float getRotation();

	bool getFriend(Boid* potentialFriend, float fov, float fovRadius);


};
