// ParticleSceneObj
//   This class is used for drawing scene objects to aid placement of lights and cameras.
// Vincent Hoon, 2006
#pragma once
#include "Particle.h"

class ParticleSceneObj :
	public Particle
{
public:
	ParticleSceneObj(int type = 0);
	~ParticleSceneObj(void);

	void DrawInstance(int frame);
	void DrawGeometry();
	void UpdatePosition(triple one, triple two = triple(0, 0, 0));
	triple GetPosition(int i) { if (i==1) return point1; else return point2; }
	void SetColor(triple rgb) { color = rgb; }	

private:	
	int objtype;
	triple point1, point2;
	triple color;
};
