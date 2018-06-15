#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;
GLuint programCubeTexture;
GLuint programModifiedAsteroidsTexture;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;

glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;
glm::vec3 cameraSide;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);
glm::vec3 rotationChangeXYZ = glm::vec3(0, 0, 0);

GLuint textureAsteroid;

GLuint cubemapTexture;

std::vector<glm::vec4> planets;

int t = 0;

std::vector<glm::vec3> shipPath;


class Particle
{
private:
	float width;
	float height;
	float depth;
public:

	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;

	Particle(glm::vec3 pos, float width, float height, float depth) {
		this->pos = pos;
		this->width = width;
		this->height = height;
		this->depth = depth;
	}

	float getMinX() {
		return std::min(pos.x, pos.x + width);
	}

	float getMinY() {
		return std::min(pos.y, pos.y + height);
	}

	float getMinZ() {
		return std::min(pos.z, pos.z + depth);
	}

	float getMaxX() {
		return std::max(pos.x, pos.x + width);
	}

	float getMaxY() {
		return std::max(pos.y, pos.y + height);
	}

	float getMaxZ() {
		return std::max(pos.z, pos.z + depth);
	}

	bool intersect(Particle  b) {
		//printf("A X%d\n", pos.x);
		//printf("A Y%d\n", pos.y);
 
			return (getMinX() <= b.getMaxX() && getMaxX() >= b.getMinX()) &&
			(getMinY() <= b.getMaxY() && getMaxY() >= b.getMinY()) &&
			(getMinZ() <= b.getMaxZ() && getMaxZ() >= b.getMinZ());
	}
};


std::vector<Particle> spaceships;
const float cubeVertices[] = {
	30.5f, 30.5f, 30.5f, 1.0f,
	30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,

	30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,

	30.5f, 30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,
	30.5f, -30.5f, -30.5f, 1.0f,

	30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,

	-30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,

	-30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,

	30.5f, 30.5f, 30.5f, 1.0f,
	30.5f, -30.5f, -30.5f, 1.0f,
	30.5f, -30.5f, 30.5f, 1.0f,

	30.5f, 30.5f, 30.5f, 1.0f,
	30.5f, 30.5f, -30.5f, 1.0f,
	30.5f, -30.5f, -30.5f, 1.0f,

	30.5f, 30.5f, -30.5f, 1.0f,
	30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,

	30.5f, 30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,

	30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
	30.5f, -30.5f, 30.5f, 1.0f,

	30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
};


void keyboard(unsigned char key, int x, int y)
{

	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch (key)
	{
	case 'z': rotationChangeXYZ.z += 0.1f; break;
	case 'x': rotationChangeXYZ.z -= 0.1f; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	static int lastMouseX = x;
	static int lastMouseY = y;
	rotationChangeXYZ.y = 0.1f*(x - lastMouseX);
	rotationChangeXYZ.x = 0.1f*(y - lastMouseY);
	lastMouseX = x;
	lastMouseY = y;
}

glm::mat4 createCameraMatrix()
{
	glm::quat rotationChange = glm::quat(rotationChangeXYZ);
	rotation = rotationChange * rotation;
	rotation = glm::normalize(rotation);
	rotationChangeXYZ = glm::vec3(0);
	cameraDir = glm::inverse(rotation) * glm::vec3(0, 0, -1);
	cameraSide = glm::inverse(rotation) * glm::vec3(1, 0, 0);

	return Core::createViewMatrixQuat(cameraPos, rotation);
}



std::vector<glm::vec3> generatePoints(glm::vec4 startingPoint, glm::vec4 endingPoint, glm::vec4 startingVector, glm::vec4 endingVector, float diff) {
	std::vector<glm::vec3> points;
	for (float i = 0; i < 1; i += diff) {
		glm::vec3 hermiteVec = glm::hermite(startingPoint, endingPoint, startingVector, endingVector, i);
		points.push_back(hermiteVec);
		printf("hermite vect %f %f %f\n", hermiteVec.x, hermiteVec.y, hermiteVec.z);
	}
	printf("size %d generated", points.size());
	return points;
}



void drawSkybox() {
	GLuint program = programCubeTexture;
	glUseProgram(program);

	Core::SetActiveTexture(cubemapTexture, "cubeSampler", program, 0);
	glm::mat4 cubeMatrix = glm::translate(glm::vec3(0) /** glm::scale(glm::vec3(planets[i].w)*/);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * cubeMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "cubeViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	Core::DrawVertexArray(cubeVertices, 36, 4);

	glUseProgram(0);

}


void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);


	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	
	Core::DrawModel(model);


	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void drawObjectWithSkyBoxTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programModifiedAsteroidsTexture;

	glUseProgram(program);

	Core::SetActiveTexture(textureId, "skybox", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Core::DrawModel(model);
	glDisable(GL_BLEND);

	glUseProgram(0);
}

