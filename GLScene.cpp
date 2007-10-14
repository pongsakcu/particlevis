#include "GLScene.h"
#include "GLPlanarSlices.h"
#include "GLOcclusionQuery.h"
#include <math.h>

// HAX
extern GLPlanarSlices testslice; 

unsigned char* genSphereTex()
{
	// return RGBA texture
	const int size = 512;
	unsigned char * texture = (unsigned char*)(new unsigned char[size*size*4]);	
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			int index = ((i*size) + j) * 4;
			triple v;
			//v.x = 2.0*(((double)i / (double)(size-1.0)) - 0.5);
			//v.y = 2.0*(((double)j / (double)(size-1.0)) - 0.5);
			v.y = -2.0*(((double)i / (double)(size-1.0)) - 0.5);
			v.x = 2.0*(((double)j / (double)(size-1.0)) - 0.5);
			double len = sqrt(v.x*v.x + v.y*v.y);
			v.z = sqrt(1.0 - len*len);
			normalize(v);

			//if (len < 0.33)
			//	v = triple(0, 0, 1.0);
		
			texture[index + 0] = (unsigned char)(128.0*v.x + 127.0);
			texture[index + 1] = (unsigned char)(128.0*v.y + 127.0);
			texture[index + 2] = (unsigned char)(128.0*v.z + 127.0);
			texture[index + 3] = (len > 1.0) ? 0 : 255;
		}
	}
	return texture;
}

GLScene::GLScene(CChildView* view, Camera * cam, ParticleSet * primary) :
view(view),
cam(cam),
primary(primary),
ambience(0.05f),
shader_pass_velocity(false),
picking(false),
axes_scale(0.5),
axes_resize(true)
{
	/*const char * peel_source =
	"	uniform sampler2D tex; \n\
		void main (void)		 	\n\
		{							\n\
			vec4 color, depth;		\n\
			vec2 coord;				\n\
			coord.x = gl_FragCoord.x / 1148.0;	\n\
			coord.y = gl_FragCoord.y / 819.0;	\n\
			depth = texture2D(tex, coord);	\n\
			if (coord.x >= 0.5) \n\
				gl_FragColor = depth; \n\
			else							\n\
				gl_FragColor.x = gl_FragColor.y = gl_FragColor.z  = gl_FragCoord.z / gl_FragCoord.w; \n\
}						\n\
	";*/
	lights[0] = triple(.25, .25, .1);
	lights[1] = triple(-.3, -.2, .075);
	useLight[0] = useLight[1] = true;
	lightB[0] = 1.0;
	lightB[1] = 0.5;

	const char * sphere_source =
	"   uniform sampler2D tex;			\n\
		void main (void)			 	\n\
		{								\n\
			vec4 t = texture2D(tex, gl_TexCoord[0].st);	\n\
			t = (t - 0.5) * 2.0;	\n\
			if (t.a <= 0.1)				\n\
				discard;				\n\
			vec3 light = vec3(0, 0, 1.0) - gl_FragCoord.xyz; \n\
			light = normalize(light); \n\
			vec3 normal = normalize(t.xyz); \n\
			float color = max(0.0, dot(normal, light)); \n\
			gl_FragColor = vec4(color, color, color, 1.0);	\n\
		} \n";

	const char * peel_source =
	"   #extension ARB_texture_rectangle: enable \n\
		uniform sampler2DRectShadow tex;	\n\
		void main (void)			 	\n\
		{								\n\
			float depth = shadow2DRect(tex, gl_FragCoord.xyz).r - 0.5;	\n\
			if (depth > 0.0)		\n\
				discard;				\n\
			gl_FragColor = gl_Color;	\n\
		} \n";

	/*
	char _fp_peel[] = 
        "!!ARBfp1.0\n"
        "OPTION ARB_fragment_program_shadow;\n"
        "TEMP R0;\n"
        "TEX R0.x, fragment.position, texture[0], SHADOWRECT;\n"
        "ADD R0.x, R0.x, -0.5;\n"
        "KIL R0.x;\n"                       
		"MOV result.color, fragment.color;\n"
        "END\n";
	glGenProgramsARB(1, &peel_program);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, peel_program);
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(_fp_peel), _fp_peel);*/

	char log[1024];	
	peel_shader.LoadFragSource(peel_source);
	
	//blur_shader.LoadFragFile("Blur.frag");
	//blur_shader.LoadVertFile("Blur.vert");
	blur_shader.LoadFragFile("Vector.frag");
	blur_shader.LoadVertFile("Vector.vert");

	tex_width = tex_height = 512;	
	frontTex = NULL;
	//frontTex->Initialize(true, true, true, true, true, 8, 8, 8, 8);
	backTex = NULL;
	//backTex->Initialize(true, true, true, true, true, 8, 8, 8, 8);


	unsigned char * sphere_texture = genSphereTex();
	glGenTextures(1, &sphereTex);
	glBindTexture(GL_TEXTURE_2D, sphereTex);
	//	Set up all the unpack options for single byte packing
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    // Use glu to load the texture + mipmaps
	gluBuild2DMipmaps( GL_TEXTURE_2D, 4, 512, 512, GL_RGBA,
						GL_UNSIGNED_BYTE, sphere_texture);
	delete[] sphere_texture;
}

