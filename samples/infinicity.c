/** @file Draw an infinite city
 *
 * @author Eric Grant
 */

// Includes
//
#include "libkuhl.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Global Vars
//
static GLuint program = 0; /**< id value for the GLSL program */

static kuhl_geometry building[10][10];
static kuhl_geometry windows[10][10];
static kuhl_geometry buildingTop[10][10];
static kuhl_geometry windowsTop[10][10];
static float isComplex[10][10];
static kuhl_geometry roads;
static float shift = 0;
static int shiftBreak = 0;
static float hardSeed[2] = {69.83, 11.17};
static float camHeight = 3, camDist = -0.5, camAngle = -7, camSlide = 0;

//Math Helpers
//

/**
 * Copies vector 1 into vector 2
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vecCpy(float vec2[3], float vec1[3]){
    vec2[0] = vec1[0];
    vec2[1] = vec1[1];
    vec2[2] = vec1[2];
    return vec2;
}

/**
 * Computes vector from subbing vec2 from vec1
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vecSub(float vec3[3], float vec1[3], float vec2[3]){
    vec3[0] = vec1[0] - vec2[0];
    vec3[1] = vec1[1] - vec2[1];
    vec3[2] = vec1[2] - vec2[2];
    return vec3;
}

/**
 * Computes vector from adding vec2 to vec1
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vecAdd(float vec3[3], float vec1[3], float vec2[3]){
    vec3[0] = vec1[0] + vec2[0];
    vec3[1] = vec1[1] + vec2[1];
    vec3[2] = vec1[2] + vec2[2];
    return vec3;
}

/**
 * Normalizes vector with 3 dimensions
 * 
 * @param vec pixel location vector
 * @return normalized vector
 */
float * vecNormalize(float vec[3]){
    float magnitude = sqrt(pow(vec[0],2.0) + pow(vec[1], 2.0) + pow(vec[2],2.0));
    vec[0] = vec[0] / magnitude;
    vec[1] = vec[1] / magnitude;
    vec[2] = vec[2] / magnitude;
    return vec;
}

/**
 * Computes cross product of vec1 and vec2
 * 
 * @param vec3 output vector
 * @param vec1 left operand
 * @param vec2 right operand
 * @return result vector
 */
float * vecCrossProd(float vec3[3], float vec1[3], float vec2[3]){
	//         0  1  2
	// vec1 = [a, b ,c]
	// vec2 = [d, e, f]
	vec3[0] = (vec1[1]*vec2[2])-(vec1[2]*vec2[1]); //bf - ce (12 - 21)
	vec3[1] = (vec1[2]*vec2[0])-(vec1[0]*vec2[2]); //cd - af (20 - 02)
	vec3[2] = (vec1[0]*vec2[1])-(vec1[1]*vec2[0]); //ae - bd (01 - 10)
	return vec3;
}

/**
 * Computes dot product of two vectors
 * 
 * @param vec1 vector 1
 * @param vec2 vector 2
 * @return dot product of input vectors
 */
float vecDotProd(float vec1[3], float vec2[3]){
    return vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2];
}

/**
 * Computes normal given two vertices on a triangle
 * 
 * @param vec4 output vector
 * @param vec1 vector 1
 * @param vec2 vector 2
 * @param vec3 vector 3
 * @return result vector
 */
float * triNormal(float vec4[3], float vec1[3], float vec2[3], float vec3[3]){
	float temp1[3], temp2[3];
	vecSub(temp1, vec1, vec2);
	vecSub(temp2, vec1, vec3);
	vecCrossProd(vec4, temp1, temp2);
	vecNormalize(vec4);
	return vec4;
}

/**
 * Generates a random number within a given range
 * 
 * @param start start number
 * @param end end number
 * @return random number
*/
float randomRange(float start, float end){
	float n = drand48(); //0 <> 1
	n = n * (end-start); // 0 <> end-start
	n = n + start; // start <> end
	return n;
}

// Object Init
//

/**
 * create a building and every aspect of it
 * 
 * @param building building object
 * @param windows windows object
 * @param buildingTop top building object
 * @param windowsTop top building windows object
 * @param prog program being used
 * @param seed seed used in random gen
 * @return isComplex bool
 */
