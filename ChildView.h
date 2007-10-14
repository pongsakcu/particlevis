/*
CChildView class

		   The child view routes UI commands to underlying objects, manages the OpenGL
		context, and in general is the master of program flow.  GLInit initializes
		OpenGL state, Draw governs the actual rendering of the particle set, and other
		functions are bound to UI events.

		Vincent Hoon, 2007
*/

#pragma once
#include "stdafx.h"
#include "GLWnd.h"
#include "ParticleSet.h"
#include "Camera.h"
#include "PNGWriter.h"
#include "AVIWriter.h"
#include "ParticleSceneObj.h"
#include "GLConsole.h"
#include "WGLFont.h"
#include "GLScene.h"

#define NORMAL 1
#define ZPICK1 2
#define ZPICK2 3
#define PARTPICK 4
#define CAMPICK 5
#define CAMPPICK 6
#define LIGHT1PICK 7
#define LIGHT2PICK 8
#define PARTHIDE 9
#define GRAPHCORNERA 10
#define GRAPHCORNERB 11
#define SELECTION 12

#define SHOWTIME 0
#define SHOWFPS 1
#define RUNBACKWARDS 2
#define RUNLOOP 3
#define SUSPEND 4
#define SAVEPNG 5
#define SAVEAVI 6
#define RECORD 7

// Some forward declarations
class PControlDialog;
class DialogPositioner;
class DialogVisibility;
class DialogStereo;
class PStatefileReader;
class PXMLReader;
class GraphView;

class CChildView : public GLWnd
{
	friend class UIProcessor;
	friend class BackgroundRenderer;
	friend class GLScene;
public:
	CChildView();
	UIProcessor* ui;
	bool living;

protected:	
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void GLInit(void);
	bool BrowseForFile(const char * title, const char * filter, char * fileNamePtr); // create a file dialog to get a filename.
	void CheckMenuID(bool value, UINT menuID);

	LARGE_INTEGER lasttime, thistime, temptime, loadtime;
	double freq, znear, zfar, ms;
	bool dragging, stopped, saveimage, saveall, pbufferEXT;
	int frame, state;
	int fps;
	int lastX, lastY, lastButton;
	int currentX, currentY;
	triple lastPoint;
	unsigned int m_texName;
	int followParticle;
	string message;
	double lastfps;
	double delay;
	triple camera, target;
	bool showFPS, showTime;
	bool runBackwards, runLoop;
	bool nextframeTag;
	bool pointEXT;	
	PStatefileReader* loading; // threaded I/O object for state files
	PXMLReader* parsing; // threaded I/O object for XML parsing
	DialogStereo* stereo;
	GLScene* mainscene;
	GLConsole* console;
	WGLFont* myfont;
	CSingleLock* master;

	Camera cam;	

	//bool axes, texturing, time, follow, drawCone;
	bool splitScreen, drawGraph, suspended, saveAVI, savePNG;
	CMenu* themenu;

	int spinval;
	char fps_text[64];

	ParticleSceneObj camObj;

	bool IncrementFrame();
	void DrawText();	

public:
	virtual ~CChildView();
	virtual bool Draw(void);	

	static CString exePath;

	static ParticleSet* primary;	// particles to be drawn
	PNGWriter pngOut;				// image capture object
	AVIWriter aviOut;
	static vector<int> watch_list;
	static bool axes, texturing, time, follow, drawCone;

	PControlDialog* controller;
	DialogPositioner* positioner;
	DialogVisibility* visibility;
	bool controllershown;

	static triple textColor;

	void SetMessage(const char* msg);
	void RedrawController();
	bool StartStop() { stopped = !stopped;  return stopped; }
	void ModifyCurrentFrame(int value);
	void SeekToFrame(int pos);
	void SetState(int newstate);
	void ResetZ() { znear = 0.1; zfar = 10.0; }
	bool IsStopped() { return stopped; }
	bool IsHidden() { return minimized; }
	void SetSpeed(double fps);
	void SetController(PControlDialog* ptr) { controller = ptr; }
	int GetFrame() { return frame; }
	void SaveImage();
	void LoadCMenu(CMenu* in) { themenu = in; }
	void UpdateMenu(void);
	
	void BackgroundRender(char * path, char * output, int width = 1000, int height = 750, int fram = 0, int buffermode = 0, bool ascii = true, char* xml = NULL);
	GLScene* Scene() { return mainscene; }
	//void SetLight(const int ref, bool enabled, triple direction, double bright, bool point = false);
	//void GetLight(const int ref, bool& enabled, triple &direction, double& bright, bool& point);

	bool GetOption(int ref);
	void SetOption(int ref, bool value);

	void ForceCamera();
	void Suspend() { suspended = true; }
	void Restore() { suspended = false; }
	void SetMinimized(bool min_state) { minimized = min_state; }
	inline bool IsMinimized() const { return minimized; }