GLScene::~GLScene(void)
{
}

// Initialize state
void GLScene::GLInit()
{
	glClearColor((GLclampf)0.2, (GLclampf)0.2, (GLclampf)0.2, (GLclampf)0); // 20% grey default background
	glEnable(GL_DEPTH_TEST);

	// set up proper matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set texture options
	char * temp = (char*)glGetString(GL_VERSION);
	double version = strtod(temp, NULL);
	if (version >= 1.2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // use clamp to edge
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);

		//if (version >= 1.3)
		if (GLEW_ARB_multisample)
		{
			glEnable(GL_MULTISAMPLE);
		}
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // otherwise just clamp
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	// Enable anisotropic filtering.  This shouldn't really impact performance on newer cards
	float largest_supported_anisotropy = 8;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);

	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_NORMALIZE);
	glPointSize(Particle::pointSize);
	//glPointSize(10.0);
	glLineWidth(1.0);

	// Lights

	//triple lights[2];
	//lights[0] = view->lights[0];
	//lights[1] = view->lights[1];
	//float L1W = view->light_points[0] ? 

	static float black[] = {0, 0, 0, 1};
	static float white[] = {1, 1, 1, 1};
	static float pos1[4] = {lights[0].x, lights[0].y, lights[0].z, 1.0}; // changed to point
	static float pos2[4] = {lights[1].x, lights[1].y, lights[1].z, 0.0};
	static float ambient[] = {ambience, ambience, ambience, 1};
	
	if (useLight[0]) glEnable(GL_LIGHT0);
	else glDisable(GL_LIGHT0);
	if (useLight[1]) glEnable(GL_LIGHT1);
	else glDisable(GL_LIGHT1);

	//if (isExtensionSupported("EXT_rescale_normal"))
	if (GLEW_EXT_rescale_normal)
		glEnable(GL_RESCALE_NORMAL);
	else
		glEnable(GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glShadeModel(GL_SMOOTH);

	float superwhite[] = {1.5, 1.5, 1.5, 1.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, superwhite);

	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	float grey[] = {0.5, 0.5, 0.5, 1.0};

	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, black);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, grey);
	glLightfv(GL_LIGHT1, GL_SPECULAR, grey);


	//glClearColor(0, 0, 0, 0);	

	// set up default matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// enable various feature flags
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//glBindTexture(GL_TEXTURE_2D, sphereTex);
}

