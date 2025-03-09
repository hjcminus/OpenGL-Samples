/******************************************************************************
 * @file	simple_shadow.cpp
 * @brief
 *****************************************************************************/

#include "../common/common.h"

struct RView {
	glm::vec3			mViewPos;
	glm::vec3			mViewTarget;
	glm::vec3			mViewUp;
};

#define FRUSTUM_NEAR_LF_BT	0
#define FRUSTUM_NEAR_RT_BT	1
#define FRUSTUM_NEAR_RT_TP	2
#define FRUSTUM_NEAR_LF_TP	3
#define FRUSTUM_FAR_LF_BT	4
#define FRUSTUM_FAR_RT_BT	5
#define FRUSTUM_FAR_RT_TP	6
#define FRUSTUM_FAR_LF_TP	7

struct FrustumVertex {
	glm::vec3		mVertices[8];
};

#if 0
void	CalcFrustumVertices(const RView &view, float fovy, float zNear, float zFar, float w_over_h, FrustumVertex &fv) {
	float tan_fovy_over_2 = tanf((fovy * 0.5f) * MATH_PI / 180.0f);

	float near_tp = zNear * tan_fovy_over_2;
	float near_bt = -near_tp;
	float near_rt = near_tp * w_over_h;
	float near_lf = -near_rt;

	float far_tp = zFar * tan_fovy_over_2;
	float far_bt = -far_tp;
	float far_rt = far_tp * w_over_h;
	float far_lf = -far_rt;

	fv.mVertices[FRUSTUM_NEAR_LF_BT] = glm::vec3(near_lf, zNear, near_bt);
	fv.mVertices[FRUSTUM_NEAR_RT_BT] = glm::vec3(near_rt, zNear, near_bt);
	fv.mVertices[FRUSTUM_NEAR_RT_TP] = glm::vec3(near_rt, zNear, near_tp);
	fv.mVertices[FRUSTUM_NEAR_LF_TP] = glm::vec3(near_lf, zNear, near_tp);

	fv.mVertices[FRUSTUM_FAR_LF_BT] = glm::vec3(far_lf, zFar, far_bt);
	fv.mVertices[FRUSTUM_FAR_RT_BT] = glm::vec3(far_rt, zFar, far_bt);
	fv.mVertices[FRUSTUM_FAR_RT_TP] = glm::vec3(far_rt, zFar, far_tp);
	fv.mVertices[FRUSTUM_FAR_LF_TP] = glm::vec3(far_lf, zFar, far_tp);

	glm::vec3 forward = normalize(view.mViewTarget - view.mViewPos);
	glm::vec3 up = normalize(view.mViewUp);
	glm::vec3 right = normalize(cross(forward, up));

	glm::mat4 rotate = glm::mat4(1.0f);

	//*
	rotate[0][0] = right[0];
	rotate[1][0] = right[1];
	rotate[2][0] = right[2];
	rotate[0][1] = forward[0];
	rotate[1][1] = forward[1];
	rotate[2][1] = forward[2];
	rotate[0][2] = up[0];
	rotate[1][2] = up[1];
	rotate[2][2] = up[2];
	//*/

	/*
	rotate[0][0] = right[0];
	rotate[0][1] = right[1];
	rotate[0][2] = right[2];
	rotate[1][0] = forward[0];
	rotate[1][1] = forward[1];
	rotate[1][2] = forward[2];
	rotate[2][0] = up[0];
	rotate[2][1] = up[1];
	rotate[2][2] = up[2];
	//*/

	glm::mat4 trans = translate(glm::mat4(1.0f), view.mViewPos);

	glm::mat4 transform = trans * rotate;

	for (int i = 0; i < 8; ++i) {
		glm::vec4 v = glm::vec4(fv.mVertices[i], 1.0f);
		v = transform * v;
		fv.mVertices[i] = glm::vec3(v);
	}
}

void CalcFrustumMinMax(const FrustumVertex &fv, glm::vec3 &min, glm::vec3 &max) {
	min = fv.mVertices[0];
	max = fv.mVertices[0];

	for (int i = 1; i < 8; ++i) {
		for (int j = 0; j < 3; ++j) {
			float value = fv.mVertices[i][j];

			if (value < min[j]) {
				min[j] = value;
			}

			if (value > max[j]) {
				max[j] = value;
			}
		}
	}
}

