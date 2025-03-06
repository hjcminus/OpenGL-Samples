/******************************************************************************
 * @file	shadow.cpp
 * @brief	This is a modified project of AMD sample
 *			using freeglut framework
 *****************************************************************************/

#include "../common/common.h"

Model model;

struct Vertex {
	float3 pos;
	float2 texCoord;
	float3 tangent;
	float3 binormal;
	float3 normal;
	float2 lmTexCoord;
};

static_assert(sizeof(Vertex) == 64, "bad size of Vertex");

#define WINDOW_CX 1024
#define WINDOW_CY 600
#define TITLE     "Shadow"

static bool Setup();
static void Shutdown();
static void Reshape(int width, int height);
static void Display();
static void Special(int key, int x, int y);

static View      rView;

int main(int argc, char **argv) {
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ComParam param;
	param.title = TITLE;
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

struct RBatch {
	uint32_t	firstIndex;
	uint32_t	nIndices;
};

struct RModel {
	RBatch * batchs;
	int      nBatchs;
	GLuint   vb;
	GLuint   ib;
};

#define R_PROGRAM_AMBIENT      0
#define R_PROGRAM_LIGHTING     1

#define R_NUM_PROGRAM          2

GLProgram	rProgram[R_NUM_PROGRAM];
RModel		rModel;
Viewport	rViewport;


struct Prog_Ambient_Uniforms {
	GLint mvp;
	GLint Base;
	GLint color;
};

struct Prog_Lighting_Uniforms {
	GLint lightPos;
	GLint camPos;
	GLint mvp;
	GLint Base;
	GLint Bump;
	GLint LightMap;
	GLint lightColor;
};

struct LightModelVertex {
	glm::vec3 pos;
	glm::vec2 texCoord;
};

struct LightModel {
	GLuint vb;
};

Prog_Ambient_Uniforms  progAmbientUniforms;
Prog_Lighting_Uniforms progLightingUniforms;

#define LIGHT_COUNT    2
#define TEXTURE_COUNT  9

glm::vec3  lights[LIGHT_COUNT];
glm::vec4  lightsColor[LIGHT_COUNT];
GLuint     rTextures[TEXTURE_COUNT];
LightModel rLightModel[LIGHT_COUNT];

int viewCX = WINDOW_CX;
int viewCY = WINDOW_CY;

static bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = WINDOW_CX;
	rViewport.height = WINDOW_CY;
	rViewport.zNear = 1.0f;
	rViewport.zFar = 4096.0f;

	V_InitView(rView, V_VIEW_MOVING_CAMERA,
		glm::vec3(860.0f, -200.0f, 100.0f), 115.0, 5.0);

	// Set up the lights
	lights[0].x = 200.0f;
	lights[0].y = 0.0f;
	lights[0].z = 750.0f;
	lights[1].x = -120.0f;
	lights[1].y = -100.0f;
	lights[1].z = -730.0f;

	lightsColor[0] = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
	lightsColor[1] = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// ambient
	if (!GL_CreateProgram(L"shadow/ambient_vs.txt", L"shadow/ambient_fs.txt", rProgram[R_PROGRAM_AMBIENT])) {
		return false;
	}

	glBindAttribLocation(rProgram[R_PROGRAM_AMBIENT].program, 0, "a_pos");
	glBindAttribLocation(rProgram[R_PROGRAM_AMBIENT].program, 1, "a_texCoord");

	if (!GL_LinkProgram(rProgram[R_PROGRAM_AMBIENT])) {
		GL_DestroyProgram(rProgram[R_PROGRAM_AMBIENT]);
		return false;
	}

	progAmbientUniforms.mvp = glGetUniformLocation(rProgram[R_PROGRAM_AMBIENT].program, "mvp");
	progAmbientUniforms.Base = glGetUniformLocation(rProgram[R_PROGRAM_AMBIENT].program, "Base");
	progAmbientUniforms.color = glGetUniformLocation(rProgram[R_PROGRAM_AMBIENT].program, "color");

	// lighting
	if (!GL_CreateProgram(L"shadow/lighting_vs.txt", L"shadow/lighting_fs.txt", rProgram[R_PROGRAM_LIGHTING])) {
		return false;
	}

	glBindAttribLocation(rProgram[R_PROGRAM_LIGHTING].program, 0, "a_pos");
	glBindAttribLocation(rProgram[R_PROGRAM_LIGHTING].program, 1, "a_texCoord");
	glBindAttribLocation(rProgram[R_PROGRAM_LIGHTING].program, 2, "a_tangent");
	glBindAttribLocation(rProgram[R_PROGRAM_LIGHTING].program, 3, "a_binormal");
	glBindAttribLocation(rProgram[R_PROGRAM_LIGHTING].program, 4, "a_normal");
	glBindAttribLocation(rProgram[R_PROGRAM_LIGHTING].program, 5, "a_lmCoord");

	if (!GL_LinkProgram(rProgram[R_PROGRAM_LIGHTING])) {
		GL_DestroyProgram(rProgram[R_PROGRAM_LIGHTING]);
		return false;
	}

	progLightingUniforms.lightPos = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "lightPos");
	progLightingUniforms.camPos = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "camPos");
	progLightingUniforms.mvp = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "mvp");
	progLightingUniforms.Base = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "Base");
	progLightingUniforms.Bump = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "Bump");
	progLightingUniforms.LightMap = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "LightMap");
	progLightingUniforms.lightColor = glGetUniformLocation(rProgram[R_PROGRAM_LIGHTING].program, "lightColor");

	//load textures
	rTextures[0] = GL_CreateTexture(L"StoneWall2.bmp", true);
	rTextures[1] = GL_CreateTexture(L"Tiles.bmp", true);
	rTextures[2] = GL_CreateTexture(L"brick2.bmp", true);
	rTextures[3] = GL_CreateTexture(L"StoneWallBump.bmp", true);
	rTextures[4] = GL_CreateTexture(L"TilesBump.bmp", true);
	rTextures[5] = GL_CreateTexture(L"brick2Bump.bmp", true);
	rTextures[6] = GL_CreateTexture(L"LightMap/Map2_LightMap0.bmp", true);
	rTextures[7] = GL_CreateTexture(L"LightMap/Map2_LightMap1.bmp", true);
	rTextures[8] = GL_CreateTexture(L"Particle.bmp", true);

	//load model
	if (!Model_Load(L"Map2.model", &model)) {
		return false;
	}

	rModel.batchs = (RBatch*)malloc(sizeof(RBatch) * model.nBatches);
	rModel.nBatchs = model.nBatches;

	rModel.vb = GL_CreateVertexBuffer(sizeof(Vertex), model.nVertices, (Vertex*)model.vertices);
	rModel.ib = GL_CreateIndexBuffer(sizeof(uint16_t), model.nIndices, (uint16_t*)model.indices);

	for (uint32_t i = 0; i < model.nBatches; ++i) {
		Batch * batch = model.batches + i;

		RBatch * rBatch = rModel.batchs + i;
		rBatch->firstIndex = batch->firstIndex;
		rBatch->nIndices = batch->nIndices;
	}

	Model_Free(&model);

	//light model
	rLightModel[0].vb = GL_CreateVertexBuffer(sizeof(LightModelVertex), 4, nullptr);
	rLightModel[1].vb = GL_CreateVertexBuffer(sizeof(LightModelVertex), 4, nullptr);

	return true;
}

