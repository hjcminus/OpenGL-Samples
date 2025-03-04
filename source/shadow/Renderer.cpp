//2016-01-20 Wed.

#include "Precompiled.h"
#include <glm/gtc/matrix_transform.hpp>

/*
================================================================================
transform
================================================================================
*/
void R_PerspectiveMatrix(const RViewport &viewport, float fovy, glm::mat4 &out) {
	out = glm::perspective(fovy, (float)viewport.width / viewport.height, viewport.zNear, viewport.zFar);
}

void R_ViewMatrix(const RView &view, glm::mat4 &out) {
	out = glm::lookAt(view.pos, view.target, view.up);
}

/*
================================================================================
buffer object
================================================================================
*/
GLuint R_CreateVertexBuffer(size_t vertexSize, size_t vertexCount, void *data) {
	GLuint obj = 0;
	glGenBuffers(1, &obj);
	glBindBuffer(GL_ARRAY_BUFFER, obj);
	glBufferData(GL_ARRAY_BUFFER, vertexSize * vertexCount, data, GL_STATIC_DRAW); //GL_DYNAMIC_DRAW
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind
	return obj;
}

GLuint R_CreateIndexBuffer(size_t indexSize, size_t indexCount, void *data) {
	GLuint obj = 0;
	glGenBuffers(1, &obj);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbind
	return obj;
}

void * R_MapVertexBuffer() {
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void R_UnmapVertexBuffer() {
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void * R_MapIndexBuffer() {
	return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void R_UnmapIndexBuffer() {
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

/*
================================================================================
texture
================================================================================
*/
GLuint R_CreateTexture(const wchar_t *fileName, bool mipmap) {
	wchar_t fullFileName[MAX_PATH];
	wsprintf(fullFileName, L"%s\\%s", TEXTURE_DIR, fileName);

	Image image = { 0 };
	if (!Image_Load(fullFileName, image)) {
		return 0;
	}

	if (image.bits != 24 && image.bits != 32) {
		Image_Free(image);
		return 0;
	}

	//check image size
	if (!Math_IsPowerOf2(image.cx) || image.cx < 16 || !Math_IsPowerOf2(image.cy) || image.cy < 16) {
		//SYS_ERROR(L"invalid image size (%d,%d) in OpenGL 2.0\n", image->mWidth, image->mHeight);
		Image_Free(image);
		return 0;
	}

	GLuint obj = 0;

	//now generate new texture
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
		else { //32
			gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image.cx, image.cy, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
		}
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//The first element corresponds to the lower left corner of the texture image
		if (24 == image.bits) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cx, image.cy, 0, GL_RGB, GL_UNSIGNED_BYTE, image.pixels);
		}
		else { //32
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
	if (!File_Read(fileName, file)) {
		return 0;
	}

	GLuint obj = 0;
	obj = glCreateShader(type);
	if (!obj) {
		File_Free(file);
		return 0;
	}

	GLchar * src = (GLchar*)malloc(file.size + 1);
	memcpy(src, file.data, file.size);
	src[file.size] = 0;

	File_Free(file);

	glShaderSource(obj, 1, &src, 0);
	glCompileShader(obj);

	GLint compileStatus = 0;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &compileStatus);

	//check compile result
	if (!compileStatus) {
		GLint infoLogLen = 0;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infoLogLen);
		if (infoLogLen > 1023) {
			//SYS_ERROR(L"error too long\n");
			glDeleteShader(obj);
			free(src);
			return 0;
		}

		char infoLog[1024];
		glGetShaderInfoLog(obj, 1024, &infoLogLen, infoLog);
		printf("shader error: %s\n", infoLog);
		free(src);
		return 0;
	}

	free(src);
	return obj;
}

bool R_CreateProgram(const wchar_t *vsFileName, const wchar_t *fsFileName, RProgram &program) {
	memset(&program, 0, sizeof(program));

	wchar_t fullFileName[MAX_PATH];

	program.program = glCreateProgram();

	wsprintf(fullFileName, L"%s\\%s", SHADER_DIR, vsFileName);
	program.vs = CreateShader(fullFileName, GL_VERTEX_SHADER);

	wsprintf(fullFileName, L"%s\\%s", SHADER_DIR, fsFileName);
	program.fs = CreateShader(fullFileName, GL_FRAGMENT_SHADER);

	if (!program.fs || !program.vs) {
		if (program.fs) {
			glDeleteShader(program.fs);
			program.fs = 0;
		}
		if (program.vs) {
			glDeleteShader(program.vs);
			program.vs = 0;
		}
		return false;
	}

	return true;
}

bool R_LinkProgram(RProgram &program) {
	if (!program.program || !program.vs || !program.fs) {
		return false;
	}

	glAttachShader(program.program, program.vs);
	glAttachShader(program.program, program.fs);

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
		glDeleteShader(program.fs);
		glDeleteShader(program.vs);
		glDeleteProgram(program.program);

		program.program = program.fs = program.vs = 0;

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
		glDeleteShader(program.fs);
		glDeleteShader(program.vs);
		glDeleteProgram(program.program);

		program.program = program.fs = program.vs = 0;

		return false;
	}

	return true;
}

void R_DestroyProgram(RProgram &program) {
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