void ToDirectionalLightLocalCoordinate(FrustumVertex &fv, const glm::vec3 &lightdir) {
	//if (lightdir.z > 0.0f) {

	//}
	glm::vec3 forward = normalize(lightdir);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::cross(forward, up);
	up = cross(right, forward);

	right = normalize(right);
	up = normalize(up);

	glm::mat4 rotate = glm::mat4(1.0f);

	rotate[0][0] = right[0];
	rotate[1][0] = right[1];
	rotate[2][0] = right[2];
	rotate[0][1] = forward[0];
	rotate[1][1] = forward[1];
	rotate[2][1] = forward[2];
	rotate[0][2] = up[0];
	rotate[1][2] = up[1];
	rotate[2][2] = up[2];

	for (int i = 0; i < 8; ++i) {
		glm::vec4 v = glm::vec4(fv.mVertices[i], 1.0f);
		v = rotate * v;
		fv.mVertices[i] = glm::vec3(v);
	}
}

void DirectionalLightProjMatrix(const RView &view, float fovy, float zNear, float zFar, float w_over_h, const glm::vec3 &lightdir, glm::mat4 &proj) {
	FrustumVertex fv;
	CalcFrustumVertices(view, fovy, zNear, zFar, w_over_h, fv);

	glm::vec3 min, max;
	CalcFrustumMinMax(fv, min, max);

	glm::vec3 lightViewCtr = (min + max) * 0.5f;

	ToDirectionalLightLocalCoordinate(fv, lightdir);
	CalcFrustumMinMax(fv, min, max);

	glm::vec3 size = max - min;
	float half_hori = size.x * 0.5f;
	float half_vert = size.z * 0.5f;
	float half_depth = size.y * 0.5f;

	proj = glm::ortho(-half_hori, half_hori, -half_vert, half_vert, -half_depth, half_depth);
}
#endif

void CalcFrustumVertices(const RView &view, float fovy, float zNear, float zFar, float w_over_h, FrustumVertex &fv) {
	float tan_fovy_over_2 = tanf((fovy * 0.5f) * MATH_PI / 180.0f);

	float near_tp = zNear * tan_fovy_over_2;
	float near_bt = -near_tp;
	float near_rt = near_tp * w_over_h;
	float near_lf = -near_rt;

	float far_tp = zFar * tan_fovy_over_2;
	float far_bt = -far_tp;
	float far_rt = far_tp * w_over_h;
	float far_lf = -far_rt;

	fv.mVertices[FRUSTUM_NEAR_LF_BT] = glm::vec3(near_lf, near_bt, -zNear);
	fv.mVertices[FRUSTUM_NEAR_RT_BT] = glm::vec3(near_rt, near_bt, -zNear);
	fv.mVertices[FRUSTUM_NEAR_RT_TP] = glm::vec3(near_rt, near_tp, -zNear);
	fv.mVertices[FRUSTUM_NEAR_LF_TP] = glm::vec3(near_lf, near_tp, -zNear);

	fv.mVertices[FRUSTUM_FAR_LF_BT] = glm::vec3(far_lf, far_bt, -zFar);
	fv.mVertices[FRUSTUM_FAR_RT_BT] = glm::vec3(far_rt, far_bt, -zFar);
	fv.mVertices[FRUSTUM_FAR_RT_TP] = glm::vec3(far_rt, far_tp, -zFar);
	fv.mVertices[FRUSTUM_FAR_LF_TP] = glm::vec3(far_lf, far_tp, -zFar);

	glm::vec3 forward = normalize(view.mViewTarget - view.mViewPos);
	glm::vec3 up = normalize(view.mViewUp);
	glm::vec3 right = normalize(cross(forward, up));

	glm::mat4 rotate = glm::mat4(1.0f);

	//*
	rotate[0][0] = right[0];
	rotate[1][0] = right[1];
	rotate[2][0] = right[2];
	rotate[0][1] = up[0];
	rotate[1][1] = up[1];
	rotate[2][1] = up[2];
	rotate[0][2] = -forward[0];
	rotate[1][2] = -forward[1];
	rotate[2][2] = -forward[2];
	//*/

	/*
	rotate[0][0] = right[0];
	rotate[0][1] = right[1];
	rotate[0][2] = right[2];
	rotate[1][0] = forward[0];
	rotate[1][1] = forward[1];
	rotate[1][2] = forward[2];
	rotate[2][0] = up[0];
	rotate[2][1] = up[1];
	rotate[2][2] = up[2];
	//*/

	glm::mat4 trans = translate(glm::mat4(1.0f), view.mViewPos);

	glm::mat4 transform = trans * rotate;

	for (int i = 0; i < 8; ++i) {
		glm::vec4 v = glm::vec4(fv.mVertices[i], 1.0f);
		v = transform * v;
		fv.mVertices[i] = glm::vec3(v);
	}
}

