#include "RGBGradient.h"
#include <sstream>

using std::stringstream;

/* Implementation of the RGBGradient class */

RGBGradient::RGBGradient(void)
{
	allocatedBufferSize = 0;
	currentID = 0;
	stepSize = 1.0;
}

RGBGradient::~RGBGradient(void)
{
}

// Create a control knot.  n = a value from [0..1] indicating position of the rgb knot.
int RGBGradient::AddControlPoint(double n, triple rgb)
{
	ControlPoint cp;
	cp.id = currentID++;
	cp.n = n;
	cp.r = rgb.x;
	cp.g = rgb.y;
	cp.b = rgb.z;

	controlList.push_back(cp);
	controlList.sort();
	return cp.id;
}
// alternate call
int RGBGradient::AddControlPoint(double n, double r, double g, double b)
{
	return AddControlPoint(n, triple(r, g, b));
}

// Calculate and store a gradient of specified resolution based on the current control knots.
void RGBGradient::CalculateGradient(int resolution)
{
	double currentN = 0, nDelta = 1.0 / (double)(resolution - 1);
	double currentR = 0, currentG = 0, currentB = 0;
	double dR = 0, dG = 0, dB = 0;

	// delete old array, prepare new
	if (allocatedBufferSize > 0)
	{
		delete [] R;
		delete [] G;
		delete [] B;
	}
	R = new double[resolution];
	G = new double[resolution];
	B = new double[resolution];

	allocatedBufferSize = resolution;
	stepSize = (1.0 / (double)(allocatedBufferSize - 1));

	list<ControlPoint>::iterator current, next, temp;
	current = controlList.begin();
	next = controlList.begin();

	currentR = (*current).r;
	currentG = (*current).g;
	currentB = (*current).b;

	for (int i = 0; i < resolution; i++) // for each index in the buffer
	{
		if (i==(resolution-1))
			currentN = 1.0;

        R[i] = max(0, min(1.0, currentR));
		G[i] = max(0, min(1.0, currentG));
		B[i] = max(0, min(1.0, currentB));

		currentN += nDelta;
		if (currentN >= (*next).n) // advance to the next knot if necessary
		{
			current = next;
			temp = next;
			if ((++temp) != controlList.end())
			{
				// calculate the appropriate "slope" of R/G/B
				next++;
				double stepno = (((*next).n - (*current).n) / nDelta);
				dR = ( (*next).r - (*current).r ) / stepno;
				dG = ( (*next).g - (*current).g ) / stepno;
				dB = ( (*next).b - (*current).b ) / stepno;
			}
			else
			{
				dR = 0;
				dG = 0;
				dB = 0;
			}
		}

		// increment the current RGB value linearly
		currentR+=dR;
		currentG+=dG;
		currentB+=dB;
	}
}

// Return rgb values corresponding to the input n=[0..1] in a triple
triple RGBGradient::ConvertToColor(double n)
{
	//if (allocatedBufferSize == 0)
	//	return triple(0, 0, 0);

	int ref = (int)(n / stepSize);
	if (ref >= allocatedBufferSize) ref = allocatedBufferSize - 1; 
	if (ref < 0) ref = 0;
	return triple(R[ref], G[ref], B[ref]);
}

// Write rgb values corresponding to the input n=[0..1] to three doubles
void RGBGradient::ConvertToColor(double n, double &r, double &g, double &b)
{
	int ref = (int)(n / stepSize);
	if (ref >= allocatedBufferSize) ref = allocatedBufferSize - 1; 
	if (ref < 0) ref = 0;
	r = R[ref];
	g = G[ref];
	b = B[ref];
}

ControlPoint RGBGradient::RemovePointByRef(int _id)
{
	list<ControlPoint>::iterator citer;
	for(citer = controlList.begin(); citer != controlList.end(); citer++)
	{
		if ((*citer).id == _id)
		{
			ControlPoint temp = *citer;
			controlList.erase(citer);
			return temp;
		}
	}
	return ControlPoint();
}

ControlPoint RGBGradient::RemovePointByN(double _n)
{
	list<ControlPoint>::iterator citer;
	for(citer = controlList.begin(); citer != controlList.end(); citer++)
	{
		if ((*citer).n == _n)
		{
			ControlPoint temp = *citer;
			controlList.erase(citer);
			return temp;
		}
	}
	return ControlPoint();
}

void RGBGradient::Clear()
{
	controlList.clear();
	if (allocatedBufferSize > 0)
	{
		delete [] R;
		delete [] G;
		delete [] B;
	}
	allocatedBufferSize = 0;
	currentID = 0;
}

bool RGBGradient::SerializeOut(string &output)
{
	stringstream outstream;
	int size = controlList.size();
	outstream << size << " ";
	list<ControlPoint>::iterator iter;
	for (iter = controlList.begin(); iter != controlList.end(); iter++)
	{
		ControlPoint p = *iter;
		outstream << p.n << " ";
		outstream << p.r << " ";
		outstream << p.g << " ";
		outstream << p.b << " ";
	}
	output = outstream.str();
	return true;
}

bool RGBGradient::SerializeIn(string input)
{
	Clear();
	stringstream instream;
	instream.str(input);
	int size;
	instream >> size;
	for (int i = 0; i < size && !instream.fail(); i++)
	{
		ControlPoint p;
		p.id = currentID++;
		instream >> p.n;
		instream >> p.r;
		instream >> p.g;
		instream >> p.b;
		controlList.push_back(p);
	}
	if (controlList.size() != currentID)
	{
		Clear();
		return false;
	}
	else
	{
		return true;
	}
}
