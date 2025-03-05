/******************************************************************************
 * @file	tessellation.cpp
 * @brief   test TCS & TES
 *****************************************************************************/

#include "../Common/Common.h"

Viewport rViewport;
View rView;

//shader
GLProgram lighting_prog;
GLProgram tessellation_prog;

//model
ModelBuffer rModel;

bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = DEFAULT_WINDOW_CX;
	rViewport.height = DEFAULT_WINDOW_CY;
	rViewport.zNear = DEFAULT_Z_NEAR;
	rViewport.zFar = DEFAULT_Z_FAR;

	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 6.0f), 0.0f, 0.0f);
	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glPatchParameteri(GL_PATCH_VERTICES, 3);

	bool b1 = GL_CreateProgram(L"tessellation/lighting_vs.txt", L"tessellation/lighting_fs.txt", lighting_prog);
	bool b2 = GL_CreateProgram(L"tessellation/tessellation_vs.txt", L"tessellation/tessellation_tcs.txt", L"tessellation/tessellation_tes.txt", L"tessellation/tessellation_fs.txt", tessellation_prog);

	if (b1 && b2) {
		GL_LinkProgram(lighting_prog);
		GL_LinkProgram(tessellation_prog);
	}

	if (!CreatePosNormalColorModel(L"cuboid.model", rModel)) {
		return false;
	}

	return true;
}

void Shutdown() {
	GL_DestroyProgram(lighting_prog);
	GL_DestroyProgram(tessellation_prog);
	DestroyPosNormalColorModel(rModel);
}

void Reshape(int width, int height) {
	if (rViewport.width == width && rViewport.height == height) {
		return;
	}

	rViewport.width = width;
	rViewport.height = height;
	glViewport(0, 0, width, height);
}

bool wireframe = false;
bool tessellation = true;

void Display() {
	glEnable(GL_DEPTH_TEST);

	glm::mat4 matProj;
	glm::mat4 matView;
	V_PerspectiveMatrix(rViewport, 22.5f, matProj);
	V_ViewMatrix(rView, matView);

	glm::mat4 matModelView = matView * rView.modelMatrix;
	glm::mat4 matModelViewProj = matProj * matView * rView.modelMatrix;

	glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);
	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GL_UseProgram(tessellation ? tessellation_prog : lighting_prog);
	{
		GL_UniformMatrix4fv(tessellation ? tessellation_prog : lighting_prog, "modelViewProj", matModelViewProj);
		GL_UniformMatrix4fv(tessellation ? tessellation_prog : lighting_prog, "modelView", matModelView);

		GL_Uniform3f(tessellation ? tessellation_prog : lighting_prog, "viewPos", rView.pos[0], rView.pos[1], rView.pos[2]);

		GL_Uniform4f(tessellation ? tessellation_prog : lighting_prog, "materialAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
		GL_Uniform4f(tessellation ? tessellation_prog : lighting_prog, "materialDiffuse", 0.5f, 0.5f, 0.5f, 1.0f);
		GL_Uniform4f(tessellation ? tessellation_prog : lighting_prog, "materialSpecular", 1.0f, 1.0f, 1.0f, 1.0f);
		GL_Uniform1f(tessellation ? tessellation_prog : lighting_prog, "materialShiness", 16.0f);

		GL_Uniform3f(tessellation ? tessellation_prog : lighting_prog, "lightDirection", 0.0f, 0.0f, -1.0f);

		GL_Uniform4f(tessellation ? tessellation_prog : lighting_prog, "lightAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
		GL_Uniform4f(tessellation ? tessellation_prog : lighting_prog, "lightDiffuse", 0.9f, 0.9f, 0.9f, 1.0f);
		GL_Uniform4f(tessellation ? tessellation_prog : lighting_prog, "lightSpecular", 1.0f, 1.0f, 1.0f, 1.0f);

		DrawPosNormalColorModel(rModel, true, false, tessellation);
	}

	glutSwapBuffers();
}

void Special(int key, int x, int y) {
	if (key == GLUT_KEY_F2) {
		tessellation = !tessellation;
		glutPostRedisplay();
	}
	else if (key == GLUT_KEY_F3) {
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ComParam param;
	param.title = "Tessellation";
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