int init_geometryBuilding(kuhl_geometry *building, kuhl_geometry *windows, kuhl_geometry *buildingTop, kuhl_geometry *windowsTop, GLuint prog, long seed)
{
	//seed random
	srand48(seed);

	//get dimensions
	float w = randomRange(0.4,0.8); //width
	float h = randomRange(0.8,2.2); //height
	float bc = 0.6; //base color
	float sh = 0; //starting height

	float ws = 0.13; //window size
	float wp = 0.02; //window padding (bottom and left)
	float wo = 0.001; //window outwards

	//Draw main building
	kuhl_geometry_new(building, prog, 16, GL_TRIANGLES);

	//verticies
	GLfloat vertexPositions[] = {
		0, sh+0, 0, //front wall
		w, sh+0, 0,
		0, sh+h, 0,
		w, sh+h, 0,
		0, sh+0, 0, //left wall
		0, sh+0, -w,
		0, sh+h, 0,
		0, sh+h, -w,
		w, sh+0, 0, //right wall
		w, sh+0, -w,
		w, sh+h, 0,
		w, sh+h, -w,
		0, sh+h, 0, //roof
		w, sh+h, 0,
		0, sh+h, -w,
		w, sh+h, -w
	};
	kuhl_geometry_attrib(building, vertexPositions, 3, "in_Position", KG_WARN);

	//color
	GLfloat colorData[] = {
		bc, bc, bc, //front wall
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc, //left wall
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc, //right wall
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc, //roof
		bc, bc, bc,
		bc, bc, bc,
		bc, bc, bc
	};
	kuhl_geometry_attrib(building, colorData, 3, "in_Color", KG_WARN);

	//normals
	GLfloat normalData[] = {
		0, 0, 1, //front
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		1, 0, 0, //left
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0, //right
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		0, 1, 0, //roof
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};
	kuhl_geometry_attrib(building, normalData, 3, "in_Normal", KG_WARN);

	//indices
	GLuint indexData[] = { 
		0, 1, 2,  
		1, 2, 3,
		4, 5, 6,
		5, 6, 7,
		8, 9, 10,
		9, 10, 11,
		12, 13, 14,
		13, 14, 15 
	};
	kuhl_geometry_indices(building, indexData, 24);

	kuhl_errorcheck();

	//Draw windows
	//
	int hw = (int)floor(w / (ws+wp)); //horizontal windows
	int vw = (int)floor(h / (ws+wp)); //vertical windows
	int tw = (int)(hw * vw); //total windows
	kuhl_geometry_new(windows, prog, tw*4*3, GL_TRIANGLES);

	GLfloat wVertexPositions[tw*4*3*3];
	GLfloat wNormalData[tw*4*3*3];
	GLfloat wColorData[tw*4*3*3];
	GLuint wIndexData[tw*4*3*3/2];
	int n1 = 0;
	int n2 = 0;
	int n3 = 0;
	float lit = 0;
	float cv[3]; //color value

	//front
	for (float i = (w/2)-(hw*(ws+wp)/2)+(wp/2); i < w-ws; i += ws+wp) {
		for (float j = wp; j < h-ws; j += ws+wp) {
			//verticies
			wVertexPositions[n1+0] = i; //bottom left
			wVertexPositions[n1+1] = sh+j;
			wVertexPositions[n1+2] = wo;
			wVertexPositions[n1+3] = i+ws; //bottom right
			wVertexPositions[n1+4] = sh+j;
			wVertexPositions[n1+5] = wo;
			wVertexPositions[n1+6] = i; //top left
			wVertexPositions[n1+7] = sh+j+ws;
			wVertexPositions[n1+8] = wo;
			wVertexPositions[n1+9] = i+ws; //top right
			wVertexPositions[n1+10] = sh+j+ws;
			wVertexPositions[n1+11] = wo;
			
			//normals
			wNormalData[n1+0] = 0; //bottom left
			wNormalData[n1+1] = 0;
			wNormalData[n1+2] = 1;
			wNormalData[n1+3] = 0; //bottom right
			wNormalData[n1+4] = 0;
			wNormalData[n1+5] = 1;
			wNormalData[n1+6] = 0; //top left
			wNormalData[n1+7] = 0;
			wNormalData[n1+8] = 1;
			wNormalData[n1+9] = 0; //top right
			wNormalData[n1+10] = 0;
			wNormalData[n1+11] = 1;

			//color
			lit = randomRange(0,1);
			if (lit > 0.5) {
				cv[0] = 0.5;
				cv[1] = 0.5;
				cv[2] = 0;
			} else {
				cv[0] = 0;
				cv[1] = 0;
				cv[2] = 0;
			}
			wColorData[n1+0] = cv[0]; //bottom left
			wColorData[n1+1] = cv[1];
			wColorData[n1+2] = cv[2];
			wColorData[n1+3] = cv[0]; //bottom right
			wColorData[n1+4] = cv[1];
			wColorData[n1+5] = cv[2];
			wColorData[n1+6] = cv[0]; //top left
			wColorData[n1+7] = cv[1];
			wColorData[n1+8] = cv[2];
			wColorData[n1+9] = cv[0]; //top right
			wColorData[n1+10] = cv[1];
			wColorData[n1+11] = cv[2];

			n1+=12;

			//indices
			wIndexData[n2+0] = n3+0;
			wIndexData[n2+1] = n3+1;
			wIndexData[n2+2] = n3+2;
			wIndexData[n2+3] = n3+1;
			wIndexData[n2+4] = n3+2;
			wIndexData[n2+5] = n3+3;
			n2 += 6;
			n3 += 4;
		}
	}

	//left
	for (float i = (w/2)-(hw*(ws+wp)/2)+(wp/2); i < w-ws; i += ws+wp) {
		for (float j = wp; j < h-ws; j += ws+wp) {
			//verticies
			wVertexPositions[n1+0] = -wo; //bottom left
			wVertexPositions[n1+1] = sh+j;
			wVertexPositions[n1+2] = -i;
			wVertexPositions[n1+3] = -wo; //bottom right
			wVertexPositions[n1+4] = sh+j;
			wVertexPositions[n1+5] = -i-ws;
			wVertexPositions[n1+6] = -wo; //top left
			wVertexPositions[n1+7] = sh+j+ws;
			wVertexPositions[n1+8] = -i;
			wVertexPositions[n1+9] = -wo; //top right
			wVertexPositions[n1+10] = sh+j+ws;
			wVertexPositions[n1+11] = -i-ws;
			
			//normals
			wNormalData[n1+0] = 1; //bottom left
			wNormalData[n1+1] = 0;
			wNormalData[n1+2] = 0;
			wNormalData[n1+3] = 1; //bottom right
			wNormalData[n1+4] = 0;
			wNormalData[n1+5] = 0;
			wNormalData[n1+6] = 1; //top left
			wNormalData[n1+7] = 0;
			wNormalData[n1+8] = 0;
			wNormalData[n1+9] = 1; //top right
			wNormalData[n1+10] = 0;
			wNormalData[n1+11] = 0;

			//color
			lit = randomRange(0,1);
			if (lit > 0.5) {
				cv[0] = 0.5;
				cv[1] = 0.5;
				cv[2] = 0;
			} else {
				cv[0] = 0;
				cv[1] = 0;
				cv[2] = 0;
			}
			wColorData[n1+0] = cv[0]; //bottom left
			wColorData[n1+1] = cv[1];
			wColorData[n1+2] = cv[2];
			wColorData[n1+3] = cv[0]; //bottom right
			wColorData[n1+4] = cv[1];
			wColorData[n1+5] = cv[2];
			wColorData[n1+6] = cv[0]; //top left
			wColorData[n1+7] = cv[1];
			wColorData[n1+8] = cv[2];
			wColorData[n1+9] = cv[0]; //top right
			wColorData[n1+10] = cv[1];
			wColorData[n1+11] = cv[2];

			n1+=12;

			//indices
			wIndexData[n2+0] = n3+0;
			wIndexData[n2+1] = n3+1;
			wIndexData[n2+2] = n3+2;
			wIndexData[n2+3] = n3+1;
			wIndexData[n2+4] = n3+2;
			wIndexData[n2+5] = n3+3;
			n2 += 6;
			n3 += 4;
		}
	}

	//left
	for (float i = (w/2)-(hw*(ws+wp)/2)+(wp/2); i < w-ws; i += ws+wp) {
		for (float j = wp; j < h-ws; j += ws+wp) {
			//verticies
			wVertexPositions[n1+0] = w+wo; //bottom left
			wVertexPositions[n1+1] = sh+j;
			wVertexPositions[n1+2] = -i;
			wVertexPositions[n1+3] = w+wo; //bottom right
			wVertexPositions[n1+4] = sh+j;
			wVertexPositions[n1+5] = -i-ws;
			wVertexPositions[n1+6] = w+wo; //top left
			wVertexPositions[n1+7] = sh+j+ws;
			wVertexPositions[n1+8] = -i;
			wVertexPositions[n1+9] = w+wo; //top right
			wVertexPositions[n1+10] = sh+j+ws;
			wVertexPositions[n1+11] = -i-ws;
			
			//normals
			wNormalData[n1+0] = 1; //bottom left
			wNormalData[n1+1] = 0;
			wNormalData[n1+2] = 0;
			wNormalData[n1+3] = 1; //bottom right
			wNormalData[n1+4] = 0;
			wNormalData[n1+5] = 0;
			wNormalData[n1+6] = 1; //top left
			wNormalData[n1+7] = 0;
			wNormalData[n1+8] = 0;
			wNormalData[n1+9] = 1; //top right
			wNormalData[n1+10] = 0;
			wNormalData[n1+11] = 0;

			//color
			lit = randomRange(0,1);
			if (lit > 0.5) {
				cv[0] = 0.5;
				cv[1] = 0.5;
				cv[2] = 0;
			} else {
				cv[0] = 0;
				cv[1] = 0;
				cv[2] = 0;
			}
			wColorData[n1+0] = cv[0]; //bottom left
			wColorData[n1+1] = cv[1];
			wColorData[n1+2] = cv[2];
			wColorData[n1+3] = cv[0]; //bottom right
			wColorData[n1+4] = cv[1];
			wColorData[n1+5] = cv[2];
			wColorData[n1+6] = cv[0]; //top left
			wColorData[n1+7] = cv[1];
			wColorData[n1+8] = cv[2];
			wColorData[n1+9] = cv[0]; //top right
			wColorData[n1+10] = cv[1];
			wColorData[n1+11] = cv[2];

			n1+=12;

			//indices
			wIndexData[n2+0] = n3+0;
			wIndexData[n2+1] = n3+1;
			wIndexData[n2+2] = n3+2;
			wIndexData[n2+3] = n3+1;
			wIndexData[n2+4] = n3+2;
			wIndexData[n2+5] = n3+3;
			n2 += 6;
			n3 += 4;
		}
	}

	kuhl_geometry_attrib(windows, wVertexPositions, 3, "in_Position", KG_WARN);
	kuhl_geometry_attrib(windows, wNormalData, 3, "in_Normal", KG_WARN);
	kuhl_geometry_attrib(windows, wColorData, 3, "in_Color", KG_WARN);
	kuhl_geometry_indices(windows, wIndexData, tw*4*3*3/2);

	int isComplex = 0;
	if (randomRange(0,1) > 0.5) {
		isComplex = 1;

		//get dimensions
		sh = h;
		float oldW = w;
		w = randomRange(ws+wp,oldW);
		if (w+0.15 < oldW) {
			w+=0.15;
		}
		h = randomRange(0.5,1.5);
		float fud = randomRange(0,(oldW-w)/2);

		//Draw main building
		kuhl_geometry_new(buildingTop, prog, 16, GL_TRIANGLES);

		//verticies
		GLfloat vertexPositions[] = {
			fud+0, sh+0, -fud+0, //front wall
			fud+w, sh+0, -fud+0,
			fud+0, sh+h, -fud+0,
			fud+w, sh+h, -fud+0,
			fud+0, sh+0, -fud+0, //left wall
			fud+0, sh+0, -fud+-w,
			fud+0, sh+h, -fud+0,
			fud+0, sh+h, -fud+-w,
			fud+w, sh+0, -fud+0, //right wall
			fud+w, sh+0, -fud+-w,
			fud+w, sh+h, -fud+0,
			fud+w, sh+h, -fud+-w,
			fud+0, sh+h, -fud+0, //roof
			fud+w, sh+h, -fud+0,
			fud+0, sh+h, -fud+-w,
			fud+w, sh+h, -fud+-w
		};
		kuhl_geometry_attrib(buildingTop, vertexPositions, 3, "in_Position", KG_WARN);

		//color
		GLfloat colorData[] = {
			bc, bc, bc, //front wall
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc, //left wall
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc, //right wall
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc, //roof
			bc, bc, bc,
			bc, bc, bc,
			bc, bc, bc
		};
		kuhl_geometry_attrib(buildingTop, colorData, 3, "in_Color", KG_WARN);

		//normals
		GLfloat normalData[] = {
			0, 0, 1, //front
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			1, 0, 0, //left
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
			1, 0, 0, //right
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
			0, 1, 0, //roof
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};
		kuhl_geometry_attrib(buildingTop, normalData, 3, "in_Normal", KG_WARN);

		//indices
		GLuint indexData[] = { 
			0, 1, 2,  
			1, 2, 3,
			4, 5, 6,
			5, 6, 7,
			8, 9, 10,
			9, 10, 11,
			12, 13, 14,
			13, 14, 15 
		};
		kuhl_geometry_indices(buildingTop, indexData, 24);

		kuhl_errorcheck();

		//Draw windows
		//
		hw = (int)floor(w / (ws+wp)); //horizontal windows
		vw = (int)floor(h / (ws+wp)); //vertical windows
		tw = (int)(hw * vw); //total windows
		kuhl_geometry_new(windowsTop, prog, tw*4*3, GL_TRIANGLES);

		GLfloat wVertexPositions[tw*4*3*3];
		GLfloat wNormalData[tw*4*3*3];
		GLfloat wColorData[tw*4*3*3];
		GLuint wIndexData[tw*4*3*3/2];
		int n1 = 0;
		int n2 = 0;
		int n3 = 0;
		float lit = 0;
		float cv[3]; //color value

		//front
		for (float i = (w/2)-(hw*(ws+wp)/2)+(wp/2); i < w-ws; i += ws+wp) {
			for (float j = wp; j < h-ws; j += ws+wp) {
				//verticies
				wVertexPositions[n1+0] = fud+i; //bottom left
				wVertexPositions[n1+1] = sh+j;
				wVertexPositions[n1+2] = -fud+wo;
				wVertexPositions[n1+3] = fud+i+ws; //bottom right
				wVertexPositions[n1+4] = sh+j;
				wVertexPositions[n1+5] = -fud+wo;
				wVertexPositions[n1+6] = fud+i; //top left
				wVertexPositions[n1+7] = sh+j+ws;
				wVertexPositions[n1+8] = -fud+wo;
				wVertexPositions[n1+9] = fud+i+ws; //top right
				wVertexPositions[n1+10] = sh+j+ws;
				wVertexPositions[n1+11] = -fud+wo;
				
				//normals
				wNormalData[n1+0] = 0; //bottom left
				wNormalData[n1+1] = 0;
				wNormalData[n1+2] = 1;
				wNormalData[n1+3] = 0; //bottom right
				wNormalData[n1+4] = 0;
				wNormalData[n1+5] = 1;
				wNormalData[n1+6] = 0; //top left
				wNormalData[n1+7] = 0;
				wNormalData[n1+8] = 1;
				wNormalData[n1+9] = 0; //top right
				wNormalData[n1+10] = 0;
				wNormalData[n1+11] = 1;

				//color
				lit = randomRange(0,1);
				if (lit > 0.5) {
					cv[0] = 0.5;
					cv[1] = 0.5;
					cv[2] = 0;
				} else {
					cv[0] = 0;
					cv[1] = 0;
					cv[2] = 0;
				}
				wColorData[n1+0] = cv[0]; //bottom left
				wColorData[n1+1] = cv[1];
				wColorData[n1+2] = cv[2];
				wColorData[n1+3] = cv[0]; //bottom right
				wColorData[n1+4] = cv[1];
				wColorData[n1+5] = cv[2];
				wColorData[n1+6] = cv[0]; //top left
				wColorData[n1+7] = cv[1];
				wColorData[n1+8] = cv[2];
				wColorData[n1+9] = cv[0]; //top right
				wColorData[n1+10] = cv[1];
				wColorData[n1+11] = cv[2];

				n1+=12;

				//indices
				wIndexData[n2+0] = n3+0;
				wIndexData[n2+1] = n3+1;
				wIndexData[n2+2] = n3+2;
				wIndexData[n2+3] = n3+1;
				wIndexData[n2+4] = n3+2;
				wIndexData[n2+5] = n3+3;
				n2 += 6;
				n3 += 4;
			}
		}

		//left
		for (float i = (w/2)-(hw*(ws+wp)/2)+(wp/2); i < w-ws; i += ws+wp) {
			for (float j = wp; j < h-ws; j += ws+wp) {
				//verticies
				wVertexPositions[n1+0] = fud-wo; //bottom left
				wVertexPositions[n1+1] = sh+j;
				wVertexPositions[n1+2] = -fud-i;
				wVertexPositions[n1+3] = fud-wo; //bottom right
				wVertexPositions[n1+4] = sh+j;
				wVertexPositions[n1+5] = -fud-i-ws;
				wVertexPositions[n1+6] = fud-wo; //top left
				wVertexPositions[n1+7] = sh+j+ws;
				wVertexPositions[n1+8] = -fud-i;
				wVertexPositions[n1+9] = fud-wo; //top right
				wVertexPositions[n1+10] = sh+j+ws;
				wVertexPositions[n1+11] = -fud-i-ws;
				
				//normals
				wNormalData[n1+0] = 1; //bottom left
				wNormalData[n1+1] = 0;
				wNormalData[n1+2] = 0;
				wNormalData[n1+3] = 1; //bottom right
				wNormalData[n1+4] = 0;
				wNormalData[n1+5] = 0;
				wNormalData[n1+6] = 1; //top left
				wNormalData[n1+7] = 0;
				wNormalData[n1+8] = 0;
				wNormalData[n1+9] = 1; //top right
				wNormalData[n1+10] = 0;
				wNormalData[n1+11] = 0;

				//color
				lit = randomRange(0,1);
				if (lit > 0.5) {
					cv[0] = 0.5;
					cv[1] = 0.5;
					cv[2] = 0;
				} else {
					cv[0] = 0;
					cv[1] = 0;
					cv[2] = 0;
				}
				wColorData[n1+0] = cv[0]; //bottom left
				wColorData[n1+1] = cv[1];
				wColorData[n1+2] = cv[2];
				wColorData[n1+3] = cv[0]; //bottom right
				wColorData[n1+4] = cv[1];
				wColorData[n1+5] = cv[2];
				wColorData[n1+6] = cv[0]; //top left
				wColorData[n1+7] = cv[1];
				wColorData[n1+8] = cv[2];
				wColorData[n1+9] = cv[0]; //top right
				wColorData[n1+10] = cv[1];
				wColorData[n1+11] = cv[2];

				n1+=12;

				//indices
				wIndexData[n2+0] = n3+0;
				wIndexData[n2+1] = n3+1;
				wIndexData[n2+2] = n3+2;
				wIndexData[n2+3] = n3+1;
				wIndexData[n2+4] = n3+2;
				wIndexData[n2+5] = n3+3;
				n2 += 6;
				n3 += 4;
			}
		}

		//right
		for (float i = (w/2)-(hw*(ws+wp)/2)+(wp/2); i < w-ws; i += ws+wp) {
			for (float j = wp; j < h-ws; j += ws+wp) {
				//verticies
				wVertexPositions[n1+0] = fud+w+wo; //bottom left
				wVertexPositions[n1+1] = sh+j;
				wVertexPositions[n1+2] = -fud-i;
				wVertexPositions[n1+3] = fud+w+wo; //bottom right
				wVertexPositions[n1+4] = sh+j;
				wVertexPositions[n1+5] = -fud-i-ws;
				wVertexPositions[n1+6] = fud+w+wo; //top left
				wVertexPositions[n1+7] = sh+j+ws;
				wVertexPositions[n1+8] = -fud-i;
				wVertexPositions[n1+9] = fud+w+wo; //top right
				wVertexPositions[n1+10] = sh+j+ws;
				wVertexPositions[n1+11] = -fud-i-ws;
				
				//normals
				wNormalData[n1+0] = 1; //bottom left
				wNormalData[n1+1] = 0;
				wNormalData[n1+2] = 0;
				wNormalData[n1+3] = 1; //bottom right
				wNormalData[n1+4] = 0;
				wNormalData[n1+5] = 0;
				wNormalData[n1+6] = 1; //top left
				wNormalData[n1+7] = 0;
				wNormalData[n1+8] = 0;
				wNormalData[n1+9] = 1; //top right
				wNormalData[n1+10] = 0;
				wNormalData[n1+11] = 0;

				//color
				lit = randomRange(0,1);
				if (lit > 0.5) {
					cv[0] = 0.5;
					cv[1] = 0.5;
					cv[2] = 0;
				} else {
					cv[0] = 0;
					cv[1] = 0;
					cv[2] = 0;
				}
				wColorData[n1+0] = cv[0]; //bottom left
				wColorData[n1+1] = cv[1];
				wColorData[n1+2] = cv[2];
				wColorData[n1+3] = cv[0]; //bottom right
				wColorData[n1+4] = cv[1];
				wColorData[n1+5] = cv[2];
				wColorData[n1+6] = cv[0]; //top left
				wColorData[n1+7] = cv[1];
				wColorData[n1+8] = cv[2];
				wColorData[n1+9] = cv[0]; //top right
				wColorData[n1+10] = cv[1];
				wColorData[n1+11] = cv[2];

				n1+=12;

				//indices
				wIndexData[n2+0] = n3+0;
				wIndexData[n2+1] = n3+1;
				wIndexData[n2+2] = n3+2;
				wIndexData[n2+3] = n3+1;
				wIndexData[n2+4] = n3+2;
				wIndexData[n2+5] = n3+3;
				n2 += 6;
				n3 += 4;
			}
		}

		kuhl_geometry_attrib(windowsTop, wVertexPositions, 3, "in_Position", KG_WARN);
		kuhl_geometry_attrib(windowsTop, wNormalData, 3, "in_Normal", KG_WARN);
		kuhl_geometry_attrib(windowsTop, wColorData, 3, "in_Color", KG_WARN);
		kuhl_geometry_indices(windowsTop, wIndexData, tw*4*3*3/2);
	}

	return isComplex;
}

