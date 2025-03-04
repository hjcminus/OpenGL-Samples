//2016-01-20 Wed.

#pragma once

/*
================================================================================
View
================================================================================
*/
struct RViewport {
	int       x;
	int       y;
	int       width;
	int       height;
	float     zNear;
	float     zFar;
};

struct RView {
	glm::vec3 pos;
	glm::vec3 target;
	glm::vec3 up;
};

/*
================================================================================
transform
================================================================================
*/
void   R_PerspectiveMatrix(const RViewport &viewport, float fovy, glm::mat4 &out);
void   R_ViewMatrix(const RView &view, glm::mat4 &out);

/*
================================================================================
buffer object
================================================================================
*/
GLuint R_CreateVertexBuffer(size_t vertexSize, size_t vertexCount, void *data);
GLuint R_CreateIndexBuffer(size_t indexSize, size_t indexCount, void *data);

void * R_MapVertexBuffer();
void   R_UnmapVertexBuffer();

void * R_MapIndexBuffer();
void   R_UnmapIndexBuffer();

/*
================================================================================
texture
================================================================================
*/
GLuint R_CreateTexture(const wchar_t *fileName, bool mipmap);

/*
================================================================================
shader
================================================================================
*/

struct RProgram {
	GLuint program;
	GLuint vs;
	GLuint fs;
};

bool R_CreateProgram(const wchar_t *vsFileName, const wchar_t *fsFileName, RProgram &program);
bool R_LinkProgram(RProgram &program);
void R_DestroyProgram(RProgram &program);
