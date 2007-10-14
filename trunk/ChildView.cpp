/*

CChildView Class

	This is the primary user interface class, and arbiter of program flow
	and user input.

	It is also way too big.

*/

#include "stdafx.h"
#include "ChildView.h"
#include "Commdlg.h"
#include "ParticleVis.h"
#include "Camera.h"
#include "PStatefileReader.h"
#include "PColormapReader.h"
#include "PVectorReader.h"
#include "PXMLReader.h"
#include "PZipReader.h"
#include "PBinaryReader.h"
#include "KeysDialog.h"
#include <string>
#include <sstream>
#include <deque>
#include <iostream>
#include <conio.h>
#include <math.h>
#include "MainFrm.h"
#include "WGLContext.h"
#include "GraphDisplay.h"
#include "GraphView.h"
#include "GLShader.h"

#include "DialogRandom.h"
#include "DialogStereo.h"
#include "DialogNumInput.h"
#include "DialogGradient.h"
#include "DialogSurfaceMap.h"
#include "DialogSlices.h"
#include "PControlDialog.h"
#include "DialogPositioner.h"
#include "DialogScene.h"
#include "ImageDialog.h"
#include "BoundsDialog.h"
#include "DialogSpriteTuner.h"
#include "DialogVectorScaler.h"
#include "DialogVisibility.h"
#include "DialogAnalyze.h"
#include "DialogInterfaceColor.h"
#include "DialogSplit.h"

#include "UIProcessor.h"
#include "GLPlanarSlices.h"
#include "BackgroundRenderer.h"

#include "glext.h"
#include "wglext.h"

//extern "C" void exit (int) throw (); // WHY I DO NOT KNOW
#include <cstdlib>
#include "ChildView.h"
using ::exit;

vector<int> CChildView::watch_list;
bool CChildView::axes, CChildView::texturing, CChildView::time, CChildView::follow, CChildView::drawCone;
CString CChildView::exePath;
triple CChildView::textColor = triple(1, 1, 1);

#define MAX_ATTRIBS 16
#define ALPHAWEIGHT 0.85

extern int particle_type;
bool map_mode = false;

int depth_passes = 0;
int counted = 0;
double countratio = 0;
bool counting = false;
unsigned char imageData[3000*2000*3];

extern double drumR;
char map_filename[256];

extern CSemaphore lock;
extern bool isWglExtensionSupported(const char *extstring);
extern bool isExtensionSupported(const char *extstring);

// GL extensions

#define INIT_ENTRY_POINT( funcname, type ) \
funcname = (type) wglGetProcAddress(#funcname);

using namespace std;

ParticleSet* CChildView::primary;
bool isExtensionSupported(const char *extstring);

// CChildView

BEGIN_MESSAGE_MAP(CChildView, GLWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_LOADDATAFILE, OnFileLoaddatafile)
	ON_WM_TIMER()
	ON_COMMAND(ID_SETQUALITY_HIGHEST, OnSetqualityHighest)
	ON_COMMAND(ID_SETQUALITY_HIGH, OnSetqualityHigh)
	ON_COMMAND(ID_SETQUALITY_MEDIU, OnSetqualityMedium)
	ON_COMMAND(ID_SETQUALITY_LOW, OnSetqualityLow)
	ON_COMMAND(ID_PARTICLES_TRACKPARTICLE, OnParticlesTrackparticle)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_VIEW_SHOWCONTROLDIALOG, OnViewShowcontroldialog)
	ON_COMMAND(ID_EDIT_RESETMARKEDPARTICLES, OnEditResetmarkedparticles)
	ON_COMMAND(ID_SETQUALITY_USEPOINTS, OnSetqualityUsepoints)
	ON_COMMAND(ID_VIEW_AXES, OnViewToggleaxes)
	ON_COMMAND(ID_PARTICLES_SETVIEWINGOPTIONS, OnParticlesSetviewingoptions)
	ON_COMMAND(ID_VIEW_USETEXTURES, OnViewUsetextures)
	ON_COMMAND(ID_VIEW_TOGGLEVELOCITYVECTORS, OnViewTogglevelocityvectors)
	ON_COMMAND(ID_EDIT_RESETCAMERA, OnEditResetcamera)
	ON_COMMAND(ID_SETCAMERAPOSITION_X, OnSetcamerapositionX)
	ON_COMMAND(ID_SETCAMERAPOSITION_Y, OnSetcamerapositionY)
	ON_COMMAND(ID_SETCAMERAPOSITION_Z, OnSetcamerapositionZ)
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_EDIT_IMAGESAVINGOPTIONS, OnEditImagesavingoptions)
	ON_COMMAND(ID_PARTICLES_SETVIEWINGVOLUME, OnParticlesSetviewingvolume)
	ON_COMMAND(ID_VIEW_DRAWPARTICLES, OnViewDrawparticles)
	ON_COMMAND(ID_SETGEOMETRYQUALITY_ULTRA, OnSetgeometryqualityUltra)
	ON_COMMAND(ID_VIEW_USELIGHTING, OnViewUselighting)
	ON_COMMAND(ID_SETCAMERAPOSITION_TOGGLEPROJECTION, OnSetcamerapositionToggleprojection)
	ON_COMMAND(ID_HELP_KEYBOARDSHORTCUTS, OnHelpKeyboardshortcuts)
	ON_COMMAND(ID_VIEW_DRAWNORMALVECTORS, OnViewDrawnormalvectors)
	ON_COMMAND(ID_VIEW_DRAWROTATIONALVELOCITYVECTORS, OnViewDrawrotationalvelocityvectors)
	ON_COMMAND(ID_SETCAMERAPOSITION_TRACKPARTICLE, OnSetcamerapositionTrackparticle)
	ON_COMMAND(ID_FILE_LOADCOLOR, OnFileLoadcolor)
	ON_COMMAND(ID_VIEW_USESPECULARHIGHLIGHTS, OnViewUsespecularhighlights)
	ON_COMMAND(ID_FILE_LOADTEXTUREMAP, OnViewLoadnewtexturemap)
	ON_COMMAND(ID_EDIT_SELECTBACKGROUNDCOLOR, OnEditSelectbackgroundcolor)
	ON_COMMAND(ID_HELP_COMMANDLINEPARAMETERS, OnHelpCmdline)
	ON_COMMAND(ID_SETCOLORSCHEME_DEFAULTWHITE, OnSetcolorschemeDefaultwhite)
	ON_COMMAND(ID_SETCOLORSCHEME_MAPTOVELOCITY, OnSetcolorschemeMaptovelocity)
	ON_COMMAND(ID_SETCOLORSCHEME_MAPTOANGULARVELOCITY, OnSetcolorschemeMaptoangularvelocity)
	ON_COMMAND(ID_EDIT_POSITIONCAMERA, OnEditPositioner)
	ON_COMMAND(ID_VIEW_ADJUSTPOINTSPRITES, OnViewAdjustpointsprites)
	ON_COMMAND(ID_FILE_OPENPARTICLEDESCRIPTOR, OnFileOpenparticledescriptor)
	ON_COMMAND(ID_EDIT_MARKPARTICLETOHIDE, OnEditMarkparticletohide)
	ON_COMMAND(ID_FILE_GENERATEBENCHMARKFRAME, OnFileGeneratebenchmarkframe)
	ON_COMMAND(ID_FILE_TERMINATEALLFILEPROCESSES, OnTerminateFileProcesses)
	ON_COMMAND(ID_GRAPH_ENABLEGRAPHDISPLAY, OnGraphEnablegraphdisplay)
	ON_COMMAND(ID_GRAPH_LOADDATAINTOGRAPH, OnGraphLoaddataintograph)
	ON_COMMAND(ID_SETTRANSPARENCY_NOTRANSPARENCY, OnSettransparencyNone)
	ON_COMMAND(ID_SETTRANSPARENCY_ADDITIVEALPHABLENDING, OnSettransparencyAdditive)
	ON_COMMAND(ID_SETTRANSPARENCY_Z, OnSettransparencyZsort)
	ON_COMMAND(ID_SETCOLORSCHEME_CLEARCOLORMAPS, OnSetcolorschemeClearcolormaps)
	ON_COMMAND(ID_SETCOLORSCHEME_EDITCURRENTGRADIENT, OnEditCurrentGradient)
	ON_COMMAND(ID_CURRENTGRADIENT_SETTOSPECTRUM, OnGradientSettospectrum)
	ON_COMMAND(ID_CURRENTGRADIENT_SETTOGREYSCALE, OnGradientSettogreyscale)
	ON_COMMAND(ID_CURRENTGRADIENT_SETTOTHERMAL, OnGradientSettothermal)
	ON_COMMAND(ID_VIEW_SETSURFACEMAPPROPERTIES, OnViewSetsurfacemapproperties)
	ON_COMMAND(ID_EDIT_COPYFRAMEDATA, OnEditCopyframedata)
	ON_COMMAND(ID_GRAPH_CREATEVELOCITYLEGEND, OnGraphCreatevelocitylegend)
	ON_COMMAND(ID_GRAPH_CREATEANGULARVELOCITYLEGEND, OnGraphCreateangularvelocitylegend)
	ON_COMMAND(ID_EDIT_SETSPRAYCONE, OnEditSetspraycone)
	ON_COMMAND(ID_EDIT_MARKPARTICLEBYID, OnEditMarkparticlebyid)
	ON_COMMAND(ID_REFERENCE_ENABLEVOLUMETRICRENDERING, OnReferenceEnablevolumetricrendering)
	ON_COMMAND(ID_REFERENCE_VOLUMETRICOPTIONS, OnReferenceVolumetricoptions)
	ON_COMMAND(ID_FILE_OPENVECTORFILE, OnFileOpenvectorfile)
	ON_COMMAND(ID_EDIT_AUTO, OnEditAuto)
	ON_COMMAND(ID_VIEW_RESIZEVECTORS, OnViewResizevectors)
	ON_COMMAND(ID_FILE_LOADNON, OnFileLoadNonColorMap)
	ON_COMMAND(ID_VIEW_RENDERTOOFFSCREENBUFFER, OnViewRendertooffscreenbuffer)
	ON_COMMAND(ID_SETGEOMETRYQUALITY_USESPHERESHADER, OnSetgeometryqualityUsesphereshader)
	ON_COMMAND(ID_SPHERESHADERTYPE_PHONGSHADING, OnSphereshadertypePhongshading)
	ON_COMMAND(ID_SPHERESHADERTYPE_PHONG, OnSphereshadertypePhong)
	ON_COMMAND(ID_SPHERESHADERTYPE_CARTOON, OnSphereshadertypeCartoon)
	ON_COMMAND(ID_SPHERESHADERTYPE_EXPERIMENTAL, OnSphereshadertypeExperimental)
	ON_COMMAND(ID_SPHERESHADERTYPE_BASICSHADING, OnSphereshadertypeBasicshading)	
	ON_COMMAND(ID_Menu_Visibility, OnMenuVisibility)
	ON_COMMAND(ID_SPHERESHADERTYPE_PERSPECTIVE, OnSphereshadertypePerspective)
	ON_COMMAND(ID_REFERENCE_CREATEPARTICLE, OnReferenceCreateparticle)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_ANALYSIS_MARKOUTGRID, OnAnalysisMarkoutgrid)
	ON_COMMAND(ID_ANALYSIS_APPLYCOLORSPLIT, OnAnalysisApplycolorsplit)
	ON_COMMAND(ID_SPHERESHADERTYPE_VELOCITYGLYPH, OnSphereshadertypeVelocityglyph)
	ON_COMMAND(ID_SPHERESHADERTYPE_MOTIONBLUR, OnSphereshadertypeMotionblur)
	ON_COMMAND(ID_ANALYSIS_LOADVOLUMEFILE, OnAnalysisLoadvolumefile)
	ON_COMMAND(ID_ANALYSIS_CREATEDENSITYMAP, OnAnalysisCreatedensitymap)
	ON_COMMAND(ID_EDIT_INTERFACECOLOROPTIONS, OnEditInterfacecoloroptions)
	ON_WM_KEYUP()
	ON_COMMAND(ID_FILE_GENERATESPHEREDESCRIPTOR, OnFileGeneratespheredescriptor)
	ON_COMMAND(ID_SETCOLORSCHEME_MAPPOSITIONTOPOS, OnSetcolorschemeMappositiontopos)
	ON_COMMAND(ID_FILE_REPORTCURRENTDATA, OnFileReportcurrentdata)
	ON_COMMAND(ID_SPHERESHADERTYPE_VELOCITYGLYPH2, OnSphereshadertypeVelocityglyph2)
