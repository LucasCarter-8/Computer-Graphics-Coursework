#include <common/camera.hpp>

Camera::Camera(const glm::vec3 eye, const glm::vec3 Target)
{
	this->eye = eye;
	this->target = target;
}

void Camera::calculateMatrices()
{
	// Calculate camera vectors
	calculateCameraVectors();

	//Calculate view matrix
	view = glm::lookAt(eye, eye + front, worldUp);

	//Calculate projection matrix
	projection = glm::perspective(fov, aspect, near, far);
}


void Camera::calculateCameraVectors()
{
	if (!jumping)
		eye.y = 0;

	front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::cross(right, front);
}
void Camera::quaternionCamera(bool thirdCamera)
{
	// Calculate camera orientation quaternion from the Euler angles
	Quaternion newOrientation(-pitch, yaw);

	//Apply SLERP to orientation
	orientation = Maths::SLERP(orientation, newOrientation, 0.2f);

	// Calculate the view matrix
	view = orientation.matrix() * Maths::translate(-eye);

	//3D Camera
	if (thirdCamera)
	{
		view = Maths::translate(cameraOffset) * view;
	}

	// Calculate the projection matrix
	projection = glm::perspective(fov, aspect, near, far);

	// Calculate camera vectors from view matrix
	right = glm::vec3(view[0][0], view[1][0], view[2][0]);
	up = glm::vec3(view[0][1], view[1][1], view[2][1]);
	front = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}