static void Shutdown() {
	glDeleteBuffers(1, &rLightModel[1].vb);
	glDeleteBuffers(1, &rLightModel[0].vb);

	glDeleteBuffers(1, &rModel.ib);
	glDeleteBuffers(1, &rModel.vb);

	if (rModel.batchs) {
		free(rModel.batchs);
		rModel.batchs = nullptr;
	}

	glDeleteTextures(TEXTURE_COUNT, rTextures);

	GL_DestroyProgram(rProgram[R_PROGRAM_LIGHTING]);
	GL_DestroyProgram(rProgram[R_PROGRAM_AMBIENT]);
}

static void Reshape(int width, int height) {
	rViewport.width = width;
	rViewport.height = height;
}

void DrawAmbient(const glm::mat4& mvpMatrix);
void DrawLight(const glm::mat4& mvpMatrix);
void DrawLightModel(const glm::mat4& mvpMatrix);

static void Display() {
	glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//setup matrix
	glm::mat4 projectionMatrix;
	V_PerspectiveMatrix(rViewport, 45.0f, projectionMatrix);

	glm::mat4 viewMatrix;
	V_ViewMatrix(rView, viewMatrix);

	glm::mat4 mvpMatrix = projectionMatrix * viewMatrix;

	DrawAmbient(mvpMatrix);
	DrawLight(mvpMatrix);
	DrawLightModel(mvpMatrix);

	glutSwapBuffers();
}

