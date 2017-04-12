#pragma once
#ifndef __KEYFRAME__
#define __KEYfRAME__

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Helicopter.h"

class KeyFrame {
public:
	KeyFrame();
	KeyFrame(std::shared_ptr<Helicopter> h);
	KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p);
	KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p, float degrees, glm::vec3 axis);
	KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p, float degrees, float rotx, float roty, float rotz);
	KeyFrame(std::shared_ptr<Helicopter> h, float x, float y, float z);
	KeyFrame(std::shared_ptr<Helicopter> h, float x, float y, float z, float degrees, glm::vec3 axis);
	KeyFrame(std::shared_ptr<Helicopter> h, float posx, float posy, float posz, float degrees, float rotx, float roty, float rotz);
	~KeyFrame();
	void setPos(glm::vec3 p);
	void setPos(float x, float y, float z);
	glm::vec3 getPos();
	void setRot(float degrees, glm::vec3 axis);
	void setRot(float degrees, float x, float y, float z);
	glm::quat getRot();
	void drawKeyFrame(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV);
	
private:
	glm::vec3 pos;
	glm::quat rot;
	Helicopter H;
};
#endif