void CalcFrustumMinMax(const FrustumVertex &fv, glm::vec3 &min, glm::vec3 &max) {
	min = fv.mVertices[0];
	max = fv.mVertices[0];

	for (int i = 1; i < 8; ++i) {
		for (int j = 0; j < 3; ++j) {
			float value = fv.mVertices[i][j];

			if (value < min[j]) {
				min[j] = value;
			}

			if (value > max[j]) {
				max[j] = value;
			}
		}
	}
}

void ToDirectionalLightLocalCoordinate(FrustumVertex &fv, const glm::vec3 &lightdir) {
	//if (lightdir.z > 0.0f) {

	//}
	glm::vec3 forward = normalize(lightdir);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::cross(forward, up);
	up = cross(right, forward);

	right = normalize(right);
	up = normalize(up);

	glm::mat4 rotate = glm::mat4(1.0f);

	rotate[0][0] = right[0];
	rotate[1][0] = right[1];
	rotate[2][0] = right[2];
	rotate[0][1] = up[0];
	rotate[1][1] = up[1];
	rotate[2][1] = up[2];
	rotate[0][2] = -forward[0];
	rotate[1][2] = -forward[1];
	rotate[2][2] = -forward[2];

	for (int i = 0; i < 8; ++i) {
		glm::vec4 v = glm::vec4(fv.mVertices[i], 1.0f);
		v = rotate * v;
		fv.mVertices[i] = glm::vec3(v);
	}
}

