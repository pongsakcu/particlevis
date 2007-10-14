/*

ParticleStore

		   The ParticleStore is a repository for frames of Particle state data.  Currently
		only the basic 4-term row of data is supported: position, orientation, and two
		velocity vectors.  The store is used with a singleton pattern of access: calling
		"GetNewStore" will create or resize the store to support the desired number of
		particles.
		  Adding frames of data can be accomplished by calling "AllocateFrame" and manually
		pulling the frames out, or using "SetParticleData."  Each Frame is a set of pointers
		to each type of state data.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "triple.h"
#include "quad.h"
#include <vector>
using std::vector;

class Frame
{
public:
	Frame() { pos = NULL; };
	~Frame() {};
	triple * pos;
	quad * axis;
	triple * d_pos;
	triple * d_angle;
};

class ParticleStore
{
public:
	ParticleStore(const int particlecount = 1);
	~ParticleStore(void);
	int GetFrameCount() const;
	int GetParticleCount() const;	
	bool SetParticleData(int frame, int particle,
		triple pos, quad quat, triple theta, triple rtheta);

	triple * GetFramePos(int frame) const;
	quad * GetFrameAxis(int frame) const;
	triple * GetFrameVel(int frame) const;
	triple * GetFrameRVel(int frame) const;
	Frame GetFrame(int frame) const;

	const int particle_count;

	static ParticleStore* GetNewStore(int count);
	bool AllocateFrame();
	static ParticleStore null_store;

	vector<Frame> data; // the state data

private:	
	Frame NewFrame();

	/*triple * current_pos;
	quad * current_axis;
	triple * current_dpos;
	triple * current_dtheta;*/

	unsigned int particles_left_in_frame;
	static ParticleStore* singleton;
};