END_MESSAGE_MAP()

GLPlanarSlices testslice;
int drawslices = -1;

CChildView::CChildView() :
lastX(-1), lastY(-1)
{
	primary = NULL;		// ParticleSet object containing scene data
	frame = 0;			// current frame of particle system
	znear = zfar = 0;	// records z-planes for truncating data
	dragging = false;	// state of mouse
	freq = -1;			// records cpu clock frequency
	primary = new ParticleSet(&lock, &(this->cam));
	state = NORMAL;		// state of interface
	ms = 20.0 / 1000.0;	// frame-rate cap
	stopped = true;		// playing back/stopped
	delay = 0;			// FPS cap

	// Modeless dialogs:
	controller = NULL;	// pointer to control dialog
	visibility = NULL;  // pointer to visibility dialog
	positioner = NULL;  // pointer to positioner dialog

	lastfps = fps = 0;
	axes = false;
	pngOut.SetFileName("temp");
	saveimage = false;
	saveall = false;
	controllershown = true;
	time = true;
	pbufferEXT = false;
	followParticle = -1; // holds a particle index to follow
	follow = false;
	message = "";
	showTime = true;
	showFPS = false;
	runBackwards = false;
	runLoop = false;
	
	camera = triple(-0.127609083,-0.057584043,0.0);	
	//target = triple(0.44464,-0.707107,0.0);
	target = triple(0.44464-0.127609083,-0.707107-0.057584043,0.0);

	normalize(target);
	drawCone = false;
	splitScreen = false;
	stereo = NULL;
	loading = NULL;
	parsing = NULL;
	nextframeTag = false;
	drawGraph = false;
	suspended = false;
	spinval = 0;
	savePNG = true;
	saveAVI = false;
	living =true;

	ui = new UIProcessor(this, primary); 
	
	exePath = "ParticleVis.exe";
	GetModuleFileName(NULL, exePath.GetBuffer(_MAX_PATH), _MAX_PATH);
	exePath.ReleaseBuffer();
	exePath.Replace("ParticleVis.exe", "");		
	GLShader::SetPath(exePath.GetString());
}

CChildView::~CChildView() // deconstructor: destroy contexts, write information to registry
{
	OnTerminateFileProcesses();
	WriteScenePrefs();

	wglDeleteContext(hRC);
	DeleteDC(pDC->m_hDC);
	aviOut.CloseFile();
	delete primary;
}

// CChildView message handlers
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!GLWnd::PreCreateWindow(cs))
		return FALSE;

	// Make our child window stretch to the edge, leave border on the Main Frame
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN; // OpenGL requires these two styles
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	controller = NULL;
	return TRUE;
}

// Initialize OpenGL stuff

void CChildView::GLInit(void)
{
	znear = 0.1;
	zfar = 750;
	cam.setRadius(600);
	cam.setZBuffer(znear, zfar);
	cam.setRotation(1, 1);
	cam.setOffset(0, 0, 0);
	cam.setAngle(60);
	cam.setScale(6);
	cam.setUpVector(0, 1, 0);
	cam.setProjection(CAM_PERSPECTIVE);
	mainscene = new GLScene(this, &cam, primary);	
	mainscene->GLInit();

	glGenTextures(1, &m_texName);	
	LoadTexture("band.bmp", &m_texName);
	glBindTexture(GL_TEXTURE_2D, m_texName);	
	
	QueryPerformanceFrequency(&lasttime);
	freq = (double)lasttime.QuadPart; // ticks per second

	Particle::transVelocityLength = 0.01f;
	LoadScenePrefs();

	primary->SetOption(LIGHTING, true);
	primary->SetOption(TEXTURING, false);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	BuildFont(pDC->m_hDC, base, 20);
	primary->PRend = new GeometryEngine(this);

	myfont = new WGLFont(GetDC()->GetSafeHdc());
	myfont->MakeFont("Courier New", 20, false);
	myfont->SetAspect(1.3333);
	myfont->SetSize(0.04);

	console = new GLConsole(myfont);
	console->SetLineSize(20);
	console->SetMaxLines(6);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	GeometryEngine::RenderDefaults();
	Invalidate(TRUE);

	master = new CSingleLock(&lock);
	strcpy(fps_text, "FPS");
	lastfps = 0.0;
}

