/*
Camera

		   The Camera class provides a simple implementation of an interactive
		OpenGL camera.  It accepts a variety of standard camera parameters,
		and is passed mouse input to manipulate the camera.  The "mode" variable
		stores which mouse button is being dragged, and the camera is translated
		and rotated accordingly.  The Draw command will load the projection/camera
		transformations into the OpenGL matrices.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once


#include "triple.h"

#define CAM_PERSPECTIVE 0
#define CAM_ORTHO 1
#define CAM_ORTHO2 2
#define CAM_LEFTEYE 3
#define CAM_RIGHTEYE 4

#ifndef X
#define X 0
#define Y 1
#define Z 2
#endif

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

#define SPINNER 0.0025

//struct triple;


class Camera
{
public:
	Camera(void);
	Camera(double x, double y, double z, double _rotX, double _rotY, double _rho);
	~Camera(void);
	bool Draw(int drawmode = 0);

	void reversal() { reverse = !reverse; }
	void toggleProjection();

	void setX(int _X);	// Input mouse X coordinates for transformation of camera
	void setY(int _Y);	// Input mouse Y coordinates for transformation of camera
	void setMode(int _mode, int x, int y) { mode = _mode; lastX = x; lastY = y; } // Record mouse clicks

	void setRadius(double r);		// Modify radius of rotation
	void setAspect(double a) { aspect = a; }	// Set aspect ratio
	void setScale(double s) { scale = s; }		// Set scale of translation movement (mode 1/2)
	void setAngle(double a) { angle = a; }		// Set viewing angle of camera
	void setUpVector(double x, double y, double z) { upvector[0] = x; upvector[1] = y; upvector[2] = z; }	// Set camera up vector
	void setZBuffer(double zn, double zf) { znear = zn; zfar = zf; }	// Set z-buffer limits
	void setProjection(int proj) { projo = proj; }	// Set projection mode (ortho, perspective, etc)
	void setSeparation(double s) { eyeSeparation = s; }
	void setFocalRatio(double fr) { focalRatio = fr; }

	void setViewDirection(int axis, double FPS = 30.0);

	void drawCenter();
	void getCamera(double * temp);			// Returns current camera coordinates
	void getQuadrant(double * temp);		// Returns "direction" camera is looking
	triple getUpVector() { return triple(upvector[0], upvector[1], upvector[2]); }
	int getProjection() const { return projo; }	// Accessor for projection mode
	double getSeparation() const { return eyeSeparation; } // Accessor for eye separation (stereo)
	double getFocalRatio() const { return focalRatio; }
	double getAngle() const { return angle; }	// Accessor for view angle
	double getRotX() const { return rotX; }
	double getRotY() const { return rotY; }
	double getRadius() const { return rho; }	// Accessor for radius
	double getZNear() const { return znear; }
	double getZFar() const { return zfar; }
	void spinCamera(int s) { rotX+=(SPINNER*s); }		// Uses "spin" value to rotate camera by fixed amount
	void setInput(bool i) { input = i; } // toggles mouse input
	bool getInput() { return input; }
	bool getShifting() { return (shifting > 0); }

	void reset() { Xoffset = 50; Yoffset = 50; Zoffset = 50; rotX = 1.0; rotY = 1.0; }
	void setOffset(double x, double y, double z) { Xoffset = x; Yoffset = y; Zoffset = z; }
	void setRotation(double x, double y) { rotX = x; rotY = y; }
	void setManualPosition(double x, double y, double z, double tx, double ty, double tz); // specifies manual camera position and lookAt point
	void StartPan(double value); // pan by constant value (theta)
	void StartTilt(double value); // tilt by constant value (phi)
	void StartZoom(double value); // zoom by constant factor (rho)
	void StopMotion(); // stop all camera motion/shifting

	void setAxisFlip(bool flip) { axisflip = flip; }
	bool getAxisFlip() { return axisflip; }
	bool getFrustumPlanes(triple points[4], triple normals[4]);

private:
	int lastX, lastY;
	int mode; // 0 = left button, 1 = middle button, 2 = right button
	int projo; // projection option.  0 = none, 1 = XY, 2 = XZ
	bool reverse; // if reverse = true, eye and target switched. 
	bool axisflip; // invert Y axis
	bool input; // if input = false mouse information is ignored
	double Xoffset, Yoffset, Zoffset;
	double rotX, rotY;
	double znear, zfar;
	double theta, phi, rho;
	double scale, aspect, spin, angle, eyeSeparation, focalRatio;
	double lastCamera[3], upvector[3];

	const double EPS;

	double theta_shift, phi_shift, rho_shift;
	int shifting;
};
