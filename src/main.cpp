#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Helicopter.h"
#include "KeyFrame.h"

#define M_PI       3.14159265358979323846   // pi

using namespace std;

bool keyToggles[256] = {false}; // only for English keyboards!

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from

shared_ptr<Program> progNormal;
shared_ptr<Program> progSimple;
shared_ptr<Camera> camera;
shared_ptr<Helicopter> helicopter;

glm::mat4 Bcr;
vector<KeyFrame> keyframes;
vector<glm::vec3> cps;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl = mods & GLFW_MOD_CONTROL;
		bool alt = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	keyToggles[(unsigned)'c'] = true;

	Bcr[0] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
	Bcr[1] = glm::vec4(-1.0f, 0.0f, 1.0f, 0.0f);
	Bcr[2] = glm::vec4(2.0f, -5.0f, 4.0f, -1.0f);
	Bcr[3] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
	Bcr *= 0.5;
	
	// For drawing the Helicopter
	progNormal = make_shared<Program>();
	progNormal->setShaderNames(RESOURCE_DIR + "normal_vert.glsl", RESOURCE_DIR + "normal_frag.glsl");
	progNormal->setVerbose(true);
	progNormal->init();
	progNormal->addUniform("P");
	progNormal->addUniform("MV");
	progNormal->addAttribute("aPos");
	progNormal->addAttribute("aNor");
	progNormal->setVerbose(false);
	
	// For drawing the frames & grid
	progSimple = make_shared<Program>();
	progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	progSimple->setVerbose(true);
	progSimple->init();
	progSimple->addUniform("P");
	progSimple->addUniform("MV");
	progSimple->setVerbose(false);
	
	helicopter = make_shared<Helicopter>();
	helicopter->init(RESOURCE_DIR, "helicopter_body1.obj", "helicopter_body2.obj", "helicopter_prop1.obj", "helicopter_prop2.obj");

	//initialize the 7 keyframes & control points
	cps.push_back(glm::vec3(0, 0, 0));
	cps.push_back(glm::vec3(-1.5, 1, 1));
	cps.push_back(glm::vec3(-0.5, 2, -1));
	cps.push_back(glm::vec3(1, 1, 1));
	cps.push_back(glm::vec3(2, 1, -1));
	cps.push_back(cps[0]);
	cps.push_back(cps[1]);
	cps.push_back(cps[2]);
	keyframes.push_back(KeyFrame(helicopter, cps[0]));
	keyframes.push_back(KeyFrame(helicopter, cps[1]));
	keyframes.push_back(KeyFrame(helicopter, cps[2]));
	keyframes.push_back(KeyFrame(helicopter, cps[3]));
	keyframes.push_back(KeyFrame(helicopter, cps[4]));
	keyframes.push_back(KeyFrame(helicopter, cps[5]));
	keyframes.push_back(KeyFrame(helicopter, cps[6]));
	keyframes.push_back(KeyFrame(helicopter, cps[7]));

	camera = make_shared<Camera>();
	
	// Initialize time.
	glfwSetTime(0.0);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}
void catmull_rom_spline() {

	glm::mat4 G;
	int ncps = cps.size();
	float u;
	float stepsize = 0.01;
	
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	// will jump for every four points
	for (int i = 0; i < ncps - 3; i += 1) {

		G[0] = glm::vec4(cps[i], 0);
		G[1] = glm::vec4(cps[i + 1], 0);
		G[2] = glm::vec4(cps[i + 2], 0);
		G[3] = glm::vec4(cps[i + 3], 0);
		u = 0;
		while (u <= 1) {
			if (u > 1) u = 1;
			glm::vec4 ubar(1, u, u*u, u*u*u);
			glm::vec4 p = G*(Bcr*ubar);
			glVertex3f(p.x, p.y, p.z);
			u += stepsize;
		}
	}
	glEnd();
}
void interpolate(shared_ptr<Program> prog, shared_ptr<MatrixStack> MV, float u) {
	glm::mat4 G;
	G[0] = glm::vec4(cps[(int)floor(u)], 0);
	G[1] = glm::vec4(cps[(int)floor(u) + 1], 0);
	G[2] = glm::vec4(cps[(int)floor(u) + 2], 0);
	G[3] = glm::vec4(cps[(int)floor(u) + 3], 0);
	u = fmod(u, 1.0);
	glm::vec4 uVec(1, u, u*u, u*u*u);
	glm::vec4 p = G*Bcr*uVec;
	MV->pushMatrix();
	MV->translate(p.x, p.y, p.z);
	helicopter->draw(prog, MV);
	MV->popMatrix();
}
void render()
{
	// Update time.
	double t = glfwGetTime();
	// Alpha is the linear interpolation parameter between 0 and 1
	float alpha = std::fmod(0.5f*t, 1.0f);
	float umax = keyframes.size() - 3;
	float u = std::fmod(t, umax);
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Use the window size for camera.
	glfwGetWindowSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'l']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();

	camera->applyViewMatrix(MV);
	
	// Draw origin frame
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
	glLineWidth(1);

	// Draw grid
	glColor3f(0.66, 0.66, 0.66);
	glBegin(GL_LINES);
	for (int i = -5; i < 5; i++) {
		glVertex3f(i, 0, -5);
		glVertex3f(i, 0, 5);
		glVertex3f(-5, 0, i);
		glVertex3f(5, 0, i);
	}
	glEnd();
	progSimple->unbind();
	GLSL::checkError(GET_FILE_LINE);
	
	// Draw the bunnies
	progNormal->bind();
	// Send projection matrix (same for all bunnies)
	glUniformMatrix4fv(progNormal->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	

	
	// The axes of rotatio for the source and target bunnies
	glm::vec3 axis0, axis1;
	axis0.x = keyToggles[(unsigned)'x'] ? 1.0 : 0.0f;
	axis0.y = keyToggles[(unsigned)'y'] ? 1.0 : 0.0f;
	axis0.z = keyToggles[(unsigned)'z'] ? 1.0 : 0.0f;
	axis1.x = keyToggles[(unsigned)'X'] ? 1.0 : 0.0f;
	axis1.y = keyToggles[(unsigned)'Y'] ? 1.0 : 0.0f;
	axis1.z = keyToggles[(unsigned)'Z'] ? 1.0 : 0.0f;

	glm::quat q0, q1;
	if(glm::length(axis0) == 0) {
		q0 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	} else {
		axis0 = glm::normalize(axis0);
		q0 = glm::angleAxis((float)(90.0f/180.0f*M_PI), axis0);
	}
	if(glm::length(axis1) == 0) {
		q1 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	} else {
		axis1 = glm::normalize(axis1);
		q1 = glm::angleAxis((float)(90.0f/180.0f*M_PI), axis1);
	}
	MV->scale(1);

	MV->pushMatrix();
	helicopter->draw(progNormal, MV);
	if (keyToggles[(unsigned)'k'] || keyToggles[(unsigned)'K']) {
		helicopter->propRotate(true);
		
		catmull_rom_spline();
		
		for (int i = 0; i < keyframes.size(); i++) {
			keyframes[i].drawKeyFrame(progNormal, MV);
		}

		interpolate(progNormal, MV, u);
	}
	else {
		helicopter->propRotate(false);
		helicopter->draw(progNormal, MV);
	}
	MV->popMatrix();

	progNormal->unbind();
	// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
