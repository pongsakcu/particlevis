// ParticleVis.h : main header file for the ParticleVis application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <vector>
using std::vector;
class GraphView;

// CParticleVisApp:
// See ParticleVis.cpp for the implementation of this class
//

class CParticleVisApp : public CWinApp
{
public:
	CParticleVisApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	vector<GraphView*> subwindow_stack;
	DECLARE_MESSAGE_MAP()
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual int Run();
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);
	afx_msg void OnFileSuspenddrawingprocesses();

private:
	bool suspendDraw;
	
};

extern CParticleVisApp theApp;