// Primary drawing function
bool CChildView::Draw()
{
	static deque<double> frametimes;
	LARGE_INTEGER frametime;
	living = true;
	static bool latch_controller = false;
	if (suspended || minimized)
	{
		RedrawController();
		return true;
	}

	// Query cpu clock to determine time elapsed since last frame
	QueryPerformanceCounter(&thistime);

	// check load status
	if (loading != NULL)
	{
		if (loading->IsFinished())
		{
			SetMessage("Reticulating splines...");
			if (nextframeTag) // print out loading summary
			{
				loading->CleanUp(primary);
				delete loading;
				loading = NULL;
				char buffer[255];
				int particles = primary->GetMaxParticles(), frames = primary->GetMaxFrames();
				if (particles > 0 && frames > 0)
				{
					if (particles > 1 && frames > 1)
						sprintf(buffer, "File load completed in %.1f seconds.  Loaded %d particles, %d frames.",
							(double)((thistime.QuadPart - loadtime.QuadPart) / freq),
							particles,
							frames);					
					else // alternate phrasing, for great justice!
						sprintf(buffer, "File load completed in %.1f seconds.  Particles: %d.  Frames: %d.",
							(double)((thistime.QuadPart - loadtime.QuadPart) / freq),
							particles,
							frames);
				}
				else
					strcpy(buffer, "File error: no frames or particles loaded.");
				SetMessage(buffer);
				nextframeTag = false;
			}
			else
				nextframeTag = true;
		}
	}
	if (parsing != NULL)
	{
		if (parsing->IsFinished())
		{
			SetMessage("XML Parsing complete.");
			delete parsing;
			parsing = NULL;
		}
	}

	if (!IncrementFrame()) // decide what frame to draw, if any
		return false; // skipping this pass	

	// FPS: Push frametime onto queue
	frametimes.push_back(thistime.QuadPart / freq);
	if (frametimes.size() > 100) frametimes.pop_front();
	if (frametimes.size() > 1)
		lastfps = (double)frametimes.size() / (frametimes.back() - frametimes.front());
	else
		lastfps = 0;
	if ((frametimes.back() - (temptime.QuadPart / freq)) >= 1.0) // 1 second: arbitrary update frequency
	{
		sprintf(fps_text, "%03.2f FPS", lastfps);
		QueryPerformanceCounter(&temptime);
	}
	lasttime = thistime;
	// End FPS

	if (spinval != 0){ cam.spinCamera(spinval); if (saveall) { saveimage = true; } }
	if (cam.getShifting()) { if (saveall) { saveimage = true; } }
	double ratio = (double)w / (double)h;	// feed the camera the current window aspect ratio
	cam.setAspect(ratio);					// to insure projection matrix is setup correctly
	if (followParticle != -1 && followParticle < primary->GetMaxParticles())
	{
		triple loc = primary->particles[followParticle]->GetPosition(frame);
		cam.setOffset(loc.x, loc.y, loc.z);
	}

	if (controller != NULL)
	{
		if (saveall && (saveimage || !stopped)) // hide the controller dialog if we're saving the image
		{
			latch_controller = true;
			controllershown = false;
			controller->ShowWindow(SW_HIDE);
		}
		else
		{
			if (latch_controller)
			{
				latch_controller = false;
				controllershown = true;
				controller->ShowWindow(SW_SHOW);
			}
			RedrawController();
		}
	}

	//if (depth_passes == 0)
	{
		if (!ParticleSet::useQueries)
			mainscene->DrawInstance(frame); // draw the scene
		else		
			mainscene->DrawOccludedInstance(frame);
	}
	/*else
	{
		for (int i = 0; i < depth_passes+1; i++)
		{
			glPolygonOffset(-1.0*i, -1.0*i);
			mainscene->DrawPeel(frame, w, h, i==0, i==(depth_passes));
		}
	}*/

	mainscene->SaveImage();

	#ifdef BENCHMARK
	SetMessage(primary->message.c_str());
	#endif
	
	DrawText(); // now add status text, console, etc.
	return true;
}
bool CChildView::IncrementFrame()
{
// decide if next frame should be drawn
	if (delay != 0)
	{
		if (((double)(thistime.QuadPart - lasttime.QuadPart) / freq) >= delay)
		{
			//QueryPerformanceCounter(&lasttime);
			if (!stopped)
			{
				if (runBackwards)
					frame--;
				else
					frame++;	// advance current frame

				if (saveall)
				{
					saveimage = true; // dump image to png
				}
			}
			if (frame>primary->GetMaxFrames()-1) 
			{
				//counting = false;
				if (runLoop)
					frame = 0;
				else
					frame = primary->GetMaxFrames()-1; // oops, we've gone too far
				if (saveall)
					saveimage = false; // stop saving the image after last frame
			}
			if (frame<0)
			{
				if (runLoop)
					frame = primary->GetMaxFrames()-1; // oops, we've gone back too far
				else
					frame = 0;
				if (saveall)
					saveimage = false; // don't repeat the first frame.
			}
		}
		else
		{
			if (!redraw)
			{
				RedrawController();
				return false;
			}
		}
	}
	else // if delay = 0 draw as fast as possible
	{
		if (!stopped)
		{
			if (runBackwards)
				frame--;
			else
				frame++;	// advance current frame
			if (saveall)
			{
				saveimage = true;
			}
		}
		if (frame>primary->GetMaxFrames()-1) // end of set
		{
			if (runLoop)
				frame = 0;
			else
				frame = primary->GetMaxFrames()-1; // oops, we've gone too far
			if (saveall)
				saveimage = false;
		}
		if (frame<0)
		{
			if (runLoop)
				frame = primary->GetMaxFrames()-1; // oops, we've gone back too far
			else
				frame = 0;
			if (saveall)
				saveimage = false; // don't repeat the first frame.
		}
	}
	return true;
}

void CChildView::DrawText()
{
	// draw frame, time, fps, whatever
	int dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);
	static float quadratic[3] = { 0, 1.0, 0 };
	static float off[3] = { 1.0, 0, 0 };
	if (time)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		char buf[64];
		//glColor3f(1, 1, 1);
		glColor3f(textColor.x, textColor.y, textColor.z);
		if (showTime)
		{
			double this_time = primary->GetTime(frame);
			sprintf(buf, "Frame %i, T=%g", frame, this_time);
			drawChars(buf, 0.02, 0.03, 0);	// GLWnd character drawing function
		}
		if (showFPS) // draw FPS
		{
			drawChars(fps_text, 0.9, 0.03, 0);
		}
		if (message != "") // draw last console message
			drawChars(message.c_str(), 0.02, 0.09, 0);
		if (spinval != 0) // add spin direction indicators "<" and ">"
		{
			int spinlen = min(abs(spinval), 5);
			string spintext;
			char direction;
			if (spinval < 0)			
				direction = '<';			
			else
				direction = '>';
			for (int i = 0; i < spinlen; i++)
				spintext+=direction;

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, dims[2], 0, dims[3], -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			myfont->DrawChars(spintext.c_str(), dims[2]/2, 25, 0, (spinval<0) ? ALIGNRIGHT : ALIGNLEFT);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
		}

		stringstream bobthebuffer;
		console->ClearMessages(); // clear console, prepare to add particle data
		vector<int>::iterator iter;
		for (iter = watch_list.begin(); iter != watch_list.end(); iter++)
		{
			int p = *iter;
			if (p >= 0 && p < primary->GetMaxParticles())
			{
				if (primary->particles[p]->IsMarked()==MARK_UNMARKED) // CRASH HERE
				{
					iter = watch_list.erase(iter);
					if (iter==watch_list.end()) break;
				}
				else
				{
					// format the console line in columns
					triple pos = primary->particles[p]->PeekVector(frame, POSVELOCITY);
					console->PushVector(pos, "Vel");
					pos = primary->particles[p]->GetPosition(frame);
					console->PushVector(pos, "Pos");
				}
			}
		}
		glDisable(GL_DEPTH_TEST);
        console->Draw();

		if (state == SELECTION && lastX >= 0) // draw a "drag box"
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, dims[2], 0, dims[3], -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glColor4f(Particle::markingColor.x, Particle::markingColor.y, Particle::markingColor.z, 0.2f);
			glBegin(GL_QUADS);
				glVertex3f((float)lastX, (float)(dims[3] - lastY), 0);
				glVertex3f((float)currentX, (float)(dims[3] - lastY), 0);
				glVertex3f((float)currentX, (float)(dims[3]- currentY), 0);
				glVertex3f((float)lastX, (float)(dims[3]- currentY), 0);
			glEnd();
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glDisable(GL_BLEND);
		}
	}
}

