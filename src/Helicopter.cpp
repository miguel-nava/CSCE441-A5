#include "Helicopter.h"
#include "Shape.h"
#include "Program.h"

//#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/quaternion.hpp>

Helicopter::Helicopter() {
	rotate_prop = false;
}

Helicopter::~Helicopter() {

}

void Helicopter::init(std::string DIR, std::string body1, std::string body2, std::string prop1, std::string prop2) {
	
	b1 = Shape();
	b1.loadMesh(DIR + body1);
	b1.init();

	b2 = Shape();
	b2.loadMesh(DIR + body2);
	b2.init();

	p1 = Shape();
	p1.loadMesh(DIR + prop1);
	p1.init();

	p2 = Shape();
	p2.loadMesh(DIR + prop2);
	p2.init();
}
void Helicopter::propRotate(bool rotate) {
	rotate_prop = rotate;
}
void Helicopter::draw(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV) {
	t = glfwGetTime();
	float theta;

	if (rotate_prop) {
		theta = (float)((int)(t * 360) % 360);
	} 
	else {
		theta = 0;
	}
	
	// Helicopter_prop1 
	MV->pushMatrix();
	MV->translate(0.0, 0.4819, 0.0);
	MV->rotate(glm::radians(theta), 0, 1, 0);
	MV->translate(0.0, -0.4819, 0.0);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	MV->popMatrix();
	p1.draw(prog);

	// Helicopter_prop2
	MV->pushMatrix();
	MV->translate(0.6228, 0.1179, 0.1365);
	MV->rotate(-glm::radians(theta), 0, 0, 1);
	MV->translate(-0.6228, -0.1179, -0.1365);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	MV->popMatrix();
	p2.draw(prog);

	// Draw the body of the helicopter
	MV->pushMatrix();
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	MV->popMatrix();
	b1.draw(prog);
	b2.draw(prog);
}
