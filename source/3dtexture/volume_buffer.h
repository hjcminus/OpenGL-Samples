//2016-10-21 Fri.

#include "../Common/Common.h"

class VolumeBuffer {
public:
	enum BlendMode { BLEND_NONE = 0, BLEND_ADDITIVE };

	VolumeBuffer(GLint format, int width, int height, int depth);
	~VolumeBuffer();

	void	SetFiltering(GLint mode);
	void	SetBlendMode(BlendMode mode);
	void	RunProgram(GLProgram &prog, bool hasTex);

	GLuint	GetTexture();

private:

	BlendMode m_blendMode;
	int		m_width;
	int		m_height;
	int		m_depth;
	GLuint	m_tex;
	GLuint	m_fbo;
	GLuint  m_vbo;

	void	DrawSlice(float z, bool hasTex);
};
