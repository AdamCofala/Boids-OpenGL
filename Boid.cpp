#pragma once
#include "Boid.h"
#include <cmath>


void Boid::update(float aspect) {

	this->pos += dir;

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

glm::vec2 Boid::getFriends()
{
	return glm::vec2();
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