// Pass mouse coordinates to Camera
void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (dragging)
	{
		// handle "positioner" mode
		if (state == CAMPICK || state == CAMPPICK || state == LIGHT1PICK || state == LIGHT2PICK || state == SELECTION)
		{
			double R = cam.getRadius();
			double rotX = cam.getRotX();
			triple newpoint = lastPoint;
			if (lastButton == 0)
			{
				newpoint.x = lastPoint.x +	(cos(rotX+(3.14159265358979/2.0))*(lastX - point.x)* (1.5 / (double)w) * R) -
															(cos(rotX)*(lastY - point.y)* (1.5 / (double)w) * R);
				newpoint.z = lastPoint.z +	(sin(rotX+(3.14159265358979/2.0))*(lastX - point.x)* (1.5 / (double)w) * R) -
															(sin(rotX)*(lastY - point.y)* (1.5 / (double)w) * R);
			}
			else
			{
				newpoint.y = lastPoint.y + (((lastY - point.y)*1.5 / (double)w) * R);
			}
			switch (state)
			{
			case CAMPICK:
				camera = newpoint;
				break;
			case CAMPPICK:
				target = newpoint;
				break;
			case LIGHT1PICK:
				mainscene->SetLight(0, newpoint);
				break;
			case LIGHT2PICK:
				mainscene->SetLight(1, newpoint);
				break;
			case SELECTION:
				currentX = point.x;
				currentY = point.y;
				break;
			}
			if (state == LIGHT1PICK || state == LIGHT2PICK)
				camObj.UpdatePosition(newpoint);
			else
			{
				camObj.UpdatePosition(camera, target);
				if (drawCone)
				{
					primary->cone1 = camera;
					primary->cone2 = target;
					primary->redrawCone = true;
				}
			}
		}
		else
		{
			cam.setX(point.x);
			cam.setY(point.y);
			mx = point.x;
			my = point.y;
		}
		this->Invalidate(FALSE);
		redraw = true;
	}
}

// Modify camera radius (zoom) on mouse wheeling
BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	double r = cam.getRadius();
	if (zDelta > 0)
	{
		r = fabs(r / 1.5);
	}
	else
		r = fabs(r * 1.5);
	cam.setRadius(r);
	this->Invalidate(FALSE);
	redraw = true;
	return true;
}

// mouse button handlers

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (state == PARTPICK) // Pick a particle to be marked
	{
		wglMakeCurrent(pDC->m_hDC, hRC);
		//int M = primary->MarkParticle(point.x, point.y, frame, MARK_HIGHLIGHT_MODE);
		int M = mainscene->QueryScene(point.x, point.y, frame, MARK_HIGHLIGHT_MODE);
		SetState(NORMAL);
		if (M >= 0)
		{
			if (follow)
			{
				followParticle = M;
				follow = false;
				message = "TABLET CAM";
				SetTimer(3, 500, NULL);
				SetTimer(1, 3500, NULL);
			}
			else
			{
					stringstream msg;
					msg << "Particle number " << M << " selected.";
					SetMessage(msg.str().c_str());				
			}
			watch_list.push_back(M);
		}	
		return;
	}
	if (state == PARTHIDE) // Pick a particle to be hidden
	{
		wglMakeCurrent(pDC->m_hDC, hRC);
		int HideP = mainscene->QueryScene(point.x, point.y, frame, MARK_HIDE_MODE);
		SetState(NORMAL);
		if (HideP >= 0)
		{			
			stringstream message;
			message << "Particle number " << HideP << " hidden.";
			SetState(NORMAL);
			SetMessage(message.str().c_str());
		}
		return;
	}
	if (state == CAMPICK || state == CAMPPICK || state == LIGHT1PICK || state == LIGHT2PICK || state == SELECTION)
	{	// various dragging modes
		lastX = point.x;
		lastY = point.y;
		switch (state)
		{
		case CAMPICK:
			lastPoint = camera;
			break;
		case CAMPPICK:
			lastPoint = target;
			break;
		case LIGHT1PICK:
			//lastPoint = lights[0];
			lastPoint = mainscene->GetLightPos(0);
			break;
		case LIGHT2PICK:
			//lastPoint = lights[1];
			lastPoint = mainscene->GetLightPos(1);
			break;
		case SELECTION:
			currentX = lastX;
			currentY = lastY;
			break;
		}		
		lastButton = 0;
	}
	if (state == GRAPHCORNERA) // for positioning the graph
	{
		lastX = point.x;
		lastY = point.y;
		state = GRAPHCORNERB;
		dragging = true;
		return;
	}

	SetCapture();
	if (dragging)
	{
		cam.setMode(3, point.x, point.y);
	}
	else
	{
		dragging = true;
		cam.setMode(0, point.x, point.y);
	}
}

// Middle button: translate camera
void CChildView::OnMButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	dragging = true;
	cam.setMode(1, point.x, point.y);
}

// Right button: translate camera
void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (state == CAMPICK || state == CAMPPICK || state == LIGHT1PICK || state == LIGHT2PICK)
	{
		lastX = point.x;
		lastY = point.y;
		switch (state)
		{
		case CAMPICK:
			lastPoint = camera;
			break;
		case CAMPPICK:			
			lastPoint = target;
			break;
		case LIGHT1PICK:
			lastPoint = mainscene->GetLightPos(0);
			break;
		case LIGHT2PICK:
			lastPoint = mainscene->GetLightPos(1);
			break;
		}
		lastButton = 2;
	}
	SetCapture();
	if (dragging)
	{
		cam.setMode(3, point.x, point.y); // go into zoom mode
	}
	else
	{
		dragging = true;
		cam.setMode(2, point.x, point.y);
	}
}

// Double clicking instantly marks a particle
void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (state != PARTPICK)
	{
		wglMakeCurrent(pDC->m_hDC, hRC);
		int M = mainscene->QueryScene(point.x, point.y, frame, MARK_HIGHLIGHT_MODE);
		if (M >= 0)
		{
			stringstream msg;
			msg << "Particle number " << M << " selected.";
			SetMessage(msg.str().c_str());				
			watch_list.push_back(M);
		}	
	}
}

// Release drag capture for each button that is unclicked
void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (state==SELECTION)
	{
		state = NORMAL;
		mainscene->SetPick(lastX, lastY, point.x, point.y);		
		ParticleSet::useQueries = true;
		Particle::setup = false;
		lastX = lastY = -1;
		SetMessage("Region marked.");
	}
	if (state==GRAPHCORNERB) state = NORMAL;
	dragging = false;
	ReleaseCapture();
}
void CChildView::OnMButtonUp(UINT nFlags, CPoint point)
{
	dragging = false;
	ReleaseCapture();
}
void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	dragging = false;
	ReleaseCapture();
}

// Open up open-file dialog, load in data
void CChildView::OnFileLoaddatafile()
{
	char buffer[MAX_PATH];
	if (BrowseForFile("Open Input File",
							"All Files\0*.*\0.dat Files\0*.DAT\0Gzipped statefile\0*.GZ;*.GZIP\0Binary statefile\0*.dem\0",
							buffer))
	{
		string filename(buffer);
		LoadFile(filename);
	}
}

// Open up add-file dialog, add in data

