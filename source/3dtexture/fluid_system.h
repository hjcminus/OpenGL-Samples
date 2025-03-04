//2016-10-25 Tue.

class VolumeBuffer;

class FluidSystem {
public:
	FluidSystem(int width, int height, int depth);
	~FluidSystem();

	void	Reset();
	void	Step(float timestep);
	void	Splat();

	VolumeBuffer * GetStateBuffer();

	

private:

	float	Frand();

	int		m_width;
	int		m_height;
	int		m_depth;

	VolumeBuffer *	m_state[2];
	VolumeBuffer *	m_prevState;
	int		m_current;

	GLProgram	m_clear_prog;
	GLProgram	m_copy_prog;
	GLProgram	m_splat_prog;
	GLProgram	m_step_prog;
};
