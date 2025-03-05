//2016-02-11 Thu.

#include "../common/common.h"

//view
Viewport rViewport;
View rView;

//fbo
GLuint fboAll;
GLuint fboFocusedObj;
GLuint fboFocusedObjBlur;

//texture
GLuint texAll;
GLuint texFocusedObj;
GLuint texFocusedObjBlur;

//render buffer
GLuint depthAll;
GLuint depthFocusedObj;

//shader
GLProgram lighting_prog;
GLProgram GaussianBlur_prog;
GLProgram combine_prog;

//vertex buffer
GLuint drawWindowVB;

//model
ModelBuffer rModel;

void destroyFBO() {
	

	if (texAll) {
		glDeleteTextures(1, &texAll);
		texAll = 0;
	}

	if (texFocusedObj) {
		glDeleteTextures(1, &texFocusedObj);
		texFocusedObj = 0;
	}

	if (texFocusedObjBlur) {
		glDeleteTextures(1, &texFocusedObjBlur);
		texFocusedObjBlur = 0;
	}

	if (depthAll) {
		glDeleteRenderbuffers(1, &depthAll);
		depthAll = 0;
	}

	if (depthFocusedObj) {
		glDeleteRenderbuffers(1, &depthFocusedObj);
		depthFocusedObj = 0;
	}

	if (fboAll) {
		glDeleteRenderbuffersEXT(1, &fboAll);
		fboAll = 0;
	}

	if (fboFocusedObj) {
		glDeleteRenderbuffersEXT(1, &fboFocusedObj);
		fboFocusedObj = 0;
	}

	if (fboFocusedObjBlur) {
		glDeleteRenderbuffersEXT(1, &fboFocusedObjBlur);
		fboFocusedObjBlur = 0;
	}
}

int blurCX = 1;
int blurCY = 1;
const int MAX_BLUR_DIM = 128;

void initFBO(int w, int h) {
	destroyFBO();

	//---------- all ----------

	//create render target
	glGenTextures(1, &texAll);
	glBindTexture(GL_TEXTURE_2D, texAll);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//create depth buffer
	glGenRenderbuffers(1, &depthAll);
	glBindRenderbuffer(GL_RENDERBUFFER, depthAll);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);

	glGenFramebuffers(1, &fboAll);
	glBindFramebuffer(GL_FRAMEBUFFER, fboAll);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texAll, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAll);

	GL_CheckFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//---------- focused ----------

	float ratio = (float)w / h;
	if (w > h) {
		blurCX = MAX_BLUR_DIM;
		blurCY = (int)(blurCX / ratio);
	}
	else {
		blurCY = MAX_BLUR_DIM;
		blurCX = (int)(blurCY * ratio);
	}

	if (blurCX < 1) {
		blurCX = 1;
	}

	if (blurCY < 1) {
		blurCY = 1;
	}

	//create render target
	glGenTextures(1, &texFocusedObj);
	glBindTexture(GL_TEXTURE_2D, texFocusedObj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, blurCX, blurCY, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//create depth buffer
	glGenRenderbuffers(1, &depthFocusedObj);
	glBindRenderbuffer(GL_RENDERBUFFER, depthFocusedObj);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, blurCX, blurCY);

	glGenFramebuffers(1, &fboFocusedObj);
	glBindFramebuffer(GL_FRAMEBUFFER, fboFocusedObj);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texFocusedObj, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthFocusedObj);

	GL_CheckFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//---------- blur ----------

	glGenTextures(1, &texFocusedObjBlur);
	glBindTexture(GL_TEXTURE_2D, texFocusedObjBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, blurCX, blurCY, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &fboFocusedObjBlur);
	glBindFramebuffer(GL_FRAMEBUFFER, fboFocusedObjBlur);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texFocusedObjBlur, 0);

	GL_CheckFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static bool Setup() {
	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = DEFAULT_WINDOW_CX;
	rViewport.height = DEFAULT_WINDOW_CY;
	rViewport.zNear = DEFAULT_Z_NEAR;
	rViewport.zFar = DEFAULT_Z_FAR;

	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 70.0f), 0.0f, 0.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	bool b1 = GL_CreateProgram(L"glow/lighting_vs.txt", L"glow/lighting_fs.txt", lighting_prog);
	bool b2 = GL_CreateProgram(L"glow/gaussian_blur_vs.txt", L"glow/gaussian_blur_fs.txt", GaussianBlur_prog);
	bool b3 = GL_CreateProgram(L"glow/combine_vs.txt", L"glow/combine_fs.txt", combine_prog);

	if (b1 && b2 && b3) {
		GL_LinkProgram(lighting_prog);
		GL_LinkProgram(GaussianBlur_prog);
		GL_LinkProgram(combine_prog);
	}

	initFBO(DEFAULT_WINDOW_CX, DEFAULT_WINDOW_CY);

	GL_CreateFullScreenVB(drawWindowVB, true);

	if (!CreatePosNormalColorModel(L"sphere.model", rModel)) {
		return false;
	}

	return true;
}

