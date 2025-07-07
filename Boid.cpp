#pragma once
#include "Boid.h"
#include <cmath>
#include <random>
# define M_PI           3.14159265358979323846  /* pi */


std::random_device rd;
std::uniform_real_distribution<float> steer(-1.0f, 1.0f);
std::mt19937 gen(rd());



void Boid::update(float alignmentStrength, float cohesionStrength, float separationStrength, float aspect, float deltaTime, float minSpeed, float maxSpeed, glm::vec2 mousePoint, bool atract) {

      if (!friends.empty()) {

			glm::vec2 alignment(0.0f, 0.0f);
			glm::vec2 cohesion(0.0f, 0.0f);
			glm::vec2 sepeatation(0.0f, 0.0f);

            blendedColor *= 0.0f;

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

            glm::vec2 steerForce(steer(gen), steer(gen));
            dir += steerForce * 0.03f;

            if (atract) {
                glm::vec2 toMouse = mousePoint - this->pos;
                float distance = glm::length(toMouse);

                if (distance > 0.01f) { // Avoid division by zero
                    float attractionStrength = 0.5f; // Adjust this value
                    glm::vec2 force = (toMouse / distance) * (attractionStrength / (distance * distance));

                    if (glm::length(force) > 10.0f) {
                        force = glm::normalize(force) * 10.0f;
                    }
                    dir += force * deltaTime;
                }
            }
      }

       // this->color = getSpeedColor(glm::length(dir), minSpeed, maxSpeed);
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


glm::vec3 Boid::getSpeedColor(float speed, float minSpeed, float maxSpeed) {
    // Clamp and normalize speed to [0, 1] range
    float normalized = glm::clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0f, 1.0f);

    // Apply smooth curves for more natural color transitions
    float smoothed = glm::smoothstep(0.0f, 1.0f, normalized);


    // Create a more vibrant and visually appealing color palette
    glm::vec3 color;

    if (smoothed < 0.33f) {
        // Slow: Blue to Cyan transition
        float t = smoothed / 0.33f;
        color = glm::mix(glm::vec3(0.1f, 0.3f, 1.0f),    // Deep blue
            glm::vec3(0.0f, 0.8f, 1.0f),     // Cyan
            t);
    }
    else if (smoothed < 0.66f) {
        // Medium: Cyan to Yellow transition
        float t = (smoothed - 0.33f) / 0.33f;
        color = glm::mix(glm::vec3(0.0f, 0.8f, 1.0f),    // Cyan
            glm::vec3(1.0f, 1.0f, 0.2f),     // Yellow
            t);
    }
    else {
        // Fast: Yellow to Red transition
        float t = (smoothed - 0.66f) / 0.34f;
        color = glm::mix(glm::vec3(1.0f, 1.0f, 0.2f),    // Yellow
            glm::vec3(1.0f, 0.2f, 0.0f),     // Red
            t);
    }

    // Add subtle brightness variation based on speed
    float brightness = 0.7f + 0.3f * smoothed;
    color *= brightness;

    // Optional: Add slight saturation boost for more vibrant colors
    float saturation = 1.0f + 0.2f * smoothed;
    glm::vec3 gray = glm::vec3(dot(color, glm::vec3(0.299f, 0.587f, 0.114f)));
    color = glm::mix(gray, color, saturation);

    return color;
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
