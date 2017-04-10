#pragma once
#ifndef __Helicopter__
#define __Helicopter__

#include <string>
#include <memory>

#include "MatrixStack.h"
#include "Shape.h"

class Helicopter {
public:
	Helicopter();
	~Helicopter();
	void init(std::string DIR, std::string body1, std::string body2, std::string prop1, std::string prop2);
	void draw(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV);
private:
	double t;
	Shape b1;
	Shape b2;
	Shape p1;
	Shape p2;

};

#endif
