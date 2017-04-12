#include "KeyFrame.h"


KeyFrame::KeyFrame() {
	pos = glm::vec3(0, 0, 0);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h) {
	H = *h;
	pos = glm::vec3(0, 0, 0);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p) {
	H = *h;
	pos = p;
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, float x, float y, float z) {
	H = *h;
	pos = glm::vec3(x, y, z);
}
KeyFrame::~KeyFrame() {

}
void KeyFrame::setPos(glm::vec3 p) {
	pos = p;
}
void KeyFrame::setPos(float x, float y, float z) {
	glm::vec3 p(x, y, z);
	setPos(p);
}
glm::vec3 KeyFrame::getPos() {
	return pos;
}
void KeyFrame::drawKeyFrame(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV) {
	MV->pushMatrix();
	MV->translate(pos);
	H.draw(prog, MV);
	MV->popMatrix();
}
