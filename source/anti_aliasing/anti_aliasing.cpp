/******************************************************************************
 * @file	anti_aliasing.cpp
 * @brief
 *****************************************************************************/

#include "../common/common.h"

Viewport rViewport;
View rView;
ModelBuffer rModel;

GLProgram lighting_prog;
GLProgram simple_prog;
GLProgram antialiasing_prog;
GLuint tex;
GLuint fbo;
GLuint fsVB;
GLuint depth;
int fboW;
int fboH;
int drawMode = 2;

float scale = 2.0f;

void destroyFBO() {
	if (depth) {
		glDeleteRenderbuffers(1, &depth);
		depth = 0;
	}

	if (tex) {
		glDeleteTextures(1, &tex);
		tex = 0;
	}

	if (fbo) {
		glDeleteRenderbuffersEXT(1, &fbo);
		fbo = 0;
	}
}

void initFBO(int w, int h) {
	destroyFBO();

	fboW = (int)(w * scale);
	fboH = (int)(h * scale);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboW, fboH, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	// create depth buffer
	glGenRenderbuffers(1, &depth);
	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fboW, fboH);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	GL_CheckFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = DEFAULT_WINDOW_CX;
	rViewport.height = DEFAULT_WINDOW_CY;
	rViewport.zNear = DEFAULT_Z_NEAR;
	rViewport.zFar = DEFAULT_Z_FAR;

	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 500.0f), 0.0f, 0.0f);
	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);
	glEnable(GL_DEPTH_TEST);

	bool b1 = GL_CreateProgram(L"anti_aliasing/lighting_vs.txt", L"anti_aliasing/lighting_fs.txt", lighting_prog);
	bool b2 = GL_CreateProgram(L"anti_aliasing/simple_vs.txt", L"anti_aliasing/simple_fs.txt", simple_prog);
	bool b3 = GL_CreateProgram(L"anti_aliasing/anti_aliasing_vs.txt", L"anti_aliasing/anti_aliasing_fs.txt", antialiasing_prog);

	if (b1 && b2 && b3) {
		GL_LinkProgram(lighting_prog);
		GL_LinkProgram(simple_prog);
		GL_LinkProgram(antialiasing_prog);
	}

	initFBO(DEFAULT_WINDOW_CX, DEFAULT_WINDOW_CY);

	GL_CreateFullScreenVB(fsVB, true);

	if (!CreatePosNormalColorModel(L"3dentity.model", rModel)) {
		return false;
	}

	return true;
}

void Shutdown() {
	DestroyPosNormalColorModel(rModel);

	GL_DestroyFullScreenVB(fsVB);

	destroyFBO();
	
	GL_DestroyProgram(simple_prog);
	GL_DestroyProgram(lighting_prog);
	GL_DestroyProgram(antialiasing_prog);
}

void Reshape(int width, int height) {
	if (rViewport.width == width && rViewport.height == height) {
		return;
	}

	rViewport.width = width;
	rViewport.height = height;
	glViewport(0, 0, width, height);

	initFBO(width, height);
}

void Display() {
	if (0 == drawMode) {
		glm::mat4 matProj;
		glm::mat4 matView;

		Viewport vp = rViewport;
		vp.width = fboW;
		vp.height = fboH;

		V_PerspectiveMatrix(vp, 45.0f, matProj);
		V_ViewMatrix(rView, matView);

		glm::mat4 matModelView = matView * rView.modelMatrix;
		glm::mat4 matModelViewProj = matProj * matView * rView.modelMatrix;


		glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		GL_UseProgram(lighting_prog);
		{
			GL_UniformMatrix4fv(lighting_prog, "modelViewProj", matModelViewProj);
			GL_UniformMatrix4fv(lighting_prog, "modelView", matModelView);

			GL_Uniform3f(lighting_prog, "viewPos", rView.pos[0], rView.pos[1], rView.pos[2]);

			GL_Uniform4f(lighting_prog, "materialAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "materialSpecular", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform1f(lighting_prog, "materialShiness", 16.0f);

			GL_Uniform3f(lighting_prog, "lightDirection", 0.0f, 0.0f, -1.0f);

			GL_Uniform4f(lighting_prog, "lightAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightSpecular", 1.0f, 1.0f, 1.0f, 1.0f);

			DrawPosNormalColorModel(rModel, true, true);
		}
		GL_UnuseProgram();
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);

		glm::mat4 matProj;
		glm::mat4 matView;

		Viewport vp = rViewport;
		vp.width = fboW;
		vp.height = fboH;

		V_PerspectiveMatrix(vp, 45.0f, matProj);
		V_ViewMatrix(rView, matView);

		glm::mat4 matModelView = matView * rView.modelMatrix;
		glm::mat4 matModelViewProj = matProj * matView * rView.modelMatrix;

		glViewport(vp.x, vp.y, vp.width, vp.height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		GL_UseProgram(lighting_prog);
		{
			GL_UniformMatrix4fv(lighting_prog, "modelViewProj", matModelViewProj);
			GL_UniformMatrix4fv(lighting_prog, "modelView", matModelView);

			GL_Uniform3f(lighting_prog, "viewPos", rView.pos[0], rView.pos[1], rView.pos[2]);

			GL_Uniform4f(lighting_prog, "materialAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "materialSpecular", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform1f(lighting_prog, "materialShiness", 16.0f);

			GL_Uniform3f(lighting_prog, "lightDirection", 0.0f, 0.0f, -1.0f);

			GL_Uniform4f(lighting_prog, "lightAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightSpecular", 1.0f, 1.0f, 1.0f, 1.0f);

			DrawPosNormalColorModel(rModel, true, true);
		}
		GL_UnuseProgram();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);

		glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GL_UseProgram(2 == drawMode ? antialiasing_prog : simple_prog);
		{
			V_OrthoFullScreenMatrix(matModelViewProj);

			GL_UniformMatrix4fv(2 == drawMode ? antialiasing_prog : simple_prog, "modelViewProj", matModelViewProj);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex);
			GL_Uniform1i(2 == drawMode ? antialiasing_prog : simple_prog, "tex", 0);

			if (2 == drawMode) {
				GL_Uniform2f(antialiasing_prog, "stTexelSize", 1.0f / (float)fboW, 1.0f / (float)fboH);
			}

			GL_DrawFullScreenRect(fsVB, true);
		}
		GL_UnuseProgram();
	}

	GL_CHECKERROR;

	glutSwapBuffers();
}

void Special(int key, int x, int y) {
	if (GLUT_KEY_F2 == key) {
		drawMode++;
		drawMode %= 3;

		switch (drawMode) {
		case 0:
			printf("default draw\n");
			break;
		case 1:
			printf("texture sampling\n");
			break;
		case 2:
			printf("anti aliasing\n");
			break;
		}
		
		glutPostRedisplay();
	}
	else if (GLUT_KEY_F3) {
		if (scale == 1) {
			scale = 2;
		}
		else {
			scale = 1;
		}

		initFBO(rViewport.width, rViewport.height);

		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	printf("   F2    - Toggle draw mode\n");
	printf("   F3    - Toggle scale/unscale framebuffer\n");

	ComParam param;
	param.title = "Anti Aliasing";
	param.windowCX = DEFAULT_WINDOW_CX;
	param.windowCY = DEFAULT_WINDOW_CY;
	param.view = &rView;
	param.Setup = Setup;
	param.Shutdown = Shutdown;
	param.Reshape = Reshape;
	param.Display = Display;
	param.Special = Special;

	Com_Run(argc, argv, param);

	return 0;
}
