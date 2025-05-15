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
	//view = glm::lookAt(eye, target, worldUp);
	view = calculateViewMatrix();

	//Calculate projection matrix
	//projection = glm::perspective(fov, aspect, near, far);
	projection = calculatePerspective();
}

void Camera::calculateCameraVectors()
{
	if (!jumping)
		eye.y = 0;
	front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
	right = Maths::normalize(Maths::cross(front, worldUp));
	up = Maths::cross(right, front);
}
void Camera::quaternionCamera(bool thirdCamera, bool fourthCamera)
{
	// Calculate camera orientation quaternion from the Euler angles
	Quaternion newOrientation(-pitch, yaw);

	//Apply SLERP to orientation
	orientation = Maths::SLERP(orientation, newOrientation, 0.2f);

	// Calculate the view matrix
	view = orientation.matrix() * Maths::translate(-eye);

	//3D Camera
	if (thirdCamera || fourthCamera)
	{
		view = Maths::translate(cameraOffset) * view;
	}

	// Calculate the projection matrix
	//projection = glm::perspective(fov, aspect, near, far);
	projection = calculatePerspective();

	// Calculate camera vectors from view matrix
	right = glm::vec3(view[0][0], view[1][0], view[2][0]);
	up = glm::vec3(view[0][1], view[1][1], view[2][1]);
	front = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}

glm::mat4 Camera::calculateViewMatrix()
{
	glm::mat4 translate; 
	glm::mat4 rotate;
	translate[0][3] = -eye.x;
	translate[1][3] = -eye.y;
	translate[2][3] = -eye.z;

	rotate[0][0] = right.x;
	rotate[0][1] = right.y;
	rotate[0][2] = right.z;
	rotate[1][0] = up.x;
	rotate[1][1] = up.y;
	rotate[1][2] = up.z;
	rotate[2][0] = -front.x;
	rotate[2][1] = -front.y;
	rotate[2][2] = -front.z;

	return Maths::transpose(translate*rotate);
}

glm::mat4 Camera::calculatePerspective()
{
	float top = near * tan(fov / 2);
	float pRight = aspect * top;
	glm::mat4 perspective;
	perspective[0][0] = near/pRight;
	perspective[1][1] = near/top;
	perspective[2][2] = -((far+near)/(far-near));
	perspective[2][3] = -1;
	perspective[3][2] = -((2*far*near)/(far-near));
	perspective[3][3] = 0;

	return perspective;
}

