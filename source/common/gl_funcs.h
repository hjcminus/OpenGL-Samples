/******************************************************************************
 * @file	gl_funcs.h
 * @brief   helper: wrap GL functions
 *****************************************************************************/

#pragma once

/*
================================================================================
buffer object
================================================================================
*/
GLuint	GL_CreateVertexBuffer(size_t vertexSize, size_t vertexCount, void *data);
GLuint	GL_CreateIndexBuffer(size_t indexSize, size_t indexCount, void *data);

void *	GL_MapVertexBuffer();
void	GL_UnmapVertexBuffer();

void *	GL_MapIndexBuffer();
void	GL_UnmapIndexBuffer();

/*
================================================================================
texture
================================================================================
*/
GLuint	GL_CreateTexture(const wchar_t *fileName, bool mipmap);	// 2D texture
GLuint	GL_Create3DTexture(GLint internalformat, int w, int h, int d);

void	GL_DeleteTexture(GLuint &texId);

GLuint	GL_LoadTexture(const wchar_t *fileName, bool mipmap);

/*
================================================================================
shader
================================================================================
*/
struct GLProgram {
	GLuint	program;
	GLuint	vs;			// vertex shader
	GLuint	tcs;		// tessellation control shader
	GLuint	tes;		// tessellation evaluation shader
	GLuint	fs;			// fragment shader
};

bool	GL_CreateProgram(const wchar_t *vsFileName, const wchar_t *fsFileName, GLProgram &program);
bool	GL_CreateProgram(const wchar_t *vsFileName, const wchar_t *tcsFileName, const wchar_t *tesFileName, const wchar_t *fsFileName, GLProgram &program);
bool	GL_LinkProgram(GLProgram &program);
bool	GL_ValidateProgram(GLProgram &program);
void	GL_DestroyProgram(GLProgram &program);

// set uniform
void	GL_Uniform1i(GLProgram &program, const char *name, int v);
void	GL_Uniform1f(GLProgram &program, const char *name, float v0);
void	GL_Uniform2f(GLProgram &program, const char *name, float v0, float v1);
void	GL_Uniform3f(GLProgram &program, const char *name, float v0, float v1, float v2);
void	GL_Uniform4f(GLProgram &program, const char *name, float v0, float v1, float v2, float v3);
void	GL_UniformMatrix4fv(GLProgram &program, const char *name, const glm::mat4 &mat);

void	GL_SetProgramTexture(GLProgram &program, const char *name, GLenum texTarget, GLuint tex, int unit);

void	GL_UseProgram(GLProgram &program);
void	GL_UnuseProgram();

/*
================================================================================
fullscreen process
================================================================================
*/
bool GL_CreateFullScreenVB(GLuint &vb, bool hasTexCoord);
void GL_DestroyFullScreenVB(GLuint &vb);
void GL_DrawFullScreenRect(GLuint vb, bool hasTexCoord);

/*
================================================================================
framebuffer
================================================================================
*/
GLuint	GL_GenFramebuffer();
void	GL_DeleteFramebuffer(GLuint &fboId);
void	GL_BindFramebuffer(GLuint fboId);
void	GL_UnbindFramebuffer();
void	GL_FramebufferTexture(GLenum texTarget, GLuint texId, GLenum attachment, int mipLevel, int zSlice);

/*
================================================================================
error
================================================================================
*/
void	GL_CheckError(const char * file, int line);
bool	GL_CheckFramebufferStatus();

#define GL_CHECKERROR  GL_CheckError(__FILE__, __LINE__)
