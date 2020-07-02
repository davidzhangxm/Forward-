//
//  Camera.cpp
//  opengl_start
//
//  Created by Xinming Zhang on 10/13/19.
//  Copyright © 2019 Xinming Zhang. All rights reserved.
//
#include <iostream>
#include "Camera.hpp"
#include <glm/gtx/string_cast.hpp>

mat4 Camera::GetViewMatrix()
{
	return lookAt(position, position + front, up);
}

void Camera::update()
{
	vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;

	if (direction == FORWARD)
		position += front * velocity;
	if (direction == BACKWARD)
		position -= front * velocity;
	if (direction == LEFT)
		position -= right * velocity;
	if (direction == RIGHT)
		position += right * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
	xOffset *= mouseSensitivity;
	yOffset *= mouseSensitivity;

	yaw += xOffset;
	pitch += yOffset;

	// Make sure when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch) {
		if (this->pitch > 89.0f) {
			this->pitch = 89.0f;
		}
		if (this->pitch < -89.0f) {
			this->pitch = -89.0f;
		}
	}

	// Update the camera
	update();
}