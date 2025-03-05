/******************************************************************************
 * @file	volume_buffer.cpp
 * @brief
 *****************************************************************************/

#include "vertex_format.h"
#include "volume_buffer.h"

VolumeBuffer::VolumeBuffer(GLint format, int width, int height, int depth) :
m_width(width),
m_height(height),
m_depth(depth),
m_tex(0), 
m_fbo(0),
m_vbo(0),
m_blendMode(BLEND_NONE)
{
	// create texture
	m_tex = GL_Create3DTexture(format, m_width, m_height, m_depth);

	// create fbo
	m_fbo = GL_GenFramebuffer();
	GL_BindFramebuffer(m_fbo);
	GL_FramebufferTexture(GL_TEXTURE_3D, m_tex, GL_COLOR_ATTACHMENT0, 0, 0);
	GL_UnbindFramebuffer();

	// create vbo
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PosTex) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VolumeBuffer::~VolumeBuffer() {
	glDeleteBuffers(1, &m_vbo);
	GL_DeleteFramebuffer(m_fbo);
	GL_DeleteTexture(m_tex);
}

void VolumeBuffer::SetFiltering(GLint mode) {
	glBindTexture(GL_TEXTURE_3D, m_tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, mode);
}

void VolumeBuffer::SetBlendMode(BlendMode mode) {
	m_blendMode = mode;
}

void VolumeBuffer::RunProgram(GLProgram &prog, bool hasTex) {
	GL_BindFramebuffer(m_fbo);

	glViewport(0, 0, m_width, m_height);
	glDisable(GL_DEPTH_TEST);

	switch (m_blendMode) {
	case BLEND_ADDITIVE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	}

	glm::mat4 mat = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	GL_UniformMatrix4fv(prog, "modelViewProj", mat);

	for (int z = 0; z < m_depth; ++z) {
		// attach texture slice to FBO
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_3D, m_tex);
		GL_FramebufferTexture(GL_TEXTURE_3D, m_tex, GL_COLOR_ATTACHMENT0, 0, z);
		// render
		DrawSlice((z + 0.5f) / (float)m_depth, hasTex);
	}

	glDisable(GL_BLEND);

	GL_UnbindFramebuffer();
}

GLuint VolumeBuffer::GetTexture() {
	return m_tex;
}

void VolumeBuffer::DrawSlice(float z, bool hasTex) {
	glEnableVertexAttribArray(0);
	if (hasTex) {
		glEnableVertexAttribArray(1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PosTex), (const GLvoid*)0);
	if (hasTex) {
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PosTex), (const GLvoid*)16);
	}

	PosTex * vert = (PosTex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	vert[0].mPos[0] = -1.0f;
	vert[0].mPos[1] = -1.0f;
	vert[0].mPos[2] = 0.0f;
	vert[0].mPos[3] = 1.0f;

	vert[1].mPos[0] = 1.0f;
	vert[1].mPos[1] = -1.0f;
	vert[1].mPos[2] = 0.0f;
	vert[1].mPos[3] = 1.0f;

	vert[2].mPos[0] = 1.0f;
	vert[2].mPos[1] = 1.0f;
	vert[2].mPos[2] = 0.0f;
	vert[2].mPos[3] = 1.0f;

	vert[3].mPos[0] = -1.0f;
	vert[3].mPos[1] = 1.0f;
	vert[3].mPos[2] = 0.0f;
	vert[3].mPos[3] = 1.0f;

	vert[0].mTex[0] = 0.0f;
	vert[0].mTex[1] = 0.0f;
	vert[0].mTex[2] = z;

	vert[1].mTex[0] = 1.0f;
	vert[1].mTex[1] = 0.0f;
	vert[1].mTex[2] = z;

	vert[2].mTex[0] = 1.0f;
	vert[2].mTex[1] = 1.0f;
	vert[2].mTex[2] = z;

	vert[3].mTex[0] = 0.0f;
	vert[3].mTex[1] = 1.0f;
	vert[3].mTex[2] = z;

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glDrawArrays(GL_QUADS, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	if (hasTex) {
		glDisableVertexAttribArray(1);
	}
}
