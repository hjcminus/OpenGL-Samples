//2016-02-18 Thu. modified from Nvidia sample: dual_depth_peeling

#include "../common/common.h"
#include <crtdbg.h>

/*
================================================================================
dual depth peeling
================================================================================
*/

//framebuffer
GLuint rDualPeelingSingleFboId;
GLuint rDualBackBlenderFboId;

//depth texture
GLuint rDualDepthTexId[2];

//color texture
GLuint rDualFrontBlenderTexId[2];
GLuint rDualBackTempTexId[2];
GLuint rDualBackBlenderTexId;

bool InitDepthPeelingRenderTargets(int viewCX, int viewCY) {
	//get texture name
	glGenTextures(2, rDualDepthTexId);
	glGenTextures(2, rDualFrontBlenderTexId);
	glGenTextures(2, rDualBackTempTexId);
	glGenTextures(1, &rDualBackBlenderTexId);
	//get framebuffer name
	glGenFramebuffers(1, &rDualPeelingSingleFboId);
	glGenFramebuffers(1, &rDualBackBlenderFboId);

	GL_CHECKERROR;

	for (int i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_RECTANGLE, rDualDepthTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, /*GL_FLOAT_RG32_NV*/ GL_RG32F, viewCX, viewCY, 0, GL_RGB, GL_FLOAT, 0); //My: store -max z, max z in R, G channel as float value
		
		// 2024.02.04 Sun. GL_FLOAT_RG32_NV not supported by AMD, use GL_RG32F instead

		GL_CHECKERROR;

		glBindTexture(GL_TEXTURE_RECTANGLE, rDualFrontBlenderTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, viewCX, viewCY, 0, GL_RGBA, GL_FLOAT, 0);

		GL_CHECKERROR;

		glBindTexture(GL_TEXTURE_RECTANGLE, rDualBackTempTexId[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, viewCX, viewCY, 0, GL_RGBA, GL_FLOAT, 0);

		GL_CHECKERROR;
	}

	GL_CHECKERROR;

	glBindTexture(GL_TEXTURE_RECTANGLE, rDualBackBlenderTexId);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, viewCX, viewCY, 0, GL_RGB, GL_FLOAT, 0);

	GL_CHECKERROR;

	//seup framebuffers
	glBindFramebuffer(GL_FRAMEBUFFER, rDualPeelingSingleFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rDualDepthTexId[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, rDualFrontBlenderTexId[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, rDualBackTempTexId[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_RECTANGLE, rDualDepthTexId[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_RECTANGLE, rDualFrontBlenderTexId[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_RECTANGLE, rDualBackTempTexId[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_RECTANGLE, rDualBackBlenderTexId, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, rDualBackBlenderFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rDualBackBlenderTexId, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_CHECKERROR;

	return true;
}

void DeleteDepthPeelingRenderTargets()
{
	glDeleteFramebuffers(1, &rDualBackBlenderFboId);
	glDeleteFramebuffers(1, &rDualPeelingSingleFboId);

	glDeleteTextures(1, &rDualBackBlenderTexId);
	glDeleteTextures(2, rDualBackTempTexId);
	glDeleteTextures(2, rDualFrontBlenderTexId);
	glDeleteTextures(2, rDualDepthTexId);
}

//dual peeling init program
GLProgram rDualPeelingInitProg;
GLint rDualPeelingInit_uModelViewMatrix;

//dual peeling peel program
GLProgram rDualPeelingPeelProg;
GLint rDualPeelingPeel_uModelViewMatrix;
GLint rDualPeelingPeel_DepthBlenderTex;
GLint rDualPeelingPeel_FrontBlenderTex;
GLint rDualPeelingPeel_Alpha;

//dual peeling blend program
GLProgram rDualPeelingBlendProg;
GLint rDualPeelingBlend_uModelViewMatrix;
GLint rDualPeelingBlend_TempTex;

//dual peeling final program
GLProgram rDualPeelingFinalProg;
GLint rDualPeelingFinal_uModelViewMatrix;
GLint rDualPeelingFinal_FrontBlenderTex;
GLint rDualPeelingFinal_BackBlenderTex;

bool InitDepthPeelingPrograms() {
	//dual peeling init program
	if (!GL_CreateProgram(L"dual_depth_peeling/dual_peeling_init_vs.txt", L"dual_depth_peeling/dual_peeling_init_fs.txt", rDualPeelingInitProg)) {
		return false;
	}

	if (!GL_LinkProgram(rDualPeelingInitProg)) {
		GL_DestroyProgram(rDualPeelingInitProg);
		return false;
	}

	rDualPeelingInit_uModelViewMatrix = glGetUniformLocation(rDualPeelingInitProg.program, "uModelViewMatrix");

	//dual peeling peel program
	if (!GL_CreateProgram(L"dual_depth_peeling/dual_peeling_peel_vs.txt", L"dual_depth_peeling/dual_peeling_peel_fs.txt", rDualPeelingPeelProg)) {
		return false;
	}

	if (!GL_LinkProgram(rDualPeelingPeelProg)) {
		GL_DestroyProgram(rDualPeelingPeelProg);
		return false;
	}

	rDualPeelingPeel_uModelViewMatrix = glGetUniformLocation(rDualPeelingPeelProg.program, "uModelViewMatrix");
	rDualPeelingPeel_DepthBlenderTex = glGetUniformLocation(rDualPeelingPeelProg.program, "DepthBlenderTex");
	rDualPeelingPeel_FrontBlenderTex = glGetUniformLocation(rDualPeelingPeelProg.program, "FrontBlenderTex");
	rDualPeelingPeel_Alpha = glGetUniformLocation(rDualPeelingPeelProg.program, "Alpha");

	//dual peeling blend program
	if (!GL_CreateProgram(L"dual_depth_peeling/dual_peeling_blend_vs.txt", L"dual_depth_peeling/dual_peeling_blend_fs.txt", rDualPeelingBlendProg)) {
		return false;
	}

	if (!GL_LinkProgram(rDualPeelingBlendProg)) {
		GL_DestroyProgram(rDualPeelingBlendProg);
		return false;
	}

	rDualPeelingBlend_uModelViewMatrix = glGetUniformLocation(rDualPeelingBlendProg.program, "uModelViewMatrix");
	rDualPeelingBlend_TempTex = glGetUniformLocation(rDualPeelingBlendProg.program, "TempTex");

	//dual peeling final program
	if (!GL_CreateProgram(L"dual_depth_peeling/dual_peeling_final_vs.txt", L"dual_depth_peeling/dual_peeling_final_fs.txt", rDualPeelingFinalProg)) {
		return false;
	}

	if (!GL_LinkProgram(rDualPeelingFinalProg)) {
		GL_DestroyProgram(rDualPeelingFinalProg);
		return false;
	}

	rDualPeelingFinal_uModelViewMatrix = glGetUniformLocation(rDualPeelingFinalProg.program, "uModelViewMatrix");
	rDualPeelingFinal_FrontBlenderTex = glGetUniformLocation(rDualPeelingFinalProg.program, "FrontBlenderTex");
	rDualPeelingFinal_BackBlenderTex = glGetUniformLocation(rDualPeelingFinalProg.program, "BackBlenderTex");
	
	return true;
}

void DeleteDepthPeelingPrograms() {
	GL_DestroyProgram(rDualPeelingFinalProg);
	GL_DestroyProgram(rDualPeelingBlendProg);
	GL_DestroyProgram(rDualPeelingPeelProg);
	GL_DestroyProgram(rDualPeelingInitProg);
}

/*
================================================================================
base program
================================================================================
*/

GLProgram rBaseProg;
GLint rBaseProg_uModelViewMatrix;
GLint rBaseProg_uAlpha;

bool InitBaseProgram() {
	//front peeling init program
	if (!GL_CreateProgram(L"dual_depth_peeling/base_vs.txt", L"dual_depth_peeling/base_fs.txt", rBaseProg)) {
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

/*
================================================================================
framework
================================================================================
*/

Viewport    rViewport;
View        rView;
ModelBuffer rModel;
float       rAlpha = 0.3f;
bool        rUseOQ = true;
int         rNumPass = 4;
GLuint      rQueryId; //query processed samples count
GLuint      rFullScreenVB;

bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = DEFAULT_WINDOW_CX;
	rViewport.height = DEFAULT_WINDOW_CY;
	rViewport.zNear = DEFAULT_Z_NEAR;
	rViewport.zFar = DEFAULT_Z_FAR;

	glViewport(0, 0, DEFAULT_WINDOW_CX, DEFAULT_WINDOW_CY);
	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 15.0f), 0.0f, 0.0f);
	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);

	if (!InitDepthPeelingRenderTargets(DEFAULT_WINDOW_CX, DEFAULT_WINDOW_CY)) {
		return false;
	}

	if (!InitDepthPeelingPrograms()) {
		return false;
	}

	if (!InitBaseProgram()) {
		return false;
	}

	if (!CreatePosNormalColorModel(L"LTM1030_RGB.model", rModel)) {
		return false;
	}

	if (!GL_CreateFullScreenVB(rFullScreenVB, false)) {
		return false;
	}

	glGenQueries(1, &rQueryId);

	return true;
}

void Shutdown() {
	glDeleteQueries(1, &rQueryId);

	GL_DestroyFullScreenVB(rFullScreenVB);
	DestroyPosNormalColorModel(rModel);
	DeleteBaseProgram();
	DeleteDepthPeelingPrograms();
	DeleteDepthPeelingRenderTargets();
}

void Reshape(int width, int height) {
	if (rViewport.width == width && rViewport.height == height) {
		return;
	}

	rViewport.width = width;
	rViewport.height = height;
	glViewport(0, 0, width, height);

	DeleteDepthPeelingRenderTargets();
	InitDepthPeelingRenderTargets(width, height);
}

const GLenum DRAW_BUFFERS[] = { 
	GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,
    GL_COLOR_ATTACHMENT5,
    GL_COLOR_ATTACHMENT6
};

int  rTestDrawMode = 1;

void Display_Base() {
	glm::mat4 perspMatrix, orthoMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 mvpPerspMatrix, mvpOrthoMatrix;

	V_PerspectiveMatrix(rViewport, 45.0f, perspMatrix);
	V_ViewMatrix(rView, viewMatrix);

	mvpPerspMatrix = perspMatrix * viewMatrix * rView.modelMatrix;

	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(rBaseProg.program);
	{
		glUniformMatrix4fv(rBaseProg_uModelViewMatrix, 1, GL_FALSE, &mvpPerspMatrix[0][0]);
		glUniform1f(rBaseProg_uAlpha, rAlpha);

		if (!GL_ValidateProgram(rBaseProg)) {
			return;
		}

		GL_CHECKERROR;

		DrawPosNormalColorModel(rModel, false, true);
	}
	glUseProgram(0);

	glDisable(GL_BLEND);

	glutSwapBuffers();
}

void Display_DualDepthPeeling() {
	glm::mat4 perspMatrix, orthoMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 mvpPerspMatrix, mvpOrthoMatrix;

	V_PerspectiveMatrix(rViewport, 45.0f, perspMatrix);
	V_OrthoFullScreenMatrix(orthoMatrix);
	V_ViewMatrix(rView, viewMatrix);

	mvpPerspMatrix = perspMatrix * viewMatrix * rView.modelMatrix;
	mvpOrthoMatrix = orthoMatrix;


	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// ---------------------------------------------------------------------
	// 1. Initialize Min-Max Depth Buffer
	// ---------------------------------------------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, rDualPeelingSingleFboId);

	// Render targets 1 and 2 store the front and back colors
	// Clear to 0.0 and use MAX blending to filter written color
	// At most one front color and one back color can be written every pass
	glDrawBuffers(2, &DRAW_BUFFERS[1]); //clear buffer 1, 2
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Render target 0 stores (-minDepth, maxDepth, alphaMultiplier)
	glDrawBuffer(DRAW_BUFFERS[0]); //draw to buffer 0
	glClearColor(-DEFAULT_MAX_DEPTH, -DEFAULT_MAX_DEPTH, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendEquation(GL_MAX);

	GL_CHECKERROR;

	glUseProgram(rDualPeelingInitProg.program);
	{
		glUniformMatrix4fv(rDualPeelingInit_uModelViewMatrix, 1, GL_FALSE, &mvpPerspMatrix[0][0]);
		DrawPosNormalColorModel(rModel, false, false);
	}
	glUseProgram(0);

	GL_CHECKERROR;

	// ---------------------------------------------------------------------
	// 2. Dual Depth Peeling + Blending
	// ---------------------------------------------------------------------

	// Since we cannot blend the back colors in the geometry passes,
	// we use another render target to do the alpha blending
	glDrawBuffer(DRAW_BUFFERS[6]);
	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	int currId = 0;

	for (int pass = 1; rUseOQ || pass < rNumPass; ++pass) {
		currId = pass % 2;
		int prevId = 1 - currId;
		int bufId = currId * 3;

		glDrawBuffers(2, &DRAW_BUFFERS[bufId + 1]);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawBuffer(DRAW_BUFFERS[bufId + 0]);
		glClearColor(-DEFAULT_MAX_DEPTH, -DEFAULT_MAX_DEPTH, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render target 0: RG32F MAX blending
		// Render target 1: RGBA MAX blending
		// Render target 2: RGBA MAX blending
		glDrawBuffers(3, &DRAW_BUFFERS[bufId + 0]);
		glBlendEquation(GL_MAX);

		glUseProgram(rDualPeelingPeelProg.program);
		{
			glUniformMatrix4fv(rDualPeelingPeel_uModelViewMatrix, 1, GL_FALSE, &mvpPerspMatrix[0][0]);
			glUniform1i(rDualPeelingPeel_DepthBlenderTex, 0);
			glUniform1i(rDualPeelingPeel_FrontBlenderTex, 1);
			glUniform1f(rDualPeelingPeel_Alpha, rAlpha);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, rDualDepthTexId[prevId]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_RECTANGLE, rDualFrontBlenderTexId[prevId]);

			DrawPosNormalColorModel(rModel, false, true);
		}
		glUseProgram(0);

		GL_CHECKERROR;

		// Full screen pass to alpha-blend the back color
		glDrawBuffer(DRAW_BUFFERS[6]);

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (rUseOQ) {
			glBeginQuery(GL_SAMPLES_PASSED, rQueryId);
		}

		glUseProgram(rDualPeelingBlendProg.program);
		{
			glUniformMatrix4fv(rDualPeelingBlend_uModelViewMatrix, 1, GL_FALSE, &mvpOrthoMatrix[0][0]);
			glUniform1i(rDualPeelingBlend_TempTex, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, rDualBackTempTexId[currId]);

			GL_DrawFullScreenRect(rFullScreenVB, false);
		}
		glUseProgram(0);

		GL_CHECKERROR;

		if (rUseOQ) {
			glEndQuery(GL_SAMPLES_PASSED);
			GLuint sample_count;
			glGetQueryObjectuiv(rQueryId, GL_QUERY_RESULT, &sample_count);
			if (sample_count == 0) {
				break;
			}
		}
	}

	glDisable(GL_BLEND);

	// ---------------------------------------------------------------------
	// 3. Final Pass
	// ---------------------------------------------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	glUseProgram(rDualPeelingFinalProg.program);
	{
		glUniformMatrix4fv(rDualPeelingFinal_uModelViewMatrix, 1, GL_FALSE, &mvpOrthoMatrix[0][0]);
		glUniform1i(rDualPeelingFinal_FrontBlenderTex, 0);
		glUniform1i(rDualPeelingFinal_BackBlenderTex, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, rDualFrontBlenderTexId[currId]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_RECTANGLE, rDualBackBlenderTexId);

		GL_DrawFullScreenRect(rFullScreenVB, false);
	}
	glUseProgram(0);

	glutSwapBuffers();

	GL_CHECKERROR;
}

void Display() {
	if (1 == rTestDrawMode) {
		Display_DualDepthPeeling();
	}
	else {
		Display_Base();
	}
}

void Special(int key, int x, int y) {
	if (GLUT_KEY_F2 == key) {
		rTestDrawMode = 1 - rTestDrawMode;
		glutPostRedisplay();
	}
	else if (GLUT_KEY_F3 == key) {
		rUseOQ = !rUseOQ;
	}
	else if (GLUT_KEY_UP == key) {
		rAlpha += 0.1f;
		if (rAlpha > 1.0f) {
			rAlpha = 1.0f;
		}
		glutPostRedisplay();
	}
	else if (GLUT_KEY_DOWN == key) {
		rAlpha -= 0.1f;
		if (rAlpha < 0.0f) {
			rAlpha = 0.0f;
		}
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ComParam param;
	param.title = "Dual Depth Peeling";
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
