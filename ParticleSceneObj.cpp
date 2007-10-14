#include ".\particlesceneobj.h"
#include <math.h>

#define SQR(value) ((value)*(value))

ParticleSceneObj::ParticleSceneObj(int type) : Particle(ParticleStore::null_store, 0)
{
    maxsize = 0;
	frames = 1;
	radius = 1.0;
	objtype = type;
	displaylist = -1;
}

ParticleSceneObj::~ParticleSceneObj(void)
{
}

void ParticleSceneObj::DrawInstance(int frame)
{
	triple up(0, 0, 1);
	triple cross(0, 0, 0);
	triple vector(0, 0, 0);
	vector.x = point2.x - point1.x;
	vector.y = point2.y - point1.y;
	vector.z = point2.z - point1.z;
	if (vector.x == 0 && vector.y == 0)
	{
        cross.x = 0;
		cross.y = sqrt(SQR(vector.x) +
						SQR(vector.y) +
						SQR(vector.z));
		cross.z = 0;
	}
	else
	{
		cross = crossProduct(vector, up);
		double vecLength = sqrt(SQR(vector.x) +
						SQR(vector.y) +
						SQR(vector.z));
		double crossLength = sqrt(SQR(cross.x) +
						SQR(cross.y) +
						SQR(cross.z));
		cross.x *= (vecLength / crossLength);
		cross.y *= (vecLength / crossLength);
		cross.z *= (vecLength / crossLength);
	}

	//glLineSize(1.0);
	glColor4f(color.x / 3.0, color.y / 3.0, color.z / 3.0, 0.33333f);
	glBegin(GL_LINES);
		glVertex3f(point1.x, point1.y, point1.z);
		glVertex3f(point1.x, 0, point1.z);

		glVertex3f(point2.x, point2.y, point2.z);
		glVertex3f(point2.x, 0, point2.z);

		glColor3f(1, 1, 1);
		glVertex3f(point1.x, point1.y, point1.z);
		if (objtype == 0)
		{
			glVertex3f(point2.x, point2.y, point2.z);

			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(	(point1.x - (point1.x-point2.x)*0.9) + cross.x * 0.05,
						(point1.y - (point1.y-point2.y)*0.9) + cross.y * 0.05,
						(point1.z - (point1.z-point2.z)*0.9) + cross.z * 0.05);

			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f( (point1.x - (point1.x-point2.x)*0.9) - cross.x * 0.05,
						(point1.y - (point1.y-point2.y)*0.9) - cross.y * 0.05,
						(point1.z - (point1.z-point2.z)*0.9) - cross.z * 0.05);
		}
		else
		{
			glVertex3f(0, 0, 0);
		}
	glEnd();

	glColor3f(color.x, color.y, color.z);
	glBegin(GL_POINTS);
		glVertex3f(point1.x, point1.y, point1.z);
		if (objtype > 0)
			glVertex3f(point2.x, point2.y, point2.z);
	glEnd();
}

void ParticleSceneObj::DrawGeometry()
{
}

void ParticleSceneObj::UpdatePosition(triple one, triple two)
{
	point1 = one;
	point2 = two;
}