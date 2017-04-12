#pragma once
#ifndef __KEYFRAME__
#define __KEYfRAME__

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Helicopter.h"

class KeyFrame {
public:
	KeyFrame();
	KeyFrame(std::shared_ptr<Helicopter> h);
	KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p);
	KeyFrame(std::shared_ptr<Helicopter> h, float x, float y, float z);
	~KeyFrame();
	void setPos(glm::vec3 p);
	void setPos(float x, float y, float z);
	void drawKeyFrame(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV);
	glm::vec3 getPos();
private:
	glm::vec3 pos;
	Helicopter H;
};
#endif