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

glm::mat4 helicopter_matrix;
glm::mat4 Bcr;

vector<glm::vec3> cps;
vector<KeyFrame> keyframes;
vector<pair<float, float> > usTable;

float smax;

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

void buildTable()
{
	float max_u = cps.size() - 3; // this is because ther eare 8 points and so 8 - 3 = 5

	if (cps.size() >= 4) {
		glm::mat4 G;
		float step_size;

		// inserting first element on to the table which is 0,0
		usTable.push_back(make_pair(0.0f, 0.0f));

		float s = 0;
		for (int u = 0; u < max_u; u++) {
			step_size = 0.2;
			float u_a = 0;
			float u_b = 0;

			// re-establish the control points accordingly for the segments
			G[0] = glm::vec4(cps[u], 0);
			G[1] = glm::vec4(cps[u + 1], 0);
			G[2] = glm::vec4(cps[u + 2], 0);
			G[3] = glm::vec4(cps[u + 3], 0);

			while (u_a < 1) {
				u_b = u_a + step_size;

				glm::vec4 u_a_(1, u_a, u_a*u_a, u_a*u_a*u_a);
				glm::vec4 u_b_(1, u_b, u_b*u_b, u_b*u_b*u_b);

				glm::vec4 p_a = G*Bcr*u_a_;
				glm::vec4 p_b = G*Bcr*u_b_;

				s += glm::length(p_b - p_a);

				u_a += step_size;

				usTable.push_back(make_pair(u + u_a, s));
			}
		}
		smax = s;
	}
	else {
		usTable.clear();
	}
}

float s2u(float s)
{
	float alpha = 0; // alpha = (s - s0) / (s1 - s0)

	float s0 = 0;
	float s1 = 1;

	float u = 0;  // u = (1 - alpha)*u0 + alpha*u1
	float u0 = 0;
	float u1 = 0;

	// find s0 and s1
	for (int i = 0; i < usTable.size(); i++) {
		float currentU = usTable[i].first;
		float currentS = usTable[i].second;
		if (currentS > s) {
			u0 = usTable[i - 1].first;
			s0 = usTable[i - 1].second;
			u1 = currentU;
			s1 = currentS;
			alpha = (s - s0) / (s1 - s0);
			u = (1 - alpha)*u0 + alpha*u1;
			return u;
		}
	}
	return 0.0f;
	
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
	
	helicopter_matrix = glm::mat4();
	helicopter = make_shared<Helicopter>();
	helicopter->init(RESOURCE_DIR, "helicopter_body1.obj", "helicopter_body2.obj", "helicopter_prop1.obj", "helicopter_prop2.obj");

	//initialize the 7 keyframes & control points
	cps.push_back(glm::vec3(0, 0, 0));
	cps.push_back(glm::vec3(-2, 3, -3));
	cps.push_back(glm::vec3(-1.5, 6, -3));
	cps.push_back(glm::vec3(3, 1, 3));
	cps.push_back(glm::vec3(-6, 3, -3));
	cps.push_back(cps[0]);
	cps.push_back(cps[1]);
	cps.push_back(cps[2]);
	keyframes.push_back(KeyFrame(helicopter, cps[0], 31.0f, 0, 1, 0)); // first
	keyframes.push_back(KeyFrame(helicopter, cps[1], 165.0f, 1, 0, 0)); // second
	keyframes.push_back(KeyFrame(helicopter, cps[2], 185.0f, 0, 0, 1));
	keyframes.push_back(KeyFrame(helicopter, cps[3], 10.0f, 0, 1, 0));
	keyframes.push_back(KeyFrame(helicopter, cps[4], 200.0f, 0, 0, 1));
	keyframes.push_back(keyframes[0]);
	keyframes.push_back(keyframes[1]);
	keyframes.push_back(keyframes[2]);

	buildTable();

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
	int i = (int)floor(u);
	
	glm::mat4 Gp;
	Gp[0] = glm::vec4(cps[i], 0);
	Gp[1] = glm::vec4(cps[i + 1], 0);
	Gp[2] = glm::vec4(cps[i + 2], 0);
	Gp[3] = glm::vec4(cps[i + 3], 0);

	glm::mat4 Gq;
	glm::quat k1 = keyframes[i].getRot();
	glm::quat k2 = keyframes[i+1].getRot();
	if (glm::dot(k1, k2) < 0) k2 = -k2;
	glm::quat k3 = keyframes[i+2].getRot();
	if (glm::dot(k2, k3) < 0) k3 = -k3;
	glm::quat k4 = keyframes[i+3].getRot();
	if (glm::dot(k3, k4) < 0) k4 = -k4;
	Gq[0] = glm::vec4(k1.x, k1.y, k1.z, k1.w);
	Gq[1] = glm::vec4(k2.x, k2.y, k2.z, k2.w);
	Gq[2] = glm::vec4(k3.x, k3.y, k3.z, k3.w);
	Gq[3] = glm::vec4(k4.x, k4.y, k4.z, k4.w);

	u = fmod(u, 1.0);
	glm::vec4 uVec(1, u, u*u, u*u*u);
	glm::vec4 p = Gp*Bcr*uVec;
	
	glm::vec4 qVec = Gq * (Bcr * uVec);
	glm::quat q(qVec[3], qVec[0], qVec[1], qVec[2]); // (w, x, y, z)
	helicopter_matrix = glm::toMat4(glm::normalize(q));
	helicopter_matrix[3] = glm::vec4(p.x, p.y, p.z, 1.0f);

	
	MV->pushMatrix();
	MV->multMatrix(helicopter_matrix);
	helicopter->draw(prog, MV);
	MV->popMatrix();
}

void render()
{
	// Update time.
	double t = glfwGetTime();
	float tmax = 20;
	float tNorm = std::fmod(t, tmax)/(tmax+1);
	//float sNorm = tNorm;
	float sNorm = 117.03*tNorm*tNorm*tNorm*tNorm*tNorm - 335.24*tNorm*tNorm*tNorm*tNorm + 338.13*tNorm*tNorm*tNorm - 140.76*tNorm*tNorm + 20.838*tNorm - 4.1E-11;
	float s = smax*sNorm;
	float u = s2u(s);
	
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

	if (keyToggles[(unsigned)' ']) {  // you can click 'v' in order to change between two different lookAt() views
		if (keyToggles[(unsigned)'v'])
			camera->applyLookAtMatrix(MV, helicopter_matrix, 0, 0, 0.1);
		else
			camera->applyLookAtMatrix(MV, helicopter_matrix, 0, 0, -5);
	} 
	else
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

	// Draw grid
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glColor3f(0.66, 0.66, 0.66);
	glLineWidth(2);
	glBegin(GL_LINES);
	for (int i = -10; i < 10; i++) {
		glVertex3f(i, 0, -10);
		glVertex3f(i, 0, 10);
		glVertex3f(-10, 0, i);
		glVertex3f(10, 0, i);
	}
	glEnd();
	progSimple->unbind();

	if (keyToggles[(unsigned)'k']) {
		progSimple->bind();
		catmull_rom_spline();
		progSimple->unbind();
	}

	GLSL::checkError(GET_FILE_LINE);
	
	// Draw the Helicopters
	progNormal->bind();
	// Send projection matrix (same for all helicopters)
	glUniformMatrix4fv(progNormal->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	
	MV->pushMatrix();
	helicopter->propRotate(true);
	interpolate(progNormal, MV, u);
	if (keyToggles[(unsigned)'k'] || keyToggles[(unsigned)'K']) {
		
		for (int i = 0; i < keyframes.size(); i++) {
			keyframes[i].drawKeyFrame(progNormal, MV);
		}
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