void CChildView::OnFileAdddatafile()
{
	char buffer[MAX_PATH];
	if (BrowseForFile("Add Input File", "All Files\0*.*\0.dat Files\0*.DAT\0", buffer))
		LoadFile(buffer, true);
}

// Load data from filename (no dialog)
void CChildView::LoadFile(string file, bool add)
{
	//bool zipped = false;
	int filetype = 0;
	if (file.find(".gz") == (file.length() - 3))
		filetype = 1;
	else if (file.find(".dem") == (file.length() - 4))
		filetype = 2;

	const char * filename = file.c_str();

	if (loading != NULL)
	{
		SetMessage("File loading already in progress.  Please wait for process to complete.");
		return;
	}
	//loading = true;
	followParticle = -1;

	CWinApp* theApp = AfxGetApp();

	if (filetype==1) // GZipped
		loading = new PZipReader(&lock);
	else if (filetype==2) // Binary
		loading = new PBinaryReader(&lock);
	else // Standard ASCII Statefile
		loading = new PStatefileReader(&lock);

	QueryPerformanceCounter(&lasttime);
	if (add)
	{
		if (loading->AddFile(primary, filename))
		{
			QueryPerformanceCounter(&loadtime);
			frame = 0;
			redraw = true;
			Invalidate(TRUE);
			theApp->WriteProfileString("Last Config", "lastLoad", filename);
			theApp->AddToRecentFileList(filename);
			message = "Started additional file load.";
		}
		else
		{
			message = "File loading failed.";
		}
	}
	else
	{
		if (loading->LoadFile(primary, filename))
		{
			QueryPerformanceCounter(&loadtime);
			frame = 0;
			redraw = true;
			Invalidate(TRUE);
			theApp->WriteProfileString("Last Config", "lastLoad", filename);
			theApp->AddToRecentFileList(filename);
			message = "Started file loading.";
		}
		else
		{
			message = "File loading failed.";
		}
	}
	SetTimer(1, 10000, NULL);
}

// Timer handler
void CChildView::OnTimer(UINT nIDEvent)
{
	if (nIDEvent==1)
	{
		message = "";
		KillTimer(1);
		KillTimer(2);
		KillTimer(3);
	}
	if (nIDEvent==2)
	{
		message = "TABLET CAM";
		SetTimer(3, 500, NULL);
		KillTimer(2);
	}
	if (nIDEvent==3)
	{
		message = "";
		SetTimer(2, 500, NULL);
		KillTimer(3);
	}
}

// Various UI handlers

void CChildView::OnSetqualityLow()
{
	Particle::quality = 0;
	UpdateMenu();
}

void CChildView::OnSetqualityMedium()
{
	Particle::quality = 1;
	UpdateMenu();
}

void CChildView::OnSetqualityHigh()
{
	Particle::quality = 2;
	UpdateMenu();
}

void CChildView::OnSetqualityHighest()
{
	Particle::quality = 3;
	UpdateMenu();
}

void CChildView::OnSetgeometryqualityUltra()
{
	Particle::quality = 4;
	UpdateMenu();
}

void CChildView::OnParticlesTrackparticle()
{
	SetState(PARTPICK); // next mouse click will be passed to ParticleSet for selection
	message = "Pick particle for marking.";
	SetTimer(1, 1500, NULL);
}

void CChildView::OnViewUsespecularhighlights()
{
	primary->SetOption(R_SPECULAR, !(primary->GetOption(R_SPECULAR)));
	UpdateMenu();
}

// Keyboard handlers
void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (ui->KeyboardInput(nChar))
	{
		UpdateMenu();
		Particle::setup = false;
	}
}
void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (ui->KeyboardEndInput(nChar))
	{
		UpdateMenu();
		Particle::setup = false;
	}
}

// Display controller dialog
void CChildView::OnViewShowcontroldialog()
{
	if (controller!=NULL)
	{
		controller->ShowWindow(SW_SHOW);
	}
	else
	{
		controller = new PControlDialog(this, this->GetParent());
		controller->Create(PControlDialog::IDD);
		controller->ShowWindow(SW_SHOW);
	}
}

// Change current frame
void CChildView::ModifyCurrentFrame(int value)
{
	frame+=value;
	if (frame < 0) frame = 0;
	if (frame >= primary->GetMaxFrames()) frame = primary->GetMaxFrames()-1;
}

// Change current max-fps
void CChildView::SetSpeed(double fps)
{
	if (fps == 100) delay = 0;
	else delay = 1.0 / fps;
}

// Seek to frame by % of total frames
void CChildView::SeekToFrame(int tframe)
{
	frame = tframe;
	if (frame >= primary->GetMaxFrames())
		frame = primary->GetMaxFrames()-1;
}

// Unmark all particles
void CChildView::OnEditResetmarkedparticles()
{
	primary->UnMarkAll();
}

// Switch point sprites on or off
void CChildView::OnSetqualityUsepoints()
{
	primary->SetOption(R_USEPOINTS, !primary->GetOption(R_USEPOINTS));
	UpdateMenu();
}

void CChildView::OnViewToggleaxes()
{
	axes = !axes;
	UpdateMenu();
}

// Open scene dialog
void CChildView::OnParticlesSetviewingoptions()
{
	wglMakeCurrent(pDC->m_hDC, hRC);
	DialogScene sd;
	sd.DoModal(&cam, primary, this);
}

// Various UI toggles
void CChildView::OnViewDrawnormalvectors()
{
	Particle::normalVector = !(Particle::normalVector);
	UpdateMenu();
}

void CChildView::OnViewUsetextures()
{
	primary->SetOption(TEXTURING, !primary->GetOption(TEXTURING));
	UpdateMenu();
}

void CChildView::OnViewUselighting()
{
	primary->SetOption(LIGHTING, !primary->GetOption(LIGHTING));
	UpdateMenu();
}

void CChildView::OnViewTogglevelocityvectors()
{
	primary->SetOption(R_VELOCITY, !primary->GetOption(R_VELOCITY));
	UpdateMenu();
}

void CChildView::OnEditResetcamera()
{
	cam.setOffset(0, 0, 0);
	cam.setRotation(1, 1);
}

// set manual camera positions
void CChildView::OnSetcamerapositionX()
{
	cam.setOffset(0, 0, 0);
	cam.setViewDirection(X, lastfps);
}

void CChildView::OnSetcamerapositionY()
{
	cam.setOffset(0, 0, 0);
	cam.setViewDirection(Y, lastfps);
}

void CChildView::OnSetcamerapositionZ()
{
	cam.setOffset(0, 0, 0);
	cam.setViewDirection(Z, lastfps);
}

void CChildView::OnClose()
{
	GLWnd::OnClose();
}

// Open image-saver dialog
void CChildView::OnEditImagesavingoptions()
{
	bool ascii = pngOut.GetAscii();

	string fname = pngOut.GetFileName();
	int fnum = pngOut.GetFileNumber();

	ImageDialog id(&pngOut, &aviOut, saveall, savePNG, saveAVI, this);
    id.DoModal();
}

void CChildView::SaveImage()
{
	this->saveimage = true; // save next frame
}
void CChildView::OnParticlesSetviewingvolume()
{
	BoundsDialog bdialog(primary, primary->GetBounds(), this);
	bdialog.DoModal();
}

void CChildView::OnViewDrawparticles()
{
	primary->SetOption(R_PARTICLES, !primary->GetOption(R_PARTICLES));
	UpdateMenu();
}

void CChildView::OnViewDrawrotationalvelocityvectors()
{
	primary->SetOption(R_ROTVELOCITY, !primary->GetOption(R_ROTVELOCITY));
    UpdateMenu();
}

// Set a check in on the menu according to some input boolean value.  Sort of a binding between the two.
void CChildView::CheckMenuID(bool value, UINT menuID)
{
	if (value)
		themenu->CheckMenuItem(menuID, MF_CHECKED);
	else
		themenu->CheckMenuItem(menuID, MF_UNCHECKED);
}

