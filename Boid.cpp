#pragma once
#include "Boid.h"
#include <cmath>
# define M_PI           3.14159265358979323846  /* pi */


void Boid::update(float alignmentStrength, float cohesionStrength, float separationStrength, float aspect, float deltaTime) {
    const float maxSpeed = 0.5f; 
    const float minSpeed = 0.2f;

    if (!friends.empty()) {
        glm::vec2 alignment(0.0f, 0.0f);
        glm::vec2 cohesion(0.0f, 0.0f);
        glm::vec2 sepeatation(0.0f, 0.0f);

        for (auto& Friend : friends) {
            alignment += glm::normalize(Friend->dir); 
            cohesion += Friend->pos;
            
            glm::vec2 diff = (this->pos - Friend->pos);
            sepeatation += diff / (float)diff.length();
        }

        alignment /= (float)friends.size();
        dir += alignment * alignmentStrength * deltaTime;

        cohesion /= (float)friends.size();
        cohesion -= this->pos;

        dir += cohesion * cohesionStrength * deltaTime;
        
        dir += sepeatation * separationStrength * deltaTime;








        float currentSpeed = glm::length(dir);
        if (currentSpeed > maxSpeed) {
            dir = glm::normalize(dir) * maxSpeed;
        }
        else if (currentSpeed < minSpeed) {
            dir = glm::normalize(dir) * minSpeed;
        }
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
