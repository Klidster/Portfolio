#pragma once

#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

struct SCamera
{
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;

	const float MovementSpeed = 5.5f;
	float MouseSensitivity = 1.f;



};


void InitCamera(SCamera& in)
{
	in.Front = glm::vec3(0.0f, 0.0f, -1.0f);
	in.Position = glm::vec3(0.0f, 0.0f, 10.0f);
	in.Up = glm::vec3(0.0f, 1.0f, 0.0f);
	in.WorldUp = in.Up;
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));

	in.Yaw = -90.f;
	in.Pitch = 0.f;
}

float cam_dist = 2.f;

void MoveAndOrientCamera(SCamera& in, glm::vec3 target, float distance, float xoffset, float yoffset)
{
	// Adjust yaw and pitch
	in.Yaw += xoffset * in.MouseSensitivity;
	in.Pitch += yoffset * in.MouseSensitivity;

	// Clamp pitch to avoid flipping
	if (in.Pitch > 89.0f) in.Pitch = 89.0f;
	if (in.Pitch < -89.0f) in.Pitch = -89.0f;

	// Convert spherical coordinates to Cartesian
	float yawRad = glm::radians(in.Yaw);
	float pitchRad = glm::radians(in.Pitch);

	glm::vec3 position;
	position.x = target.x + distance * cos(pitchRad) * cos(yawRad);
	position.y = target.y + distance * sin(pitchRad);
	position.z = target.z + distance * cos(pitchRad) * sin(yawRad);

	in.Position = position;
	in.Front = glm::normalize(target - in.Position);
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));
	in.Up = glm::normalize(glm::cross(in.Right, in.Front));
}