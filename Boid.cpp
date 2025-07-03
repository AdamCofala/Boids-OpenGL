#pragma once
#include "Boid.h"
#include <cmath>
# define M_PI           3.14159265358979323846  /* pi */


void Boid::update(float alignmentStrength, float cohesionStrength, float separationStrength, float aspect, float deltaTime, float minSpeed, float maxSpeed) {
      if (!friends.empty()) {

        glm::vec2 alignment(0.0f, 0.0f);
        glm::vec2 cohesion(0.0f, 0.0f);
        glm::vec2 sepeatation(0.0f, 0.0f);

        glm::vec3 blendedColor(0.0f);

        for (auto& Friend : friends) {

            alignment += glm::normalize(Friend->dir); 

            cohesion += Friend->pos;
            
            glm::vec2 diff = (this->pos - Friend->pos);
            sepeatation += diff / ((float)diff.length()+0.000001f);

            blendedColor += Friend->color;
        }

        blendedColor /= friends.size();
        this->color = glm::mix(this->color, blendedColor, 0.05f);

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

   // bounceBoundaries(aspect);
    handleBoundaries(aspect);
}

void Boid::handleBoundaries(float aspect) {
	if (this->pos.x > aspect + 0.1f) this->pos.x = -aspect - 0.1f;
	else if (this->pos.x < -aspect - 0.1f) this->pos.x = aspect + 0.1f;

	if (this->pos.y > 1.1f) this->pos.y = -1.1f;
	else if (this->pos.y < -1.1f) this->pos.y = 1.1f;
}

void Boid::bounceBoundaries(float aspect) {
    if (this->pos.x >= aspect) {
        this->pos.x = aspect;
        this->dir.x *= -1;
    }
    else if (this->pos.x <= -aspect) {
        this->pos.x = -aspect;
        this->dir.x *= -1;
    }

    if (this->pos.y >= 1.0f) {
        this->pos.y = 1.0f;
        this->dir.y *= -1;
    }
    else if (this->pos.y <= -1.0f) {
        this->pos.y = -1.0f;
        this->dir.y *= -1;
    }
}



float Boid::getRotation()
{
	glm::vec2 normalD = glm::normalize(dir);
	return atan2f(-normalD.y,normalD.x);
}


bool Boid::getFriend(Boid* potentialFriend, float fov, float fovRadius)
{

	glm::vec2 toFriend = potentialFriend->pos - this->pos ;
	float cos = glm::dot(glm::normalize(dir), glm::normalize(-toFriend));

	if (glm::length(toFriend) < fovRadius && cos<= fov/2.0f) return true;

	return false;
}
