/******************************************************************************
 * @file	common.cpp
 * @brief
 *****************************************************************************/

#include "common.h"

#if defined(_WIN32)
# include <mmsystem.h>
# pragma comment(lib, "winmm")
#else
# include <chrono>
# include <unistd.h>
#endif

/*
================================================================================
Common
================================================================================
*/
const wchar_t * DATA_DIR;
const wchar_t * TEXTURE_DIR;
const wchar_t * MODEL_DIR;
const wchar_t * SHADER_DIR;

static void ExtractParentDir(const wchar_t *path, wchar_t *dir) {
	dir[0] = 0;
	int l = (int)wcslen(path);
	for (int i = l - 1; i >= 0; --i) {
		if (path[i] == L'\\' || path[i] == L'/') {
			int fileDirElementCount = i;
			memcpy(dir, path, fileDirElementCount * sizeof(wchar_t));
			dir[fileDirElementCount] = 0;
			break;
		}
	}
}

static void OnReshape(int width, int height);
static void OnMouse(int button, int state, int x, int y);
static void OnMotion(int x, int y);
static void OnMouseWheel(int wheel, int direction, int x, int y);
static void OnSpecial(int key, int x, int y);
static void OnSpecialUp(int key, int x, int y);
static void OnKeyboard(unsigned char key, int x, int y);
static void OnKeyboardUp(unsigned char key, int x, int y);
static void OnIdle();
static void OnDisplay();
static void OnTime(int value);
static void OnClose();

static ComParam	comParam;

static int		rCursorDeltaX;
static int		rCursorDeltaY;
static int		rForward;
static int		rRight;
static float	CURSOR_ROTATION_SCALE = 0.2f;
float			gMovingSpeed = 10.0f;

// mouse
static bool		leftButtonDown;
static int		cursorPosX;
static int		cursorPosY;
static int		previousPosX;
static int		previousPosY;

void Com_Run(int argc, char **argv, const ComParam &param) {
#if defined(_WIN32)
	// setup timer
	timeBeginPeriod(1);
#endif

	comParam = param;

	// setup directory path

	static wchar_t dataDir[MAX_PATH];
	static wchar_t textureDir[MAX_PATH];
	static wchar_t modelDir[MAX_PATH];
	static wchar_t shaderDir[MAX_PATH];

#if defined(_WIN32)
	wchar_t buffer1[MAX_PATH], buffer2[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), buffer1, MAX_PATH);

	ExtractParentDir(buffer1, buffer2); // .\msvc\bin\Debug
	ExtractParentDir(buffer2, buffer1); // .\msvc\bin
	ExtractParentDir(buffer1, buffer2); // .\msvc
	ExtractParentDir(buffer2, buffer1); // .

	wsprintf(dataDir,    L"%s/data",          buffer1);
	wsprintf(textureDir, L"%s/data/textures", buffer1);
	wsprintf(modelDir, L"%s/data/models", buffer1);
	wsprintf(shaderDir, L"%s/data/shaders", buffer1);
#endif

#if defined(__linux__)
	wchar_t buffer1[MAX_PATH], buffer2[MAX_PATH];

	mbstowcs(buffer1, argv[0], MAX_PATH);

	ExtractParentDir(buffer1, buffer2); // .\gmake2\bin
	ExtractParentDir(buffer2, buffer1); // .\gmake2
	ExtractParentDir(buffer1, buffer2); // .

	swprintf_s(dataDir, MAX_PATH, L"%s/data", buffer2);
	swprintf_s(textureDir, MAX_PATH, L"%s/data/textures", buffer2);
	swprintf_s(modelDir, MAX_PATH, L"%s/data/models", buffer2);
	swprintf_s(shaderDir, MAX_PATH, L"%s/data/shaders", buffer2);
