/******************************************************************************
 * @file	common.h
 * @brief   include third-party libraries
 *          common functions shared by all the projects
 *****************************************************************************/

#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM is a header only library. Hence, there is nothing to build to use it. To use GLM, a
//programmer only has to include <glm/glm.hpp> in his program. This include provides
//all the GLSL features implemented by GLM.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#if defined (__linux__)
# define MAX_PATH	256
# define swprintf_s	swprintf
# define printf_s	printf
#endif

#if defined(_WIN32) && defined(_DEBUG)
# include <crtdbg.h>
#endif

//typedef unsigned __int32 uint32;
typedef float    vec2_t[2];
typedef float    vec3_t[3];
typedef float    vec4_t[4];
typedef vec2_t   float2;
typedef vec3_t   float3;
typedef vec4_t   float4;

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;

//typedef unsigned short uint16;

//other module
#include "image.h"
#include "model.h"
#include "gl_funcs.h"

/*

    (up)
     y
     |       (forward)
     |      -z
 	 |      /
 	 |     /
 	 |    /
 	 |   /
 	 |  /
 	 | /
 	 |/_____________ x (right)


*/

/*
================================================================================
Common
================================================================================
*/
extern const wchar_t * DATA_DIR;
extern const wchar_t * TEXTURE_DIR;
extern const wchar_t * MODEL_DIR;
extern const wchar_t * SHADER_DIR;

extern float gMovingSpeed;

struct View;

struct ComParam {
	const char * title;
	int          windowCX;
	int          windowCY;
	View *       view;
	bool(*Setup)();
	void(*Shutdown)();
	void(*Reshape)(int width, int height);
	void(*Display)();
	void(*Special)(int key, int x, int y);
};

void		Com_Run(int argc, char **argv, const ComParam &param);
uint32_t	Com_GetMS();

void	Sys_Sleep(int ms);

/*
================================================================================
math
================================================================================
*/
#define MATH_PI            3.141592654f   // same as D3DX_PI

int    Math_IsPowerOf2(int n);
float  Math_DegToRad(float d);
float  Math_RadToDeg(float r);

/*
================================================================================
file
================================================================================
*/

struct File {
	size_t original_size;
	size_t appended_size;
	byte * data;
};

#if defined(__linux__)
void _wfopen_s(FILE** f, const wchar_t* filename, const wchar_t* md);
#endif

bool   File_Read(const wchar_t *fileName, File &file, bool appendNullTernimator);
void   File_Free(File &file);

/*
================================================================================
View
================================================================================
*/
#define DEFAULT_WINDOW_CX 800
#define DEFAULT_WINDOW_CY 600

#define DEFAULT_Z_NEAR    1.0f
#define DEFAULT_Z_FAR     1024.0f

#define DEFAULT_MAX_DEPTH 1.0f

extern const float DEFAULT_BACKGROUND_COLOR[];

struct Viewport {
	int       x;
	int       y;
	int       width;
	int       height;
	float     zNear;
	float     zFar;
};

#define V_VIEW_MOVING_CAMERA 1
#define V_VIEW_MOVING_OBJECT 2

struct View {
	int       type;
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	float     yaw;
	float     pitch;
	glm::mat4 modelMatrix;
};

void   V_InitView(View &view, int type, const glm::vec3 &pos, float yaw, float pitch);

/*
================================================================================
transform
================================================================================
*/
void   V_PerspectiveMatrix(const Viewport &viewport, float fovy, glm::mat4 &out);
void   V_OrthographicMatrix(float left, float bottom, float width, float height, float near_, float far_, glm::mat4& out);
void   V_OrthoFullScreenMatrix(glm::mat4 &out);
void   V_ViewMatrix(const View &view, glm::mat4 &out);

/*
================================================================================
model
================================================================================
*/

struct VertexPosNormalRGB {
	glm::vec3     pos;
	glm::vec3     normal;
	glm::vec3     color;
};

struct VertexPosNormalRGBA {
	glm::vec3     pos;
	glm::vec3     normal;
	glm::vec4     color;
};

struct ModelBuffer {
	GLsizei		vertexSize;
	GLuint      vb;
	GLuint      ib;
	uint32_t	indexSize;
	uint32_t	indexCount;
};

bool CreatePosNormalColorModel(const wchar_t *fileName, ModelBuffer &model);
void DestroyPosNormalColorModel(ModelBuffer &model);
void DrawPosNormalColorModel(const ModelBuffer &model, bool passNormal, bool passColor, bool drawPatch = false);
