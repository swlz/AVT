//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: Jo�o Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "avtFreeType.h"

// classes added by students
#include "Camera.h"
#include "Rover.h"
#include "Rocks.h"
#include <time.h>
#include "Texture_Loader.h"

using namespace std;

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2

int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with meshes
vector<struct MyMesh> terrain;
vector<struct MyMesh> static_objects;
vector<struct MyMesh> dynamic_objects;
Rover rover;
vector<Rocks> rocks;
vector<float> static_x_pos;
vector<float> static_y_pos;
vector<float> dynamic_x_pos;
vector<float> dynamic_y_pos;
float terrain_x = 40.0f;
float terrain_y = 40.0f;
float max_terrain = 19.0f;
float min_terrain = -19.0f;
int range = max_terrain - min_terrain + 1;
float max_angle = 180.0f;
float min_angle = -180.0f;
int range_angle = max_angle - min_angle + 1;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId[NUMBER_POINT_LIGHTS];
GLint sPos_uniformId[NUMBER_SPOT_LIGHTS];
GLint sDir_uniformId[NUMBER_SPOT_LIGHTS];
GLint sCut_uniformId[NUMBER_SPOT_LIGHTS];
GLint dPos_uniformId;
GLint lnum_point_light, lnum_spot_light;
GLuint tex_loc, tex_loc1, tex_loc2;

// Cameras
Camera cameras[3];
int activeCamera = 0;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
//float lightPos[4] = { 4.0f, 6.0f, 2.0f, 1.0f };


float dirLight[4]{ 1.0f, 1000.0f, 1.0f, 0.0f };
float pointLights[NUMBER_POINT_LIGHTS][4]= { {-0.0f, 20.0f, -0.0f, 1.0f},
				{-0.5f, 20.0f, -0.5f, 1.0f},
					{-1.0f, 20.0f, -1.0f, 1.0f},
					{1.0f, 20.0f, 1.0f, 1.0f},
					{0.5f, 20.0f, 0.5f, 1.0f},
					{1.5f, 20.0f, 1.5f, 1.0f}
};
float spotLights[NUMBER_SPOT_LIGHTS][4] = { 
	{rover.position[0][0]+0.3, rover.position[0][1]+0.2, rover.position[0][2]+0.9, 1.0f},
{rover.position[0][0]-0.3, rover.position[0][1]+0.2, rover.position[0][2]+0.9, 1.0f}
};

bool spot_enabled = true;
bool point_enabled = true;
bool direct_enabled = true;

int prev_time = 0;

// ------------------------------------------------------------
//
// collision
//
bool checkForCollisionWithRover(Rover rover, MyMesh object) {

	if (rover.position[0][0] + rover.max_pos[0] >= object.position[0] &&
		object.position[0] + object.max_pos_vert[0] >= rover.position[0][0]) {
		//printf("coll 1\n");
		if (rover.position[0][1] + rover.max_pos[1] >= object.position[1] &&
			object.position[1] + object.max_pos_vert[1] >= rover.position[0][1]) {
			//printf("coll 2\n");
		}
	}

	if (rover.position[0][0] + rover.min_pos[0] >= object.position[0] &&
		object.position[0] + object.min_pos_vert[0] >= rover.position[0][0]) {
		//printf("coll 3\n");
		if (rover.position[0][1] + rover.min_pos[1] >= object.position[1] &&
			object.position[1] + object.min_pos_vert[1] >= rover.position[0][1]) {
			//printf("coll 4\n");
		}
	}
	return true;
}

unsigned char* loadImageIntoArray(const char* strFileName, int width, int height) {
	FILE* image_file;
	image_file = fopen(strFileName, "rb");

	if (image_file == NULL) {
		printf("Image file not found!\n");
	}
	int channels = 3;
	unsigned char* out = (unsigned char*) malloc(channels*width*height);
	fread(out, channels*width*height, 1, image_file);
	fclose(image_file);

	return out;
}

void animate(int value) {
	int elapsed_time = glutGet(GLUT_ELAPSED_TIME);
	int DELTA_T = elapsed_time - prev_time;
	prev_time = elapsed_time;

	rover.direction[0] = cos(rover.angle) * 0 - sin(rover.angle) * 1;
	rover.direction[2] = sin(rover.angle) * 0 - cos(rover.angle) * 1;

	rover.position[0][0] += rover.direction[0] * rover.speed * DELTA_T;

	rover.position[0][2] += rover.direction[2] * rover.speed * DELTA_T;

	glutPostRedisplay();
	glutTimerFunc(1000 * DELTA_T, animate, 0);
}

