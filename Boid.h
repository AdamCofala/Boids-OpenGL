#pragma once
#include <glm/glm.hpp>
#include <vector>

class Boid {
public:
	Boid(glm::vec2 p, glm::vec2 d, glm::vec3 c={0,0,0}) : pos(p), dir(d), color(c){}
	glm::vec2 pos;
	glm::vec2 dir;
	glm::vec3 color;
	glm::vec3 blendedColor = { 0,0,0 };

	std::vector<Boid*> friends;

	void update(float aligmentStength, float cohesionStrength, float seperationStrength, float aspect,float deltaTime, float minSpeed, float maxSpeed, glm::vec2 mousePoint, bool atract, bool repel);

	void handleBoundaries(float aspect);


	void bounceBoundaries(float aspect);

	glm::vec3 getSpeedColor(float speed, float minSpeed, float maxSpeed);
	
	float getRotation();

	bool getFriend(Boid* potentialFriend, float fov, float fovRadius);


};