#endif

	DATA_DIR    = dataDir;
	TEXTURE_DIR = textureDir;
	MODEL_DIR   = modelDir;
	SHADER_DIR  = shaderDir;

	// setup render environment
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	int screen_cx = glutGet(GLUT_SCREEN_WIDTH);
	int screen_cy = glutGet(GLUT_SCREEN_HEIGHT);

	int pos_x = (screen_cx - comParam.windowCX) >> 1;
	int pos_y = (screen_cy - comParam.windowCY) >> 1;

	glutInitWindowPosition(pos_x, pos_y);
	glutInitWindowSize(comParam.windowCX, comParam.windowCY);
	glutCreateWindow(param.title);

	if (GLEW_OK != glewInit()) {
		return;
	}

	if (comParam.Setup && !comParam.Setup()) {
		return;
	}

	// init callback functions
	glutReshapeFunc(param.Reshape);
	glutMouseFunc(OnMouse);
	glutMotionFunc(OnMotion);
	glutMouseWheelFunc(OnMouseWheel);
	glutSpecialFunc(OnSpecial);
	glutSpecialUpFunc(OnSpecialUp);
	glutKeyboardFunc(OnKeyboard);
	glutKeyboardUpFunc(OnKeyboardUp);
	glutIdleFunc(OnIdle);
	glutDisplayFunc(param.Display);
	glutCloseFunc(OnClose);

	glutMainLoop();
}

uint32_t Com_GetMS() {
#if defined(_WIN32)
	return timeGetTime();
#else
	auto time_now = std::chrono::system_clock::now();
	auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
	return (uint32_t)duration_in_ms.count();
#endif
}

void Sys_Sleep(int ms) {
#if defined(_WIN32)
	Sleep((DWORD)ms);
#elif defined(__linux__)
	usleep(ms * 1000);
#endif
}

static void UpdateView(float deltaYaw, float deltaPitch, int forward, int right) {
	if (V_VIEW_MOVING_CAMERA == comParam.view->type) {
		comParam.view->pos.x = comParam.view->pos.x + comParam.view->forward.x * forward * gMovingSpeed;
		comParam.view->pos.y = comParam.view->pos.y + comParam.view->forward.y * forward * gMovingSpeed;
		comParam.view->pos.z = comParam.view->pos.z + comParam.view->forward.z * forward * gMovingSpeed;

		comParam.view->pos.x = comParam.view->pos.x + comParam.view->right.x * right * gMovingSpeed;
		comParam.view->pos.y = comParam.view->pos.y + comParam.view->right.y * right * gMovingSpeed;
		comParam.view->pos.z = comParam.view->pos.z + comParam.view->right.z * right * gMovingSpeed;

		comParam.view->yaw += deltaYaw;
		if (comParam.view->yaw > 360.0f) {
			comParam.view->yaw -= 360.0f;
		}
		if (comParam.view->yaw < 0.0f) {
			comParam.view->yaw += 360.0f;
		}

		comParam.view->pitch += deltaPitch;
		if (comParam.view->pitch > 89.0f) {
			comParam.view->pitch = 89.0f;
		}
		if (comParam.view->pitch < -89.0f) {
			comParam.view->pitch = -89.0f;
		}

		V_InitView(*comParam.view, V_VIEW_MOVING_CAMERA, comParam.view->pos, comParam.view->yaw, comParam.view->pitch);
	}
	else {
		comParam.view->yaw += -deltaYaw;
		if (comParam.view->yaw > 360.0f) {
			comParam.view->yaw -= 360.0f;
		}
		if (comParam.view->yaw < 0.0f) {
			comParam.view->yaw += 360.0f;
		}

		comParam.view->pitch += -deltaPitch;
		if (comParam.view->pitch > 360.0f) {
			comParam.view->pitch -= 360.0f;
		}
		if (comParam.view->pitch < 0.0f) {
			comParam.view->pitch += 360.0f;
		}

		glm::mat4 rotateXMatrix = glm::rotate(glm::mat4(1.0f), Math_DegToRad(comParam.view->yaw), glm::vec3(comParam.view->up));
		comParam.view->modelMatrix = glm::rotate(rotateXMatrix, Math_DegToRad(comParam.view->pitch), glm::vec3(comParam.view->right));
		// no translation for now
	}
	
	glutPostRedisplay();
}

// glut callbacks

static void OnMouse(int button, int state, int x, int y) {
	if (GLUT_LEFT_BUTTON == button) {
		if (GLUT_DOWN == state) {
			leftButtonDown = true;
			cursorPosX = x;
			cursorPosY = y;
			previousPosX = x;
			previousPosY = y;
		}
		else if (GLUT_UP == state) {
			leftButtonDown = false;
			return;
		}
	}
}

static void OnMotion(int x, int y) {
	cursorPosX = x;
	cursorPosY = y;
}

