#include "Camera.h"
#include "MatrixStack.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath> 
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() :
	aspect(1.0f),
	fovy((float)(45.0*M_PI/180.0)),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.60, 0.15),
	translations(0.0f, 0.0f, -17.0f), // A-WHOOP
	rfactor(0.01f),
	tfactor(0.001f),
	sfactor(0.005f)
{
}

Camera::~Camera()
{
}

void Camera::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	if(shift) {
		state = Camera::TRANSLATE;
	} else if(ctrl) {
		state = Camera::SCALE;
	} else {
		state = Camera::ROTATE;
	}
}

void Camera::mouseMoved(float x, float y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;
	switch(state) {
		case Camera::ROTATE:
			rotations += rfactor * dv;
			break;
		case Camera::TRANSLATE:
			translations.x -= translations.z * tfactor * dv.x;
			translations.y += translations.z * tfactor * dv.y;
			break;
		case Camera::SCALE:
			translations.z *= (1.0f - sfactor * dv.y);
			break;
	}
	mousePrev = mouseCurr;
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const
{
	// Modify provided MatrixStack
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const
{
	MV->translate(translations);
	MV->rotate(rotations.y, glm::vec3(1.0f, 0.0f, 0.0f));
	MV->rotate(rotations.x, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::applyLookAtMatrix(std::shared_ptr<MatrixStack> MV, glm::mat4 helicopter_matrix, float x, float y, float z)
{	
	glm::mat4 view = glm::inverse(helicopter_matrix);
	MV->translate(x, y, z);
	MV->rotate(glm::radians(-90.0f), 0, 1, 0);
	MV->multMatrix(view);
}