void rolling_rocks_animate(int value) {
	int elapsed_time = glutGet(GLUT_ELAPSED_TIME);
	int DELTA_T = elapsed_time - prev_time;
	prev_time = elapsed_time;

	for (int i = 0; i < rocks.size(); i++) {
		if (DELTA_T >= 30) {
			rocks[i].speed += 0.001;
		}
		rocks[i].direction[0] = cos(rocks[i].angle) * 0 - sin(rocks[i].angle) * 1;
		rocks[i].direction[2] = sin(rocks[i].angle) * 0 - cos(rocks[i].angle) * 1;

		rocks[i].position[0] += rocks[i].direction[0] * rocks[i].speed * DELTA_T;
		rocks[i].position[2] += rocks[i].direction[0] * rocks[i].speed * DELTA_T;
		if (rocks[i].position[0] > 19.9 || rocks[i].position[0] < -19.9
			|| rocks[i].position[2] > 19.9 || rocks[i].position[2] < -19.9) {
			rocks[i].position[0] = (rand() % range + min_terrain) + 0.9;
			rocks[i].position[2] = (rand() % range + min_terrain) + 0.9;
			rocks[i].angle = (rand() % range_angle + min_angle);
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1000 * DELTA_T, rolling_rocks_animate, 0);
}

void timer(int value)
{
	std::ostringstream oss;
	oss << "MARS ROVER" << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	//PUT YOUR CODE HERE
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
}


// ------------------------------------------------------------
//
// Render stufff
//

void drawWheels() {
	GLint loc;
	float angle = 0;
	for (int i = 1; i < 5; i++) {
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, rover.body[i].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, rover.body[i].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, rover.body[i].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, rover.body[i].mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, rover.position[i][0], rover.position[i][1], rover.position[i][2]);
		if (i == 1 || i == 2) {
			angle = rover.angle;
			if (rover.angle > 0.6) {
				angle = 0.6;
			}
			if (rover.angle < -0.6) {
				angle = -0.6;
			}
			rotate(MODEL, (-angle * 180) / 3.14, 0, 1, 0);
		}
		if (i == 3 || i == 4) {
			angle = rover.angle;
			if (rover.angle > 0.6) {
				angle = 0.6;
			}
			if (rover.angle < -0.6) {
				angle = -0.6;
			}
			rotate(MODEL, (angle * 180) / 3.14, 0, 1, 0);
		}

		rotate(MODEL, -90, 1, 0, 0);
		rotate(MODEL, 90, 0, 0, 1);


		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(rover.body[i].vao);

		glDrawElements(rover.body[i].type, rover.body[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		popMatrix(MODEL);
	}
}

void drawBody() {
	GLint loc;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, rover.body[0].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, rover.body[0].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, rover.body[0].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, rover.body[0].mat.shininess);
	
	pushMatrix(MODEL);
	translate(MODEL, rover.position[0][0], rover.position[0][1], rover.position[0][2]);
	rotate(MODEL, (rover.angle * 180) / 3.14, 0, 1, 0);
	pushMatrix(MODEL);
	translate(MODEL, -0.5, -0.5, -0.5);

	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(rover.body[0].vao);

	glDrawElements(rover.body[0].type, rover.body[0].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
	translate(MODEL, 0, -0.5, 0);
	drawWheels();
	popMatrix(MODEL);
}

void drawTerrain() {
	GLint loc;

	// send the material
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, terrain[0].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, terrain[0].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, terrain[0].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, terrain[0].mat.shininess);
	const char* strFileName = "mars.png";
	int image_width = 1133;
	int image_height = 566;
	unsigned int* tex_mars_terrain;
	GLuint terrain_texture;

	Texture2D_Loader(&tex_loc, strFileName, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_loc);
	glUniform1i(tex_loc, 1);

	pushMatrix(MODEL);
	rotate(MODEL, -90, 1, 0, 0);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(terrain[0].vao);

	glDrawElements(terrain[0].type, terrain[0].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);
}

void drawStaticObject() {
	GLint loc;
	for (int i = 0; i < static_objects.size(); ++i) {
		static_objects[i].position[0] = static_x_pos[i];
		static_objects[i].position[1] = 0.0f;
		static_objects[i].position[2] = static_y_pos[i];
		static_objects[i].position[3] = 1.0f;
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, static_objects[i].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, static_objects[i].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, static_objects[i].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, static_objects[i].mat.shininess);
		pushMatrix(MODEL);
		translate(MODEL, static_objects[i].position[0], static_objects[i].position[1], static_objects[i].position[2]);


		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(static_objects[i].vao);

		glDrawElements(static_objects[i].type, static_objects[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

	}
}

void drawRocks() {
	GLint loc;
	for (int i = 0; i < rocks.size(); ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, rocks[i].amesh[0].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, rocks[i].amesh[0].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, rocks[i].amesh[0].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, rocks[i].amesh[0].mat.shininess);
		pushMatrix(MODEL);
		translate(MODEL, rocks[i].position[0], rocks[i].position[1], rocks[i].position[2]);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(rocks[i].amesh[0].vao);

		glDrawElements(rocks[i].amesh[0].type, rocks[i].amesh[0].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

	}
}


void renderScene(void) {
	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	lookAt(cameras[activeCamera].camPos[0] + 0.01, cameras[activeCamera].camPos[1], cameras[activeCamera].camPos[2], cameras[activeCamera].camTarget[0],
		cameras[activeCamera].camTarget[1], cameras[activeCamera].camTarget[2], 0, 1, 0);

	// use our shader

	glUseProgram(shader.getProgramIndex());

	glutTimerFunc(0, animate, 0);
	glutTimerFunc(0, rolling_rocks_animate, 0);

	//send the light position in eye coordinates

	float res[4];
	GLfloat pointLights_ambient[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat pointLights_diffuse[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat pointLights_specular[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float counter = 0;
	for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
		multMatrixPoint(VIEW, pointLights[i], res);
		glUniform4fv(lPos_uniformId[i], 1, res);

		glUniform4f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("pointLights[" + to_string(i) + "].materials.ambient").c_str()), (pointLights_ambient[0] + counter) * 0.1f, (pointLights_ambient[1] + counter) * 0.1f, (pointLights_ambient[2] + counter) * 0.1f, pointLights_ambient[3]);
		glUniform4f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("pointLights[" + to_string(i) + "].materials.diffuse").c_str()), (pointLights_diffuse[0] + counter), (pointLights_diffuse[1] + counter), (pointLights_diffuse[2] + counter), pointLights_diffuse[3]);
		glUniform4f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("pointLights[" + to_string(i) + "].materials.specular").c_str()), (pointLights_specular[0] + counter), (pointLights_specular[1] + counter), (pointLights_specular[2] + counter), pointLights_specular[3]);
		glUniform1f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("pointLights[" + to_string(i) + "].materials.shininess").c_str()), 100.0f);
		counter += 0.1f;
		glLightfv(GL_LIGHT1 + i, GL_AMBIENT, pointLights_ambient);
		glLightfv(GL_LIGHT1 + i, GL_DIFFUSE, pointLights_diffuse);
		glLightfv(GL_LIGHT1 + i, GL_SPECULAR, pointLights_specular);
		glLightfv(GL_LIGHT1 + i, GL_POSITION, pointLights[i]);
	}

	GLfloat spotLights_ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat spotLights_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat spotLights_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
		multMatrixPoint(VIEW, spotLights[i], res);
		glUniform4fv(sPos_uniformId[i], 1, res);
		multMatrixPoint(VIEW, rover.direction, res);
		glUniform4fv(sDir_uniformId[i], 1, res);
		glUniform1f(sCut_uniformId[i], 0.2f);

		glUniform4f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].materials.ambient").c_str()), spotLights_ambient[0], spotLights_ambient[1], spotLights_ambient[2], spotLights_ambient[3]);
		glUniform4f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].materials.diffuse").c_str()), spotLights_diffuse[0], spotLights_diffuse[1], spotLights_diffuse[2], spotLights_diffuse[3]);
		glUniform4f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].materials.specular").c_str()), spotLights_specular[0], spotLights_specular[1], spotLights_specular[2], spotLights_specular[3]);
		glUniform1f(glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].materials.shininess").c_str()), 100.0f);

		glLightfv(GL_LIGHT2 + i, GL_AMBIENT, spotLights_ambient);
		glLightfv(GL_LIGHT2 + i, GL_DIFFUSE, spotLights_diffuse);
		glLightfv(GL_LIGHT2 + i, GL_SPECULAR, spotLights_specular);
		glLightfv(GL_LIGHT2 + i, GL_POSITION, spotLights[i]);
	
	}
	multMatrixPoint(VIEW, dirLight, res);
	glUniform4fv(dPos_uniformId, 1, res);
	GLfloat dirLight_ambient[4] = {0.3f, 0.24f, 0.14f, 1.0f};
	GLfloat dirLight_diffuse[4] = {0.7f, 0.42f, 0.26f, 1.0f};
	GLfloat dirLight_specular[4]= {0.5f, 0.5f, 0.5f, 1.0f};
	glUniform4f(glGetUniformLocation(shader.getProgramIndex(), "dirLight.materials.ambient"), dirLight_ambient[0], dirLight_ambient[1], dirLight_ambient[2], dirLight_ambient[3]);
	glUniform4f(glGetUniformLocation(shader.getProgramIndex(), "dirLight.materials.diffuse"), dirLight_diffuse[0], dirLight_diffuse[1], dirLight_diffuse[2], dirLight_diffuse[3]);
	glUniform4f(glGetUniformLocation(shader.getProgramIndex(), "dirLight.materials.specular"), dirLight_specular[0], dirLight_specular[1], dirLight_specular[2], dirLight_specular[3]);
	glUniform1f(glGetUniformLocation(shader.getProgramIndex(), "dirLight.materials.shininess"), 100.0f);

	glLightfv(GL_LIGHT0, GL_AMBIENT, dirLight_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dirLight_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, dirLight_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, dirLight);	

	

	//for (int i = 0; i < static_objects.size(); i++) {
	//	checkForCollisionWithRover(rover, static_objects[i]);
	//}

	// camera follows rover
	float dist = 5;
	float height = cameras[2].camPos[1];
	cameras[2].camPos[0] = rover.position[0][0] - (rover.direction[0] * dist);
	cameras[2].camPos[1] = rover.position[0][1] - (rover.direction[1] * dist);
	cameras[2].camPos[2] = rover.position[0][2] - (rover.direction[2] * dist);
	cameras[2].camTarget[0] = rover.position[0][0];
	cameras[2].camTarget[1] = rover.position[0][1];
	cameras[2].camTarget[2] = rover.position[0][2];


	pushMatrix(MODEL);
	drawTerrain();
	pushMatrix(MODEL);
	drawBody();
	popMatrix(MODEL);
	drawRocks();
	drawStaticObject();
	popMatrix(MODEL);
	
	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	if (cameras[activeCamera].type == 0) {
		float ratio = 1.0f;
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
	if (cameras[activeCamera].type == 1) {
		ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	}
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

	case '1':
		activeCamera = 0;
		break;
	case '2':
		activeCamera = 1;
		break;
	case '3':
		activeCamera = 2;
		break;
	case 'p':
		rover.angle -= 0.1f; 
		break;
	case 'o':
		rover.angle += 0.1f;
		break;
	case 'q':
		rover.speed += 0.0001f;
		break;
	case 'a':
		rover.speed -= 0.0001f;
		break;

	case 27:
		glutLeaveMainLoop();
		break;

	case 'c':
		//printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		if (point_enabled == false) {
			for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
				glEnable(GL_LIGHT1 + i);
			}
			
			point_enabled = true;
		}
		
		if (point_enabled == true) {
			for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
				glDisable(GL_LIGHT1 + i);
			}
			point_enabled = false;
		}
		break;
	case 'h':
		if (spot_enabled == false) {
			for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
				glEnable(GL_LIGHT2 + i);
			}
			spot_enabled = true;
		}

		if (spot_enabled == true) {
			for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
				glDisable(GL_LIGHT2 + i);
			}
			spot_enabled = false;
		}
		break;
	case 'm': glEnable(GL_MULTISAMPLE); break;
	case 'n': glDisable(GL_MULTISAMPLE); break;
	case 'N': 
		if (direct_enabled == false) {
			glEnable(GL_LIGHT0);
			direct_enabled = true;
		}
		if (direct_enabled == true) {
			glDisable(GL_LIGHT0);
			direct_enabled = false;
		}
		break;
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