static void Shutdown() {
	GL_DestroyProgram(lighting_prog);
	GL_DestroyProgram(GaussianBlur_prog);
	GL_DestroyProgram(combine_prog);

	destroyFBO();

	GL_DestroyFullScreenVB(drawWindowVB);

	DestroyPosNormalColorModel(rModel);
}

static void Reshape(int width, int height) {
	if (rViewport.width == width && rViewport.height == height) {
		return;
	}

	//reset viewport
	rViewport.width = width;
	rViewport.height = height;

	initFBO(width, height);
}

int drawMode = 0;
int blurCoreSize = 5;

const int MAX_RANDOM_OBJ = 20;

glm::vec3 pos[MAX_RANDOM_OBJ];
glm::vec4 color[MAX_RANDOM_OBJ];

float Frand() {
	return rand() / (float)RAND_MAX;
}


void Draw_All() {
	if (0 != drawMode) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboAll);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
	}

	glEnable(GL_DEPTH_TEST);

	glm::mat4 matProj;
	glm::mat4 matView;
	V_PerspectiveMatrix(rViewport, 22.5f, matProj);
	V_ViewMatrix(rView, matView);

	glm::mat4 matModelView = matView * rView.modelMatrix;
	glm::mat4 matModelViewProj = matProj * matView * rView.modelMatrix;

	glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);
	//glClearColor(DEFAULT_BACKGROUND_COLOR[0], DEFAULT_BACKGROUND_COLOR[1], DEFAULT_BACKGROUND_COLOR[2], 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GL_UseProgram(lighting_prog);
	{
		GL_UniformMatrix4fv(lighting_prog, "modelViewProj", matModelViewProj);
		GL_UniformMatrix4fv(lighting_prog, "modelView", matModelView);

		GL_Uniform3f(lighting_prog, "viewPos", rView.pos[0], rView.pos[1], rView.pos[2]);

		GL_Uniform4f(lighting_prog, "materialAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
		GL_Uniform4f(lighting_prog, "materialDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
		GL_Uniform4f(lighting_prog, "materialSpecular", 1.0f, 1.0f, 1.0f, 1.0f);
		GL_Uniform1f(lighting_prog, "materialShiness", 16.0f);

		GL_Uniform3f(lighting_prog, "lightDirection", 0.0f, 0.0f, -1.0f);

		GL_Uniform4f(lighting_prog, "lightAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
		GL_Uniform4f(lighting_prog, "lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
		GL_Uniform4f(lighting_prog, "lightSpecular", 1.0f, 1.0f, 1.0f, 1.0f);

		DrawPosNormalColorModel(rModel, true, false);

		for (int i = 0; i < MAX_RANDOM_OBJ; ++i) {
			glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), pos[i]);
			
			glm::mat4 newModelView = matView * rView.modelMatrix * matTranslate;
			glm::mat4 newModelViewProj = matProj * newModelView;

			GL_UniformMatrix4fv(lighting_prog, "modelViewProj", newModelViewProj);
			GL_UniformMatrix4fv(lighting_prog, "modelView", newModelView);

			GL_Uniform4f(lighting_prog, "materialDiffuse", color[i].x, color[i].y, color[i].z, 1.0f);

			DrawPosNormalColorModel(rModel, true, false);
		}
	}
	GL_UnuseProgram();

	if (0 != drawMode) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);
	}
}

