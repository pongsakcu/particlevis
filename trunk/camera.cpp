#include <math.h>
#include "stdafx.h"
#include "Camera.h"
//#include "triple.h"

//using namespace std;

#ifndef clamp
#define clamp(x,a,b) ( max( (a), min((b), (x))) )
#endif

Camera::Camera(void) :
EPS(1e-12)
{
	lastX = lastY = 0;
	mode = 0;
	reverse = false;
	projo = CAM_PERSPECTIVE;
	Xoffset = Yoffset = Zoffset = 0;
	rho = 1.0;
	scale = 1.0;
	lastCamera[X] = lastCamera[Y] = lastCamera[Z] = 0;
	upvector[X] = upvector[Y] = upvector[Z] = 0;	
	upvector[Y] = 1.0;
	aspect = 1.0;
	spin = 0;
	angle = 65;
	znear = 0.1;
	zfar = 10.0;
	input = true;
	eyeSeparation = 0.01;
	focalRatio = 0.85;
	axisflip = true;

	theta_shift = phi_shift = 0;
	rho_shift = 1.0;
	shifting = 0;
}

// Secondary constructor: sets offset/rotation/radius of camera
Camera::Camera(double x, double y, double z, double _rotX, double _rotY, double _rho) :
EPS(1e-12)
{
	lastX = lastY = 0;
	mode = 0;
	reverse = false;
	projo = CAM_PERSPECTIVE;
	Xoffset = x;
	Yoffset = y;
	Zoffset = z;
	rotX = _rotX;
	rotY = _rotY;
	rho = _rho;
	scale = 1.0;
	lastCamera[X] = lastCamera[Y] = lastCamera[Z] = 0;
	upvector[X] = upvector[Y] = upvector[Z] = 0;	
	upvector[Y] = 1.0;
	aspect = 1.0;
	spin = 0;
	angle = 75;
	znear = 0.1;
	zfar = 10.0;
	eyeSeparation = 0.015;
	focalRatio = 0.5;
}

Camera::~Camera(void)
{
}