static void OnMouseWheel(int wheel, int direction, int x, int y) {
	UpdateView(0, 0, direction * 10, 0);
}

static void OnSpecial(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		rForward = 1;
		break;
	case GLUT_KEY_DOWN:
		rForward = -1;
		break;
	case GLUT_KEY_LEFT:
		rRight = -1;
		break;
	case GLUT_KEY_RIGHT:
		rRight = 1;
		break;
	}

	if (comParam.Special) {
		comParam.Special(key, x, y);
	}
}

static void OnSpecialUp(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
		rForward = 0;
		break;
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		rRight = 0;
		break;
	}
}

static void OnKeyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
	case 'W':
		rForward = 1;
		break;
	case 's':
	case 'S':
		rForward = -1;
		break;
	case 'a':
	case 'A':
		rRight = -1;
		break;
	case 'd':
	case 'D':
		rRight = 1;
		break;
	}
}

static void OnKeyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
	case 'W':
	case 's':
	case 'S':
		rForward = 0;
		break;
	case 'a':
	case 'A':
	case 'd':
	case 'D':
		rRight = 0;
		break;
	}
}

static void OnIdle() {
	static uint32_t time = 0;
	if (!time) {
		time = Com_GetMS();
	}
	else {
		uint32_t current = Com_GetMS();
		if (current - time >= 16) {
			rCursorDeltaX = cursorPosX - previousPosX;
			rCursorDeltaY = cursorPosY - previousPosY;

			previousPosX = cursorPosX;
			previousPosY = cursorPosY;

			int deltaX = 0;
			int deltaY = 0;

			if (leftButtonDown) {
				deltaX = rCursorDeltaX;
				deltaY = rCursorDeltaY;
			}

			if (deltaX || deltaY || rForward || rRight) {
				float deltaYaw = (float)-deltaX * CURSOR_ROTATION_SCALE;
				float deltaPitch = (float)-deltaY * CURSOR_ROTATION_SCALE;

				UpdateView(deltaYaw, deltaPitch, rForward, rRight);
			}
			time = current;
		}
	}
}

static void OnTime(int value) {
	//
}

static void OnClose() {
	if (comParam.Shutdown) {
		comParam.Shutdown();
	}

#if defined(_WIN32)
	timeEndPeriod(1);
#endif
}

/*
================================================================================
math
================================================================================
*/
int Math_IsPowerOf2(int n) {
	return n && !(n & (n - 1));
}

float Math_DegToRad(float d) {
	return d * MATH_PI / 180.0f;
}

float Math_RadToDeg(float r) {
	return r * 180.0f / MATH_PI;
}

/*
================================================================================
file
================================================================================
*/

#if defined(__linux__)
void _wfopen_s(FILE** f, const wchar_t* filename, const wchar_t* md) {
	char utf8_filename[MAX_PATH];
	char utf8_mode[MAX_PATH];

	wcstombs(utf8_filename, filename, MAX_PATH);
	wcstombs(utf8_mode, md, MAX_PATH);

	*f = fopen(utf8_filename, utf8_mode);
}
#endif

bool File_Read(const wchar_t *fileName, File &file, bool appendNullTernimator) {
	memset(&file, 0, sizeof(file));

	FILE * f = nullptr;
	_wfopen_s(&f, fileName, L"rb");
	if (!f) {
		return false;
	}

	fseek(f, 0, SEEK_END);
	file.original_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (file.original_size) {
		if (appendNullTernimator) {
			file.data = (byte*)malloc(file.original_size + 1);
			fread(file.data, 1, file.original_size, f);
			file.data[file.original_size] = 0;
			file.appended_size = file.original_size + 1;
			fclose(f);
			f = nullptr;
		}
		else {
			file.data = (byte*)malloc(file.original_size);
			fread(file.data, 1, file.original_size, f);
			file.appended_size = file.original_size;
			fclose(f);
			f = nullptr;
		}
	}

	return file.original_size > 0;
}

void File_Free(File &file) {
	if (file.data) {
		free(file.data);
		file.data = nullptr;
	}
	file.original_size = 0;
	file.appended_size = 0;
}

/*
================================================================================
View
================================================================================
*/
const float DEFAULT_BACKGROUND_COLOR[] = { 0.75f, 0.75f, 0.75f };

