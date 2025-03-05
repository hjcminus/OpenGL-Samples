/******************************************************************************
 * @file	fluid_system.cpp
 * @brief
 *****************************************************************************/

#include "volume_buffer.h"
#include "fluid_system.h"

FluidSystem::FluidSystem(int width, int height, int depth) :
m_width(width),
m_height(height),
m_depth(depth),
m_current(0)
{
	m_state[0] = new VolumeBuffer(GL_RGBA16F, width, height, depth);
	m_state[1] = new VolumeBuffer(GL_RGBA16F, width, height, depth);
	m_prevState = new VolumeBuffer(GL_RGBA16F, width, height, depth);

	bool b1 = GL_CreateProgram(L"texture_3d/clear_vs.txt", L"texture_3d/clear_fs.txt", m_clear_prog);
	bool b2 = GL_CreateProgram(L"texture_3d/copy_vs.txt", L"texture_3d/copy_fs.txt", m_copy_prog);
	bool b3 = GL_CreateProgram(L"texture_3d/splat_vs.txt", L"texture_3d/splat_fs.txt", m_splat_prog);
	bool b4 = GL_CreateProgram(L"texture_3d/waves_vs.txt", L"texture_3d/waves_fs.txt", m_step_prog);

	if (!b1 || !b2 || !b3 || !b4) {
		printf("FluidSystem init error\n");
	}
	else {
		GL_LinkProgram(m_clear_prog);
		GL_LinkProgram(m_copy_prog);
		GL_LinkProgram(m_splat_prog);
		GL_LinkProgram(m_step_prog);
	}

	Reset();
}

FluidSystem::~FluidSystem() {
	GL_DestroyProgram(m_step_prog);
	GL_DestroyProgram(m_splat_prog);
	GL_DestroyProgram(m_copy_prog);
	GL_DestroyProgram(m_clear_prog);

	delete m_prevState;
	delete m_state[1];
	delete m_state[0];
}

// reset the simulation by clearing buffers
void FluidSystem::Reset() {
	GL_UseProgram(m_clear_prog);

	GL_Uniform4f(m_clear_prog, "color", 0.0f, 0.0f, 0.0f, 0.0f);

	m_state[m_current]->RunProgram(m_clear_prog, false);
	m_state[1 - m_current]->RunProgram(m_clear_prog, false);
	m_prevState->RunProgram(m_clear_prog, false);

	GL_UnuseProgram();
}

// step the simulation
void FluidSystem::Step(float timestep) {
	GL_UseProgram(m_copy_prog);

	// copy previous state
	GL_SetProgramTexture(m_copy_prog, "tex", GL_TEXTURE_3D, m_state[1 - m_current]->GetTexture(), 0);
	m_prevState->RunProgram(m_copy_prog, true);

	GL_UnuseProgram();

	GL_UseProgram(m_step_prog);

	// compute new state
	GL_SetProgramTexture(m_step_prog, "currentTex", GL_TEXTURE_3D, m_state[m_current]->GetTexture(), 0);
	GL_SetProgramTexture(m_step_prog, "prevTex", GL_TEXTURE_3D, m_prevState->GetTexture(), 1);
	GL_Uniform3f(m_step_prog, "voxelSize", 1.0f / m_width, 1.0f / m_height, 1.0f / m_depth);
	GL_Uniform1f(m_step_prog, "damping", 0.99f);
	m_state[1 - m_current]->RunProgram(m_step_prog, true);

	GL_UnuseProgram();

	m_current = 1 - m_current;
}

void FluidSystem::Splat() {
	GL_UseProgram(m_splat_prog);

	GL_Uniform3f(m_splat_prog, "center", Frand(), Frand(), Frand());
	GL_Uniform1f(m_splat_prog, "radius", 0.1f);
	GL_Uniform4f(m_splat_prog, "color", Frand(), Frand(), Frand(), 1.0f);

	m_state[m_current]->SetBlendMode(VolumeBuffer::BLEND_ADDITIVE);
	m_state[m_current]->RunProgram(m_splat_prog, true);
	m_state[m_current]->SetBlendMode(VolumeBuffer::BLEND_NONE);

	GL_UnuseProgram();
}

VolumeBuffer * FluidSystem::GetStateBuffer() {
	return m_state[m_current];
}

float FluidSystem::Frand() {
	return rand() / (float)RAND_MAX;
}