void Draw_Focused() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboFocusedObj);

	glEnable(GL_DEPTH_TEST);

	glDrawBuffer(GL_NONE);
	{
		Viewport vp = rViewport;
		vp.width = blurCX;
		vp.height = blurCY;

		glm::mat4 matProj;
		glm::mat4 matView;
		V_PerspectiveMatrix(vp, 22.5f, matProj);
		V_ViewMatrix(rView, matView);

		glm::mat4 matModelView = matView * rView.modelMatrix;
		glm::mat4 matModelViewProj = matProj * matView * rView.modelMatrix;

		glViewport(vp.x, vp.y, vp.width, vp.height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GL_UseProgram(lighting_prog);
		{
			GL_UniformMatrix4fv(lighting_prog, "modelViewProj", matModelViewProj);
			GL_UniformMatrix4fv(lighting_prog, "modelView", matModelView);

			GL_Uniform3f(lighting_prog, "viewPos", rView.pos[0], rView.pos[1], rView.pos[2]);

			GL_Uniform4f(lighting_prog, "materialAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "materialDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform4f(lighting_prog, "materialSpecular", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform1f(lighting_prog, "materialShiness", 16.0f);

			GL_Uniform3f(lighting_prog, "lightDirection", 0.0f, 0.0f, -1.0f);

			GL_Uniform4f(lighting_prog, "lightAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightSpecular", 1.0f, 1.0f, 1.0f, 1.0f);

			DrawPosNormalColorModel(rModel, true, false);

			for (int i = 0; i < MAX_RANDOM_OBJ; ++i) {
				glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), pos[i]);

				glm::mat4 newModelView = matView * rView.modelMatrix * matTranslate;
				glm::mat4 newModelViewProj = matProj * newModelView;

				GL_UniformMatrix4fv(lighting_prog, "modelViewProj", newModelViewProj);
				GL_UniformMatrix4fv(lighting_prog, "modelView", newModelView);

				GL_Uniform4f(lighting_prog, "materialDiffuse", color[i].x, color[i].y, color[i].z, 1.0f);

				DrawPosNormalColorModel(rModel, true, false);
			}
		}
		GL_UnuseProgram();
	}

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
	{
		glEnable(GL_DEPTH_TEST);

		Viewport vp = rViewport;
		vp.width = blurCX;
		vp.height = blurCY;

		glm::mat4 matProj;
		glm::mat4 matView;
		V_PerspectiveMatrix(vp, 22.5f, matProj);
		V_ViewMatrix(rView, matView);

		glm::mat4 matModelView = matView * rView.modelMatrix;
		glm::mat4 matModelViewProj = matProj * matView * rView.modelMatrix;

		glViewport(vp.x, vp.y, vp.width, vp.height);
		glClear(GL_COLOR_BUFFER_BIT);

		GL_UseProgram(lighting_prog);
		{
			GL_UniformMatrix4fv(lighting_prog, "modelViewProj", matModelViewProj);
			GL_UniformMatrix4fv(lighting_prog, "modelView", matModelView);

			GL_Uniform3f(lighting_prog, "viewPos", rView.pos[0], rView.pos[1], rView.pos[2]);

			GL_Uniform4f(lighting_prog, "materialAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "materialDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform4f(lighting_prog, "materialSpecular", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform1f(lighting_prog, "materialShiness", 16.0f);

			GL_Uniform3f(lighting_prog, "lightDirection", 0.0f, 0.0f, -1.0f);

			GL_Uniform4f(lighting_prog, "lightAmbient", 0.3f, 0.3f, 0.3f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);
			GL_Uniform4f(lighting_prog, "lightSpecular", 1.0f, 1.0f, 1.0f, 1.0f);

			DrawPosNormalColorModel(rModel, true, false);

			for (int i = 0; i < MAX_RANDOM_OBJ / 2; ++i) {
				glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), pos[i]);

				glm::mat4 newModelView = matView * rView.modelMatrix * matTranslate;
				glm::mat4 newModelViewProj = matProj * newModelView;

				GL_UniformMatrix4fv(lighting_prog, "modelViewProj", newModelViewProj);
				GL_UniformMatrix4fv(lighting_prog, "modelView", newModelView);

				GL_Uniform4f(lighting_prog, "materialDiffuse", color[i].x, color[i].y, color[i].z, 1.0f);

				DrawPosNormalColorModel(rModel, true, false);
			}
		}
		GL_UnuseProgram();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

void Draw_Blur() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboFocusedObjBlur);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
	{
		glDisable(GL_DEPTH_TEST);

		GL_UseProgram(GaussianBlur_prog);
		{
			glm::mat4 matProj;
			V_OrthoFullScreenMatrix(matProj);

			glm::mat4 matModelViewProj = matProj;

			Viewport vp = rViewport;
			vp.width = blurCX;
			vp.height = blurCY;

			glViewport(vp.x, vp.y, vp.width, vp.height);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			GL_UniformMatrix4fv(GaussianBlur_prog, "modelViewProj", matModelViewProj);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texFocusedObj);
			GL_Uniform1i(GaussianBlur_prog, "tex", 0);

			GL_Uniform2f(GaussianBlur_prog, "stTexelSize", 1.0f / blurCX, 1.0f / blurCY);
			GL_Uniform1i(GaussianBlur_prog, "blurCoreSize", blurCoreSize);

			GL_DrawFullScreenRect(drawWindowVB, true);
		}
		GL_UnuseProgram();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

void Draw_Combine() {
	GL_UseProgram(combine_prog);
	{
		glm::mat4 matProj;
		V_OrthoFullScreenMatrix(matProj);

		glm::mat4 matModelViewProj = matProj;

		glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GL_UniformMatrix4fv(combine_prog, "modelViewProj", matModelViewProj);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texAll);
		GL_Uniform1i(combine_prog, "tex1", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texFocusedObjBlur);
		GL_Uniform1i(combine_prog, "tex2", 1);

		GL_DrawFullScreenRect(drawWindowVB, true);
	}
	GL_UnuseProgram();
}