/**
 * create the roads object with textures
 * 
 */
void init_geometryRoads(){
	kuhl_geometry_new(&roads, program, 4, GL_TRIANGLES);
	GLfloat vertexPositions[] = {
		0, 0, 0,
		10, 0, 0,
		10, 0, 10,
		0, 0, 10
	};
	kuhl_geometry_attrib(&roads, vertexPositions, 3, "in_Position", KG_WARN);
	GLuint indexData[] = { 
		0, 1, 2,  
		0, 2, 3
	};
	kuhl_geometry_indices(&roads, indexData, 6);
	GLfloat colorData[] = {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
		1, 1, 1
	};
	kuhl_geometry_attrib(&roads, colorData, 3, "in_Color", KG_WARN);
	GLfloat texcoordData[] = {
		0, 0,
		10, 0,
		10, 10,
		0, 10
	};
	kuhl_geometry_attrib(&roads, texcoordData, 2, "in_TexCoord", KG_WARN);
	GLuint texId = 0;
	kuhl_read_texture_file_wrap("../images/road.png", &texId, GL_REPEAT, GL_REPEAT);
	kuhl_geometry_texture(&roads, texId, "tex", KG_WARN);
}

/**
 * Generate a unique seed
 * 
 * @param cP column point
 * @param rP row point
 * @return seed value
 */
