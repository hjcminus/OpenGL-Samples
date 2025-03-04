//2016-10-20 Thu.

#include "../common/common.h"
#include <crtdbg.h>

#include "fluid_system.h"
#include "volume_render.h"
#include "volume_buffer.h"

Viewport rViewport;
View rView;

FluidSystem  *fluidSys;
VolumeRender *volumeRender;

bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = DEFAULT_WINDOW_CX;
	rViewport.height = DEFAULT_WINDOW_CY;
	rViewport.zNear = DEFAULT_Z_NEAR;
	rViewport.zFar = DEFAULT_Z_FAR;

	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 4.0f), 0.0f, 0.0f);
	glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);

	fluidSys = new FluidSystem(128, 128, 128);
	fluidSys->Splat();

	volumeRender = new VolumeRender(fluidSys->GetStateBuffer());
	volumeRender->SetDensity(0.5f);
	volumeRender->SetBrightness(1.0f);

	return true;
}

void Shutdown() {
	delete volumeRender;
	delete fluidSys;
}

void Reshape(int width, int height) {
	if (rViewport.width == width && rViewport.height == height) {
		return;
	}

	rViewport.width = width;
	rViewport.height = height;
	glViewport(0, 0, width, height);
}

static bool animate = true;
static bool drawWireframe = true;
static bool drawSimple = false;

void Display() {
	//printf("Display\n");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 matProj;
	V_PerspectiveMatrix(rViewport, 70.0f, matProj);

	glm::mat4 matView;
	V_ViewMatrix(rView, matView);

	glm::mat4 matModelView = matView * rView.modelMatrix;

	if (animate) {
		if (((rand() & 0xff) > 220)) {
			fluidSys->Splat();
		}
		fluidSys->GetStateBuffer()->SetFiltering(GL_NEAREST);
		fluidSys->Step(1.0f);
	}

	glViewport(0, 0, rViewport.width, rViewport.height);
	fluidSys->GetStateBuffer()->SetFiltering(GL_LINEAR);
	volumeRender->SetVolume(fluidSys->GetStateBuffer());
	
	if (drawSimple) {
		volumeRender->DrawSimple(matProj, matModelView);
	}
	else {
		volumeRender->Render(matProj, matModelView);
	}

	if (drawWireframe) {
		volumeRender->DrawWireframe(matProj, matModelView);
	}

	glutSwapBuffers();

	GL_CHECKERROR;

	Sleep(50);
	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	if (GLUT_KEY_F2 == key) {
		drawWireframe = !drawWireframe;
		glutPostRedisplay();
	}

	if (GLUT_KEY_F3 == key) {
		drawSimple = !drawSimple;
		glutPostRedisplay();
	}

	if (GLUT_KEY_F4 == key) {
		animate = !animate;
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	printf("   F2    - Toggle draing the extends of the volume in wireframe\n");
	printf("   F3    - Toggle drawing in simple mode\n");
	printf("   F4    - Toggle animation\n");

	ComParam param;
	param.title = "3D Texture";
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