// Update all menu check marks
void CChildView::UpdateMenu(void)
{
	CheckMenuID(primary->GetOption(R_VELOCITY), ID_VIEW_TOGGLEVELOCITYVECTORS);
	CheckMenuID(axes, ID_VIEW_AXES);
	CheckMenuID(primary->GetOption(R_PARTICLES), ID_VIEW_DRAWPARTICLES);
	CheckMenuID(primary->GetOption(R_USEPOINTS), 145);
	CheckMenuID(primary->GetOption(TEXTURING), ID_VIEW_USETEXTURES);
	CheckMenuID(primary->GetOption(LIGHTING), ID_VIEW_USELIGHTING);
	CheckMenuID(primary->GetOption(R_TRANSPARENT), ID_VIEW_BOUNDDRAW);
	CheckMenuID(primary->GetOption(R_ROTVELOCITY), ID_VIEW_DRAWROTATIONALVELOCITYVECTORS);
	CheckMenuID(Particle::normalVector, ID_VIEW_DRAWNORMALVECTORS);
	CheckMenuID(followParticle!=-1, ID_SETCAMERAPOSITION_TRACKPARTICLE);
	CheckMenuID(primary->GetOption(R_SPECULAR), ID_VIEW_USESPECULARHIGHLIGHTS);

	CheckMenuID(Particle::quality==0, ID_SETQUALITY_LOW);
	CheckMenuID(Particle::quality==1, ID_SETQUALITY_MEDIU);
	CheckMenuID(Particle::quality==2, ID_SETQUALITY_HIGH);
	CheckMenuID(Particle::quality==3, ID_SETQUALITY_HIGHEST);
	CheckMenuID(Particle::quality==4, ID_SETGEOMETRYQUALITY_ULTRA);
	CheckMenuID(primary->GetOption(R_SPHERESHADER), ID_SETGEOMETRYQUALITY_USESPHERESHADER);

	CheckMenuID(drawGraph, ID_GRAPH_ENABLEGRAPHDISPLAY);

	CheckMenuID(!primary->GetOption(R_TRANSPARENT), ID_SETTRANSPARENCY_NOTRANSPARENCY);
	CheckMenuID(primary->GetOption(R_TRANSPARENT) && !primary->GetOption(R_ALPHASORT), ID_SETTRANSPARENCY_ADDITIVEALPHABLENDING);
	CheckMenuID(primary->GetOption(R_TRANSPARENT) && primary->GetOption(R_ALPHASORT), ID_SETTRANSPARENCY_Z);

	CheckMenuID(suspended, ID_FILE_SUSPENDDRAWINGPROCESSES);
	int color_mode = primary->GetColorMode();
	CheckMenuID(color_mode == COLORWHITE, ID_SETCOLORSCHEME_DEFAULTWHITE);
	CheckMenuID(color_mode == COLORVEL, ID_SETCOLORSCHEME_MAPTOVELOCITY);
	CheckMenuID(color_mode == COLORRVEL, ID_SETCOLORSCHEME_MAPTOANGULARVELOCITY);

	redraw = true;
	Particle::setup = false;
}

// Toggle between perspective and orthogonal camera projections
void CChildView::OnSetcamerapositionToggleprojection()
{
	if (cam.getProjection()==0)
		cam.setProjection(1);
	else
		cam.setProjection(0);
}

void CChildView::OnHelpCmdline()
{
	AfxMessageBox(ui->OnHelpCmdline().c_str());
}

// Save preferences to registry
void CChildView::WriteScenePrefs()
{
	ui->WriteScenePrefs();
}

// Load preferences from registry
void CChildView::LoadScenePrefs()
{
	ui->ReadScenePrefs();
	Particle::setup = false;
}