static int rCursorDeltaX;
static int rCursorDeltaY;
static int rForward;
static int rRight;

static float CURSOR_ROTATION_SCALE = 0.2f;
static float MOVING_SPEED = 10.0f;

static void Special(int key, int x, int y) {
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
}

static void DrawAmbient(const glm::mat4 &mvpMatrix) {
	glDisable(GL_BLEND);
	glUseProgram(rProgram[R_PROGRAM_AMBIENT].program);

	glUniformMatrix4fv(progAmbientUniforms.mvp, 1, GL_FALSE, &mvpMatrix[0][0]); // set uniform value
	glm::vec4 color = glm::vec4(0.1f);
	glUniform4fv(progAmbientUniforms.color, 1, &color.x);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, rModel.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rModel.ib);

	for (int i = 0; i < rModel.nBatchs; ++i) {
		RBatch * rBatch = rModel.batchs + i;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rTextures[i]);

		glDrawElements(GL_TRIANGLES, rBatch->nIndices, GL_UNSIGNED_SHORT, (const GLvoid*)(rBatch->firstIndex * sizeof(uint16_t)));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);

	glEnable(GL_BLEND);
}

static void DrawLight(const glm::mat4 &mvpMatrix) {
	glUseProgram(rProgram[R_PROGRAM_LIGHTING].program);

	glUniform3fv(progLightingUniforms.camPos, 1, &rView.pos.x);
	glUniformMatrix4fv(progLightingUniforms.mvp, 1, GL_FALSE, &mvpMatrix[0][0]); // set uniform value
	glUniform1i(progLightingUniforms.Base, 0);
	glUniform1i(progLightingUniforms.Bump, 1);
	glUniform1i(progLightingUniforms.LightMap, 2);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, rModel.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)32);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)44);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)56);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rModel.ib);

	for (int l = 0; l < LIGHT_COUNT; ++l) {
		glUniform3fv(progLightingUniforms.lightPos, 1, &lights[l].x);
		glUniform4fv(progLightingUniforms.lightColor, 1, &lightsColor[l].x);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rTextures[6 + l]);

		for (int i = 0; i < rModel.nBatchs; ++i) {
			RBatch * rBatch = rModel.batchs + i;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, rTextures[i]);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, rTextures[i + 3]);

			glDrawElements(GL_TRIANGLES, rBatch->nIndices, GL_UNSIGNED_SHORT, (const GLvoid*)(rBatch->firstIndex * sizeof(uint16_t)));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);

	glUseProgram(0);
}

void DrawLightModel(const glm::mat4 &mvpMatrix) {
	//glm::vec3 forward = rView.target - rView.pos;
	glm::vec3 forward = rView.forward;
	forward = glm::normalize(forward);
	glm::vec3 right = glm::cross(forward, rView.up);
	right = glm::normalize(right);
	glm::vec3 up = glm::cross(right, forward);
	up = glm::normalize(up);

	glUseProgram(rProgram[R_PROGRAM_AMBIENT].program);

	glUniformMatrix4fv(progAmbientUniforms.mvp, 1, GL_FALSE, &mvpMatrix[0][0]); //set uniform value

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rTextures[8]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	for (int l = 0; l < LIGHT_COUNT; ++l) {
		glUniform4fv(progAmbientUniforms.color, 1, &lightsColor[l].x);

		glBindBuffer(GL_ARRAY_BUFFER, rLightModel[l].vb);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LightModelVertex), (const GLvoid*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(LightModelVertex), (const GLvoid*)12);

		LightModelVertex * v = (LightModelVertex*)GL_MapVertexBuffer();
		{
			v[0].pos = lights[l] + 60.0f * (-right - up);
			v[0].texCoord = glm::vec2(0.0f, 0.0f);
			v[1].pos = lights[l] + 60.0f * (right - up);
			v[1].texCoord = glm::vec2(1.0f, 0.0f);
			v[2].pos = lights[l] + 60.0f * (right + up);
			v[2].texCoord = glm::vec2(1.0f, 1.0f);
			v[3].pos = lights[l] + 60.0f * (-right + up);
			v[3].texCoord = glm::vec2(0.0f, 1.0f);
		}
		GL_UnmapVertexBuffer();
		
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
}