// wieder einkommentieren
	cameras[activeCamera].camPos[0] = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	cameras[activeCamera].camPos[2] = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	cameras[activeCamera].camPos[1] = rAux * sin(betaAux * 3.14f / 180.0f);

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	cameras[activeCamera].camPos[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cameras[activeCamera].camPos[2] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cameras[activeCamera].camPos[1] = r * sin(beta * 3.14f / 180.0f);

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight_phong.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight_phong.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
		lPos_uniformId[i] = glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("pointLights[" + to_string(i) + "].position").c_str());
	}
	for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
		sPos_uniformId[i] = glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].position").c_str());
		sDir_uniformId[i] = glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].direction").c_str());
		sCut_uniformId[i] = glGetUniformLocation(shader.getProgramIndex(), (const GLchar*)("spotLights[" + to_string(i) + "].cutoff").c_str());
	}
	dPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "dirLight.direction");
	
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	//tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	//tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	MyMesh amesh;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	// set the camera position based on its spherical coordinates
	cameras[activeCamera].camPos[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cameras[activeCamera].camPos[2] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cameras[activeCamera].camPos[1] = r * sin(beta * 3.14f / 180.0f);


	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;


	// create geometry and VAO of quad

	amesh = createQuad(terrain_x, terrain_y);
	float diff_terrain[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff_terrain, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	terrain.push_back(amesh);


	float amb_cone[] = { 0.4f, 0.15f, 0.1f, 1.0f };
	float diff_cone[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec_cone[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissiv_cone[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// create geometry and VAO of the Cone
	for (int i = 0; i < 10; i++) {
		amesh = createCone(1.5f, 0.5f, 20);
		memcpy(amesh.mat.ambient, amb_cone, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff_cone, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec_cone, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissiv_cone, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		static_objects.push_back(amesh);
	}

// create rover
	float amb_rover[] = { 0.2f, 0.1f, 0.1f, 1.0f };
	float diff_rover[] = { 0.3f, 0.3f, 0.2f, 1.0f };
	float spec_rover[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive_rover[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	amesh = createCube();
	memcpy(amesh.mat.ambient, amb_rover, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff_rover, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec_rover, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive_rover, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	rover.body.push_back(amesh);

	float innerRadius = 0.03f;
	float outerRadius = 0.3f;
	int rings = 20;
	int sides = 5;
	for (int i = 1; i < 5; i++) {
		amesh = createTorus(innerRadius, outerRadius, rings, sides);
		memcpy(amesh.mat.ambient, amb_rover, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff_rover, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec_rover, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive_rover, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		rover.body.push_back(amesh);
	}
	// get max and min position of rover
	rover.max_pos[0] = rover.body[0].max_pos_vert[0]; 
	rover.max_pos[1] = rover.body[0].max_pos_vert[1];
	rover.max_pos[2] = rover.body[0].max_pos_vert[2];

	rover.min_pos[0] = rover.body[0].min_pos_vert[0];
	rover.min_pos[1] = rover.body[0].min_pos_vert[1];
	rover.min_pos[2] = rover.body[0].min_pos_vert[2];

	for (int i = 0; i < 40; i++) {
		amesh = createSphere(0.2, 3);
		memcpy(amesh.mat.ambient, amb_rover, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff_rover, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec_rover, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive_rover, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		Rocks new_rock;
		new_rock.amesh.push_back(amesh);
		new_rock.position[0] = (rand() % range + min_terrain) + 0.9;
		new_rock.position[2] = (rand() % range + min_terrain) + 0.9;
		new_rock.angle = (rand() % range_angle + min_angle);
		rocks.push_back(new_rock);
	}


	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
		glEnable(GL_LIGHT1 + i);
	}
	for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
		glEnable(GL_LIGHT2 + i);
	}
	
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {


	// init cameras
	// top
	cameras[0].camPos[1] = 20;

	// top ortho
	cameras[1].camPos[1] = 20;
	cameras[1].type = 1;


	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(3, 3); //version prof provided was 4.3
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow("MARS ROVER");


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	for (int i = 0; i < static_objects.size(); i++) {
		//TODO check case 0,0
		static_x_pos.push_back((rand() % range + min_terrain) + 0.9);
		static_y_pos.push_back((rand() % range + min_terrain) + 0.9);
	}

	//  GLUT main loop
	glutMainLoop();

	return(0);
}


