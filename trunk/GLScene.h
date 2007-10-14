/*
GLScene

		  The GLScene class encapsulates a high level view of the current scene.
		Scene-level information such as the camera, lights, and scene elements are
		stored in a scene object.  High level algorithms for rendering (e.g. depth
		peeling, offscreen rendering) are also implemented here.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "ChildView.h"
#include "ParticleSet.h"
#include "Camera.h"
#include "RenderTexture.h"
#include "GLShader.h"

#define MAXLIGHTS 2

class GLScene
{
public:
	GLScene(CChildView* view, Camera* cam, ParticleSet * primary);
	~GLScene(void);

	void GLInit();
	void DrawInstance(int frame, bool clear_frame = true);
	void DrawOccludedInstance(int frame);
	void DrawPeel(int frame, int width, int height, bool first_pass, bool last_pass);

	int QueryScene(int x, int y, int frame, int mode);

	void SaveImage();
	bool RenderToImage(int frame, int x, int y, int AA);
	
	void SetLight(const int ref, bool enabled, triple direction, double bright, bool pointsource);
	void SetLight(const int ref, triple pos);
	void GetLight(const int ref, bool &enabled, triple &direction, double &bright, bool &pointsource);
	triple GetLightPos(const int ref);
	float ambience;

	void EnableVelocity() { shader_pass_velocity = true; }
	void DisableVelocity() { shader_pass_velocity = false; }

	void SetPick(int x, int y, int x2, int y2);
	void SetAxes(float scale, bool resize) { axes_scale = scale; axes_resize = resize; }
	float GetAxesScale() { return axes_scale; }
	bool GetAxesResizing() { return axes_resize; }
private:
	CChildView * view;
	Camera * cam;
	ParticleSet * primary;
	RenderTexture * backTex;
	RenderTexture * frontTex;
	RenderTexture * off_storage;

	triple lights[MAXLIGHTS];
	double lightB[MAXLIGHTS];
	bool useLight[MAXLIGHTS];
	bool light_points[MAXLIGHTS];

	bool shader_pass_velocity;

	float axes_scale;
	bool axes_resize;

	GLShader peel_shader, blur_shader;

	unsigned int depth_tex;	
	int tex_width, tex_height;
	unsigned int sphereTex;
	bool picking;
	int pick_x, pick_y, pick_x2, pick_y2;
	int viewport[4];

	void DrawAxes(double D, double A);
};
