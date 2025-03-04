//2016-10-25 Tue.

class VolumeBuffer;

class VolumeRender {
public:
	VolumeRender(VolumeBuffer *volume);
	~VolumeRender();

	void			Render(const glm::mat4 &proj, const glm::mat4 &modelview);
	void			DrawSimple(const glm::mat4 &proj, const glm::mat4 &modelview);
	void			DrawWireframe(const glm::mat4 &proj, const glm::mat4 &modelview);

	void			SetVolume(VolumeBuffer *volume);
	void			SetDensity(float x);
	void			SetBrightness(float x);

private:

	void			LoadPrograms();
	void			DrawCube(bool hasTex);


	VolumeBuffer *	m_volume;
	float			m_density;
	float			m_brightness;
	GLuint			m_vbo;
	GLuint			m_ibo;

	GLProgram		m_base_prog; //for debug
	GLProgram		m_simple_prog;//for debug
	GLProgram		m_raymarch_prog;
};