long getSeed(int cP, int rP){
	long outP;
	float temp[] = {(float)cP+0.525, (float)rP+0.164};
	outP = (long)(vecNf_dot(temp, hardSeed, 2)*1000);
	return outP;
}

/**
 * Creates the initial pool of buildings
 * 
 */
void init_buildings(){
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {	
			long seedNum = getSeed(i, j);
			isComplex[i][j] = init_geometryBuilding(&building[i][j], &windows[i][j], &buildingTop[i][j], &windowsTop[i][j], program, seedNum);
		}
	}
}

/**
 * Adds a far away row of buildings
 * 
 */
void addRowFar(){
	//delete current close row
	for (int n = 0; n < 10; n++) {
		kuhl_geometry_delete(&building[n][0]);
		kuhl_geometry_delete(&windows[n][0]);
		if (isComplex[n][0] == 1) {
			kuhl_geometry_delete(&buildingTop[n][0]);
			kuhl_geometry_delete(&windowsTop[n][0]);
		}
	}
	//move rows back
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 9; j++) {
			building[i][j] = building[i][j+1];
			windows[i][j] = windows[i][j+1];
			buildingTop[i][j] = buildingTop[i][j+1];
			windowsTop[i][j] = windowsTop[i][j+1];
			isComplex[i][j] = isComplex[i][j+1];
		}
	}
	//add new far row
	for (int n = 0; n < 10; n++) {
		long seedNum = getSeed(n, 9-shiftBreak);
		isComplex[n][9] = init_geometryBuilding(&building[n][9], &windows[n][9], &buildingTop[n][9], &windowsTop[n][9], program, seedNum);
	}
}

