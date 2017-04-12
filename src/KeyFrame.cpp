#include "KeyFrame.h"

#define M_PI       3.14159265358979323846   // pi


KeyFrame::KeyFrame() {
	pos = glm::vec3(0, 0, 0);
	rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h) {
	H = *h;
	pos = glm::vec3(0, 0, 0);
	rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p) {
	H = *h;
	pos = p;
	rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p, float degrees, glm::vec3 axis) {
	H = *h;
	pos = p;
	rot = glm::angleAxis((float)(90.0f / 180.0f*M_PI), axis);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, glm::vec3 p, float degrees, float rotx, float roty, float rotz) {
	H = *h;
	pos = p;
	rot = glm::angleAxis((float)(90.0f / 180.0f*M_PI), glm::vec3(rotx, roty, rotz));
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, float x, float y, float z) {
	H = *h;
	pos = glm::vec3(x, y, z);
	rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, float x, float y, float z, float degrees, glm::vec3 axis) {
	H = *h;
	pos = glm::vec3(x, y, z);
	rot = glm::angleAxis((float)(90.0f / 180.0f*M_PI), axis);
}
KeyFrame::KeyFrame(std::shared_ptr<Helicopter> h, float posx, float posy, float posz, float degrees, float rotx, float roty, float rotz) {
	H = *h;
	pos = glm::vec3(posx, posy, posz);
	rot = glm::angleAxis((float)(90.0f / 180.0f*M_PI), glm::vec3(rotx, roty, rotz));
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
void KeyFrame::setRot(float degrees, glm::vec3 axis) {
	rot = glm::angleAxis((float)(90.0f / 180.0f*M_PI), axis);
}
void KeyFrame::setRot(float degrees, float x, float y, float z) {
	rot = glm::angleAxis((float)(90.0f / 180.0f*M_PI), glm::vec3(x,y,z));
}
glm::quat KeyFrame::getRot() {
	return rot;
}
void KeyFrame::drawKeyFrame(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV) {
	MV->pushMatrix();
	MV->translate(pos);
	MV->multMatrix(glm::toMat4(rot));
	H.draw(prog, MV);
	MV->popMatrix();
}