// Draw an instance of the entire scene.  Erase the previous frame if "clear_frame" is set.
void GLScene::DrawInstance(int frame, bool clear_frame)
{	
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(Particle::lineWidth);
	glPointSize(Particle::pointSize);

	// lighting constants
	static float black[] = {0, 0, 0, 1};
	float ambient[] = {ambience, ambience, ambience, 1};
	static float white[] = {1, 1, 1, 1};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	float pos1[4] = {lights[0].x, lights[0].y, lights[0].z, light_points[0] ? 1.0 : 0.0}; // directional!
	float pos2[4] = {lights[1].x, lights[1].y, lights[1].z, light_points[1] ? 1.0 : 0.0};
	if (useLight[0]) glEnable(GL_LIGHT0);	else glDisable(GL_LIGHT0);
	if (useLight[1]) glEnable(GL_LIGHT1);	else glDisable(GL_LIGHT1);
	// use nicer filtering?

	// begin drawing
	if (clear_frame)
	{
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// setup matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	if (picking)
	{
		double x = pick_x, y = (viewport[3]-pick_y);
		double x2 = pick_x2, y2 = (viewport[3]-pick_y2);
		double mx, my, width, height;

		if (x < x2)
		{
			mx = x;
			width = x2 - x;
		}
		else
		{
			mx = x2;
			width = x - x2;
		}
		if (y < y2)
		{
			my = y;
			height = y2 - y;
		}
		else
		{
			my = y2;
			height = y - y2;
		}
		gluPickMatrix(mx + (width/2), my + (height/2), width, height, viewport);
	}

	/*if (view->stereoMode) // Disabled stereo view logic
	{
		if (view->splitScreen)
			view->cam.setProjection(CAM_RIGHTEYE);
		else
			view->cam.setProjection(CAM_LEFTEYE);
		glDrawBuffer(GL_BACK_LEFT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (view->splitScreen)
		{
			cam.setAspect(((double)w / 2.0) / (double)h);
			glViewport(0, 0, w / 2, h);
		}
	}*/
	cam->Draw();
	double camcoord[6];
	cam->getCamera(camcoord);
	Particle::SetCamera( triple(camcoord[0], camcoord[1], camcoord[2]),
							triple(camcoord[3], camcoord[4], camcoord[5]) );

	//for (int s = 0; s < (stereoMode ? 2 : 1); s++) // More disabled stereo view logic
		/*if (s == 1 && stereoMode)
		{
			glDrawBuffer(GL_BACK_RIGHT);
			if (splitScreen)
				glViewport((w/2) + 1, 0, w / 2, h);
			else
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			if (splitScreen)
				cam->setProjection(CAM_LEFTEYE);
			else
				cam->setProjection(CAM_RIGHTEYE);
			cam->Draw();
		}*/

	// we must respecify the lights every frame
	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);		

	glColor4d(1, 1, 1, 1);

	if (!shader_pass_velocity)
	{
		primary->DrawFrame(frame); // standard draw call
	}
	else
	{
		// Pass velocity to sphere shader
		primary->vBuf.EnableTexCoords();
		primary->vBuf_trans.EnableTexCoords();
		primary->DrawFrame(frame);
		primary->vBuf.DisableTexCoords();
		primary->vBuf_trans.DisableTexCoords();
	}
		
	// draw volumetric slices
	if (testslice.Enabled())
	{			
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		testslice.Draw(cam);
	}

	// draw the axes / cam-cone
	glDisable(GL_TEXTURE_2D);
	bool axes = view->axes;
	int state = view->state;
	if (axes || (state == CAMPICK || state == CAMPPICK || state == LIGHT1PICK || state == LIGHT2PICK))
	{
		double D = axes_scale * (axes_resize ? cam->getRadius() : 1.0);
		if (Particle::quality >= 3)	// anti-alias the lines if quality is >= highest
		{
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_BLEND);
			glDepthMask(GL_FALSE);
		}
		else
		{
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
		glDisable(GL_LIGHTING);
		glColor4d(1, 1, 1, 1);

		glLineWidth(3.0);
		if (state == CAMPICK || state == CAMPPICK || state == LIGHT1PICK || state == LIGHT2PICK)
			view->camObj.DrawInstance(NULL);

		if (axes)
		{
			glLineWidth(1.0);
			DrawAxes(D, 1.0);
		}

		if (Particle::quality >= 3)	// disable AA
		{
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}

	/*if (view->counting) // old code for counting exposed white/black pixels
	{
		glFinish();
		double red, green, blue;
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, imageData);
		glFinish();
		int black = 0, white = 0;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w*3; j+=3)
			{
				red = (int)imageData[(i*w)*3 + j + 0];
				green = (int)imageData[(i*w)*3 + j + 1];
				blue = (int)imageData[(i*w)*3 + j + 2];
				if ( red == 255 && green == 255 && blue == 255)
					white++;
				if ( red == 0 && green == 0 && blue == 0)
					black++;
			}
		}
		double r = (double)white / (double)black;
		countratio = r;
		counted = 1;
		char buf[255];
		sprintf(buf, "Frame Ratio: %g (%d white / %d black)", countratio, white, black);
		message = string(buf);
	}*/		

	view->redraw = false;
}

void GLScene::SaveImage()
{
    // dump image if enabled
	if (view->saveimage)
	{
		glFinish(); // block until all drawing commands are complete
		view->saveimage = false;
		if (view->savePNG)
			view->pngOut.WriteImage();
		if (view->saveAVI)
			view->aviOut.AddFrame();
		//if (controller!=NULL && controllershown)
		//	controller->ShowWindow(SW_SHOW);
	}
}

bool GLScene::RenderToImage(int frame, int x, int y, int AA)
{
	off_storage = new RenderTexture(x, y, false, false);
	if (!off_storage->Initialize(true, true, false, false, false, AA))
		return false;


	GLdouble bgcolor[4];
	glGetDoublev(GL_COLOR_CLEAR_VALUE, bgcolor);

	off_storage->BeginCapture();	
		GLInit();
		cam->setAspect((double)x / (double)y);
		glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
		
		glBindTexture(GL_TEXTURE_2D, view->m_texName);

		DrawInstance(frame, true);
		view->saveimage = true;
		SaveImage();
	off_storage->EndCapture();
	
	delete off_storage;
	int dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);
	cam->setAspect(((double)dims[2]) / ((double)dims[3]));
	return true;
}


