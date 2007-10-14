#include "UIProcessor.h"
#include "stdafx.h"
#include "ParticleVis.h"
#include "Camera.h"
#include "PStatefileReader.h"
#include "PXMLReader.h"
#include "PControlDialog.h"
#include <deque>
#include <math.h>
#include "MainFrm.h"
#include "GLPlanarSlices.h"
#include "DialogSlices.h"
#include "DialogGradient.h"

using namespace std;

extern int depth_passes;
extern double drumR;
extern CSemaphore lock;
extern GLPlanarSlices testslice;

#define SPINNER 0.0025

UIProcessor::UIProcessor(CChildView* mainwindow, ParticleSet* primary)
{
	this->mainwindow = mainwindow;
	this->primary = primary;
	this->primary_cam = &mainwindow->cam;
}

UIProcessor::~UIProcessor(void)
{
}


// Invokes batch mode when arguments are passed (input filename, output filename, width, height)
// This is huge and unwieldy, must cleanup
bool UIProcessor::ProcessArgs(char* argv)
{
	wglMakeCurrent(mainwindow->pDC->m_hDC, mainwindow->hRC);
	char tokens[] = " \t";
	char * reader = NULL, *path = NULL, *texture = NULL, *output = NULL, *xml = NULL;
	if (strlen(argv) < 2)
		return false;	// bail out if no args
	bool background = false, ascii = true, w_or_h = false;
	int tempW = 1024, tempH = 768;
	int fram = 0, buffermode = 0, colormode = 0;

	mainwindow->LoadScenePrefs();

	deque<string> arglist;
	reader = strtok(argv, tokens);
	while (reader != NULL)
	{
		arglist.push_back(string(reader));
		reader = strtok(NULL, tokens);
	}

	string parser, temp;
	do
	{
		parser = arglist.front();
		arglist.pop_front();
		if (parser == "-b")
		{
			background = true;
		}
		if (parser == "-bf")
		{
			background = true;
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			path = new char[temp.size()+1];
			strcpy(path, temp.c_str());
		}
		if (parser == "-f")
		{
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			path = new char[temp.size()+1];
			strcpy(path, temp.c_str());
		}
		if (parser == "-o")
		{
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			output = new char[temp.size()+1];
			strcpy(output, temp.c_str());
		}
		if (parser == "-tex")
		{
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			texture = new char[temp.size()+1];
			strcpy(texture, temp.c_str());			
		}
		if (parser == "-x")
		{
			w_or_h = true;
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			tempW = atoi(temp.c_str());
		}
		if (parser == "-y")
		{
			w_or_h = true;
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			tempH = atoi(temp.c_str());
		}
		if (parser == "-maxframes")
		{
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			fram = atoi(temp.c_str());
		}
		if (parser == "-pbuffer")
		{
			buffermode = 1;
		}
		if (parser == "-noascii")
		{
			ascii = false;
			mainwindow->pngOut.SetAscii(ascii);
		}
		if (parser == "-cam")
		{
			if (arglist.size() < 6) return false;
			triple from, to;
			from.x = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			from.y = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			from.z = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			to.x = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			to.y = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			to.z = strtod( arglist.front().c_str(), NULL); arglist.pop_front();

			mainwindow->cam.setManualPosition(from.x, from.y, from.z, to.x, to.y, to.z);
		}
		if (parser == "-cone")
		{
			if (arglist.size() < 8) return false;
			triple from, to;
			double angle, drum_r;

			from.x = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			from.y = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			from.z = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			to.x = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			to.y = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			to.z = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			angle = strtod( arglist.front().c_str(), NULL); arglist.pop_front();
			drum_r = strtod( arglist.front().c_str(), NULL); arglist.pop_front();

			mainwindow->camera = from;
			mainwindow->target = to;
			primary->coneangle = angle;
			drumR = drum_r;
			
			mainwindow->DrawCone(true);
		}
		if (parser == "-color")
		{
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			colormode = atoi(temp.c_str());
			if (colormode == 1) mainwindow->OnSetcolorschemeMaptovelocity();
			if (colormode == 2) mainwindow->OnSetcolorschemeMaptoangularvelocity();
		}
		if (parser == "-vector")
		{
			if (arglist.size() == 0) return false;
			int vec_type = atoi( arglist.front().c_str() ); arglist.pop_front();
			if (vec_type == 1)
				primary->SetOption(R_VELOCITY, !primary->GetOption(R_VELOCITY));
			if (vec_type == 2)
				primary->SetOption(R_ROTVELOCITY, !primary->GetOption(R_ROTVELOCITY));
			if (vec_type == 3)
				Particle::normalVector = !(Particle::normalVector);
		}
		if (parser == "-stereo")
		{
			mainwindow->stereoMode = true;
		}
		if (parser == "-?")
		{
			mainwindow->OnHelpCmdline();
		}
		if (parser == "-nolights")
		{
			primary->SetOption(LIGHTING, false);
		}
		if (parser == "-d")
		{
			if (arglist.size() == 0) return false;
			temp = arglist.front();	arglist.pop_front();
			xml = new char[temp.size()+1];
			strcpy(xml, temp.c_str());
		}
		if (parser == "-map")
		{
			/*map_mode = true;
			if (arglist.size() == 0) return false;
			strcpy(map_filename, arglist.front().c_str());
			arglist.pop_front();*/
		}
	} while (arglist.size() > 0);

	if (xml != NULL && !background)
	{
		PXMLReader* loader = new PXMLReader(&lock);
		loader->LoadFile(primary, xml);
	}

	if (texture != NULL)
	{
		UINT m_texName;
		mainwindow->LoadTexture(texture, &m_texName);
		glBindTexture(GL_TEXTURE_2D, m_texName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // clamp to edge
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		primary->SetOption(TEXTURING, true);
	}

	if (background && path != NULL)
	{
		mainwindow->BackgroundRender(path, output, tempW, tempH, fram, buffermode, ascii, xml);
	}
	else
	{
		if (output != NULL)
		{
			string strout(output);
			mainwindow->pngOut.SetFileName(strout); // set output filename
			mainwindow->pngOut.SetAscii(ascii);
			delete output;
		}

		if (path != NULL)
		{
			mainwindow->LoadFile(path); // load up our input file
			delete path;
		}
		if (w_or_h)
		{
			((CMainFrame*)AfxGetMainWnd())->SetWindowSize(tempW, tempH);
		}
	}
	return true;
}

// Display command line arguments popup
string UIProcessor::OnHelpCmdline()
{
	stringstream buf;
	buf << " Command-line arguments for ParticleVis: " << endl;
	buf << " -f filename: \tspecify input filename" << endl;
	buf << " -bf filename:\tspecify input filename for background rendering" <<endl;
	buf << " -d filename:\tspecify XML particle descriptor file" << endl;
	buf << " -tex filename:\tspecify a bitmap to load in for textures" << endl;
	buf << " -nolights    \tdisable lighting (on by default)" << endl;
	buf << " -o filename: \tspecify output filename for image dumps" << endl;
	buf << " -x n:        \tspecify width of render-window in pixels" << endl;
	buf << " -y n:        \tspecify height of render-window in pixels" << endl;
	buf << " -maxframes n:\tspecify max number of frames to render in background mode" << endl;
	buf << " -pbuffer:    \tuse pbuffers to render in background mode, allowing hardware acceleration" << endl;
	buf << " -noascii:    \tdisable dump of ASCII data in background mode" << endl;
	buf << " -cam xyz xyz:\tforce camera to point from xyz to xyz" << endl;
	buf << " -cone xyz xyz a d:\tadd translucent cone pointing from xyz to xyz, angle = a, drum-radius = d." << endl;
	buf << " -color n:    \tspecify false-color mode: 1 = translational velocity, 2 = angular velocity." << endl;
	buf << " -vector n:   \tswitch on vectors of type: 1 = translational, 2 = angular, 3 = normals" << endl;
	buf << "              \tyou may specify -vector multiple times." << endl;
	buf << " -stereo      \tenable stereo output" << endl;
	buf << " -4x          \tenable 4x multisampling" << endl;
	buf << " -8x          \tenable 8x multisampling" << endl;
	//buf << " -map filename\tload surface map with any state file that is loaded" << endl;
	buf << " -?:             \tdisplay this dialog" << endl;
	return buf.str();
}

// Accept keyboard input.  If settings change, return true.
bool UIProcessor::KeyboardInput(unsigned int nChar)
{
	stringstream msg;
	Camera &cam = (mainwindow->cam);
	double fps = mainwindow->lastfps;
	if (mainwindow->saveall) // since we're recording, force function viewchanges to be slower
		fps = 30.0;

	switch (nChar) // virtual keys
	{
	case VK_PRIOR: // Page Up
		mainwindow->spinval++;
		return false;
	case VK_NEXT: // Page Down
		mainwindow->spinval--;
		return false;
	case VK_HOME: // Home
		mainwindow->spinval = 0;
		return false;
	case VK_F2: // F2: X axis
		cam.setViewDirection(X, fps);
		return false;
	case VK_F3: // F3: Y axis
		cam.setViewDirection(Y, fps);
		return false;
	case VK_F4: // F4: Z axis
		cam.setViewDirection(Z, fps);
		return false;
	case VK_F5:
		primary->SetOption(TEXTURING, !primary->GetOption(TEXTURING));
		return true;
	case VK_LEFT: // left arrow
		cam.StartPan(-2.0 * PI / 180.0);
		return false;
	case VK_RIGHT: // right arrow
		cam.StartPan(2.0 * PI / 180.0);
		return false;
	case VK_UP: // up arrow
		cam.StartTilt(2.0 * PI / 180.0);
		return false;
	case VK_DOWN: // down arrow
		cam.StartTilt(-2.0 * PI / 180.0);
		return false;
	case VK_OEM_PLUS: // +
	case VK_ADD: // KP+
		cam.StartZoom(0.98);
		return false;
	case VK_OEM_MINUS: // -
	case VK_SUBTRACT: // KP-
		cam.StartZoom(1.02);
		return false;
	case VK_OEM_1:
		mainwindow->SetState(SELECTION); // visibility selector
		mainwindow->SetMessage("Select region for marking.");
		return false;
	}
	
	if (isupper(nChar))
		nChar = tolower(nChar);
	switch (nChar) // standard keys
	{
	case 'p':
		mainwindow->OnParticlesTrackparticle();
		break;
	case 'f':
		mainwindow->OnSetcamerapositionTrackparticle();
		break;
	case 'a':
		mainwindow->OnEditAuto();
		break;
	case 's':
		mainwindow->saveimage = true;
		break;
	case 'z':
		mainwindow->controller->OnBnClickedButtonfirst();
		break;
	case 'x':
		mainwindow->controller->OnBnClickedButtonprev();
		break;
	case 'c':
		mainwindow->controller->OnBnClickedButtonstart();
		break;
	case 'v':
		mainwindow->controller->OnBnClickedButtonfwd();
		break;
	case 'b':
		mainwindow->controller->OnBnClickedButtonlast();
		break;
	case '1':
		Particle::quality = 0;
		break;
	case '2':
		Particle::quality = 1;
		break;
	case '3':
		Particle::quality = 2;
		break;
	case '4':
		Particle::quality = 3;
		break;
	case '5':
		Particle::quality = 4;
		break;
	case '6':
		primary->SetOption(R_SPHERESHADER, !primary->GetOption(R_SPHERESHADER));
		break;
	case '7':
		{
			DialogSlices ds(&testslice, mainwindow);
			if (ds.DoModal()==IDOK)
				testslice.RenderTextures();
		}
		break;
	case '0':
		primary->AddGrid(0.1, 0.1);
		break;
	case '9':
		primary->AddSplit();
		break;
	case '8':
		primary->ClearColor();
		testslice.SetBlending( (testslice.GetBlending() + 1) % 2 );
		break;
	case 219: // [
		//cam.setSeparation(fabs(cam.getSeparation() - 0.0005));
		//msg << "Eye-Sep " << cam.getSeparation();
		//SetMessage(msg.str().c_str());
		/*if (stereo==NULL)
		{
			stereo = new DialogStereo(cam, this);
			stereo->Create(DialogStereo::IDD, this);
		}
		stereo->ShowWindow(SW_SHOW);*/
		if (depth_passes > 0)
			depth_passes--;
		break;
	case 221: // ]
		//cam.setSeparation(cam.getSeparation() + 0.0005);
		//msg << "Eye-Sep " << cam.getSeparation();
		//SetMessage(msg.str().c_str());
		//if (stereo!=NULL)
		//	stereo->ShowWindow(SW_HIDE);
		depth_passes++;
		break;
	case 'l':
		primary->SetOption(LIGHTING, !primary->GetOption(LIGHTING));
		break;
	//case 116:
	case 't':
		primary->SetOption(TEXTURING, !primary->GetOption(TEXTURING));
		break;
	case 27: // Escape
		((CMainFrame*)AfxGetMainWnd())->Restore();
		cam.setInput(true);
		mainwindow->SetState(NORMAL);
		if (mainwindow->GetOption(RECORD) && !mainwindow->IsStopped()) // is we're actively recording, stop it
			mainwindow->StartStop();
		break;	
	case 190: // .
		ParticleSet::useQueries = !ParticleSet::useQueries;
		break;
	case 188: // ,
		mainwindow->OnTerminateFileProcesses();
		break;
	case 191: // '/'		
		primary->InvertSelection();
		break;
	case 'h':
		mainwindow->state = PARTHIDE;
		break;
	case 'o':
		mainwindow->DrawCone(!mainwindow->drawCone);
		break;
	case 'g':
		Particle::SetSurfaceMapScale((Particle::GetSurfaceMapScale() + 1) % 3);
		if (Particle::GetSurfaceMapScale()==LOG){mainwindow->SetMessage("Log Scale.");}
		if (Particle::GetSurfaceMapScale()==LINEAR){mainwindow->SetMessage("Linear Scale.");}
		if (Particle::GetSurfaceMapScale()==QUAD){mainwindow->SetMessage("Quadratic Scale.");}
		break;
	case 'e':
		{
			DialogGradient dg(&(Particle::grad));
			dg.DoModal();
		}
		break;
	case 'i':
		mainwindow->OnEditMarkparticlebyid();
		break;
	case 'u':
		primary->UnMarkAll();
		break;
	}
	return true;
}

bool UIProcessor::KeyboardEndInput(unsigned int nChar)
{
	switch (nChar)
	{
	case VK_LEFT: // left arrow
	case VK_RIGHT: // right arrow
	case VK_UP: // up arrow
	case VK_DOWN: // down arrow
	case VK_OEM_PLUS: // +
	case VK_ADD: // KP+
	case VK_OEM_MINUS: // -
	case VK_SUBTRACT: // KP-
		mainwindow->cam.StopMotion();
		break;
	}
	return false;
}

void UIProcessor::LoadGradientPreset(int preset)
{
	Particle::grad.Clear();
	switch (preset)
	{
	case SPECTRUM:
		Particle::grad.AddControlPoint(0, triple(0, 0, 1));
		Particle::grad.AddControlPoint(0.33, triple(0, 1, 0));
		Particle::grad.AddControlPoint(0.66, triple(1, 1, 0));
		Particle::grad.AddControlPoint(1.0, triple(1, 0, 0));
		break;
	case GREYSCALE:
		Particle::grad.AddControlPoint(0, triple(0, 0, 0));
		Particle::grad.AddControlPoint(1, triple(1, 1, 1));	
		break;
	case THERMAL:
		Particle::grad.AddControlPoint(0, triple(0, 0, 0));
		Particle::grad.AddControlPoint(0.33, triple(1, 0, 0));
		Particle::grad.AddControlPoint(0.66, triple(1, 1, 0));
		Particle::grad.AddControlPoint(1.0, triple(1, 1, 1));
		break;
	case SPRAY1:
		Particle::grad.AddControlPoint(0, triple(0, 0, 1));
		Particle::grad.AddControlPoint(0.15, triple(0, 1, 0));
		Particle::grad.AddControlPoint(0.3, triple(1, 0, 0));
		Particle::grad.CalculateGradient(2048);
		break;
	case SPRAY2:
		Particle::grad.AddControlPoint(0, triple(0, 0, 0));
		Particle::grad.AddControlPoint(0.25, triple(0, 0, 1));
		Particle::grad.AddControlPoint(0.5, triple(0, 1, 0));
		Particle::grad.AddControlPoint(0.75, triple(1, 0, 0));
		Particle::grad.AddControlPoint(1.0, triple(1, 1, 1));		
		break;
	}
	Particle::grad.CalculateGradient(2048);
	Particle::setup = false; // "dirty" flag
}

void UIProcessor::ReadScenePrefs()
{
	double double_data, double_data_2;
	int int_data;
	bool bool_data;

	if (GetRegistryData("minZ", double_data) && GetRegistryData("maxZ", double_data_2))
		primary_cam->setZBuffer(double_data, double_data_2);
	if (GetRegistryData("angle", double_data))
		primary_cam->setAngle(double_data);
	if (GetRegistryData("distance", double_data))
		primary_cam->setRadius(double_data);

	if (GetRegistryData("maxVelocity", double_data))
		Particle::maxVelocityMap = double_data;
	if (GetRegistryData("maxRVelocity", double_data))
		Particle::maxRVelocityMap = double_data;
	if (GetRegistryData("maxFieldVector", double_data))
		Particle::maxFieldVectorMap = double_data;

	if (GetRegistryData("transVelocityLength", double_data))
		Particle::transVelocityLength = double_data;
	if (GetRegistryData("rotVelocityLength", double_data))
		Particle::rotVelocityLength = double_data;
	if (GetRegistryData("fieldVelocityLength", double_data))
		Particle::fieldVelocityLength = double_data;
	if (GetRegistryData("pathLength", int_data))
		Particle::trailLength = int_data;	

	if (GetRegistryData("flipAxis", bool_data))
		primary_cam->setAxisFlip(bool_data);
	if (GetRegistryData("projection", int_data))
		primary_cam->setProjection(int_data);
	if (GetRegistryData("showFPS", bool_data))	
		mainwindow->showFPS = bool_data;

	if (GetRegistryData("lineWidth", double_data))
		Particle::lineWidth = double_data;
	if (GetRegistryData("pointSize", double_data))
		Particle::pointSize = double_data;

	// Lights!
	{
		bool light_enable, l_point;
		triple light;
		double light_bright, lightX, lightY, lightZ;
		if (GetRegistryData("light1", light_enable))
		{
			GetRegistryData("light1X", lightX);
			GetRegistryData("light1Y", lightY);
			GetRegistryData("light1Z", lightZ);
			GetRegistryData("light1_bright", light_bright);	
			GetRegistryData("light1_point", l_point);
			mainwindow->Scene()->SetLight(0, light_enable, triple(lightX, lightY, lightZ), light_bright, l_point);
		}
		if (GetRegistryData("light2", light_enable))
		{
			GetRegistryData("light2X", lightX);
			GetRegistryData("light2Y", lightY);
			GetRegistryData("light2Z", lightZ);
			GetRegistryData("light2_bright", light_bright);	
			GetRegistryData("light2_point", l_point);
			mainwindow->Scene()->SetLight(1, light_enable, triple(lightX, lightY, lightZ), light_bright, l_point);
		}
	}
	// Colors
	triple triple_data;
	//if (GetRegistryData("useMarkColor", bool_data)) // loading the use-mark option disabled
	//	Particle::useMarkColor = bool_data;
	if (GetRegistryData("markColor", triple_data))
		Particle::markingColor = triple_data;
	if (GetRegistryData("pathColor1", triple_data))
		Particle::pathColor1 = triple_data;
	if (GetRegistryData("pathColor2", triple_data))
		Particle::pathColor2 = triple_data;
}

bool UIProcessor::GetRegistryData(string key, double& value)
{
	UINT n;
	double *data;
	if (AfxGetApp()->GetProfileBinary("Last Config", key.c_str(), (LPBYTE*)&data, &n)!=0)
	{
		value = *data;
		delete[] data;
		return true;
	}
	delete[] data;
	return false;
}

bool UIProcessor::GetRegistryData(string key, triple& value)
{
	UINT n;
	triple *data;
	if (AfxGetApp()->GetProfileBinary("Last Config", key.c_str(), (LPBYTE*)&data, &n)!=0)
	{
		value = *data;
		delete[] data;
		return true;
	}
	delete[] data;
	return false;
}

bool UIProcessor::GetRegistryData(string key, int& value)
{
	UINT n;
	int *data;
	if (AfxGetApp()->GetProfileBinary("Last Config", key.c_str(), (LPBYTE*)&data, &n)!=0)
	{
		value = *data;
		delete[] data;
		return true;
	}
	delete[] data;
	return false;
}

bool UIProcessor::GetRegistryData(string key, bool& value)
{
	UINT n;
	bool *data;
	if (AfxGetApp()->GetProfileBinary("Last Config", key.c_str(), (LPBYTE*)&data, &n)!=0)
	{
		value = *data;
		delete[] data;
		return true;
	}
	delete[] data;
	return false;
}

bool UIProcessor::WriteRegistryData(string key, bool value)
{
	CWinApp* theApp = AfxGetApp();
	return theApp->WriteProfileBinary("Last Config", key.c_str(), (LPBYTE)&value, sizeof(bool));	
}

bool UIProcessor::WriteRegistryData(string key, int value)
{
	CWinApp* theApp = AfxGetApp();
	return theApp->WriteProfileBinary("Last Config", key.c_str(), (LPBYTE)&value, sizeof(int));
}

bool UIProcessor::WriteRegistryData(string key, double value)
{
	CWinApp* theApp = AfxGetApp();
	return theApp->WriteProfileBinary("Last Config", key.c_str(), (LPBYTE)&value, sizeof(double));
}

bool UIProcessor::WriteRegistryData(string key, triple value)
{	
	CWinApp* theApp = AfxGetApp();
	return theApp->WriteProfileBinary("Last Config", key.c_str(), (LPBYTE)&value, sizeof(triple));
}


void UIProcessor::WriteScenePrefs()
{
	double angle = primary_cam->getAngle();
	double minZ = primary_cam->getZNear();
	double maxZ = primary_cam->getZFar();
	double distance = primary_cam->getRadius();
	double maxvelocity = Particle::maxVelocityMap;
	double maxRvelocity = Particle::maxRVelocityMap;
	double maxFvector = Particle::maxFieldVectorMap;
	double maxlength = Particle::transVelocityLength;
	double maxRlength = Particle::rotVelocityLength;
	double maxFlength = Particle::fieldVelocityLength;
	int pathlength = Particle::trailLength;
	bool flipaxis = primary_cam->getAxisFlip();
	bool showFPS = mainwindow->showFPS;
	int projection = primary_cam->getProjection();

	bool light1_enable, light2_enable;
	bool l1_point, l2_point;
	triple light1, light2;
	double light1_bright, light2_bright;
	double light1X, light1Y, light1Z;
	double light2X, light2Y, light2Z;	
	mainwindow->Scene()->GetLight(0, light1_enable, light1, light1_bright, l1_point);
	mainwindow->Scene()->GetLight(1, light2_enable, light2, light2_bright, l2_point);
	light1X = light1.x; light1Y = light1.y; light1Z = light1.z;
	light2X = light2.x; light2Y = light2.y; light2Z = light2.z;

	CWinApp* theApp = AfxGetApp();
	WriteRegistryData("angle", angle);
	WriteRegistryData("minZ", minZ);
	WriteRegistryData("maxZ", maxZ);
	WriteRegistryData("distance", distance);

	WriteRegistryData("maxVelocity", maxvelocity);
	WriteRegistryData("maxRVelocity", maxRvelocity);
	WriteRegistryData("maxFieldVector", maxFvector);

	WriteRegistryData("transVelocityLength", maxlength);
	WriteRegistryData("rotVelocityLength", maxRlength);
	WriteRegistryData("fieldVelocityLength", maxFlength);
	WriteRegistryData("pathLength", pathlength);

	WriteRegistryData("flipAxis", flipaxis);
	WriteRegistryData("showFPS", showFPS);
	WriteRegistryData("projection", projection);

	WriteRegistryData("light1", light1_enable);
	WriteRegistryData("light1X", light1X);
	WriteRegistryData("light1Y", light1Y);
	WriteRegistryData("light1Z", light1Z);
	WriteRegistryData("light1_bright", light1_bright);	
	WriteRegistryData("light1_point", l1_point);

	WriteRegistryData("light2", light2_enable);
	WriteRegistryData("light2X", light2X);
	WriteRegistryData("light2Y", light2Y);
	WriteRegistryData("light2Z", light2Z);	
	WriteRegistryData("light2_bright", light2_bright);
	WriteRegistryData("light2_point", l2_point);

	WriteRegistryData("lineWidth", Particle::lineWidth);
	WriteRegistryData("pointSize", Particle::pointSize);
	WriteRegistryData("useMarkColor", Particle::useMarkColor);
	WriteRegistryData("markColor", Particle::markingColor);
	WriteRegistryData("pathColor1", Particle::pathColor1);
	WriteRegistryData("pathColor2", Particle::pathColor2);
}

void UIProcessor::AutoSceneSettings()
{
	double max_pos = Particle::GetMaxPosition();
	double max_trans = Particle::GetMaxVelocity();
	double max_rot = Particle::GetMaxRVelocity();
	double max_field = Particle::GetMaxFieldVelocity();
	if (max_pos > 0)
	{
		primary_cam->setRadius(max_pos * 3.0);
		primary_cam->setZBuffer(max_pos / 100.0, max_pos * 50.0);
		
		Particle::maxVelocityMap = max_trans; // maximum velocity, for scaling
		Particle::transVelocityLength = max_pos / 10.0 / max_trans; // maximum length of vectors
	}

	if (max_rot > 0)
	{
		Particle::maxRVelocityMap = max_rot; // same as above for rotation		
		Particle::rotVelocityLength = max_pos / 10.0 / max_rot; // maximum length of vectors
	}
	
	if (max_field > 0 && max_pos > 0)
	{
		Particle::maxFieldVectorMap = max_field; // maximum field vector length		
		Particle::fieldVelocityLength = max_pos / 10.0 / max_field; // maximum length of vectors	
	}
	Particle::setup = false; // "dirty" flag
}

// Print out description of current ParticleSet
string UIProcessor::ProduceSummary()
{
	char buffer[255];
	string current("");
	sprintf(buffer, "%d particles\n", primary->GetMaxParticles());
	current.append(buffer);

	sprintf(buffer, "%d frames\n", primary->GetMaxFrames());
	current.append(buffer);

	sprintf(buffer, "Max position: %03g\n", Particle::GetMaxPosition());
	current.append(buffer);

	sprintf(buffer, "Max trans. velocity mag: %03g\n", Particle::GetMaxVelocity());
	current.append(buffer);

	sprintf(buffer, "Max rot. velocity mag: %03g\n", Particle::GetMaxRVelocity());
	current.append(buffer);	

	int vecs = primary->vectorSet->GetFrameCount();
	if (vecs > 0)
	{
		sprintf(buffer, "%d frames of vector data loaded.", vecs);
		current.append(buffer);
	}
	return current;
}