void CalcDirectionalLightMatrix(const RView &view, float fovy, float zNear, float zFar, float w_over_h, const glm::vec3 &lightdir, glm::mat4 &matProj, glm::mat4 &matView) {
	FrustumVertex fv;
	CalcFrustumVertices(view, fovy, zNear, zFar, w_over_h, fv);

	glm::vec3 min, max;
	CalcFrustumMinMax(fv, min, max);

	glm::vec3 lightViewCtr = (min + max) * 0.5f;

	ToDirectionalLightLocalCoordinate(fv, lightdir);
	CalcFrustumMinMax(fv, min, max);

	glm::vec3 size = max - min;
	float half_hori = size.x * 0.5f;
	float half_vert = size.y * 0.5f;
	float half_depth = size.z * 0.5f;

	matProj = glm::ortho(-half_hori, half_hori, -half_vert, half_vert, -half_depth, half_depth);

	glm::vec3 lightTarget = lightViewCtr + lightdir;
	matView = glm::lookAt(lightViewCtr, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

#define WINDOW_CX 800
#define WINDOW_CY 600

#define SHADOW_MAP_DIM 2048

#define Z_NEAR 1.0f
#define Z_FAR  128.0

#define OVERLAY_DIM 256.0f

struct RModel {
	GLuint		vb;
	GLuint		ib;
	uint32_t	indexSize;
	uint32_t	indexCount;
};

struct RVertex {
	glm::vec3 pos;
	glm::vec3 normal;
};

static_assert(sizeof(RVertex) == 24, "bad size of RVertex");

struct RVertexOverlay {
	glm::vec3 pos;
	glm::vec2 texCoord;
};

struct Prog_Scene_Uniforms {
	GLint    WorldTransformMatrix;
	GLint    WorldMatrix;
	GLint    ShadowProjectionTransformMatrix;
	GLint    DiffuseColor;
	GLint    LightDirectionVector;
	GLint    ShadowMapSampler;
};

struct Prog_ShadowMap_Uniforms {
	GLint    WorldTransformMatrix;
};

struct Prog_OverLay_Uniforms {
	GLint    mvp;
	GLint    Sampler0;
};

#define PROGRAM_SCENE     0
#define PROGRAM_SHADOWMAP 1
#define PROGRAM_OVERLAY   2

static Viewport  rViewport;
static View      rView;
static GLProgram rProgram[3];
static RModel    rModel;
static RModel    rModel_Overlay;
static RModel    rModel_Floor;
static Prog_Scene_Uniforms     rSceneUniforms;
static Prog_ShadowMap_Uniforms rShadowMapUniforms;
static Prog_OverLay_Uniforms   rOverlayUniforms;

/*
================================================================================
frame buffer
================================================================================
*/
struct FrameBuffer {
	GLuint fbo;
	GLuint depthTexture;
	// GLuint depthRenderBuffer;
};

static FrameBuffer rFrameBuffer;

static bool CreateFrameBuffer(int width, int height, FrameBuffer &frameBuffer) {
	memset(&frameBuffer, 0, sizeof(frameBuffer));

	GLenum status = GL_FRAMEBUFFER_COMPLETE;

	glGenTextures(1, &frameBuffer.depthTexture);
	glBindTexture(GL_TEXTURE_2D, frameBuffer.depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glGenRenderbuffers(1, &frameBuffer.depthRenderBuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer.depthRenderBuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &frameBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);
	// Attach depth texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, frameBuffer.depthTexture, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer.depthRenderBuffer, 0);

	// There are no color buffers attached
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	// Restore to normal framebuffer operation
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	
	return GL_FRAMEBUFFER_COMPLETE == status;
}

static void DestroyFrameBuffer(FrameBuffer &frameBuffer) {
	if (frameBuffer.fbo) {
		//if (frameBuffer.depthRenderBuffer) {
		//	glDeleteRenderbuffers(1, &frameBuffer.depthRenderBuffer);
		//}

		if (frameBuffer.depthTexture) {
			glDeleteTextures(1, &frameBuffer.depthTexture);
		}

		glDeleteFramebuffers(1, &frameBuffer.fbo);

		frameBuffer.fbo = 0;
		frameBuffer.depthTexture = 0;
		//frameBuffer.depthRenderBuffer = 0;
	}
}

const float FOVY = 45.0f;

static bool Setup() {
	/* //some test code
	RView v;
	v.mViewPos = glm::vec3(0.0f);
	v.mViewTarget = glm::vec3(0.0f, 1.0f, 0.0f);
	v.mViewUp = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 lightDir = glm::vec3(-1.0f, -1.0f, -1.0f);

	glm::mat4 proj;
	DirectionalLightProjMatrix(v, FOVY, 1.0f, 4096.0f, 800.0f / 600.0f, lightDir, proj);
	*/

	rViewport.x = 0;
	rViewport.y = 0;
	rViewport.width = WINDOW_CX;
	rViewport.height = WINDOW_CY;
	rViewport.zNear = Z_NEAR;
	rViewport.zFar = Z_FAR;;

	// scene program
	if (!GL_CreateProgram(L"simple_shadow/scene_vs.txt", L"simple_shadow/scene_fs.txt", rProgram[PROGRAM_SCENE])) {
		printf("load scene shaders error\n");
		return false;
	}

	glBindAttribLocation(rProgram[PROGRAM_SCENE].program, 0, "a_pos");
	glBindAttribLocation(rProgram[PROGRAM_SCENE].program, 1, "a_normal");

	if (!GL_LinkProgram(rProgram[PROGRAM_SCENE])) {
		GL_DestroyProgram(rProgram[PROGRAM_SCENE]);
		printf("link scene program error\n");
		return false;
	}

	rSceneUniforms.WorldTransformMatrix = glGetUniformLocation(rProgram[PROGRAM_SCENE].program, "WorldTransformMatrix");
	rSceneUniforms.WorldMatrix = glGetUniformLocation(rProgram[PROGRAM_SCENE].program, "WorldMatrix");
	rSceneUniforms.ShadowProjectionTransformMatrix = glGetUniformLocation(rProgram[PROGRAM_SCENE].program, "ShadowProjectionTransformMatrix");
	rSceneUniforms.DiffuseColor = glGetUniformLocation(rProgram[PROGRAM_SCENE].program, "DiffuseColor");
	rSceneUniforms.LightDirectionVector = glGetUniformLocation(rProgram[PROGRAM_SCENE].program, "LightDirectionVector");
	rSceneUniforms.ShadowMapSampler = glGetUniformLocation(rProgram[PROGRAM_SCENE].program, "ShadowMapSampler");

	// shadow program
	if (!GL_CreateProgram(L"simple_shadow/shadow_map_vs.txt", L"simple_shadow/shadow_map_fs.txt", rProgram[PROGRAM_SHADOWMAP])) {
		printf("load shadow_map shaders error\n");
		return false;
	}

	glBindAttribLocation(rProgram[PROGRAM_SHADOWMAP].program, 0, "a_pos");

	if (!GL_LinkProgram(rProgram[PROGRAM_SHADOWMAP])) {
		GL_DestroyProgram(rProgram[PROGRAM_SHADOWMAP]);
		printf("link shadow_map program error\n");
		return false;
	}

	rShadowMapUniforms.WorldTransformMatrix = glGetUniformLocation(rProgram[PROGRAM_SHADOWMAP].program, "WorldTransformMatrix");

	// overlay program
	if (!GL_CreateProgram(L"simple_shadow/overlay_vs.txt", L"simple_shadow/overlay_fs.txt", rProgram[PROGRAM_OVERLAY])) {
		printf("load overlay shaders error\n");
		return false;
	}

	glBindAttribLocation(rProgram[PROGRAM_OVERLAY].program, 0, "a_pos");
	glBindAttribLocation(rProgram[PROGRAM_OVERLAY].program, 1, "a_texCoord");

	if (!GL_LinkProgram(rProgram[PROGRAM_OVERLAY])) {
		GL_DestroyProgram(rProgram[PROGRAM_OVERLAY]);
		printf("link overlay program error\n");
		return false;
	}

	rOverlayUniforms.mvp = glGetUniformLocation(rProgram[PROGRAM_OVERLAY].program, "mvp");
	rOverlayUniforms.Sampler0 = glGetUniformLocation(rProgram[PROGRAM_OVERLAY].program, "Sampler0");

	// load model
	Model model;
	if (!Model_Load(L"star.model", &model)) { //star.model teapot.model dragon.model
	    printf("could load model\n");
		return false;
	}

	rModel.vb = GL_CreateVertexBuffer(model.vertexSize, model.nVertices, model.vertices);
	rModel.ib = GL_CreateIndexBuffer(model.indexSize, model.nIndices, model.indices);
	rModel.indexSize = model.indexSize;
	rModel.indexCount = model.nIndices;

	Model_Free(&model);

	// overlay model
	
	rModel_Overlay.vb = GL_CreateVertexBuffer(sizeof(RVertexOverlay), 4, nullptr);
	rModel_Overlay.ib = 0;
	rModel_Overlay.indexCount = 0;
	rModel_Overlay.indexSize = 0;

	glBindBuffer(GL_ARRAY_BUFFER, rModel_Overlay.vb);
	RVertexOverlay * pv = (RVertexOverlay*)GL_MapVertexBuffer();

	pv[0].pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pv[0].texCoord = glm::vec2(0.0f, 0.0f);
	pv[1].pos = glm::vec3(OVERLAY_DIM, 0.0f, 0.0f);
	pv[1].texCoord = glm::vec2(1.0f, 0.0f);
	pv[2].pos = glm::vec3(OVERLAY_DIM, OVERLAY_DIM, 0.0f);
	pv[2].texCoord = glm::vec2(1.0f, 1.0f);
	pv[3].pos = glm::vec3(0.0f, OVERLAY_DIM, 0.0f);
	pv[3].texCoord = glm::vec2(0.0f, 1.0f);

	GL_UnmapVertexBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// floor model
	rModel_Floor.vb = GL_CreateVertexBuffer(sizeof(RVertex), 4, nullptr);
	rModel_Floor.ib = 0;
	rModel_Floor.indexCount = 0;
	rModel_Floor.indexSize = 0;

	glBindBuffer(GL_ARRAY_BUFFER, rModel_Floor.vb);
	RVertex * floor_v = (RVertex*)GL_MapVertexBuffer();

	floor_v[0].pos = glm::vec3(-32.0f, -12.0f, 32.0f);
	floor_v[0].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	floor_v[1].pos = glm::vec3(32.0f, -12.0f, 32.0f);
	floor_v[1].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	floor_v[2].pos = glm::vec3(32.0f, -12.0f, -32.0f);
	floor_v[2].normal = glm::vec3(0.0f, 1.0f, 0.0f);
	floor_v[3].pos = glm::vec3(-32.0f, -12.0f, -32.0f);
	floor_v[3].normal = glm::vec3(0.0f, 1.0f, 0.0f);

	GL_UnmapVertexBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (!CreateFrameBuffer(SHADOW_MAP_DIM, SHADOW_MAP_DIM, rFrameBuffer)) {
		printf("Could not create framebuffer\n");
		return false;
	}

	V_InitView(rView, V_VIEW_MOVING_OBJECT, glm::vec3(0.0f, 0.0f, 32.0f), 0.0f, 0.0f);
	glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	return true;
}

static void Shutdown() {
	glDeleteBuffers(1, &rModel_Floor.vb);
	glDeleteBuffers(1, &rModel_Overlay.vb);
	glDeleteBuffers(1, &rModel.ib);
	glDeleteBuffers(1, &rModel.vb);

	DestroyFrameBuffer(rFrameBuffer);
	GL_DestroyProgram(rProgram[PROGRAM_OVERLAY]);
	GL_DestroyProgram(rProgram[PROGRAM_SHADOWMAP]);
	GL_DestroyProgram(rProgram[PROGRAM_SCENE]);
}

static void Reshape(int width, int height) {
	rViewport.width = width;
	rViewport.height = height;
	glutPostRedisplay();
}

/*

     Y
     |
	 |
	 |______X
    /
   /
  Z

  view pos: 0, 0, 32
  light dir, toward object: -1, -1, -1

*/

static bool gDirectionalLight = false;

static void Display() {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 scaleMatrix;

    // draw shadow map to framebuffer

	glBindFramebuffer(GL_FRAMEBUFFER, rFrameBuffer.fbo);

	glDrawBuffer(GL_NONE);

	Viewport vp;
	vp.x = 0;
	vp.y = 0;
	vp.width = SHADOW_MAP_DIM;
	vp.height = SHADOW_MAP_DIM;
	vp.zNear = Z_NEAR;
	vp.zFar = Z_FAR;

	glViewport(0, 0, SHADOW_MAP_DIM, SHADOW_MAP_DIM);

	float clearDepth = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &clearDepth);


	//float clearColorZero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//glClearBufferfv(GL_COLOR, 0, clearColorZero);

	glEnable(GL_DEPTH_TEST);

	//GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, buffers);

	View lightView;

	lightView.type = V_VIEW_MOVING_OBJECT;
	lightView.pos = glm::vec3(32.0f, 32.0f, 0.0f);
	glm::vec3 f = glm::vec3(-1.0f, -1.0f, 0.0f);
	lightView.forward = glm::normalize(f);
	lightView.up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 r = glm::cross(lightView.forward, lightView.up);
	lightView.right = glm::normalize(r);
	glm::vec3 u = glm::cross(r, lightView.forward);
	lightView.up = glm::normalize(u);
	lightView.yaw = 0.0f;
	lightView.pitch = 0.0f;

	glm::mat4 ShadowProjectionTransformMatrix;
	glm::mat4 WorldToLightTransformMatrix;

	if (gDirectionalLight) {

		V_PerspectiveMatrix(vp, FOVY, projectionMatrix);
		V_ViewMatrix(lightView, WorldToLightTransformMatrix);

	}
	else {

		RView v;
		v.mViewPos = rView.pos;
		v.mViewTarget = rView.pos + rView.forward;
		v.mViewUp = rView.up;

		CalcDirectionalLightMatrix(v, FOVY, vp.zNear, vp.zFar, 1.0f  /*800.0f / 600.0f*/, lightView.forward, projectionMatrix, WorldToLightTransformMatrix);

	}

	scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	ShadowProjectionTransformMatrix = projectionMatrix * WorldToLightTransformMatrix * rView.modelMatrix * scaleMatrix;

	glUseProgram(rProgram[PROGRAM_SHADOWMAP].program);
	glUniformMatrix4fv(rShadowMapUniforms.WorldTransformMatrix, 1, GL_FALSE, &ShadowProjectionTransformMatrix[0][0]); // set uniform value

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, rModel.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rModel.ib);

	glDrawElements(GL_TRIANGLES, rModel.indexCount, rModel.indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// draw floor
	glm::mat4 ShadowProjectionTransformMatrix2;
	ShadowProjectionTransformMatrix2 = projectionMatrix * WorldToLightTransformMatrix;
	glUniformMatrix4fv(rShadowMapUniforms.WorldTransformMatrix, 1, GL_FALSE, &ShadowProjectionTransformMatrix2[0][0]); //set uniform value

	glBindBuffer(GL_ARRAY_BUFFER, rModel_Floor.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDrawBuffer(GL_BACK);

	glViewport(rViewport.x, rViewport.y, rViewport.width, rViewport.height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw scene
#if 1

	V_PerspectiveMatrix(rViewport, FOVY, projectionMatrix);
	V_ViewMatrix(rView, viewMatrix);

	scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	glm::mat4 WorldMatrix = rView.modelMatrix;
	glm::mat4 WorldTransformMatrix = projectionMatrix * viewMatrix * rView.modelMatrix * scaleMatrix;

	glUseProgram(rProgram[PROGRAM_SCENE].program);

	glUniformMatrix4fv(rSceneUniforms.WorldTransformMatrix, 1, GL_FALSE, &WorldTransformMatrix[0][0]); // set uniform value
	glUniformMatrix4fv(rSceneUniforms.WorldMatrix, 1, GL_FALSE, &WorldMatrix[0][0]);
	glUniformMatrix4fv(rSceneUniforms.ShadowProjectionTransformMatrix, 1, GL_FALSE, &ShadowProjectionTransformMatrix[0][0]);

	float diffuseModel[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 lightDir = glm::vec3(1.0f, 1.0f, 0.0f); // toward light position
	lightDir = glm::normalize(lightDir);

	glUniform4fv(rSceneUniforms.DiffuseColor, 1, diffuseModel);
	glUniform3fv(rSceneUniforms.LightDirectionVector, 1, &lightDir.x);
	glUniform1i(rSceneUniforms.ShadowMapSampler, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rFrameBuffer.depthTexture);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, rModel.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)12);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rModel.ib);

	glDrawElements(GL_TRIANGLES, rModel.indexCount, rModel.indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// draw floor
	WorldTransformMatrix = projectionMatrix * viewMatrix;
	WorldMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(rSceneUniforms.WorldTransformMatrix, 1, GL_FALSE, &WorldTransformMatrix[0][0]); // set uniform value
	glUniformMatrix4fv(rSceneUniforms.WorldMatrix, 1, GL_FALSE, &WorldMatrix[0][0]);
	glUniformMatrix4fv(rSceneUniforms.ShadowProjectionTransformMatrix, 1, GL_FALSE, &ShadowProjectionTransformMatrix2[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, rModel_Floor.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (const GLvoid*)12);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
#endif

	//draw overlay
	//glViewport(rViewport.x, rViewport.y, rViewport.width * 0.5f, rViewport.height * 0.5f);
	//glViewport(0, 0, OVERLAY_DIM, OVERLAY_DIM);
	glViewport(rViewport.width - OVERLAY_DIM, rViewport.height - OVERLAY_DIM, OVERLAY_DIM, OVERLAY_DIM);
	/*
	Viewport  vp2;
	vp2.x = 0;
	vp2.y = OVERLAY_DIM * 400;
	vp2.width = OVERLAY_DIM * 0.5f;
	vp2.height = OVERLAY_DIM * 0.5f;
	vp2.zNear = rViewport.zNear;
	vp2.zFar = rViewport.zFar;
	V_OrthographicMatrix(vp2, projectionMatrix);
	*/
	V_OrthographicMatrix(OVERLAY_DIM * 0.15f, OVERLAY_DIM * 0.4f, OVERLAY_DIM * 0.25f, OVERLAY_DIM * 0.25f, -1.0f, 1.0f, projectionMatrix);
	V_ViewMatrix(rView, viewMatrix);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	float offset_x = rViewport.width - OVERLAY_DIM - 2.0f;
	float offset_y = rViewport.height - OVERLAY_DIM - 2.0f;

	//glm::mat4 OverlayTransMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(offset_x, offset_y, 0.0f));
	glm::mat4 OverlayTransMatrix = glm::mat4();
	glm::mat4 mvpOverlayMatrix = projectionMatrix * OverlayTransMatrix;

	glUseProgram(rProgram[PROGRAM_OVERLAY].program);
	glUniform1i(rOverlayUniforms.Sampler0, 0);

	glUniformMatrix4fv(rOverlayUniforms.mvp, 1, GL_FALSE, &mvpOverlayMatrix[0][0]); //set uniform value
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rFrameBuffer.depthTexture);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, rModel_Overlay.vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertexOverlay), (const GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RVertexOverlay), (const GLvoid*)12);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// swap chain
	glutSwapBuffers();
}

static void Special(int key, int x, int y) {
	if (GLUT_KEY_F2 == key) {
		gDirectionalLight = !gDirectionalLight;
		if (gDirectionalLight) {
			printf("directional light\n");
		}
		else {
			printf("spot light\n");
		}
		glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ComParam param;
	param.title = "Simple Shadow";
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