static void Display() {
	if (0 == drawMode) {
		Draw_All();
	}
	else {
		Draw_All();
		Draw_Focused();
		Draw_Blur();
		Draw_Combine();
	}
	glutSwapBuffers();
}

static void Special(int key, int x, int y) {	
	if (key == GLUT_KEY_F2) {
		drawMode++;
		drawMode %= 2;
		glutPostRedisplay();
	}
	else if (GLUT_KEY_PAGE_UP == key) {
		blurCoreSize += 2;
		if (blurCoreSize > 65) {
			blurCoreSize = 65;
		}
		printf("blur core size %d\n", blurCoreSize);
		glutPostRedisplay();
	}
	else if (GLUT_KEY_PAGE_DOWN == key) {
		blurCoreSize -= 2;
		if (blurCoreSize < 3) {
			blurCoreSize = 3;
		}
		printf("blur core size %d\n", blurCoreSize);
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	srand(1);

	for (int i = 0; i < MAX_RANDOM_OBJ; ++i) {
		pos[i].x = rand() % 20 - 10.0f;
		pos[i].y = rand() % 20 - 10.0f;
		pos[i].z = rand() % 20 - 10.0f;

		color[i].x = Frand();
		color[i].y = Frand();
		color[i].z = Frand();
		color[i].w = 1.0f;
	}

	ComParam param;
	param.title = "glow";
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
