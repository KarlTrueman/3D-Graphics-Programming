#include "Camera.h"

namespace Helpers
{
	static constexpr float KMaxVerticalAngle{ glm::radians(85.0f) }; //must be less than 90 to avoid gimbal lock

	// Parameters are the camera's starting position, starting rotations and optionally speeds for translatation and rotation
	void Camera::Initialise(const glm::vec3& startPos, const glm::vec3& startRots, float translationPerSecond, float rotationPerSecond)
	{
		m_position = startPos;
		m_rotations = startRots;
		m_translationPerSecond = translationPerSecond;
		m_rotationPerSecond = rotationPerSecond;
		m_originalPosition = startPos;
		m_originalRotations = startRots;

		std::cout << "\nCamera Controls" << std::endl;
		std::cout << "WASD - moves the camera left, right (x axis) and into and out of the scene (Z axis)" << std::endl;
		std::cout << "Up and Down arrows - moves the camera up and down the Y axis." << std::endl;
		std::cout << "Space Bar - resets the view to its initial state." << std::endl;
		std::cout << "Hold the left mouse button while moving the mouse to rotate the camera" << std::endl;
		std::cout << "Holding left CTRL key accelerates movement and rotation" << std::endl << std::endl;
	}

	// To avoid large values I like to clamp rotations to 0-359 degrees
	// I also precent rotation around the x axis going too far to avoid the potential for Gimbol lock
	void Camera::ClampRotations()
	{
		m_rotations.y = fmodf(m_rotations.y, glm::radians(360.0f));

		if (m_rotations.y < 0)
			m_rotations.y += glm::radians(360.0f);

		if (m_rotations.x > KMaxVerticalAngle)
			m_rotations.x = KMaxVerticalAngle;
		else if (m_rotations.x < -KMaxVerticalAngle)
			m_rotations.x = -KMaxVerticalAngle;
	}

	// The look vector can be calulcated from the inverse of the rotation matrix, third column
	glm::vec3 Camera::GetLookVector() const
	{
		glm::vec4 forward{ glm::inverse(m_rotationMatrix) * glm::vec4(0, 0, -1, 1) };
		return glm::vec3(forward);
	}

	// The right vector can be calulcated from the inverse of the rotation matrix, first column
	glm::vec3 Camera::GetRightVector() const
	{
		glm::vec4 right{ glm::inverse(m_rotationMatrix) * glm::vec4(1, 0, 0, 1) };
		return glm::vec3(right);
	}

	// The up vector can be calulcated from the inverse of the rotation matrix, second column
	glm::vec3 Camera::GetUpVector() const
	{
		glm::vec4 up{ glm::inverse(m_rotationMatrix) * glm::vec4(0, 1, 0, 1) };
		return glm::vec3(up);
	}

	// Calculate rotation matrix from current axis rotations
	glm::mat4 Camera::CalcRotationMatrix() const
	{
		glm::mat4 orientation(1.0f);
		orientation = glm::rotate(orientation, m_rotations.x, glm::vec3(1, 0, 0));
		orientation = glm::rotate(orientation, m_rotations.y, glm::vec3(0, 1, 0));
		return orientation;
	}

	// Update camera position and rotations and handle user input
	void Camera::Update(GLFWwindow* window, float timePassedSecs)
	{
		assert(window);

		// Not if it is being handled by IMGUI
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
			return ;

		float worldUnitsPerSecond{ m_translationPerSecond };
		float radiansPerSecond{ m_rotationPerSecond };

		// Accelerate movement and rotation if left control key held
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			worldUnitsPerSecond *= 10.0f;
			radiansPerSecond *= 10.0f;
		}

		// Even more if right pressed
		if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		{
			worldUnitsPerSecond *= 100.0f;
			radiansPerSecond *= 100.0f;
		}

		m_currentMovement = glm::vec3(0);

		// Keyboard Input
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // Forward		
			m_currentMovement = GetLookVector() * worldUnitsPerSecond;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // Backwards		
			m_currentMovement = -GetLookVector() * worldUnitsPerSecond;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // Left		
			m_currentMovement = -GetRightVector() * worldUnitsPerSecond;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // Right
			m_currentMovement = +GetRightVector() * worldUnitsPerSecond;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // Up
			m_currentMovement = +GetUpVector() * worldUnitsPerSecond;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // Down
			m_currentMovement = -GetUpVector() * worldUnitsPerSecond;

		// Mouse Input
		{
			static int lastState = GLFW_RELEASE;
			static glm::vec2 initialPos;

			int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
			if (state != lastState)
			{
				if (state == GLFW_PRESS)
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				else
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

				lastState = state;

				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);

				initialPos = glm::vec2((float)xpos, (float)ypos);
				m_currentRotation = glm::vec3(0);
			}
			else if (state == GLFW_PRESS)
			{
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);

				float pitchChange = ((float)xpos - initialPos.x) * 0.001f * radiansPerSecond;
				float yawChange = ((float)ypos - initialPos.y) * 0.001f * radiansPerSecond;

				m_currentRotation.y = pitchChange;
				m_currentRotation.x = yawChange;
			}
		}

		// Reset
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			m_rotations = m_originalRotations;
			m_position = m_originalPosition;
			m_currentMovement = glm::vec3(0);
			m_currentRotation = glm::vec3(0);
		}

		// Update
		m_position += m_currentMovement * timePassedSecs;
		m_rotations += m_currentRotation * timePassedSecs;

		ClampRotations();

		m_rotationMatrix = CalcRotationMatrix();
	}
}