// Keyboard shortcuts dialog
void CChildView::OnHelpKeyboardshortcuts()
{	
	string helppath = exePath + "help\\ShortcutKeys.pdf";
	ShellExecute(NULL, "open", helppath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// Redraws the controller toolbox
void CChildView::RedrawController()
{
	if (controller != NULL && controllershown) // redraw controller
	{
		controller->UpdateScrollBars();
		controller->UpdatePosition(frame); //(int)(100.0 * frame /(primary->GetMaxFrames()-1)) );
		controller->Invalidate(FALSE);
	}
}

void CChildView::OnSetcamerapositionTrackparticle()
{
	if (followParticle != -1)
	{
		followParticle = -1;
		message = "Camera released follow mode.";
		SetTimer(1, 1500, NULL);
		return;
	}
	else
	{
		follow = true;
		SetState(PARTPICK);
		message = "Pick particle for follow mode.";
		SetTimer(1, 1500, NULL);
	}
}

void CChildView::OnEditMarkparticletohide()
{
	state = PARTHIDE;
	SetMessage("Pick particle to hide.");
}

void CChildView::OnFileLoadcolor()
{
	char buffer[MAX_PATH];
	if (BrowseForFile("Open Colormap File", "All Files\0*.*\0.dat Files\0*.DAT\0", buffer))
	{
		PColormapReader loader;
		loader.LoadFile(primary, buffer);
		frame = 0;
		redraw = true;
		Invalidate(TRUE);
		Particle::setup = false;
	}	
}

void CChildView::OnFileLoadNonColorMap()
{
	char buffer[MAX_PATH];
	if (BrowseForFile("Open Colormap File", "All Files\0*.*\0.dat Files\0*.DAT\0", buffer))
	{
		PColormapReader loader;
		loader.SetPersistence(false);
		loader.LoadFile(primary, buffer);
		frame = 0;
		redraw = true;
		Invalidate(TRUE);
		Particle::setup = false;
	}
}

void CChildView::OnViewLoadnewtexturemap()
{	
	char fileName[MAX_PATH];
	if (BrowseForFile("Load New Texture", "Bitmaps\0*.bmp\0", fileName))
	{
		LoadTexture(fileName, &m_texName);
		glBindTexture(GL_TEXTURE_2D, m_texName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // clamp to edge
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
}

bool CChildView::BrowseForFile(const char * title, const char * filter, char * fileNamePtr)
{
	OPENFILENAME browse;
	string lastDir;
	fileNamePtr[0] = '\0';
	CWinApp* theApp = AfxGetApp();
	lastDir = theApp->GetProfileString("Last Config", "lastLoad");
	int trim = lastDir.rfind("\\");
	lastDir = lastDir.substr(0, trim);

	ZeroMemory(&browse, sizeof(browse));
	browse.lStructSize = sizeof(browse);
	browse.hwndOwner = this->m_hWnd;	
	browse.lpstrFile = fileNamePtr;
	browse.lpstrFilter = filter;
	browse.lpstrTitle = title;
	browse.nMaxFile = MAX_PATH;
	browse.nFilterIndex = 1;	
	browse.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	browse.lpstrInitialDir = lastDir.c_str();

	return (GetOpenFileName(&browse)!=0); // create the file dialog
}

void CChildView::OnEditSelectbackgroundcolor()
{
	CColorDialog cd;
	cd.DoModal();
	COLORREF CR = cd.GetColor();
	int r = GetRValue(CR);
	int g = GetGValue(CR);
	int b = GetBValue(CR);
	glClearColor(r / 255.0, g / 255.0, b / 255.0, 0.0);
}

// Set a message to be displayed at lower left for 6 seconds
void CChildView::SetMessage(const char *msg)
{
	message = string(msg);
	SetTimer(1, 6000, NULL);
}

// Option function for enabling and disabling various flags
void CChildView::SetOption(int ref, bool value)
{
	switch (ref)
	{
	case SHOWTIME:
		showTime = value;
		break;
	case SHOWFPS:
		showFPS = value;
		break;
	case RUNBACKWARDS:
		runBackwards = value;
		break;
	case RUNLOOP:
		runLoop = value;
		break;
	case SUSPEND:
		suspended = value;
		break;
	case SAVEPNG:
		savePNG = value;
		break;
	case SAVEAVI:
		saveAVI = value;
		break;
	case RECORD:
		saveall = value;
		break;
	}
}

// Option retrieval function
bool CChildView::GetOption(int ref)
{
	switch (ref)
	{
	case SHOWTIME:
		return showTime;
	case SHOWFPS:
		return showFPS;
	case RUNBACKWARDS:
		return runBackwards;
	case RUNLOOP:
		return runLoop;
	case SUSPEND:
		return suspended;
	case SAVEPNG:
		return savePNG;
	case SAVEAVI:
		return saveAVI;
	case RECORD:
		return saveall;
	}
	return false;
}

void CChildView::OnSetcolorschemeDefaultwhite()
{
	primary->SetColorMode(COLORWHITE);
	UpdateMenu();
}

void CChildView::OnSetcolorschemeMaptovelocity() // use velocity
{
	primary->SetColorMode(COLORVEL);
	UpdateMenu();
}

void CChildView::OnSetcolorschemeMaptoangularvelocity() // use angular velocity
{
	primary->SetColorMode(COLORRVEL);
	UpdateMenu();
}

void CChildView::OnSetcolorschemeMappositiontopos()
{
	primary->SetColorMode(COLORPOS);
	UpdateMenu();
}

void CChildView::OnEditPositioner()
{
	if (positioner==NULL)
	{
		positioner = new DialogPositioner(this, this);
		positioner->Create(DialogPositioner::IDD, this);
		positioner->ShowWindow(SW_SHOW);
		positioner->OnInitDialog();
	}
	else
	{
		positioner->ShowWindow(SW_SHOW);
		positioner->UpdateMode(state);
	}
}

// Set a specific client state (for mouse events)
void CChildView::SetState(int newstate)
{
	state = newstate;
	switch (state)
	{
	case NORMAL:
		followParticle = -1;
		break;
	case SELECTION:
		lastX = lastY = -1;
		break;
	case CAMPICK:
	case CAMPPICK:
		camObj.UpdatePosition(camera, target);
		camObj.SetColor( triple(0, 0, 1) );
		break;
	case LIGHT1PICK:
		camObj.UpdatePosition(mainscene->GetLightPos(0));
		camObj.SetColor( triple(0.1, 0.9, 0) );
		break;
	case LIGHT2PICK:
		camObj.UpdatePosition(mainscene->GetLightPos(1));
		camObj.SetColor( triple(0, 0.6, 0.2) );
		break;
	default:
		break;
	}
	if (positioner!=NULL)
		positioner->UpdateMode(state);

}

// Point camera at specific coordinates, disabling mouse manipulation
void CChildView::ForceCamera()
{
	cam.setManualPosition(camera.x, camera.y, camera.z, target.x, target.y, target.z);
	SetState(NORMAL);
	SetMessage("Pres Escape to revert camera to normal.");
}

// Set option to draw transparent cone
void CChildView::DrawCone(bool drawcone)
{
	drawCone = drawcone;
	if (drawCone)
	{
		primary->showCone = true;
		primary->cone1 = camera;
		primary->cone2 = target;
		primary->redrawCone = true;
	}
	else
		primary->showCone = false;
}

// An experimental sprite tuning dialog
void CChildView::OnViewAdjustpointsprites()
{
	DialogSpriteTuner* d;
	d = new DialogSpriteTuner(this);
	d->Create(DialogSpriteTuner::IDD);
}

// Parse XML geometry file
void CChildView::OnFileOpenparticledescriptor()
{
	if (parsing != NULL)
	{
		SetMessage("Parsing already in progress.  Please wait for process to complete.");
		return;
	}	
	char buffer[MAX_PATH];
	if (BrowseForFile("Open XML Particle Descriptor", "XML Files\0*.xml\0All Files\0*.*\0", buffer))
	{
		parsing = new PXMLReader(&lock);
		parsing->LoadFile(primary, buffer);
		redraw = true;
		Invalidate(TRUE);
	}
}

// Generates a single frame of randomly positioned particles, mainly for measuring performance
void CChildView::OnFileGeneratebenchmarkframe()
{
	if (loading == NULL)
	{
		DialogRandom dr(primary);
		dr.DoModal();
	}
	else
		SetMessage("Cannot generate frame: file loading already in progress.");
}

// Cut off the file thread or XML thread, retaining current data
void CChildView::OnTerminateFileProcesses()
{	
	if (loading != NULL)
	{
		loading->Terminate();
		SetMessage("File loading terminated.");
	}
	if (parsing != NULL)
	{
		parsing->Terminate();
		SetMessage("Parser file processes terminated.");
	}
}

void CChildView::OnSettransparencyNone()
{
	primary->SetOption(R_TRANSPARENT, false);
	UpdateMenu();
}

void CChildView::OnSettransparencyAdditive()
{
	primary->SetOption(R_TRANSPARENT, true);
	primary->SetOption(R_ALPHASORT, false);
	UpdateMenu();
}

void CChildView::OnSettransparencyZsort()
{
	primary->SetOption(R_TRANSPARENT, true);
	primary->SetOption(R_ALPHASORT, true);
	UpdateMenu();
}

void CChildView::OnSetgeometryqualityUsesphereshader() // toggle it
{
	primary->SetOption(R_SPHERESHADER, !primary->GetOption(R_SPHERESHADER));
	UpdateMenu();
}

void CChildView::OnSetcolorschemeClearcolormaps()
{
	primary->ClearColor(); Particle::setup = false;
}

void CChildView::OnEditCurrentGradient()
{
	DialogGradient dg(&(Particle::grad));
	dg.DoModal();
}

void CChildView::OnGradientSettospectrum()
{
	ui->LoadGradientPreset(UIProcessor::SPECTRUM); Particle::setup = false;
}

void CChildView::OnGradientSettogreyscale()
{
	ui->LoadGradientPreset(UIProcessor::GREYSCALE); Particle::setup = false;
}

void CChildView::OnGradientSettothermal()
{
	ui->LoadGradientPreset(UIProcessor::THERMAL); Particle::setup = false;
}

void CChildView::OnViewSetsurfacemapproperties()
{
	int scalemode;
	double scale;
	scalemode = Particle::GetSurfaceMapScale();
	scale = Particle::map_scale;

	DialogSurfaceMap dmap(scalemode, scale, this);
	dmap.DoModal();

	if (scale > 0) Particle::map_scale = scale;
	Particle::map_scale = scale;
	if (scalemode >= 0) Particle::SetSurfaceMapScale(scalemode);
}

void CChildView::OnEditCopyframedata()
{
	if (primary->GetMaxFrames() > 0)
	{
		if ( !OpenClipboard() )
		{
			AfxMessageBox( "Cannot open the Clipboard" );
			return;
		}
		// Remove the current Clipboard contents
		if( !EmptyClipboard() )
		{
			AfxMessageBox( "Cannot empty the Clipboard" );
			return;
		}

		stringstream output;
		primary->CopyFrameToString(frame, output);
		string joe = output.str();	  
		HANDLE textdata;
		textdata = GlobalAlloc(NULL, (SIZE_T)(joe.size() + 1));
		void * memory = GlobalLock(textdata);
		memcpy(memory, joe.c_str(), joe.size() + 1);

		if (::SetClipboardData(CF_TEXT, textdata) == NULL)
		{
			AfxMessageBox( "Unable to set Clipboard data" );
			CloseClipboard();
			return;
		}

		CloseClipboard();
	}
}

void CChildView::OnEditSetspraycone() // hax, must be reformatted
{
	double offset;
	DialogNumInput drumoffset("Spray Offset", offset, this);
	drumoffset.DoModal();

	double angle = 25.0 * PI / 180.0;
    double x = offset * cos(angle);
	double y = offset * sin(angle);

	camera = triple(-x,-y,0.0);
	target = triple(0.44464-x,-0.707107-y,0.0);
}
void CChildView::OnEditMarkparticlebyid()
{
	double f = 0;
	string title = "Particle ID";
	DialogNumInput num(title, f, this);
	num.DoModal();

	int pid = (int)f;
    if (pid >= 0 && pid < primary->GetMaxParticles())
	{
		primary->MarkParticle(pid, MARK_HIGHLIGHT_MODE);
		watch_list.push_back(pid);
	}
}

void CChildView::OnReferenceEnablevolumetricrendering()
{
	testslice.Enable(!testslice.Enabled());
}

void CChildView::OnReferenceVolumetricoptions()
{
	DialogSlices ds(&testslice, this);		
	ds.DoModal();
}

void CChildView::BackgroundRender(char * path, char * output, int width, int height, int fram, int buffermode, bool ascii, char* xml)
{
	BackgroundRenderer brender;
	brender.BGRender(this, path, output, width, height, fram, buffermode, ascii, xml);
}
void CChildView::OnFileOpenvectorfile()
{
	char buffer[MAX_PATH];
	if (BrowseForFile("Open Vector Input File",
							"All Files\0*.*\0.dat Files\0*.DAT\0",
							buffer))
	{
		PVectorReader loader;
		loader.LoadFile(primary, buffer);
	}
}

void CChildView::OnEditAuto()
{
	ui->AutoSceneSettings();
}

void CChildView::OnViewResizevectors()
{
	DialogVectorScaler vectorSizer(this);
	if (!stopped)
	{
		stopped = true;
		vectorSizer.DoModal();
		stopped = false;
	}
	else
		vectorSizer.DoModal();
}

void CChildView::OnViewRendertooffscreenbuffer()
{
	double x = 0, y = 0, AA = 0;
	DialogNumInput a("X Dimension", x, this);
	a.DoModal();
	DialogNumInput b("Y Dimension", y, this);
	b.DoModal();
	DialogNumInput c("Anti-Aliasing Level", AA, this);
	c.DoModal();
	if (x > 0 && y > 0)
	{
		if (!mainscene->RenderToImage(frame, (int)x, (int)y, (int)AA))
		{
			AfxMessageBox("Failed to render to image!");
		}
	}
}

void CChildView::OnSphereshadertypePhongshading() // "Phong" (Phong normals, Blinn-Phong lighting)
{
	primary->SetOption(R_SPHERESHADER, true);
	if (Particle::quality < 4)
		primary->SetShader("Fixed.vert", "Sphere2.frag");
	else
		primary->SetShader("Fixed.vert", "Sphere.high.frag");
	UpdateMenu();
}
void CChildView::OnSphereshadertypePhong() // Phong + Outlines
{
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("Fixed.vert", "Sphere.outlines.frag");
	UpdateMenu();
}
void CChildView::OnSphereshadertypeCartoon() // "Toon" mode, restrict intensity levels
{
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("Fixed.vert", "Sphere.toon.frag");
	UpdateMenu();
}

void CChildView::OnSphereshadertypeExperimental() // Radial shader, fast + good for blending
{	
	primary->SetOption(R_SPHERESHADER, true);
	if (primary->GetOption(LIGHTING))
		primary->SetShader("Fixed.vert", "Sphere.frag");
	else
		primary->SetShader("OneLight.vert", "SolidDisk.frag");
	UpdateMenu();
}
void CChildView::OnSphereshadertypeBasicshading() // Simplest shader (for speed)
{	
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("OneLight.vert", "OneLight.frag");
	UpdateMenu();
}
void CChildView::OnSphereshadertypePerspective() // Shader with perspective-correct intersection test
{
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("PerspFixed.vert", "PerspSphere.frag");
	UpdateMenu();

}

void CChildView::OnSphereshadertypeVelocityglyph() // Velocity glyph shader
{
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("Vector.vert", "Vector.frag", true);
	UpdateMenu();
}

void CChildView::OnSphereshadertypeVelocityglyph2()
{
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("VectorM.vert", "VectorM.frag", true);
	UpdateMenu();
}

void CChildView::OnSphereshadertypeMotionblur() // Enable options for the motion blur shader
{
	primary->SetOption(R_TRANSPARENT, true);
	primary->SetOption(R_ALPHASORT, true);
	primary->SetOption(R_SPHERESHADER, true);
	primary->SetShader("Blur.vert", "Blur.frag", true);
	UpdateMenu();
}

void CChildView::OnMenuVisibility()
{
	if (visibility==NULL)
	{
		visibility = new DialogVisibility(this, primary);
		visibility->Create(DialogVisibility::IDD, this);
		visibility->ShowWindow(SW_SHOW);
	}	
	else
		visibility->ShowWindow(SW_SHOW);
}

// Subgraph functions
void CChildView::OnGraphEnablegraphdisplay()
{
	drawGraph = !drawGraph;
	UpdateMenu();
}

void CChildView::OnGraphLoaddataintograph()
{
	char fileName[MAX_PATH];
	if (BrowseForFile("Load Graph Data", ".dat Files\0*.dat\0All Files\0*.*\0", fileName))
	{
		GraphView* sub = new GraphView();
		if (AddGraphView(sub, 250, 250))
			sub->LoadGraphData(fileName);
		UpdateMenu();
	}
}

void CChildView::OnGraphCreatevelocitylegend()
{
	GraphView* sub = new GraphView();
	if (AddGraphView(sub, 200, 450))
		sub->LoadGraphLegend(POSVELOCITY);	
}

void CChildView::OnGraphCreateangularvelocitylegend()
{
	GraphView* sub = new GraphView();
	if (AddGraphView(sub, 200, 450))
		sub->LoadGraphLegend(ROTVELOCITY);
}

void CChildView::OnReferenceCreateparticle()
{
	int pid = 0;
	if (watch_list.size() > 0)
		pid = watch_list.back();

	GraphView* newgraph = new GraphView(0.042);
	DialogAnalyze analyze(this, primary, pid, newgraph);
	if (AddGraphView(newgraph, 450, 350))
	{
		newgraph->ShowWindow(SW_HIDE);
		if (analyze.DoModal() == IDCANCEL)
			newgraph->Kill();
		else
			newgraph->ShowWindow(SW_SHOW);
	}		
}

bool CChildView::AddGraphView(GraphView* graph, int width, int height)
{
	CString csWndClass = AfxRegisterWndClass(CS_OWNDC, 0, 0, 0);
	CParticleVisApp* theapp = (CParticleVisApp*)AfxGetApp();
	if (graph->CreateEx(WS_EX_TOOLWINDOW, (LPCTSTR)csWndClass, "Graph Window", WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW, 50, 50, width, height, m_hWnd, NULL))
	{
		graph->SetWindowPos(this, 100, 100, width, height, NULL);
		theapp->subwindow_stack.push_back(graph);
		return true;
	}
	else return false;
}

void CChildView::OnDropFiles(HDROP hDropInfo)
{
	char filename[255];
	DragQueryFile(hDropInfo, 0, filename, 255);
	string file(filename);
	int strlen = file.length();
	if (file.substr(strlen - 3, 3).compare("xml") == 0)
	{
		if (parsing == NULL)
		{
			parsing = new PXMLReader(&lock);
			parsing->LoadFile(primary, filename);
		}
		else
			SetMessage("Already parsing an XML file.");
	}
	else
		LoadFile(filename);
	DragFinish(hDropInfo);
	redraw = true;
	Invalidate(TRUE);
	GLWnd::OnDropFiles(hDropInfo);
}

void CChildView::OnAnalysisMarkoutgrid()
{
	DialogSplit gridder(primary, true, this);
	gridder.DoModal();
	Particle::setup = false; // "dirty" flag
}
void CChildView::OnAnalysisApplycolorsplit()
{
	DialogSplit splitter(primary, false, this);
	splitter.DoModal();
	Particle::setup = false; // "dirty" flag
}

void CChildView::OnAnalysisLoadvolumefile()
{
	char buffer[MAX_PATH];
	if (BrowseForFile("Load Volumetric Data", ".raw Files\0*.raw\0All Files\0*.*\0", buffer))
		testslice.LoadVOLFile(string(buffer));
}
void CChildView::OnAnalysisCreatedensitymap()
{
	double data;
	DialogNumInput num("Enter cube's voxel dimension", data, this);
	num.DoModal();
	if (data >= 1 && data <= 1024)
		testslice.ComputeGranularTemp(primary->GetDataStore(), (int)data);
}

void CChildView::OnEditInterfacecoloroptions()
{
	DialogInterfaceColor colors(this);
	colors.DoModal();
	Particle::setup = false; // redraw particles
}
void CChildView::OnFileGeneratespheredescriptor()
{
	double radius = 0;
	DialogNumInput getradius("Input Sphere Radius", radius, this);
	getradius.DoModal();
	GeometryEngine &engine(*(primary->PRend));
	engine.NewList(0);
	engine.AddSphere(radius, triple(0, 0, 0));
	engine.EndList();
	engine.AssignLists(primary->particles, primary->GetMaxParticles());
	Particle::setup = false;
}

void CChildView::OnFileReportcurrentdata()
{
	string summary = ui->ProduceSummary();
	AfxMessageBox(summary.c_str(), MB_OK | MB_ICONINFORMATION);
}