// Initializes the projection and modelview matrices
bool Camera::Draw(int drawmode /* = 0 */)
{
	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	if (shifting > 0)
	{
		shifting--;
		rotX+=theta_shift;
		rotY+=phi_shift;
		rho*=rho_shift;
		if (rotY < EPS) // avoid singularities using EPS constant.
			rotY = EPS;
		if (rotY > PI-EPS)
			rotY = PI-EPS;
		if (rho < EPS)
			rho = EPS;
	}


	triple right(0, 0, 0);
	triple up(upvector[X], upvector[Y], upvector[Z]);
	triple pos(lastCamera[X], lastCamera[Y], lastCamera[Z]);
	triple target(Xoffset, Yoffset, Zoffset);
	triple dir = (target - pos);
	double L, R, B, T, wd2, ndfl, rad = (angle / 360.0) * PI;
	double focal = length(dir) * focalRatio; // 4.0

	if (projo == CAM_LEFTEYE || projo == CAM_RIGHTEYE)
	{
		normalize(up);
		normalize(dir);
		right = crossProduct(dir, up);
		normalize(right);
		right = right * eyeSeparation / 2.0;
		wd2 = znear * tan(rad);
		ndfl = znear / focal;
	}

	switch (projo)
	{
		case CAM_PERSPECTIVE:
		gluPerspective(angle, aspect, znear, zfar);
		break;
		case CAM_ORTHO:
		glOrtho(aspect*-rho, aspect*rho, -rho, rho, -zfar, zfar);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

		case CAM_ORTHO2: // deprecated
		glOrtho(0, 110, -0.5, 107.5, -zfar, zfar);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotated(-90, 1, 0, 0);
		break;

		case CAM_LEFTEYE:
		glLoadIdentity();
		L = -aspect*wd2 + 0.5*eyeSeparation*ndfl;
		R = aspect*wd2 + 0.5*eyeSeparation*ndfl;
		T = wd2;
		B = -wd2;
		glFrustum(L, R, B, T, znear, zfar);
		break;

		case CAM_RIGHTEYE:
		glLoadIdentity();
		L = -aspect*wd2 - 0.5*eyeSeparation*ndfl;
		R = aspect*wd2 - 0.5*eyeSeparation*ndfl;
		T = wd2;
		B = -wd2;
		glFrustum(L, R, B, T, znear, zfar);
		break;
	}

	if (projo == CAM_LEFTEYE) right = right * -1;

	theta = rotX;
	phi = rotY;	

	double sphereX = (rho*sin(phi) * cos(theta));
	double sphereY = (rho*cos(phi));
	double sphereZ = (rho*sin(phi) * sin(theta));

	// from spherical coordinates (rho, theta, phi)
	if (input)
	{
		lastCamera[X] = sphereX + Xoffset;
		lastCamera[Y] = sphereY + Yoffset;
		lastCamera[Z] = sphereZ + Zoffset;		
	}
	else
	{
		sphereX = lastCamera[X] - Xoffset;
		sphereY = lastCamera[Y] - Yoffset;
		sphereZ = lastCamera[Z] - Zoffset;
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (!((sphereX == 0) && (sphereZ == 0)))
	{
	gluLookAt(	lastCamera[X]+right.x, lastCamera[Y]+right.y, lastCamera[Z]+right.z,
				Xoffset+right.x, Yoffset+right.y, Zoffset+right.z,
				0, 1, 0);
				//upvector[X], upvector[Y], upvector[Z]);
	}
	else
	{
		// gluLookAt doesn't work when viewer is incident to up-vector
		// Just orient manually in this case
		if ((lastCamera[Y] - Yoffset) >= 0)
		{
			glRotated(-90, 0, 1, 0);
			glRotated(-90, 0, 0, 1);
		}
		else
		{
			glRotated(-90, 0, 1, 0);
			glRotated(90, 0, 0, 1);
		}
		glTranslated(-Xoffset, -lastCamera[Y], -Zoffset);
	}
	if (upvector[X]==1)
		glRotatef(90, 0, 0, 1);
	if (upvector[Z]==1)
		glRotatef(-90, 1, 0, 0);

	return true;
}

// Sets X coordinate (horizontal mouse movement)
void Camera::setX(int _X)
{
	if (input)
	{
		switch (mode)
		{
		case 0:
			rotX -= ((lastX - _X) / 100.0);
			break;
		case 1:
		case 2:
			Xoffset += cos(rotX+3.14159265358979/2) * scale * (lastX - _X);
			Zoffset += sin(rotX+3.14159265358979/2) * scale * (lastX - _X);
			break;
		case 3:
			break;
		}
		lastX = _X;
	}
}

// Sets Y coordinate (vertical mouse movement)
void Camera::setY(int _Y)
{
	if (input)
	{
		switch (mode)
		{
		case 0:
			rotY += (((lastY - _Y)*(axisflip ? 1.0 : -1.0)) / 100.0);
			if (rotY < EPS) // avoid singularities using EPS constant.
				rotY = EPS;
			if (rotY > PI-EPS)
				rotY = PI-EPS;
			break;
		case 1:
			Yoffset += scale * (lastY - _Y);
			break;
		case 2:
			Xoffset -= cos(rotX) * scale * (lastY - _Y);
			Zoffset -= sin(rotX) * scale * (lastY - _Y);
			break;
		case 3:
			setRadius(fabs(rho * (1.0 + (lastY - _Y) * -.01)));
			break;
		}
		lastY = _Y;
	}
}

// Returns the 6 current camera coordinates
void Camera::getCamera(double * temp)
{
	temp[0] = lastCamera[0];
	temp[1] = lastCamera[1];
	temp[2] = lastCamera[2];
	temp[3] = Xoffset;
	temp[4] = Yoffset;
	temp[5] = Zoffset;
}

// Returns 3 camera coordinates (relative position)
void Camera::getQuadrant(double * temp)
{
	if (!reverse)
	{
		temp[0] = lastCamera[0]-Xoffset;
		temp[1] = lastCamera[1]-Yoffset;
		temp[2] = lastCamera[2]-Zoffset;	
	}
	else
	{
		temp[0] = lastCamera[0]+Xoffset;
		temp[1] = lastCamera[1]+Yoffset;
		temp[2] = lastCamera[2]+Zoffset;
	}
}

// cycle through projection modes
void Camera::toggleProjection()
{
	mode = -1;
	projo = (projo+1)%3; 
	if (projo==CAM_ORTHO)
	{
		theta = rotX = 3.14159265358979 / 2;
		phi = rotY = 3.14159265358979 / 2;
	}
	if (projo==CAM_ORTHO2)
	{
		theta = rotX = 3.14159265358979;
		phi = rotY = 3.14159265358979 / 2;
	}
	lastCamera[X] = (rho*sin(phi) * cos(theta)) + Xoffset;
	lastCamera[Z] = (rho*sin(phi) * sin(theta)) + Zoffset;
	lastCamera[Y] = (rho*cos(phi)) + Yoffset;	
}

// Draw a vertex at camera lookAt point
void Camera::drawCenter()
{
	glVertex3d(Xoffset, Yoffset, Zoffset);
}

// Set radius of "orbit"
void Camera::setRadius(double r)
{
	rho = r;
	scale = r / 100.0;
}

// Force camera to look from x/y/z to tx/ty/tz
void Camera::setManualPosition(double x, double y, double z, double tx, double ty, double tz) // specifies manual camera position and lookAt point
{
	lastCamera[X] = x;
	lastCamera[Y] = y;
	lastCamera[Z] = z;
	Xoffset = tx;
	Yoffset = ty;
	Zoffset = tz;
	input = false;
}

void Camera::setViewDirection(int axis, double FPS)
{
	// used to insure any chained setviews result in "forward" axes (PI/2 or 0)	
	rotX = fmod(rotX, 2.0*PI);
    
	if (rotX < 0)
		rotX += 2.0*PI;
	//rotY = fmod(rotY, PI);
	if (rotY > PI-EPS) rotY = PI - EPS;
	//setOffset(0, 0, 0);
	
	// take a half of a second at most
	shifting = clamp((int)(FPS/2.0), 1, 24);

	rho_shift = 1.0;

	// calculate the distance to shift the view, depending on current angle.
	switch (axis)
	{
		case X:
			if (rotX < PI/2)
				theta_shift = (0+EPS - rotX) / (double)shifting;
			else if (rotX <= PI)
				theta_shift = (PI - EPS - rotX) / (double)shifting;
			else if (rotX <= 3.0*PI/2.0)
				theta_shift = (PI - EPS - rotX) / (double)shifting;	
			else
				theta_shift = (2*PI + EPS - rotX) / (double)shifting;	

			phi_shift = (PI/2.0 - rotY) /  (double)shifting;
			break;
		case Y:
			if (rotX < PI/2)
				theta_shift = (0+EPS - rotX) / (double)shifting;
			else if (rotX <= PI)
				theta_shift = (PI - EPS - rotX) / (double)shifting;
			else if (rotX <= 3.0*PI/2.0)
				theta_shift = (PI - EPS - rotX) / (double)shifting;	
			else
				theta_shift = (2*PI + EPS - rotX) / (double)shifting;
			phi_shift = (0+EPS - rotY) /  (double)shifting;
			break;
		case Z:
			if (rotX <= PI)
				theta_shift = (PI/2.0 - EPS - rotX) / (double)shifting;
			else
				theta_shift = (3.0*PI/2.0 + EPS - rotX) / (double)shifting;			
			phi_shift = (PI/2.0 - rotY) /  (double)shifting;
			break;
	}	
}

void Camera::StartPan(double value)
{
	shifting = 1000;
	theta_shift = value;
	phi_shift = 0;
	rho_shift = 1.0;
}

void Camera::StartTilt(double value)
{
	shifting = 1000;
	phi_shift = value;
	theta_shift = 0;
	rho_shift = 1.0;
}

void Camera::StartZoom(double value)
{
	shifting = 1000;
	rho_shift = value;
	theta_shift = phi_shift = 0;
}

void Camera::StopMotion()
{
	shifting = 0;
	theta_shift = phi_shift = 0;
	rho_shift = 1.0;
}

bool Camera::getFrustumPlanes(triple points[4], triple normals[4])
{
	triple right(0, 0, 0);
	triple up(upvector[X], upvector[Y], upvector[Z]);
	triple pos(lastCamera[X], lastCamera[Y], lastCamera[Z]);
	triple target(Xoffset, Yoffset, Zoffset);
	triple dir = (target - pos);	
	double rad = (angle / 360.0) * PI;

	normalize(up);
	normalize(dir);
	right = crossProduct(dir, up);
	normalize(right);

	triple Y_axis = crossProduct(right, dir).Normalized();

	if (projo == CAM_PERSPECTIVE)
	{
		double near_height = 2.0 * tan(rad) * znear;
		double near_width = near_height * aspect;
		double far_height = 2.0 * tan(rad) * zfar;
		double far_width  = far_height * aspect;

		triple near_center = dir * znear + pos;
		// clockwise from "upper left"
		points[0] = near_center-(Y_axis*near_height/2.0)-(right*near_width/2.0);
		points[1] = points[0];
		points[2] = near_center+(Y_axis*near_height/2.0)+(right*near_width/2.0);
		points[3] = points[2];

        normals[0] =  crossProduct((points[0] - pos).Normalized(), Y_axis).Normalized(); // left
		normals[1] =  crossProduct((points[1] - pos).Normalized(), right*-1.0).Normalized(); // top
		normals[2] =  crossProduct((points[2] - pos).Normalized(), Y_axis*-1.0).Normalized(); // right
		normals[3] =  crossProduct((points[3] - pos).Normalized(), right).Normalized(); // bottom

		points[0] = points[1] = points[2] = points[3] = pos;
		return true;
	}
	else return false;
}