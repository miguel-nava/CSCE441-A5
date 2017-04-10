#pragma once
#ifndef __KEYFRAME__
#define __KEYfRAME__

#include <glm/glm.hpp>
#include "Helicopter.h"

class KeyFrame {
public:
	KeyFrame();
	~KeyFrame();
	void setPos(glm::vec3 p);
	void setPos(float x, float y, float z);
	void drawKeyFrame(shared_ptr<Program> prog, shared_ptr<MatrixStack> MV);
private:
	glm::vec3 pos;
	Helicopter H;
};
#endif