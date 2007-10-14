/*
GraphView

	GraphView is a GLWnd/CWnd derived window class that manages
	a GraphDisplay object.  The drawing and buffer-swapping of
	this object must be managed by the spawning thread.
	The class was designed to work well with a WS_EX_TOOLWINDOW
	extended window style.

	By Vincent Hoon, 2005
*/
#pragma once
#include "stdafx.h"
#include "glwnd.h"
#include <vector>
#include "GraphDisplay.h"
#include "Particle.h"

class GraphView :
	public GLWnd
{
public:
	GraphView(double font_size = 0.05);
	~GraphView(void);
	DECLARE_MESSAGE_MAP()

public:
	void GLInit(void);
	bool Draw(void);
	void SetFrame(int current) { current_frame = current; }
	void LoadGraphData(const char* filename) { thisgraph->LoadFile(filename); }
	void LoadGraphLegend(int type) { thisgraph->LoadLegend(type); }
	//void LoadParticle(Particle* p, int datatype);
	void LoadParticle(Particle* p, std::vector<double>::const_iterator times,  int datatype);
	void Kill() { kill = true; }
	bool IsDead() { return kill; }

protected:
	int current_frame;
	GraphDisplay* thisgraph;
	bool init;
	bool kill;
	bool dragging;
	int last_button;
	int scale;
	double x, y;
	double fontsize;
public:
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};