/**
 * Adds a near row of buildings
 * 
 */
void addRowNear(){
	//delete current near row
	for (int n = 0; n < 10; n++) {
		kuhl_geometry_delete(&building[n][9]);
		kuhl_geometry_delete(&windows[n][9]);
		if (isComplex[n][9] == 1) {
			kuhl_geometry_delete(&buildingTop[n][9]);
			kuhl_geometry_delete(&windowsTop[n][9]);
		}
	}
	//move rows forward
	for (int i = 9; i >= 0; i--) {
		for (int j = 9; j > 0; j--) {
			building[i][j] = building[i][j-1];
			windows[i][j] = windows[i][j-1];
			buildingTop[i][j] = buildingTop[i][j-1];
			windowsTop[i][j] = windowsTop[i][j-1];
			isComplex[i][j] = isComplex[i][j-1];
		}
	}
	//add new back row
	for (int n = 0; n < 10; n++) {
		long seedNum = getSeed(n, -shiftBreak);
		isComplex[n][0] = init_geometryBuilding(&building[n][0], &windows[n][0], &buildingTop[n][0], &windowsTop[n][0], program, seedNum);
	}
}

// Input
//

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (kuhl_keyboard_handler(window, key, scancode, action, mods))
		return;
	if(action == GLFW_PRESS || action == GLFW_REPEAT) {
		if(key == GLFW_KEY_SPACE)
		{
			shift -= 0.1;
		}
		if(key == GLFW_KEY_B)
		{
			shift += 0.1;
		}
		//debug keys
		if(key == GLFW_KEY_S)
		{
			camDist -= 0.1;
		}
		if(key == GLFW_KEY_W)
		{
			camDist += 0.1;
		}
		if(key == GLFW_KEY_A)
		{
			camSlide -= 0.1;
		}
		if(key == GLFW_KEY_D)
		{
			camSlide += 0.1;
		}
		if(key == GLFW_KEY_Q)
		{
			camHeight -= 0.1;
		}
		if(key == GLFW_KEY_E)
		{
			camHeight += 0.1;
		}
		if(key == GLFW_KEY_R)
		{
			camAngle -= 0.1;
		}
		if(key == GLFW_KEY_F)
		{
			camAngle += 0.1;
		}
	}
}

