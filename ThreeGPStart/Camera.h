#pragma once

#include "ExternalLibraryHeaders.h"

namespace Helpers
{
	// Simple first person camera
	class Camera
	{
		// Original values for recall
		glm::vec3 m_originalPosition{ 0 };
		glm::vec3 m_originalRotations{ 0 };

		// Current values
		glm::vec3 m_position{ 0 };
		glm::vec3 m_rotations{ 0 };

		// Current changes
		glm::vec3 m_currentMovement{ 0 };
		glm::vec3 m_currentRotation{ 0 };

		// Speeds
		float m_translationPerSecond{ 10.0f };
		float m_rotationPerSecond{ 1.0f };

		// Held here to improve performance as used a lot
		glm::mat4 m_rotationMatrix{ 1 };

		glm::mat4 CalcRotationMatrix() const;
		void ClampRotations();
	public:
		Camera() = default;
		~Camera() = default;

		// Parameters are the camera's starting position, starting rotations and optionally speeds for translatation and rotation
		void Initialise(const glm::vec3& startPos, const glm::vec3& startRots, float translationPerSecond = 200.0f, float rotationPerSecond = 1.5f);

		// Set world translation
		void SetPosition(const glm::vec3& newPos) { m_position = newPos; }

		// Set world rotations
		void SetRotations(const glm::vec3& newRots) { m_rotations = newRots; ClampRotations(); }

		// The camera needs updating to handle user input
		void Update(GLFWwindow* window, float timePassedSecs);

		// Returns the current position of the camera
		glm::vec3 GetPosition() const { return m_position; }

		// Returns the forward looking vector
		glm::vec3 GetLookVector() const;

		// Returns a vector that points to the right of where the camera is facing
		glm::vec3 GetRightVector() const;

		// Returns a vector indicating the direction of up
		glm::vec3 GetUpVector() const;

		// Helper to allow easy outputting of camera values
		std::string ToString() const {
			return "Pos x:" + std::to_string(m_position.x) +
				" y:" + std::to_string(m_position.y) +
				" z:" + std::to_string(m_position.z) +
				"Rot x:" + std::to_string(m_rotations.x) +
				" y:" + std::to_string(m_rotations.y) +
				" z:" + std::to_string(m_rotations.z);
		}
	};
}
