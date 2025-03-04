//2016-10-25 Tue.

#include "vertex_format.h"
#include "volume_buffer.h"
#include "volume_render.h"

VolumeRender::VolumeRender(VolumeBuffer *volume) :m_volume(volume), m_vbo(0), m_ibo(0)
{
	LoadPrograms();

	PosTex vertex[8];

	vertex[0].mPos[0] = 1;
	vertex[0].mPos[1] = 1;
	vertex[0].mPos[2] = 1;
	vertex[0].mPos[3] = 1;

	vertex[1].mPos[0] = -1;
	vertex[1].mPos[1] = 1;
	vertex[1].mPos[2] = 1;
	vertex[1].mPos[3] = 1;

	vertex[2].mPos[0] = -1;
	vertex[2].mPos[1] = -1;
	vertex[2].mPos[2] = 1;
	vertex[2].mPos[3] = 1;

	vertex[3].mPos[0] = 1;
	vertex[3].mPos[1] = -1;
	vertex[3].mPos[2] = 1;
	vertex[3].mPos[3] = 1;

	vertex[4].mPos[0] = 1;
	vertex[4].mPos[1] = 1;
	vertex[4].mPos[2] = -1;
	vertex[4].mPos[3] = 1;

	vertex[5].mPos[0] = -1;
	vertex[5].mPos[1] = 1;
	vertex[5].mPos[2] = -1;
	vertex[5].mPos[3] = 1;

	vertex[6].mPos[0] = -1;
	vertex[6].mPos[1] = -1;
	vertex[6].mPos[2] = -1;
	vertex[6].mPos[3] = 1;

	vertex[7].mPos[0] = 1;
	vertex[7].mPos[1] = -1;
	vertex[7].mPos[2] = -1;
	vertex[7].mPos[3] = 1;

	vertex[0].mTex[0] = 1;
	vertex[0].mTex[1] = 1;
	vertex[0].mTex[2] = 1;

	vertex[1].mTex[0] = 0;
	vertex[1].mTex[1] = 1;
	vertex[1].mTex[2] = 1;

	vertex[2].mTex[0] = 0;
	vertex[2].mTex[1] = 0;
	vertex[2].mTex[2] = 1;

	vertex[3].mTex[0] = 1;
	vertex[3].mTex[1] = 0;
	vertex[3].mTex[2] = 1;

	vertex[4].mTex[0] = 1;
	vertex[4].mTex[1] = 1;
	vertex[4].mTex[2] = 0;

	vertex[5].mTex[0] = 0;
	vertex[5].mTex[1] = 1;
	vertex[5].mTex[2] = 0;

	vertex[6].mTex[0] = 0;
	vertex[6].mTex[1] = 0;
	vertex[6].mTex[2] = 0;

	vertex[7].mTex[0] = 1;
	vertex[7].mTex[1] = 0;
	vertex[7].mTex[2] = 0;

	//create vbo
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosTex) * 8, vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint indices[24];

	//front
	indices[0] = 0;
	indices[1] = 3;
	indices[2] = 7;
	indices[3] = 4;

	//back
	indices[4] = 1;
	indices[5] = 5;
	indices[6] = 6;
	indices[7] = 2;

	//top
	indices[8] = 2;
	indices[9] = 3;
	indices[10] = 0;
	indices[11] = 1;

	//bottom
	indices[12] = 7;
	indices[13] = 6;
	indices[14] = 7;
	indices[15] = 4;

	//left
	indices[16] = 2;
	indices[17] = 6;
	indices[18] = 7;
	indices[19] = 3;

	//right
	indices[20] = 0;
	indices[21] = 4;
	indices[22] = 5;
	indices[23] = 1;

	//create ibo
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 4 * 6, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbind
}

VolumeRender::~VolumeRender() {
	glDeleteBuffers(1, &m_ibo);
	glDeleteBuffers(1, &m_vbo);
	GL_DestroyProgram(m_raymarch_prog);
	GL_DestroyProgram(m_simple_prog);
	GL_DestroyProgram(m_base_prog);
}

void VolumeRender::Render(const glm::mat4 &proj, const glm::mat4 &modelview) {
	GL_UseProgram(m_raymarch_prog);

	glm::mat4 matModelViewProj = proj * modelview;

	glm::mat4 modelviewInv = glm::inverse(modelview);

	GL_UniformMatrix4fv(m_raymarch_prog, "modelViewProj", matModelViewProj);
	GL_UniformMatrix4fv(m_raymarch_prog, "modelViewInv", modelviewInv);

	GL_SetProgramTexture(m_raymarch_prog, "volumeTex", GL_TEXTURE_3D, m_volume->GetTexture(), 0);
	GL_Uniform1i(m_raymarch_prog, "steps", 128);
	GL_Uniform1f(m_raymarch_prog, "stepsize", 1.7f / 128.0f);
	GL_Uniform1f(m_raymarch_prog, "brightness", m_brightness);
	GL_Uniform1f(m_raymarch_prog, "density", m_density);
	//GL_Uniform1f(m_raymarch_prog, "threshold", 0.99f);
	GL_Uniform3f(m_raymarch_prog, "boxMin", -1.0f, -1.0f, -1.0f);
	GL_Uniform3f(m_raymarch_prog, "boxMax", 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawCube(false);

	GL_UnuseProgram();
}

void VolumeRender::DrawSimple(const glm::mat4 &proj, const glm::mat4 &modelview) {
	GL_UseProgram(m_simple_prog);

	glm::mat4 matModelViewProj = proj * modelview;

	GL_UniformMatrix4fv(m_simple_prog, "modelViewProj", matModelViewProj);
	GL_SetProgramTexture(m_simple_prog, "tex", GL_TEXTURE_3D, m_volume->GetTexture(), 0);

	DrawCube(true);

	GL_UnuseProgram();
}

void VolumeRender::DrawWireframe(const glm::mat4 &proj, const glm::mat4 &modelview) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_UseProgram(m_base_prog);

	glm::mat4 matModelViewProj = proj * modelview;

	GL_UniformMatrix4fv(m_base_prog, "modelViewProj", matModelViewProj);

	DrawCube(false);

	GL_UnuseProgram();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void VolumeRender::DrawCube(bool hasTex) {
	glEnableVertexAttribArray(0);
	if (hasTex) {
		glEnableVertexAttribArray(1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PosTex), (const GLvoid*)0);
	if (hasTex) {
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PosTex), (const GLvoid*)16);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glDrawElements(GL_QUADS, 4 * 6, GL_UNSIGNED_INT, (const GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	if (hasTex) {
		glDisableVertexAttribArray(1);
	}
}

void VolumeRender::SetVolume(VolumeBuffer *volume) {
	m_volume = volume;
}

void VolumeRender::SetDensity(float x) {
	m_density = x;
}

void VolumeRender::SetBrightness(float x) {
	m_brightness = x;
}

void VolumeRender::LoadPrograms() {
	bool b1 = GL_CreateProgram(L"texture_3d/base_vs.txt", L"texture_3d/base_fs.txt", m_base_prog);
	if (b1) {
		GL_LinkProgram(m_base_prog);
	}

	bool b2 = GL_CreateProgram(L"texture_3d/simple_vs.txt", L"texture_3d/simple_fs.txt", m_simple_prog);
	if (b2) {
		GL_LinkProgram(m_simple_prog);
	}

	bool b3 = GL_CreateProgram(L"texture_3d/raymarch_vs.txt", L"texture_3d/raymarch_fs.txt", m_raymarch_prog);
	if (b3) {
		GL_LinkProgram(m_raymarch_prog);
	}


}