void GLScene::DrawPeel(int frame, int width, int height, bool first_pass, bool last_pass)
{
	if (!first_pass)
	{
		RenderTexture *temp = frontTex;
		frontTex = backTex;
		backTex = temp;
	}

	if (frontTex == NULL)
		frontTex = new RenderTexture(width, height, false, true);
	if (backTex == NULL)
		backTex = new RenderTexture(width, height, false, true);
	
	if (tex_width != width || tex_height != height)
	{		
		frontTex->Reset(width, height, true, true);
		//if (!frontTex->Initialize(true, true, true, true, true, 8, 8, 8, 8))
		if (!frontTex->Initialize(true, true, false, false, false, 0, 24, 0, 0, 0))
			AfxMessageBox("Failed to init front.");
		
		backTex->Reset(width, height, true, true);
		//if (!backTex->Initialize(true, true, true, true, true, 8, 8, 8, 8))
		if (!backTex->Initialize(true, true, false, false, false, 0, 24, 0, 0, 0))
			AfxMessageBox("Failed to init back.");
	
		//glBindTexture(GL_TEXTURE_RECTANGLE_NV, depth_tex);
		//glTexImage2D(backTex->GetTextureTarget(), 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		/*glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_DEPTH_COMPONENT, width, height, 0, 
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);*/
		tex_width = width;
		tex_height = height;
	}

	frontTex->EnableTextureTarget();		
	frontTex->BindDepth();		
	glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	//glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_COMPARE_FUNC_ARB, GL_GREATER);
	glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(frontTex->GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	frontTex->DisableTextureTarget();

	backTex->EnableTextureTarget();
	backTex->BindDepth();
	glTexParameteri(backTex->GetTextureTarget(), GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	glTexParameteri(backTex->GetTextureTarget(), GL_TEXTURE_COMPARE_FUNC_ARB, GL_NONE);	
	glTexParameteri(backTex->GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(backTex->GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(backTex->GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(backTex->GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	backTex->DisableTextureTarget();
	glUniform1iARB(glGetUniformLocationARB(peel_shader.GetProgramID(), "tex"), 0);

	if (!last_pass)
	{
		backTex->EnableTextureTarget();
		backTex->BeginCapture();
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
	if (!first_pass)
	{
		//glUseProgramObjectARB(peel_program);
		peel_shader.Bind();
		frontTex->EnableTextureTarget();
		frontTex->BindDepth();

		glEnable(GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		//glUseProgramObjectARB(GL_NONE);
		peel_shader.UnBind();
	}
	
	DrawInstance(frame);

	if (!first_pass)
	{
		frontTex->DisableTextureTarget();
	}
	if (!last_pass)
	{
		backTex->EndCapture();
		backTex->DisableTextureTarget();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	frontTex->DisableTextureTarget();
	backTex->DisableTextureTarget();
	glUseProgramObjectARB(GL_NONE);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

int GLScene::QueryScene(int x, int y, int frame, int mode)
{	
	int hit_id = -1;
	ParticleSet::useQueries = false;
	bool disableTransparency = false, disableShaders = false, disablePoints = false;
	if (primary->GetOption(R_TRANSPARENT))
	{
		primary->SetOption(R_TRANSPARENT, false);
		disableTransparency = true;
	}
	if (primary->GetOption(R_SPHERESHADER))
	{
		primary->SetOption(R_SPHERESHADER, false);
		disableShaders = true;
	}	
	if (primary->GetOption(R_USEPOINTS))
	{
		primary->SetOption(R_USEPOINTS, false);
        disablePoints = true;
	}

	hit_id = primary->MarkParticle(x, y, frame, mode);	

	if (disableTransparency) primary->SetOption(R_TRANSPARENT, true);
	if (disableShaders) primary->SetOption(R_SPHERESHADER, true);
	if (disablePoints) primary->SetOption(R_USEPOINTS, true);

	Particle::setup = false;
	//if (hit_id >= 0)
		//primary->MarkParticle(hit_id, mode);
	return hit_id;
}

// Draw an instance of the scene for use with occlusion queries.
// x, y - x2, y2 is an optional rectangle of coordinates to restrict the query to.
void GLScene::DrawOccludedInstance(int frame)
{	
	int N = primary->GetMaxParticles();
	ParticleSet::useQueries = false;
	bool disableTransparency = false, disableShaders = false, disablePoints = false;

	if (!GLEW_ARB_occlusion_query)
	{
		AfxMessageBox("Occlusion queries unsupported.");
		return;
	}

	GLuint sampleCount;
	GLOcclusionQuery::InitQueries(N);

	if (pick_x != -1)
	{
		glGetIntegerv(GL_VIEWPORT, viewport);
		picking = true;
	}

	if (primary->GetOption(R_TRANSPARENT))
	{
		primary->SetOption(R_TRANSPARENT, false);
		disableTransparency = true;
	}

	if (primary->GetOption(R_SPHERESHADER))
	{
		primary->SetOption(R_SPHERESHADER, false);
		disableShaders = true;
	}
	if (primary->GetOption(R_USEPOINTS))
	{
		primary->SetOption(R_USEPOINTS, false);
        disablePoints = true;
	}

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	DrawInstance(frame, true);
	
	glDepthMask(GL_FALSE);	
	glDepthFunc(GL_EQUAL);
	ParticleSet::useQueries = true;
	DrawInstance(frame, false); // query each particle

	for (int i = 0; i < N; i++)
	{
		if (primary->IsVisible(i))
		{
			glGetQueryObjectuivARB(GLOcclusionQuery::queries[i], GL_QUERY_RESULT_ARB, &sampleCount);
			if (ParticleSet::currentQuery.occluded_set ?
				(sampleCount == 0) : (sampleCount > 0))
			{
				if (ParticleSet::currentQuery.hide)
					primary->particles[i]->MarkParticle(MARK_HIDE_MODE); // hide it
				if (ParticleSet::currentQuery.mark)
					primary->particles[i]->MarkParticle(MARK_HIGHLIGHT_MODE); // flag it
				if (ParticleSet::currentQuery.flag_notrans)
					primary->particles[i]->canTransparent = false;
			}
		}
	}
	
	ParticleSet::useQueries = false;
	//glDeleteQueriesARB(N, queries);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (picking)
	{
		pick_x = -1;
		picking = false;
	}
	Particle::setup = false;
	if (disableTransparency)	
		primary->SetOption(R_TRANSPARENT, true);	
	if (disableShaders)
		primary->SetOption(R_SPHERESHADER, true);
	if (disablePoints)
		primary->SetOption(R_USEPOINTS, true);
	DrawInstance(frame, true);	
}

// Change light parameters
void GLScene::SetLight(const int ref, bool enabled, triple direction, double bright, bool pointsource)
{
	if (ref >= 0 && ref < MAXLIGHTS)
	{
		useLight[ref] = enabled;
		lights[ref] = direction;
		lightB[ref] = bright;
		light_points[ref] = pointsource;
	}
}

void GLScene::SetLight(const int ref, triple pos)
{
	if (ref >= 0 && ref < MAXLIGHTS)
		lights[ref] = pos;
}

// Return light parameters
void GLScene::GetLight(const int ref, bool &enabled, triple &direction, double &bright, bool &pointsource)
{
	if (ref >= 0 && ref < MAXLIGHTS)
	{
		enabled = useLight[ref];
		direction = lights[ref];
		bright = lightB[ref];
        pointsource = light_points[ref];
	}	
}

triple GLScene::GetLightPos(const int ref)
{
	if (ref >= 0 && ref < MAXLIGHTS)
		return lights[ref];
	return triple(0, 0, 0);
}

void GLScene::DrawAxes(double D, double A)
{
	glBegin(GL_LINES);
		glColor4d(0, 0, 1, A); // blue = X
			glVertex3d(0, 0, 0);	glVertex3d(1.0*D, 0, 0);
			glVertex3d(1*D, 0, 0);	glVertex3d(0.95*D, 0.05*D, 0);
			glVertex3d(1*D, 0, 0);	glVertex3d(0.95*D, -.05*D, 0);

			glVertex3d(1.02*D, 0.0*D, 0);	glVertex3d(1.04*D, 0.02*D, 0);
			glVertex3d(1.02*D, 0.02*D, 0);	glVertex3d(1.04*D, 0.0*D, 0);

		glColor4d(1, 0, 0, A);	// red = Y
			glVertex3d(0, 0, 0);	glVertex3d(0, 1.0*D, 0);
			glVertex3d(0, 1*D, 0);	glVertex3d(-.05*D, 0.95*D, 0);
			glVertex3d(0, 1*D, 0);	glVertex3d(0.05*D, 0.95*D, 0);

			glVertex3d(0, 1.04*D, 0);	glVertex3d(-0.01*D, 1.05*D, 0);
			glVertex3d(0, 1.04*D, 0);	glVertex3d(0.01*D, 1.05*D, 0);
			glVertex3d(0, 1.02*D, 0);	glVertex3d(0, 1.04*D, 0);

		glColor4d(0, 1, 0, A); // green = Z
			glVertex3d(0, 0, 0);	glVertex3d(0, 0, 1.0*D);
			glVertex3d(0, 0, 1*D);	glVertex3d(0, 0.05*D, 0.95*D);
			glVertex3d(0, 0, 1*D);	glVertex3d(0, -.05*D, 0.95*D);

			glVertex3d(0, 0.0*D, 1.03*D);	glVertex3d(0, 0.0*D, 1.05*D);
			glVertex3d(0, 0.0*D, 1.05*D);	glVertex3d(0, 0.02*D, 1.03*D);
			glVertex3d(0, 0.02*D, 1.03*D);	glVertex3d(0, 0.02*D, 1.05*D);
	glEnd();
}

void GLScene::SetPick(int x, int y, int x2, int y2)
{		
	pick_x = x;
	pick_y = y;
	if (x == x2) x2++;
	if (y == y2) y2++;
	pick_x2 = x2;
	pick_y2 = y2;
}