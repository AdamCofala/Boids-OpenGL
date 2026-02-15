#pragma once
#include "Boid.h"
#include <cmath>
#include <random>
# define M_PI           3.14159265358979323846  /* pi */

static thread_local std::mt19937 gen([] {
	std::random_device rd;
	return rd();
}());
std::uniform_real_distribution<float> steer(-1.0f, 1.0f);

void Boid::update(

    float alignmentStrength,
    float cohesionStrength,
    float separationStrength,
    float aspect,
    float deltaTime,
    float minSpeed,
    float maxSpeed,
    glm::vec2 mousePoint, 
    bool atract, 
    bool repel,
    bool bounce,
    bool speedBasedColor

    ) {

		glm::vec2 runAway(0.0f, 0.0f);
		isPanicked = false;


		if (!friends.empty()) {

			glm::vec2 alignment(0.0f, 0.0f);
			glm::vec2 cohesion(0.0f, 0.0f);
			glm::vec2 sepeatation(0.0f, 0.0f);
			blendedColor *= 0.0f;

			for (auto& Friend : friends) {

				alignment += glm::normalize(Friend->dir);

				cohesion += Friend->pos;

				glm::vec2 diff = (this->pos - Friend->pos);
				sepeatation += diff / ((float)diff.length() + 0.000001f);

				blendedColor += Friend->color;
			}

			blendedColor /= friends.size();

			alignment /= (float)friends.size();
			dir += alignment * alignmentStrength * deltaTime;

			cohesion /= (float)friends.size();
			cohesion -= this->pos;

			if (isPredator) cohesion *= 2.0f;
			dir += cohesion * cohesionStrength * deltaTime;

			dir += sepeatation * separationStrength * deltaTime;
		}

		if (!predators.empty() && !isPredator) {
			float predatorAvoidanceStrength = 0.1f;

			for (auto& predator : predators) {
				glm::vec2 toPredator = predator->pos - this->pos;
				float distance = glm::length(toPredator);

				float avoidanceStrength = predatorAvoidanceStrength / (distance * distance + 0.01f);
				glm::vec2 avoidanceDir = -glm::normalize(toPredator);

				runAway += avoidanceDir * avoidanceStrength;

			}
		
		}

		dir += runAway * deltaTime;

		if (speedBasedColor && !isPredator) {
			visColor = getSpeedColor(glm::length(dir), minSpeed, maxSpeed);
		}
		else if(!isPredator){
		    if(!friends.empty()) this->color= glm::mix(this->color, blendedColor, 0.05f);
            visColor = color;
		}
		else {
			visColor = { 1.0f, 1.0f ,1.0f } ;
		}

		glm::vec2 steerForce(steer(gen), steer(gen));
		dir += steerForce * 0.03f;

		limitSpeed(minSpeed, maxSpeed);

		if (atract) {
			addForce(5.3f, mousePoint, deltaTime);
		}
	   
		if (repel) {
			addForce(-5.3f, mousePoint, deltaTime);
		}
    
		this->pos += dir * deltaTime;

	    if(bounce) bounceBoundaries(aspect);
	    handleBoundaries(aspect);
}

void Boid::handleBoundaries(float aspect) {

	if (this->pos.x > aspect + 0.1f) this->pos.x = -aspect - 0.1f;
	else if (this->pos.x < -aspect - 0.1f) this->pos.x = aspect + 0.1f;

	if (this->pos.y > 1.1f) this->pos.y = -1.1f;
	else if (this->pos.y < -1.1f) this->pos.y = 1.1f;
}

void Boid::addForce(float strength, glm::vec2 p, float deltaTime)
{
	glm::vec2 toMouse = p - this->pos;
			float distance = glm::length(toMouse);

	if (distance > 0.01f) {
			float force = strength * exp(-distance * 2.0f);
		dir += glm::normalize(toMouse) * force * deltaTime;
	}

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

void Boid::limitSpeed(float &minSpeed, float &maxSpeed) {

	float currentSpeed = glm::length(dir);
	if (currentSpeed > maxSpeed) {
		dir = glm::normalize(dir) * maxSpeed;
	}
	else if (currentSpeed < minSpeed) {
		dir = glm::normalize(dir) * minSpeed;
	}

}

glm::vec3 Boid::getSpeedColor(float speed, float minSpeed, float maxSpeed) {
    // Clamp and normalize speed to [0, 1] range
    float normalized = glm::clamp((speed - minSpeed*1.5f) / (maxSpeed - minSpeed*1.5f), 0.0f, 1.0f);

    glm::vec3 color;

    // Define color points for the gradient
    glm::vec3 slowColor = glm::vec3(0.25f, 0.15f, 0.60f); // Deep Blue
    glm::vec3 fastColor = glm::vec3(0.6f, 0.6f, 1.70f); // Bright Green (can be changed to yellow: 0.9f, 0.8f, 0.1f)

   
    color = glm::mix(slowColor, fastColor, normalized);
    return color;
}

float Boid::getRotation()
{
	glm::vec2 normalD = glm::normalize(dir);
	return atan2f(-normalD.y,normalD.x);
}

bool Boid::getFriend(Boid* potentialFriend, float fov, float fovRadius)
{

	glm::vec2 toFriend = potentialFriend->pos - this->pos;
	float distSq = glm::dot(toFriend, toFriend);
	float radiusSq = fovRadius * fovRadius;

	if (distSq >= radiusSq) return false;

	if (potentialFriend->isPredator) {
		predators.push_back(potentialFriend);
	}

	float halfFov = fov * 0.5f;
	float cosVal = glm::dot(glm::normalize(dir), glm::normalize(-toFriend));

	if (cosVal <= halfFov) {
		if (potentialFriend != this) {
			if (potentialFriend->isPredator) return true;
			friends.push_back(potentialFriend);
			return true;
		}
	}

	return false;
}
