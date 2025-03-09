/******************************************************************************
 * @file	gl_funcs.cpp
 * @brief   helper: wrap GL functions
 *****************************************************************************/

#include "common.h"

/*
================================================================================
buffer object
================================================================================
*/
GLuint GL_CreateVertexBuffer(size_t vertexSize, size_t vertexCount, void *data) {
	GLuint obj = 0;
	glGenBuffers(1, &obj);
	glBindBuffer(GL_ARRAY_BUFFER, obj);

	/* glBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
		usage:
	      STATIC
		    The data store contents will be modified once and used many times.
	      DYNAMIC
		    The data store contents will be modified repeatedly and used many times.
	*/

	glBufferData(GL_ARRAY_BUFFER, vertexSize * vertexCount, data, GL_STATIC_DRAW); // GL_DYNAMIC_DRAW
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
	return obj;
}

GLuint GL_CreateIndexBuffer(size_t indexSize, size_t indexCount, void *data) {
	GLuint obj = 0;
	glGenBuffers(1, &obj);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind
	return obj;
}

void * GL_MapVertexBuffer() {
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void GL_UnmapVertexBuffer() {
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void * GL_MapIndexBuffer() {
	return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void GL_UnmapIndexBuffer() {
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

/*
================================================================================
texture
================================================================================
*/
GLuint GL_CreateTexture(const wchar_t *fileName, bool mipmap) {
	wchar_t fullFileName[MAX_PATH];

#if defined(_MSC_VER)
	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", TEXTURE_DIR, fileName);
#endif

#if defined(__GNUC__)
   swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", TEXTURE_DIR, fileName);
#endif

	Image image = { 0 };
	if (!Image_Load(fullFileName, image)) {
		return 0;
	}

	if (image.bits != 24 && image.bits != 32) {
		Image_Free(image);
		return 0;
	}

	// check image size
	if (!Math_IsPowerOf2(image.cx) || image.cx < 16 || !Math_IsPowerOf2(image.cy) || image.cy < 16) {
		// SYS_ERROR(L"invalid image size (%d,%d) in OpenGL 2.0\n", image->mWidth, image->mHeight);
		Image_Free(image);
		return 0;
	}

	GLuint obj = 0;

	// now generate new texture
	glGenTextures(1, &obj);
	glBindTexture(GL_TEXTURE_2D, obj);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (mipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		if (24 == image.bits) {
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.cx, image.cy, GL_RGB, GL_UNSIGNED_BYTE, image.pixels);
		}
		else { // 32 bits
			gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image.cx, image.cy, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
		}
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// The first element corresponds to the lower left corner of the texture image
		if (24 == image.bits) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cx, image.cy, 0, GL_RGB, GL_UNSIGNED_BYTE, image.pixels);
		}
		else { // 32 bits
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cx, image.cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
		}
	}

	Image_Free(image);

	glBindTexture(GL_TEXTURE_2D, 0);

	return obj;
}

GLuint GL_Create3DTexture(GLint internalformat, int w, int h, int d) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLint mode = GL_CLAMP_TO_BORDER;
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, mode);
	glTexImage3D(GL_TEXTURE_3D, 0, internalformat, w, h, d, 0, GL_RGBA, GL_FLOAT, 0);
	return tex;
}

void GL_DeleteTexture(GLuint &texId) {
	glDeleteTextures(1, &texId);
	texId = 0;
}

GLuint GL_LoadTexture(const wchar_t *fileName, bool mipmap) {
	wchar_t fullFileName[MAX_PATH];
	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", TEXTURE_DIR, fileName);

	Image image = { 0 };
	if (!Image_Load(fullFileName, image)) {
		return 0;
	}

	if (image.bits != 24 && image.bits != 32) {
		Image_Free(image);
		return 0;
	}

	// check image size
	if (!Math_IsPowerOf2(image.cx) || image.cx < 16 || !Math_IsPowerOf2(image.cy) || image.cy < 16) {
		// SYS_ERROR(L"invalid image size (%d,%d) in OpenGL 2.0\n", image->mWidth, image->mHeight);
		Image_Free(image);
		return 0;
	}

	GLuint obj = 0;

	// now generate new texture
	glGenTextures(1, &obj);
	glBindTexture(GL_TEXTURE_2D, obj);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (mipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		if (24 == image.bits) {
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.cx, image.cy, GL_RGB, GL_UNSIGNED_BYTE, image.pixels);
		}
		else { // 32 bits
			gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image.cx, image.cy, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
		}
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// The first element corresponds to the lower left corner of the texture image
		if (24 == image.bits) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cx, image.cy, 0, GL_RGB, GL_UNSIGNED_BYTE, image.pixels);
		}
		else { // 32 bits
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cx, image.cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
		}
	}

	Image_Free(image);

	glBindTexture(GL_TEXTURE_2D, 0);

	return obj;
}