	void HideController() { controllershown = false; }
	void LoadFile(string filename, bool add = false);
	void WriteScenePrefs();
	void LoadScenePrefs();
	void DrawCone(bool drawcone);
	bool AddGraphView(GraphView* graph, int width = 250, int height = 250);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnFileLoaddatafile();
	afx_msg void OnFileAdddatafile();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSetqualityHighest();
	afx_msg void OnSetqualityHigh();
	afx_msg void OnSetqualityMedium();
	afx_msg void OnSetqualityLow();
	afx_msg void OnParticlesTrackparticle();	
	afx_msg void OnViewShowcontroldialog();
	afx_msg void OnEditResetmarkedparticles();
	afx_msg void OnSetqualityUsepoints();
	afx_msg void OnViewToggleaxes();
	afx_msg void OnParticlesSetviewingoptions();
	afx_msg void OnViewUsetextures();
	afx_msg void OnViewTogglevelocityvectors();
	afx_msg void OnEditResetcamera();
	afx_msg void OnSetcamerapositionX();
	afx_msg void OnSetcamerapositionY();
	afx_msg void OnSetcamerapositionZ();
	afx_msg void OnClose();
	afx_msg void OnEditImagesavingoptions();
	afx_msg void OnParticlesSetviewingvolume();
	afx_msg void OnViewDrawparticles();
	afx_msg void OnSetgeometryqualityUltra();
	afx_msg void OnViewUselighting();
	afx_msg void OnSetcamerapositionToggleprojection();
	afx_msg void OnSetcamerapositionTrackparticle();

	afx_msg void OnHelpKeyboardshortcuts();
	afx_msg void OnHelpCmdline();

	afx_msg void OnViewDrawnormalvectors();
	afx_msg void OnViewDrawrotationalvelocityvectors();
	
	afx_msg void OnFileLoadcolor();
	afx_msg void OnViewUsespecularhighlights();
	afx_msg void OnViewLoadnewtexturemap();
	afx_msg void OnEditSelectbackgroundcolor();	
	afx_msg void OnSetcolorschemeDefaultwhite();
	afx_msg void OnSetcolorschemeMaptovelocity();
	afx_msg void OnSetcolorschemeMaptoangularvelocity();
	afx_msg void OnEditPositioner();
	afx_msg void OnViewAdjustpointsprites();
	afx_msg void OnFileOpenparticledescriptor();
	afx_msg void OnEditMarkparticletohide();
	afx_msg void OnFileGeneratebenchmarkframe();
	afx_msg void OnTerminateFileProcesses();
	afx_msg void OnGraphEnablegraphdisplay();
	afx_msg void OnGraphLoaddataintograph();
	afx_msg void OnSettransparencyNone();
	afx_msg void OnSettransparencyAdditive();
	afx_msg void OnSettransparencyZsort();
	afx_msg void OnSetcolorschemeClearcolormaps();
	afx_msg void OnEditCurrentGradient();
	afx_msg void OnGradientSettospectrum();
	afx_msg void OnGradientSettogreyscale();
	afx_msg void OnGradientSettothermal();
	afx_msg void OnViewSetsurfacemapproperties();
	afx_msg void OnFileOpenzippeddatafile();
	afx_msg void OnEditCopyframedata();
	afx_msg void OnGraphCreatevelocitylegend();
	afx_msg void OnGraphCreateangularvelocitylegend();
	afx_msg void OnEditSetspraycone();
	afx_msg void OnEditMarkparticlebyid();
	afx_msg void OnReferenceEnablevolumetricrendering();
	afx_msg void OnReferenceVolumetricoptions();
	afx_msg void OnFileOpenvectorfile();
	afx_msg void OnEditAuto();
	afx_msg void OnViewResizevectors();
	afx_msg void OnFileLoadNonColorMap();
	afx_msg void OnViewRendertooffscreenbuffer();
	afx_msg void OnSetgeometryqualityUsesphereshader();
	afx_msg void OnSphereshadertypePhongshading();
	afx_msg void OnSphereshadertypePhong();
	afx_msg void OnSphereshadertypeCartoon();
	afx_msg void OnSphereshadertypeExperimental();
	afx_msg void OnSphereshadertypeBasicshading();
	afx_msg void OnMenuVisibility();
	afx_msg void OnSphereshadertypePerspective();
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnReferenceCreateparticle();	
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnAnalysisMarkoutgrid();
	afx_msg void OnAnalysisApplycolorsplit();
	afx_msg void OnSphereshadertypeVelocityglyph();
	afx_msg void OnSphereshadertypeMotionblur();
	afx_msg void OnAnalysisLoadvolumefile();
	afx_msg void OnAnalysisCreatedensitymap();
	afx_msg void OnEditInterfacecoloroptions();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFileGeneratespheredescriptor();
	afx_msg void OnSetcolorschemeMappositiontopos();
	afx_msg void OnFileReportcurrentdata();
	afx_msg void OnSphereshadertypeVelocityglyph2();
};