// Drawing
//

void display()
{
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	viewmat_begin_frame();
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		//view port
		viewmat_begin_eye(viewportID);
		int viewport[4];
		viewmat_get_viewport(viewport, viewportID);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(.2,.2,.2,0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		// view & projection matrix
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);
		mat4f_lookat_new(viewMat, camSlide, camHeight, shift+camDist, 0, 0, camAngle+shift, 0, 1, 0);

		//when a new city row needs to be loaded
		if (floor(shift) < shiftBreak) {
			//we have moved forward one row
			shiftBreak = (int)floor(shift);
			addRowFar();
		} else if (floor(shift) > shiftBreak) {
			//we have moved back one row
			shiftBreak = (int)floor(shift);
			addRowNear();
		}

		// model view
		float modelview[16];
		mat4f_mult_mat4f_many(modelview, viewMat, NULL);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value

		//draw geometry
		//
		float transMat[16];
		mat4f_translate_new(transMat, -5, 0, -9.8+shiftBreak);
		mat4f_mult_mat4f_many(modelview, viewMat, transMat, NULL);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
							1, // number of 4x4 float matrices
							0, // transpose
							modelview); // value
		kuhl_geometry_draw(&roads);

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {	
			mat4f_translate_new(transMat, i-4.8, 0, -j+shiftBreak);
			mat4f_mult_mat4f_many(modelview, viewMat, transMat, NULL);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
							1, // number of 4x4 float matrices
							0, // transpose
							modelview); // value
			kuhl_errorcheck();
			kuhl_geometry_draw(&building[i][j]);
			kuhl_geometry_draw(&windows[i][j]);
				if (isComplex[i][j] == 1) {
					kuhl_geometry_draw(&buildingTop[i][j]);
					kuhl_geometry_draw(&windowsTop[i][j]);
				}
			}
		}
		

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

// Main
//

int main(int argc, char** argv)
{
	//init
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	program = kuhl_create_program("infinicity.vert", "infinicity.frag");
	glUseProgram(program);
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on config file. */

	//values here are ignored after the viewmat is replaced in display
	static float initCamPos[3]  = {0,3,1}; // location of camera
	static float initCamLook[3] = {0,0,-6}; // a point the camera is facing at
	static float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);

	//print help
	printf("Move camera with 'space' and 'b'.\n");

	//create objects
	init_buildings();
	init_geometryRoads(&roads, program);

	//main loop
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