void handleCollisions() {
	glm::vec3 v2 = glm::vec3(0);
	for (int i = 0; i < spaceships.size(); i++) {
		Particle shipA = spaceships[i];
		int shipX = shipA.pos.x;
		int shipY = shipA.pos.y;
		int shipZ = shipA.pos.z;
		for (int j = 0; j < spaceships.size(); j++) {
			Particle shipB = spaceships[j];
			if (shipA.pos != shipB.pos) {
				bool colliding = shipA.intersect(shipB);
				if (colliding) {
					glm::vec3 bounceOffA = v2 - (shipB.pos - shipA.pos) * 0.01;
					glm::vec3 bounceOffB = v2 - (shipA.pos - shipB.pos) * 0.01;
					shipA.vel += bounceOffA;
					shipA.pos += bounceOffA;
					shipB.vel += bounceOffB;
					shipB.pos += bounceOffB;
					//printf("%s\n", "true");
				}
				else {
					//printf("%s\n", "false");
				}
			}
		}
		glm::mat4 enemyMatrix = glm::translate(shipA.pos) * glm::scale(glm::vec3(0.25f));
		drawObjectColor(&shipModel, enemyMatrix, glm::vec3(0.6f));
	}
}

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	glm::mat4 id;

	
	drawSkybox();

	//glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.25f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	//glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * shipInitialTransformation;
	//drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	
	glm::mat4 shipModelMatrix = glm::translate(shipPath[t % shipPath.size()]) * glm::rotate(glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	//printf("shipPath current y: %d\n", shipPath[t % shipPath.size()].y);
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	
	for (int i = 0; i < planets.size(); i++)
	{
		glm::mat4 planetModelMatrix = glm::translate(glm::vec3(planets[i])) * glm::scale(glm::vec3(planets[i].w));
		drawObjectWithSkyBoxTexture(&sphereModel, planetModelMatrix, cubemapTexture);
	}
	
		glm::vec3 sum = glm::vec3(0);
	for (int j = 0; j < spaceships.size(); j++)
	{
		sum += spaceships[j].vel;
	}

	for (int i = 0; i < spaceships.size(); i++)
	{
		float weightV1 = 0.01;
		float weightV2 = 0.0011;
		float weightV3 = 0.01;
		glm::vec3 v3 = glm::vec3(0);

		glm::vec3 v1Attract = glm::normalize(shipPath[t % shipPath.size()] - spaceships[i].pos);
		//glm::vec3 v2Separation = separationV2(spaceships[i]);
		glm::vec3 v3Alignment = ((sum / spaceships.size()) - spaceships[i].vel);
		spaceships[i].vel += (weightV1 * v1Attract) + /*(weightV2 * v2Separation) +*/ (weightV3 * v3Alignment);
		spaceships[i].pos += spaceships[i].vel; 
		//printf("x%d\n", spaceships[i].pos.x);
		//printf("y%d\n", spaceships[i].pos.y);
	} 
	handleCollisions();
	//moveObject(&shipModel,shipPath);
	t++;


	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programCubeTexture = shaderLoader.CreateProgram("shaders/shader_cube_tex.vert", "shaders/shader_cube_tex.frag");
	programModifiedAsteroidsTexture = shaderLoader.CreateProgram("shaders/shader_asteroids_tex.vert", "shaders/shader_asteroids_tex.frag");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	textureAsteroid = Core::LoadTexture("textures/asteroid2.png");
	cubemapTexture = Core::setupCubeMap("textures/xpos.png", "textures/xneg.png", "textures/ypos.png", "textures/yneg.png", "textures/zpos.png", "textures/zneg.png");
	
	for (int i = 0; i < 10; i++)
	{
		glm::vec3 position = glm::ballRand(30.0f);
		float scale = glm::linearRand(0.5f, 5.0f);
		planets.push_back(glm::vec4(position, scale));
	}
	shipPath = generatePoints(glm::vec4(0, 0, 0, 0), glm::vec4(0, 0, 2, 0), glm::vec4(0, 2 , 0, 0), glm::vec4(3, -7, 0, 0), 0.005);
	printf("shipPath size: %d", shipPath.size());
	// initialization
	const int n = 150;
/*
	RigidBody * body[n];
	double timeRigid = t;
	double dt = 0.005;
	for (int i = 0; i < n; ++i) {
		RigidBody::Function force = RigidBody::Function(2.0, glm::vec3(0,0,0), glm::quat(0,0,0);
		RigidBody::Function torque(2, vec3(0, 0, 0), quat(0, 0, 0);
		// Set the initial state of the rigid bodies .
		body[i] = new RigidBody(1,mat3(2,2,2), force, torque);
		// Part of the physics update step .
		for (i = 0; i < n; ++i) {
			body[i].Update(t, dt);
			t += dt;
		}*/
	int shipWidth = 5;
	int shipHeight = 5;
	int shipDepth = 5;
	for (int i = 1; i <= 102; i++) {
		Particle enemy(glm::vec3(1 + i / 15, 1, 1), shipWidth, shipHeight, shipDepth);
		spaceships.push_back(enemy);
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(programCubeTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("CGP");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