void V_InitView(View &view, int type, const glm::vec3 &pos, float yaw, float pitch) {
	view.type = type;
	view.pos = pos;

	glm::vec4 forward = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	glm::vec4 right = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), Math_DegToRad(yaw), glm::vec3(up));

	forward = rotationMatrix * forward;
	right = rotationMatrix * right;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	rotationMatrix = glm::rotate(glm::mat4(1.0f), Math_DegToRad(pitch), glm::vec3(right));

	forward = rotationMatrix * forward;
	right = rotationMatrix * right;

	view.forward = glm::normalize(glm::vec3(forward));
	view.right = glm::normalize(glm::vec3(right));
	view.up = glm::vec3(up);

	view.yaw = yaw;
	view.pitch = pitch;

	view.modelMatrix = glm::mat4(1.0f);
}

/*
================================================================================
transform
================================================================================
*/
void V_PerspectiveMatrix(const Viewport &viewport, float fovy, glm::mat4 &out) {
	out = glm::perspective(fovy * 3.1415926f / 180.0f, (float)viewport.width / viewport.height, viewport.zNear, viewport.zFar);
}

void V_OrthographicMatrix(float left, float bottom, float width, float height, float near_, float far_, glm::mat4& out) {
	out = glm::ortho(left, left + width, bottom, bottom + height, near_, far_);
}

void V_OrthoFullScreenMatrix(glm::mat4 &out) {
	out = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
}

void V_ViewMatrix(const View &view, glm::mat4 &out) {
	glm::vec3 target = view.pos + view.forward;
	out = glm::lookAt(view.pos, target, view.up);
}

/*
================================================================================
model
================================================================================
*/
bool CreatePosNormalColorModel(const wchar_t *fileName, ModelBuffer &model) {
	model.vb = 0;
	model.ib = 0;
	model.indexSize = 0;
	model.indexCount = 0;

	Model m;
	if (!Model_Load(fileName, &m)) {
		return false;
	}

	if (m.vertexSize != sizeof(VertexPosNormalRGB) && m.vertexSize != sizeof(VertexPosNormalRGBA)) {
		Model_Free(&m);
		return false;
	}

	model.vertexSize = m.vertexSize;
	model.vb = GL_CreateVertexBuffer(m.vertexSize, m.nVertices, m.vertices);
	model.ib = GL_CreateIndexBuffer(m.indexSize, m.nIndices, m.indices);
	model.indexSize = m.indexSize;
	model.indexCount = m.nIndices;

	Model_Free(&m);

	return true;
}

void DestroyPosNormalColorModel(ModelBuffer &model) {
	if (model.ib) {
		glDeleteBuffers(1, &model.ib);
		model.ib = 0;
	}
	
	if (model.vb) {
		glDeleteBuffers(1, &model.vb);
		model.vb = 0;
	}

	model.indexSize = 0;
	model.indexCount = 0;
}

void DrawPosNormalColorModel(const ModelBuffer &model, bool passNormal, bool passColor, bool drawPatch) {
	glEnableVertexAttribArray(0);
	int nextAttrIdx = 1;
	if (passNormal) {
		glEnableVertexAttribArray(nextAttrIdx);
		nextAttrIdx++;
	}
	if (passColor) {
		glEnableVertexAttribArray(nextAttrIdx);
	}

	glBindBuffer(GL_ARRAY_BUFFER, model.vb);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, model.vertexSize, (const GLvoid*)0);
	nextAttrIdx = 1;
	if (passNormal) {
		glVertexAttribPointer(nextAttrIdx, 3, GL_FLOAT, GL_FALSE, model.vertexSize, (const GLvoid*)12);
		nextAttrIdx++;
	}
	if (passColor) {
		glVertexAttribPointer(nextAttrIdx, 3, GL_FLOAT, GL_FALSE, model.vertexSize, (const GLvoid*)24);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ib);

	glDrawElements(drawPatch ? GL_PATCHES : GL_TRIANGLES, model.indexCount, model.indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	nextAttrIdx = 1;
	if (passNormal) {
		glDisableVertexAttribArray(nextAttrIdx);
		nextAttrIdx++;
	}
	if (passColor) {
		glDisableVertexAttribArray(nextAttrIdx);
	}

	glDisableVertexAttribArray(1);
}
