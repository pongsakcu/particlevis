/*
	RGBGradient

	The RGBGradient class constructs and stores a gradient of linearly 
	interpolated RGB values and will convert a normalized range
	of input (0..1) to the stored color ramp.

	Each RGBGradient object represents a single color gradient.  To
	create and store the gradient, control "knots" are added by
	repeatedly calling the AddControlPoint function.  Before the
	object can be used, the CalculateGradient function must be
	called with a fixed resolution; the object will then render
	and store the color ramp internally.

	To use a calculated gradient, simply call the ConvertToColor
	function with an input n=[0..1], and an RGB value will be returned.

	Example: The standard four color "rainbow" gradient.

	RGBGradient grad;
	grad.AddControlPoint(0, triple(0, 0, 1));
	grad.AddControlPoint(0.33, triple(0, 1, 0));
	grad.AddControlPoint(0.66, triple(1, 1, 0));
	grad.AddControlPoint(1.0, triple(1, 0, 0));
	grad.CalculateGradient(2048);	
	triple rgb = grad.ConvertToColor(input);

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#ifndef RGBGRAD
#define RGBGRAD
#include "stdafx.h"
#include <list>
#include <string>
#include "triple.h"
using std::list;
using std::string;

struct ControlPoint
{
	int id;			// unique ID
	double n;		// from 0 to 1.0
	double r;		// the color itself
	double b;
	double g;
	bool operator<(ControlPoint &right) { return n < right.n; }
};

class RGBGradient
{
public:
	RGBGradient();
	~RGBGradient(void);
	int AddControlPoint(double n, triple rgb); // pass in triple
	int AddControlPoint(double n, double r, double g, double b); // pass in doubles
	ControlPoint RemovePointByRef(int id);
	ControlPoint RemovePointByN(double n);
	void Clear();

	int Allocated() { return allocatedBufferSize; }

	void CalculateGradient(int resolution);
	triple ConvertToColor(double n); // create and return triple
	//void ConvertToColor(double n, triple &rgb); // store in existing triple	
	void ConvertToColor(double n, triple& rgb) // Write rgb values corresponding to the input n=[0..1] to a triple
	{
		int ref = (int)(n / stepSize);
		if (ref >= allocatedBufferSize) ref = allocatedBufferSize - 1; 
		if (ref < 0) ref = 0;
		rgb.x = R[ref];
		rgb.y = G[ref];
		rgb.z = B[ref];
	}

	void ConvertToColor(double n, double &r, double &g, double &b); // store in existing doubles
	list<ControlPoint>::const_iterator PeekControlList() { return controlList.begin(); }
	int ControlSize() { return controlList.size(); }

	// I/O to strings
	bool SerializeIn(string input);
    bool SerializeOut(string &output); 
private:
	double *R, *G, *B;			// color buffers
	int allocatedBufferSize;	// size of current buffers
	double stepSize;			// 1.0 / (allocatedBufferSize - 1)
	int currentID;				// counter for cpoint ID
	list<ControlPoint> controlList;	// main list of color control points

};
#endif