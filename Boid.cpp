#pragma once
#include "Boid.h"
#include <cmath>
# define M_PI           3.14159265358979323846  /* pi */


void Boid::update(float alignmentStength, float cohesionStrength, float seperationStrength, float aspect, float deltaTime) {


	if (!friends.empty()) {

		float speed = dir.length();

		glm::vec2 alignment(0.0f, 0.0f);

		for (auto& Friend : friends) {

			alignment += Friend->dir;


		}
		alignment /= (float)friends.size();

		dir += alignment*alignmentStength;
		dir = glm::normalize(dir) * speed;
	}



	this->pos += dir * deltaTime;

	handleBoundaries(aspect);
}

void Boid::handleBoundaries(float aspect) {
	if (this->pos.x > aspect + 0.1f) this->pos.x = -aspect - 0.1f;
	else if (this->pos.x < -aspect - 0.1f) this->pos.x = aspect + 0.1f;

	if (this->pos.y > 1.1f) this->pos.y = -1.1f;
	else if (this->pos.y < -1.1f) this->pos.y = 1.1f;
}

float Boid::getRotation()
{
	glm::vec2 normalD = glm::normalize(dir);
	return atan2f(-normalD.y,normalD.x);
}


glm::vec2 Boid::getSeparation()
{
	return glm::vec2();
}

glm::vec2 Boid::getAlignment()
{
	return glm::vec2();
}

glm::vec2 Boid::getCohesion()
{
	return glm::vec2();
}

bool Boid::getFriend(Boid* potentialFriend, float fov, float fovRadius)
{

	glm::vec2 toFriend = potentialFriend->pos - this->pos ;
	float cos = glm::dot(glm::normalize(dir), glm::normalize(-toFriend));

	if (glm::distance(pos, potentialFriend->pos) < fovRadius &&  cos <= fov/2) return true;

	return false;
}
