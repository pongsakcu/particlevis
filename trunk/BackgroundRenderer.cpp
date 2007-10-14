#include "backgroundrenderer.h"
#include "PStatefileReader.h"
#include "PXMLReader.h"
#include "PNGWriter.h"
#include "ParticleSet.h"

BackgroundRenderer::BackgroundRenderer(void)
{
}

BackgroundRenderer::~BackgroundRenderer(void)
{
}

bool BackgroundRenderer::BGRender(CChildView *child, char * path, char * output, int width, int height, int fram, int buffermode, bool ascii, char * xml)
{
	WGLContext context(child->GetDC());	
	CDC* offDC;
	this->w = width;
	this->h = height;
	bool pbufferEXT;
	PNGWriter pngOut;

	if (buffermode > 0)
	{
		if (context.IsWglExtensionSupported("WGL_ARB_pbuffer") && context.IsWglExtensionSupported("WGL_ARB_pixel_format"))
		{
			/* Initialize WGL_ARB_pbuffer entry points. */
			/*INIT_ENTRY_POINT(wglCreatePbufferARB, PFNWGLCREATEPBUFFERARBPROC);
			INIT_ENTRY_POINT(wglGetPbufferDCARB, PFNWGLGETPBUFFERDCARBPROC );
			INIT_ENTRY_POINT(wglReleasePbufferDCARB, PFNWGLRELEASEPBUFFERDCARBPROC );
			INIT_ENTRY_POINT(wglDestroyPbufferARB, PFNWGLDESTROYPBUFFERARBPROC );
			INIT_ENTRY_POINT(wglQueryPbufferARB, PFNWGLQUERYPBUFFERARBPROC );
			INIT_ENTRY_POINT(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);*/
			pbufferEXT = true;
			context.CreatePBuffer(w, h);
		}
		else
		{
			fprintf(stderr, "PBuffers not supported!");
			pbufferEXT = false;
			context.Create();
		}
	}


	GLInit(context);
	unsigned int tex;
	glGenTextures(2, &tex);
	child->LoadTexture("band.bmp", &tex);

	PStatefileReader loader;
	loader.LoadFile(child->primary, path); // load up our input file
	while (!loader.IsFinished());	
		Sleep(100);
	
	if (output != NULL)
	{
		string strout(output);
		pngOut.SetFileName(strout); // set output filename
	}
	else
	{
		pngOut.SetFileName("temp");
	}
	pngOut.SetAscii(ascii);	// ascii file output

	child->time = false;

	ParticleSet * primary = child->primary;

	primary->SetOption(TEXTURING, false); // texturing
	primary->SetOption(LIGHTING, true); // lighting
	primary->SetOption(R_VELOCITY, true);
	primary->SetColorMode(COLORVEL);
	//primary->SetOption(R_SPECULAR, true);

	if (fram == 0)
		fram = primary->GetMaxFrames();
	if (fram > 1)
	{
		primary->SetOption(TEXTURING, true); // texturing
		primary->SetOption(LIGHTING, true); // lighting
	}

	// green light; now begin
	if (fram > 1)
		glEnable(GL_LIGHTING);

	primary->PRend = new GeometryEngine(child);
	if (xml != NULL)
	{
		PXMLReader xml_reader;
		xml_reader.LoadFile(primary, xml); // load up the xml config
	}
	while (!loader.IsFinished())
		Sleep(100);

	child->axes = false;
	glColor4d(1, 1, 1, 1);
	for (int i = 0; i < fram; i++)
	{
		//wglMakeCurrent(offDC->m_hDC, offscreen);
			
		if (i >= primary->GetMaxFrames())
			break;
		child->redraw = true;
		child->Draw();	// draws to buffer
		//SwapBuffers(pDC->m_hDC);
		glFlush();
		glFinish();
		pngOut.WriteImage(); // save image
		//cam.spinCamera(TWOPI / fram);
		child->frame++;
	}

	/*wglDeleteContext( offscreen );
	if (pbufferEXT)
	{
		wglReleasePbufferDCARB( pbuf, pbufDC );
		wglDestroyPbufferARB( pbuf );
	}*/

	AfxGetApp()->ExitInstance();
	exit(0);
	return false;
}

void BackgroundRenderer::GLInit(WGLContext & context)
{
	glViewport(0, 0, w, h);
	//glClearColor((GLclampf)0.2, (GLclampf)0.2, (GLclampf)0.2, 0);
	glClearColor((GLclampf)1.0, (GLclampf)1.0, (GLclampf)1.0, 0);
	glColor4d(1, 1, 1, 1);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_NORMALIZE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Lighting stuff

	static float black[] = {0, 0, 0, 1};
	static float white[] = {1, 1, 1, 1};
	static float pos1[4] = {2.0, 2.0, 10.0, 0.0};
	static float pos2[4] = {-60, 40, -20, 0.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	//glEnable(GL_NORMALIZE);
	if (context.IsExtensionSupported("EXT_rescale_normal"))
		glEnable(GL_RESCALE_NORMAL);
	else
		glEnable(GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
	glShadeModel(GL_SMOOTH);
	//glShadeModel(GL_FLAT);

	float superwhite[] = {1.5, 1.5, 1.5, 1.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, superwhite);

	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	float grey[] = {0.5, 0.5, 0.5, 1.0};

	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
	glLightfv(GL_LIGHT1, GL_AMBIENT, black);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, grey);
	glLightfv(GL_LIGHT1, GL_SPECULAR, black);

	//if (!pbufferEXT)
	//	glReadBuffer(GL_FRONT);

	// load data etc.

	//if (!wglMakeCurrent(offDC->m_hDC, offscreen))
	//	AfxMessageBox("GL Init failed.");

	//if (controller!=NULL)
	//	controller->ShowWindow(SW_HIDE);
	Particle::quality = 4;	// switch on precision rendering
	//axes = false;				// no axes

	//controllershown = false;
}