/*
================================================================================
shader
================================================================================
*/
static GLuint CreateShader(const wchar_t *fileName, GLenum type) {
	File file;
	if (!File_Read(fileName, file, true)) {
		return 0;
	}

	GLuint obj = 0;
	obj = glCreateShader(type);
	if (!obj) {
		File_Free(file);
		return 0;
	}

	const GLchar * src = (const GLchar *)file.data;

	glShaderSource(obj, 1, &src, 0);
	glCompileShader(obj);

	File_Free(file);

	GLint compileStatus = 0;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &compileStatus);

	// check compile result
	if (!compileStatus) {
		GLint infoLogLen = 0;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infoLogLen);
		if (infoLogLen > 1023) {
			// SYS_ERROR(L"error too long\n");
			glDeleteShader(obj);
			return 0;
		}

		char infoLog[1024];
		glGetShaderInfoLog(obj, 1024, &infoLogLen, infoLog);
		printf("shader error: %s\n", infoLog);
		return 0;
	}

	return obj;
}

bool GL_CreateProgram(const wchar_t *vsFileName, const wchar_t *fsFileName, GLProgram &program) {
	memset(&program, 0, sizeof(program));

	wchar_t fullFileName[MAX_PATH];

	program.program = glCreateProgram();

#if defined(_MSC_VER)
	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", SHADER_DIR, vsFileName);
	program.vs = CreateShader(fullFileName, GL_VERTEX_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", SHADER_DIR, fsFileName);
	program.fs = CreateShader(fullFileName, GL_FRAGMENT_SHADER);
#endif

#if defined(__GNUC__)
	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", SHADER_DIR, vsFileName);
	program.vs = CreateShader(fullFileName, GL_VERTEX_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", SHADER_DIR, fsFileName);
	program.fs = CreateShader(fullFileName, GL_FRAGMENT_SHADER);
#endif

	if (!program.fs || !program.vs) {
		if (program.fs) {
			glDeleteShader(program.fs);
			program.fs = 0;
		}
		if (program.vs) {
			glDeleteShader(program.vs);
			program.vs = 0;
		}

		glDeleteProgram(program.program);

		return false;
	}

	return true;
}

bool GL_CreateProgram(const wchar_t *vsFileName, const wchar_t *tcsFileName, const wchar_t *tesFileName, const wchar_t *fsFileName, GLProgram &program) {
	memset(&program, 0, sizeof(program));	// fill zero

	wchar_t fullFileName[MAX_PATH];

	program.program = glCreateProgram();

#if defined(_MSC_VER)
	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", SHADER_DIR, vsFileName);
	program.vs = CreateShader(fullFileName, GL_VERTEX_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", SHADER_DIR, tcsFileName);
	program.tcs = CreateShader(fullFileName, GL_TESS_CONTROL_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", SHADER_DIR, tesFileName);
	program.tes = CreateShader(fullFileName, GL_TESS_EVALUATION_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", SHADER_DIR, fsFileName);
	program.fs = CreateShader(fullFileName, GL_FRAGMENT_SHADER);
#endif

#if defined(__GNUC__)
	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", SHADER_DIR, vsFileName);
	program.vs = CreateShader(fullFileName, GL_VERTEX_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", SHADER_DIR, tcsFileName);
	program.tcs = CreateShader(fullFileName, GL_TESS_CONTROL_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", SHADER_DIR, tesFileName);
	program.tes = CreateShader(fullFileName, GL_TESS_EVALUATION_SHADER);

	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", SHADER_DIR, fsFileName);
	program.fs = CreateShader(fullFileName, GL_FRAGMENT_SHADER);
#endif

	if (!program.fs || !program.vs || !program.tcs || !program.tes) {
		if (program.fs) {
			glDeleteShader(program.fs);
			program.fs = 0;
		}

		if (program.tes) {
			glDeleteShader(program.tes);
			program.tes = 0;
		}

		if (program.tcs) {
			glDeleteShader(program.tcs);
			program.tcs = 0;
		}

		if (program.vs) {
			glDeleteShader(program.vs);
			program.vs = 0;
		}
		return false;
	}

	return true;
}

bool GL_LinkProgram(GLProgram &program) {
	if (!program.program || !program.vs || !program.fs) {
		return false;
	}

	glAttachShader(program.program, program.vs);
	glAttachShader(program.program, program.fs);

	if (program.tcs) {
		glAttachShader(program.program, program.tcs);
	}

	if (program.tes) {
		glAttachShader(program.program, program.tes);
	}

	glLinkProgram(program.program);

	GLint linkStatus = 0;
	glGetProgramiv(program.program, GL_LINK_STATUS, &linkStatus);

	if (!linkStatus) {
		GLint infoLogLen = 0;
		glGetProgramiv(program.program, GL_INFO_LOG_LENGTH, &infoLogLen);

		char infoLog[1024];
		glGetProgramInfoLog(program.program, 1024, &infoLogLen, infoLog);

		printf("link error: %s\n", infoLog);

		glDetachShader(program.program, program.fs);
		glDetachShader(program.program, program.vs);

		if (program.tes) {
			glDetachShader(program.program, program.tes);
			glDeleteShader(program.tes);
		}

		if (program.tcs) {
			glDetachShader(program.program, program.tcs);
			glDeleteShader(program.tcs);
		}

		glDeleteShader(program.fs);
		glDeleteShader(program.vs);
		glDeleteProgram(program.program);

		program.program = program.fs = program.vs = program.tcs = program.tes = 0;

		return false;
	}
	else {
		return true;
	}
}

bool GL_ValidateProgram(GLProgram &program) {
	if (!program.program) {
		return false;
	}

	glValidateProgram(program.program);

	GLint validateStatus = 0;
	glGetProgramiv(program.program, GL_VALIDATE_STATUS, &validateStatus);

	if (!validateStatus) {
		GLint infoLogLen = 0;
		glGetProgramiv(program.program, GL_INFO_LOG_LENGTH, &infoLogLen);

		char infoLog[1024];
		glGetProgramInfoLog(program.program, 1024, &infoLogLen, infoLog);

		printf("validate error: %s\n", infoLog);

		glDetachShader(program.program, program.fs);
		glDetachShader(program.program, program.vs);

		if (program.tes) {
			glDetachShader(program.program, program.tes);
			glDeleteShader(program.tes);
		}

		if (program.tcs) {
			glDetachShader(program.program, program.tcs);
			glDeleteShader(program.tcs);
		}

		glDeleteShader(program.fs);
		glDeleteShader(program.vs);
		glDeleteProgram(program.program);

		program.program = program.fs = program.vs = program.tcs = program.tes = 0;

		return false;
	}
	else {
		return true;
	}
}

void GL_DestroyProgram(GLProgram &program) {
	if (program.tes) {
		if (program.program) {
			glDetachShader(program.program, program.tes);
		}
		glDeleteShader(program.tes);
		program.tes = 0;
	}

	if (program.tcs) {
		if (program.program) {
			glDetachShader(program.program, program.tcs);
		}
		glDeleteShader(program.tcs);
		program.tcs = 0;
	}

	if (program.fs) {
		if (program.program) {
			glDetachShader(program.program, program.fs);
		}
		glDeleteShader(program.fs);
		program.fs = 0;
	}

	if (program.vs) {
		if (program.program) {
			glDetachShader(program.program, program.vs);
		}
		glDeleteShader(program.vs);
		program.vs = 0;
	}
	
	if (program.program) {
		glDeleteProgram(program.program);
		program.program = 0;
	}
}

void GL_Uniform1i(GLProgram &program, const char *name, int v) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glUniform1i(loc, v);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_Uniform1f(GLProgram &program, const char *name, float v0) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glUniform1f(loc, v0);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_Uniform2f(GLProgram &program, const char *name, float v0, float v1) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glUniform2f(loc, v0, v1);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_Uniform3f(GLProgram &program, const char *name, float v0, float v1, float v2) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glUniform3f(loc, v0, v1, v2);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_Uniform4f(GLProgram &program, const char *name, float v0, float v1, float v2, float v3) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glUniform4f(loc, v0, v1, v2, v3);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_UniformMatrix4fv(GLProgram &program, const char *name, const glm::mat4 &mat) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_SetProgramTexture(GLProgram &program, const char *name, GLenum texTarget, GLuint tex, int unit) {
	GLint loc = glGetUniformLocation(program.program, name);
	if (loc >= 0) {
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(texTarget, tex);
		glUniform1i(loc, unit);
	}
	else {
		printf_s("error get location of uniform %s\n", name);
	}
}

void GL_UseProgram(GLProgram &program) {
	glUseProgram(program.program);
}

void GL_UnuseProgram() {
	glUseProgram(0);
}

/*
================================================================================
fullscreen process
================================================================================
*/
bool GL_CreateFullScreenVB(GLuint &vb, bool hasTexCoord) {
	GLsizei s = hasTexCoord ? 20 : 12;

	vb = GL_CreateVertexBuffer(s, 4, nullptr);
	if (!vb) {
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vb);
	if (hasTexCoord) {
		float *pv = (float*)GL_MapVertexBuffer();

		pv[0] = 0.0f;
		pv[1] = 0.0f;
		pv[2] = 0.0f;
		pv[3] = pv[0];
		pv[4] = pv[1];

		pv += 5;

		pv[0] = 1.0f;
		pv[1] = 0.0f;
		pv[2] = 0.0f;
		pv[3] = pv[0];
		pv[4] = pv[1];

		pv += 5;

		pv[0] = 1.0f;
		pv[1] = 1.0f;
		pv[2] = 0.0f;
		pv[3] = pv[0];
		pv[4] = pv[1];

		pv += 5;

		pv[0] = 0.0f;
		pv[1] = 1.0f;
		pv[2] = 0.0f;
		pv[3] = pv[0];
		pv[4] = pv[1];
	}
	else {
		glm::vec3 * pv = (glm::vec3*)GL_MapVertexBuffer();

		pv[0] = glm::vec3(0.0f, 0.0f, 0.0f);
		pv[1] = glm::vec3(1.0f, 0.0f, 0.0f);
		pv[2] = glm::vec3(1.0f, 1.0f, 0.0f);
		pv[3] = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	GL_UnmapVertexBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void GL_DestroyFullScreenVB(GLuint &vb) {
	if (vb) {
		glDeleteBuffers(1, &vb);
		vb = 0;
	}
}

void GL_DrawFullScreenRect(GLuint vb, bool hasTexCoord) {
	glEnableVertexAttribArray(0);
	if (hasTexCoord) {
		glEnableVertexAttribArray(1);
	}

	GLsizei s = hasTexCoord ? 20 : 12;

	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, s, (const GLvoid*)0);
	if (hasTexCoord) {
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, s, (const GLvoid*)12);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	if (hasTexCoord) {
		glDisableVertexAttribArray(1);
	}
}

/*
================================================================================
framebuffer
================================================================================
*/
GLuint GL_GenFramebuffer() {
	GLuint fboId = 0;
	glGenFramebuffersEXT(1, &fboId);
	return fboId;
}

void GL_DeleteFramebuffer(GLuint &fboId) {
	glDeleteFramebuffers(1, &fboId);
	fboId = 0;
}

void GL_BindFramebuffer(GLuint fboId) {
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
}

void GL_UnbindFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GL_FramebufferTexture(GLenum texTarget, GLuint texId, GLenum attachment, int mipLevel, int zSlice) {
	if (GL_TEXTURE_1D == texTarget) {
		glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_1D, texId, mipLevel);
	}
	else if (GL_TEXTURE_3D == texTarget) {
		glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_3D, texId, mipLevel, zSlice);
	}
	else {
		// Default is GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, or cube faces
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texTarget, texId, mipLevel);
	}
}

/*
================================================================================
error
================================================================================
*/
void GL_CheckError(const char * file, int line) {
	GLint error = glGetError();
	if (error) {
		const char * error_str = nullptr;
		switch (error) {
			case GL_INVALID_ENUM: error_str = "GL_INVALID_ENUM"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error_str = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_INVALID_VALUE: error_str = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: error_str = "GL_INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY: error_str = "GL_OUT_OF_MEMORY"; break;
			default: error_str = "unknown error"; break;
		}

		printf_s("error at %s, %d: %s\n", file, line, error_str);

		error = 0; // nice place to hang a breakpoint in compiler... :)
	}
}

bool GL_CheckFramebufferStatus()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		printf("Unsupported framebuffer format\n");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		printf("Framebuffer incomplete, missing attachment\n");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		printf("Framebuffer incomplete, attached images must have same dimensions\n");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		printf("Framebuffer incomplete, attached images must have same format\n");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		printf("Framebuffer incomplete, missing draw buffer\n");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		printf("Framebuffer incomplete, missing read buffer\n");
		break;
	default:
		printf("Error %x\n", status);
		break;
	}

	return GL_FRAMEBUFFER_COMPLETE == status;
}