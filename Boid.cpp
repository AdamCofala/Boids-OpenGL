#pragma once
#include "Boid.h"
#include <cmath>


void Boid::update() {


	this->pos += dir;
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
