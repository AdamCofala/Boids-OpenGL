#pragma once
#include <glm/glm.hpp>
#include <vector>

class Boid {
public:
	Boid(glm::vec2 p, glm::vec2 d, glm::vec3 c={0,0,0}) : pos(p), dir(d), color(c){}
	glm::vec2 pos;
	glm::vec2 dir;
	glm::vec3 color;

	std::vector<Boid> friends;

	void update(float aspect);

	void handleBoundaries(float aspect);
	
	float getRotation();
	glm::vec2 getFriends();
	glm::vec2 getSeparation();
	glm::vec2 getAlignment();
	glm::vec2 getCohesion();


};
