/******************************************************************************
 * @file	depth_peeling.cpp
 * @brief   An order-independent transparency (OIT) rendering algorithm 
 *****************************************************************************/

#include "../common/common.h"

// window size
#define WINDOW_CX	800
#define WINDOW_CY	600

// z range
#define Z_NEAR		1.0f
#define Z_FAR		1024.0f

struct RVertex {
	glm::vec3		pos;
	glm::vec3		normal;
	glm::vec3		color;
};

struct RModel {
	GLuint			vb;
	GLuint			ib;
	uint32_t		indexSize;
	uint32_t		indexCount;
};

Viewport	rViewport;
View		rView;
RModel		rModel;

GLuint		rFrontFboId[2];
GLuint		rFrontDepthTexId[2];
GLuint		rFrontColorTexId[2];
GLuint		rFrontColorBlenderTexId;
GLuint		rFrontColorBlenderFboId;

bool InitDepthPeelingRenderTargets(int viewCX, int viewCY) {
	glGenTextures(2, rFrontDepthTexId);
	glGenTextures(2, rFrontColorTexId);
	glGenFramebuffers(2, rFrontFboId);

	for (int i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_RECTANGLE, rFrontDepthTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT32F, viewCX, viewCY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);


		glBindTexture(GL_TEXTURE_RECTANGLE, rFrontColorTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, viewCX, viewCY, 0, GL_RGBA, GL_FLOAT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, rFrontFboId[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, rFrontDepthTexId[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rFrontColorTexId[i], 0);
		GL_CheckFramebufferStatus();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GL_CHECKERROR;

	glGenTextures(1, &rFrontColorBlenderTexId);
	glBindTexture(GL_TEXTURE_RECTANGLE, rFrontColorBlenderTexId);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, viewCX, viewCY, 0, GL_RGBA, GL_FLOAT, 0);

	glGenFramebuffers(1, &rFrontColorBlenderFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, rFrontColorBlenderFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, rFrontDepthTexId[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rFrontColorBlenderTexId, 0);
	GL_CheckFramebufferStatus();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// check error
	GL_CHECKERROR;

	return true;
}

void DeleteDepthPeelingRenderTargets()
{
	glDeleteFramebuffers(2, rFrontFboId);
	glDeleteFramebuffers(1, &rFrontColorBlenderFboId);
	glDeleteTextures(2, rFrontDepthTexId);
	glDeleteTextures(2, rFrontColorTexId);
	glDeleteTextures(1, &rFrontColorBlenderTexId);
}

GLuint rQueryId; // query processed samples count

// front peeling init program
GLProgram rFrontPeelingInitProg;
GLint rFrontPeelingInit_uModelViewMatrix;
GLint rFrontPeelingInit_uAlpha;

// front peeling peel program
GLProgram rFrontPeelingPeelProg;
GLint rFrontPeelingPeel_uModelViewMatrix;
GLint rFrontPeelingPeel_uAlpha;
GLint rFrontPeelingPeel_DepthTex;

// front peeling blend program
GLProgram rFrontPeelingBlendProg;
GLint rFrontPeelingBlend_uModelViewMatrix;
GLint rFrontPeelingBlend_TempTex;

// front peeling final program
GLProgram rFrontPeelingFinalProg;
GLint rFrontPeelingFinal_uModelViewMatrix;
GLint rFrontPeelingFinal_uBackgroundColor;
GLint rFrontPeelingFinal_ColorTex;

bool InitOITPrograms() {
	// front peeling init program
	if (!GL_CreateProgram(L"depth_peeling/front_peeling_init_vs.txt", L"depth_peeling/front_peeling_init_fs.txt", rFrontPeelingInitProg)) {
		return false;
	}

	if (!GL_LinkProgram(rFrontPeelingInitProg)) {
		GL_DestroyProgram(rFrontPeelingInitProg);
		return false;
	}

	rFrontPeelingInit_uModelViewMatrix = glGetUniformLocation(rFrontPeelingInitProg.program, "uModelViewMatrix");
	rFrontPeelingInit_uAlpha = glGetUniformLocation(rFrontPeelingInitProg.program, "uAlpha");

	// front peeling peel program
	if (!GL_CreateProgram(L"depth_peeling/front_peeling_peel_vs.txt", L"depth_peeling/front_peeling_peel_fs.txt", rFrontPeelingPeelProg)) {
		return false;
	}

	if (!GL_LinkProgram(rFrontPeelingPeelProg)) {
		GL_DestroyProgram(rFrontPeelingPeelProg);
		return false;
	}

	rFrontPeelingPeel_uModelViewMatrix = glGetUniformLocation(rFrontPeelingPeelProg.program, "uModelViewMatrix");
	rFrontPeelingPeel_uAlpha = glGetUniformLocation(rFrontPeelingPeelProg.program, "uAlpha");
	rFrontPeelingPeel_DepthTex = glGetUniformLocation(rFrontPeelingPeelProg.program, "DepthTex");

	// front peeling blend program
	if (!GL_CreateProgram(L"depth_peeling/front_peeling_blend_vs.txt", L"depth_peeling/front_peeling_blend_fs.txt", rFrontPeelingBlendProg)) {
		return false;
	}

	if (!GL_LinkProgram(rFrontPeelingBlendProg)) {
		GL_DestroyProgram(rFrontPeelingBlendProg);
		return false;
	}

	rFrontPeelingBlend_uModelViewMatrix = glGetUniformLocation(rFrontPeelingBlendProg.program, "uModelViewMatrix");
	rFrontPeelingBlend_TempTex = glGetUniformLocation(rFrontPeelingBlendProg.program, "TempTex");

	// front peeling final program
	if (!GL_CreateProgram(L"depth_peeling/front_peeling_final_vs.txt", L"depth_peeling/front_peeling_final_fs.txt", rFrontPeelingFinalProg)) {
		return false;
	}

	if (!GL_LinkProgram(rFrontPeelingFinalProg)) {
		GL_DestroyProgram(rFrontPeelingFinalProg);
		return false;
	}

	rFrontPeelingFinal_uModelViewMatrix = glGetUniformLocation(rFrontPeelingFinalProg.program, "uModelViewMatrix");
	rFrontPeelingFinal_uBackgroundColor = glGetUniformLocation(rFrontPeelingFinalProg.program, "uBackgroundColor");
	rFrontPeelingFinal_ColorTex = glGetUniformLocation(rFrontPeelingFinalProg.program, "ColorTex");

	return true;
}

void DeleteOITPrograms() {
	GL_DestroyProgram(rFrontPeelingFinalProg);
	GL_DestroyProgram(rFrontPeelingBlendProg);
	GL_DestroyProgram(rFrontPeelingPeelProg);
	GL_DestroyProgram(rFrontPeelingInitProg);
}

// base program
GLProgram rBaseProg;
GLint rBaseProg_uModelViewMatrix;
GLint rBaseProg_uAlpha;

bool InitBaseProgram() {
	// front peeling init program
	if (!GL_CreateProgram(L"depth_peeling/base_vs.txt", L"depth_peeling/base_fs.txt", rBaseProg)) {
		return false;
	}

	if (!GL_LinkProgram(rBaseProg)) {
		GL_DestroyProgram(rBaseProg);
		return false;
	}

	rBaseProg_uModelViewMatrix = glGetUniformLocation(rBaseProg.program, "uModelViewMatrix");
	rBaseProg_uAlpha = glGetUniformLocation(rBaseProg.program, "uAlpha");

	return true;
}

void DeleteBaseProgram() {
	GL_DestroyProgram(rBaseProg);
}

bool InitModel() {
	// load test model
	Model model;
	if (!Model_Load(L"LTM1030_RGB.model", &model)) { //star.model teapot.model dragon.model LTM1030_RGB
		return false;
	}

	rModel.vb = GL_CreateVertexBuffer(model.vertexSize, model.nVertices, model.vertices);
	rModel.ib = GL_CreateIndexBuffer(model.indexSize, model.nIndices, model.indices);
	rModel.indexSize = model.indexSize;
	rModel.indexCount = model.nIndices;

	Model_Free(&model);

	return true;
}

void DeleteModel() {
	glDeleteBuffers(1, &rModel.ib);
	glDeleteBuffers(1, &rModel.vb);
}

GLuint rFullScreenVB;

bool InitFullScreenVB(int viewCX, int viewCY) {
	// fullscreen vertex buffer
	rFullScreenVB = GL_CreateVertexBuffer(sizeof(glm::vec3), 4, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, rFullScreenVB);
	glm::vec3 * pv = (glm::vec3*)GL_MapVertexBuffer();

	pv[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	pv[1] = glm::vec3(viewCX, 0.0f, 0.0f);
	pv[2] = glm::vec3(viewCX, viewCY, 0.0f);
	pv[3] = glm::vec3(0.0f, viewCY, 0.0f);

	GL_UnmapVertexBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void UpdateFullScreenVB(int viewCX, int viewCY) {
	glBindBuffer(GL_ARRAY_BUFFER, rFullScreenVB);
	glm::vec3 * pv = (glm::vec3*)GL_MapVertexBuffer();

	pv[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	pv[1] = glm::vec3(viewCX, 0.0f, 0.0f);
	pv[2] = glm::vec3(viewCX, viewCY, 0.0f);
	pv[3] = glm::vec3(0.0f, viewCY, 0.0f);

	GL_UnmapVertexBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DeleteFullScreenVB() {
	glDeleteBuffers(1, &rFullScreenVB);
}

bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = WINDOW_CX;
	rViewport.height = WINDOW_CY;
	rViewport.zNear = Z_NEAR;
	rViewport.zFar = Z_FAR;

	glViewport(0, 0, WINDOW_CX, WINDOW_CY);

	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 12.0f), 0.0f, 0.0f);

	if (!InitDepthPeelingRenderTargets(WINDOW_CX, WINDOW_CY)) {
		return false;
	}

	if (!InitOITPrograms()) {
		return false;
	}

	if (!InitBaseProgram()) {
		return false;
	}

	if (!InitModel()) {
		return false;
	}

	if (!InitFullScreenVB(WINDOW_CX, WINDOW_CY)) {
		return false;
	}

	glGenQueries(1, &rQueryId);

	return true;
}

void Shutdown() {
	glDeleteQueries(1, &rQueryId);

	DeleteFullScreenVB();
	DeleteModel();
	DeleteBaseProgram();
	DeleteOITPrograms();
	DeleteDepthPeelingRenderTargets();
}

void Reshape(int width, int height) {
	if (rViewport.width == width && rViewport.height == height) {
		return;
	}

	// reset viewport
	rViewport.width = width;
	rViewport.height = height;

	DeleteDepthPeelingRenderTargets();
	InitDepthPeelingRenderTargets(width, height);

	UpdateFullScreenVB(width, height);

	glViewport(0, 0, width, height);
}

void DrawModel() {
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, rModel.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)24);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rModel.ib);

	glDrawElements(GL_TRIANGLES, rModel.indexCount, rModel.indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void RenderFullscreenQuad() {
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, rFullScreenVB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*)0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
}

#define MAX_PEELED_LAYERS 64

float transmittance = 0.3f;
int  rTestDrawMode = 1;

const float BACKGROUND_COLOR[] = { 0.75f, 0.75f, 0.75f };

void Display() {
	glm::mat4 perspMatrix, orthoMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 mvpPerspMatrix, mvpOrthoMatrix;

	V_PerspectiveMatrix(rViewport, 45.0f, perspMatrix);
	V_OrthographicMatrix(rViewport.x, rViewport.y, rViewport.x + rViewport.width, rViewport.y + rViewport.width,
		-1.0f, 1.0f, orthoMatrix);
	V_ViewMatrix(rView, viewMatrix);

	mvpPerspMatrix = perspMatrix * viewMatrix * rView.modelMatrix;
	mvpOrthoMatrix = orthoMatrix;

	if (0 == rTestDrawMode) {

		glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(rBaseProg.program);
		{
			glUniformMatrix4fv(rBaseProg_uModelViewMatrix, 1, GL_FALSE, &mvpPerspMatrix[0][0]);
			glUniform1f(rBaseProg_uAlpha, transmittance);

			if (!GL_ValidateProgram(rBaseProg)) {
				return;
			}

			GL_CHECKERROR;

			DrawModel();
		}
		glUseProgram(0);

		glDisable(GL_BLEND);

	}
	else {

		glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. peel the first layer
		glBindFramebuffer(GL_FRAMEBUFFER, rFrontColorBlenderFboId);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		glUseProgram(rFrontPeelingInitProg.program);
		{
			glUniformMatrix4fv(rFrontPeelingInit_uModelViewMatrix, 1, GL_FALSE, &mvpPerspMatrix[0][0]);
			glUniform1f(rFrontPeelingInit_uAlpha, transmittance);

			if (!GL_ValidateProgram(rFrontPeelingInitProg)) {
				return;
			}

			GL_CHECKERROR;

			DrawModel();
		}
		glUseProgram(0);

		// 2. depth peeling + blending
		for (int layer = 1; layer < MAX_PEELED_LAYERS; ++layer) {
			// 2.1 peel the next depth layer

			int currId = layer % 2;
			int prevId = 1 - currId;

			glBindFramebuffer(GL_FRAMEBUFFER, rFrontFboId[currId]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);

			glBeginQuery(GL_SAMPLES_PASSED, rQueryId);

			glUseProgram(rFrontPeelingPeelProg.program);
			{
				glUniformMatrix4fv(rFrontPeelingPeel_uModelViewMatrix, 1, GL_FALSE, &mvpPerspMatrix[0][0]);
				glUniform1f(rFrontPeelingPeel_uAlpha, transmittance);
				glUniform1i(rFrontPeelingPeel_DepthTex, 0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_RECTANGLE, rFrontDepthTexId[prevId]);

				if (!GL_ValidateProgram(rFrontPeelingPeelProg)) {
					return;
				}

				DrawModel();
			}
			glUseProgram(0);

			glEndQuery(GL_SAMPLES_PASSED);

			GL_CHECKERROR;

			// 2.2 blend the current layer

			glBindFramebuffer(GL_FRAMEBUFFER, rFrontColorBlenderFboId);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);

			// UNDER operator
			glBlendEquation(GL_FUNC_ADD);
			glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

			glUseProgram(rFrontPeelingBlendProg.program);
			{
				glUniformMatrix4fv(rFrontPeelingBlend_uModelViewMatrix, 1, GL_FALSE, &mvpOrthoMatrix[0][0]);
				glUniform1i(rFrontPeelingBlend_TempTex, 0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_RECTANGLE, rFrontColorTexId[currId]);

				if (!GL_ValidateProgram(rFrontPeelingBlendProg)) {
					return;
				}

				RenderFullscreenQuad();
			}
			glUseProgram(0);

			glDisable(GL_BLEND);

			GL_CHECKERROR;

			GLuint sample_count;
			glGetQueryObjectuiv(rQueryId, GL_QUERY_RESULT, &sample_count);
			if (sample_count == 0) {
				break;
			}
		}

		// 3. compositing pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to main draw buffer
		glDrawBuffer(GL_BACK);

		glDisable(GL_DEPTH_TEST);

		glUseProgram(rFrontPeelingFinalProg.program);
		{
			glUniformMatrix4fv(rFrontPeelingFinal_uModelViewMatrix, 1, GL_FALSE, &mvpOrthoMatrix[0][0]);
			glUniform3f(rFrontPeelingFinal_uBackgroundColor, BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2]);
			glUniform1i(rFrontPeelingFinal_ColorTex, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, rFrontColorBlenderTexId);

			if (!GL_ValidateProgram(rFrontPeelingFinalProg)) {
				return;
			}

			RenderFullscreenQuad();
		}
		glUseProgram(0);

		GL_CHECKERROR;

	}

	glutSwapBuffers();
}

void Special(int key, int x, int y) {
	if (GLUT_KEY_F2 == key) {
		rTestDrawMode = 1 - rTestDrawMode;
		glutPostRedisplay();
	}
	else if (GLUT_KEY_UP == key) {
		transmittance += 0.1f;
		if (transmittance > 1.0f) {
			transmittance = 1.0f;
		}
		glutPostRedisplay();
	}
	else if (GLUT_KEY_DOWN == key) {
		transmittance -= 0.1f;
		if (transmittance < 0.0f) {
			transmittance = 0.0f;
		}
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ComParam param;
	param.title = "Depth Peeling";
	param.windowCX = WINDOW_CX;
	param.windowCY = WINDOW_CY;
	param.view = &rView;
	param.Setup = Setup;
	param.Shutdown = Shutdown;
	param.Reshape = Reshape;
	param.Display = Display;
	param.Special = Special;

	Com_Run(argc, argv, param);

	return 0;
}
