// DialogScene.cpp : implementation file
//

#include "stdafx.h"
#include <string>
#include "ParticleVis.h"
#include "GLScene.h"
#include "DialogScene.h"
#include "UIProcessor.h"

// DialogScene dialog

IMPLEMENT_DYNAMIC(DialogScene, CDialog)
DialogScene::DialogScene(CWnd* pParent /*=NULL*/)
	: CDialog(DialogScene::IDD, pParent)
	, up_vector_radio(1)
{
}

DialogScene::~DialogScene()
{
}

void DialogScene::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, angle);
	DDX_Control(pDX, IDC_EDIT2, zmin);
	DDX_Control(pDX, IDC_EDIT3, zmax);
	DDX_Control(pDX, IDC_EDIT4, distance);
	DDX_Control(pDX, IDC_EDIT5, max_velocity);
	DDX_Control(pDX, IDC_EDIT6, max_vector);
	DDX_Control(pDX, IDC_EDIT7, path_length);
	DDX_Control(pDX, IDC_CHECK3, camOverride);
	DDX_Control(pDX, IDC_EDIT8, posX);
	DDX_Control(pDX, IDC_EDIT9, posY);
	DDX_Control(pDX, IDC_EDIT10, posZ);
	DDX_Control(pDX, IDC_EDIT11, lookX);
	DDX_Control(pDX, IDC_EDIT12, lookY);
	DDX_Control(pDX, IDC_EDIT13, lookZ);
	DDX_Control(pDX, IDC_EDIT17, alpha);
	DDX_Control(pDX, IDC_LIGHTEDIT1X, light1X);
	DDX_Control(pDX, IDC_LIGHTEDIT1Y, light1Y);
	DDX_Control(pDX, IDC_LIGHTEDIT1Z, light1Z);
	DDX_Control(pDX, IDC_CHECK6, useLight1);
	DDX_Control(pDX, IDC_LIGHTEDIT2X, light2X);
	DDX_Control(pDX, IDC_LIGHTEDIT2Y, light2Y);
	DDX_Control(pDX, IDC_LIGHTEDIT2Z, light2Z);
	DDX_Control(pDX, IDC_CHECK7, useLight2);
	DDX_Control(pDX, IDC_LIGHTEDIT1Z2, bright1);
	DDX_Control(pDX, IDC_LIGHTEDIT1Z3, bright2);
	DDX_Control(pDX, IDC_CHECK5, showTime);
	DDX_Control(pDX, IDC_CHECK8, showFPS);
	//DDX_Control(pDX, IDC_CHECK9, runBackwards);
	//DDX_Control(pDX, IDC_CHECK10, runLoop);
	DDX_Control(pDX, IDC_EDIT14, max_r_velocity);
	DDX_Control(pDX, IDC_YAXISCHECK, reverseYAxis);
	DDX_Control(pDX, IDC_CHECK12, camOrtho);
	DDX_Control(pDX, IDC_EDIT15, max_rvector);
	DDX_Control(pDX, IDC_EDIT16, max_vectorfield);
	DDX_Control(pDX, IDC_EDITMAXFIELD, maxFieldVector);
	DDX_Control(pDX, IDC_L1_PT_CHECK, l1_point);
	DDX_Control(pDX, IDC_L2_PT_CHECK, l2_point);
	DDX_Radio(pDX, IDC_RADIO_UPX, up_vector_radio);
	DDX_Control(pDX, IDC_AMBIENTEDIT, ambience);
	DDX_Control(pDX, IDC_EDITLINEWIDTH, default_line_width);
	DDX_Control(pDX, IDC_EDITPOINTSIZE, pointSize);
	DDX_Control(pDX, IDC_AXESCHECK, resize_axes);
	DDX_Control(pDX, IDC_AXESEDIT, scale_axes);
}


BEGIN_MESSAGE_MAP(DialogScene, CDialog)
	ON_BN_CLICKED(ID_APPLY, OnBnClickedApply)
	ON_BN_CLICKED(IDC_BUTTONAUTOCONFIG, OnBnClickedButtonautoconfig)
	ON_BN_CLICKED(IDC_DEFLIGHT_BUTTON, OnBnClickedDeflightButton)
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()


// DialogScene message handlers

void DialogScene::OnOK()
{
	SetValues();
	CDialog::OnOK();
}

BOOL DialogScene::OnInitDialog()
{
	CDialog::OnInitDialog();
	LoadValues();
	return TRUE;
}

void DialogScene::LoadValues()
{
	char buffer[64];

	sprintf(buffer, "%g", cam->getAngle());
	angle.SetWindowText(buffer);

	sprintf(buffer, "%g", cam->getZNear());
	zmin.SetWindowText(buffer);

	sprintf(buffer, "%g", cam->getZFar());
	zmax.SetWindowText(buffer);

	sprintf(buffer, "%g", cam->getRadius());
	distance.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::maxVelocityMap);
	max_velocity.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::maxRVelocityMap);
	max_r_velocity.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::maxFieldVectorMap);
	maxFieldVector.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::transVelocityLength);
	max_vector.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::rotVelocityLength);
	max_rvector.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::fieldVelocityLength);
	max_vectorfield.SetWindowText(buffer);

	int trailLen = (int)(Particle::trailLength);
	sprintf(buffer, "%d", trailLen);
	path_length.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::lineWidth);
	default_line_width.SetWindowText(buffer);

	sprintf(buffer, "%g", Particle::pointSize);
	pointSize.SetWindowText(buffer);

	if (cam->getInput())
		camOverride.SetCheck(BST_UNCHECKED);
	else
		camOverride.SetCheck(BST_CHECKED);

	if (view->GetOption(SHOWTIME))
		showTime.SetCheck(BST_CHECKED);
	else
		showTime.SetCheck(BST_UNCHECKED);
	if (view->GetOption(SHOWFPS))
		showFPS.SetCheck(BST_CHECKED);
	else
		showFPS.SetCheck(BST_UNCHECKED);

	if (cam->getProjection()==CAM_ORTHO)
		camOrtho.SetCheck(BST_CHECKED);
	else
		camOrtho.SetCheck(BST_UNCHECKED);

	double tri[6];
	cam->getCamera(tri);

	// Camera position
	sprintf(buffer, "%g", tri[X]);
	posX.SetWindowText(buffer);
	sprintf(buffer, "%g", tri[Y]);
	posY.SetWindowText(buffer);
	sprintf(buffer, "%g", tri[Z]);
	posZ.SetWindowText(buffer);

	// Camera target
	sprintf(buffer, "%g", tri[3]);
	lookX.SetWindowText(buffer);
	sprintf(buffer, "%g", tri[4]);
	lookY.SetWindowText(buffer);
	sprintf(buffer, "%g", tri[5]);
	lookZ.SetWindowText(buffer);

	// Camera up vector
	triple up = cam->getUpVector();
	if (up.x == 1) up_vector_radio = X;
	if (up.y == 1) up_vector_radio = Y;
	if (up.z == 1) up_vector_radio = Z;

	bool Lenabled, point;
	double bright;
	triple direction;

	// Light 1
	view->Scene()->GetLight(0, Lenabled, direction, bright, point);
	if (Lenabled)
		useLight1.SetCheck(BST_CHECKED);
	else
		useLight1.SetCheck(BST_UNCHECKED);
	if (point)
		l1_point.SetCheck(BST_CHECKED);
	else
		l1_point.SetCheck(BST_UNCHECKED);
    sprintf(buffer, "%g", direction.x);	light1X.SetWindowText(buffer);
	sprintf(buffer, "%g", direction.y);	light1Y.SetWindowText(buffer);
	sprintf(buffer, "%g", direction.z);	light1Z.SetWindowText(buffer);
	sprintf(buffer, "%g", bright);		bright1.SetWindowText(buffer);

	// Light 2
	view->Scene()->GetLight(1, Lenabled, direction, bright, point);
	if (Lenabled)
		useLight2.SetCheck(BST_CHECKED);
	else
		useLight2.SetCheck(BST_UNCHECKED);
	if (point)
		l2_point.SetCheck(BST_CHECKED);
	else
		l2_point.SetCheck(BST_UNCHECKED);
    sprintf(buffer, "%g", direction.x);	light2X.SetWindowText(buffer);
	sprintf(buffer, "%g", direction.y);	light2Y.SetWindowText(buffer);
	sprintf(buffer, "%g", direction.z);	light2Z.SetWindowText(buffer);
	sprintf(buffer, "%g", bright);		bright2.SetWindowText(buffer);

	sprintf(buffer, "%g", view->Scene()->ambience);
	ambience.SetWindowText(buffer);

	sprintf(buffer, "%g", primary->GetAlpha());
	alpha.SetWindowText(buffer);

	if (cam->getAxisFlip())
		reverseYAxis.SetCheck(BST_CHECKED);
	else
		reverseYAxis.SetCheck(BST_UNCHECKED);

	if (view->Scene()->GetAxesResizing())
		resize_axes.SetCheck(BST_CHECKED);
	else
		resize_axes.SetCheck(BST_UNCHECKED);
	sprintf(buffer, "%g", view->Scene()->GetAxesScale());
	scale_axes.SetWindowText(buffer);

	UpdateData(FALSE);
}
void DialogScene::SetValues()
{
	UpdateData();
	double value, value2;
	char buffer[64];
	char* temp;

    angle.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value > 0)
		cam->setAngle(value);

	zmin.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	zmax.GetWindowText(buffer, 64);
	value2 = strtod(buffer, &temp);

	if (value > 0 && value2 > 0 && value2 > value)
		cam->setZBuffer(value, value2);

	distance.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value != 0)
		cam->setRadius(value);

	// maximum velocity
	max_velocity.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	Particle::maxVelocityMap = value;

	// maximum rotational velocity
	max_r_velocity.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	Particle::maxRVelocityMap = value;

	// maximum field vector
	maxFieldVector.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	Particle::maxFieldVectorMap = value;

	// trans. vector length
	max_vector.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value != 0)
		Particle::transVelocityLength = value;

	// rot. vector length
	max_rvector.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value != 0)
		Particle::rotVelocityLength = value;

	// field vector length
	max_vectorfield.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value != 0)
		Particle::fieldVelocityLength = value;

	// path length
	path_length.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value >= 0)
		Particle::trailLength = (int)value;

	// line width
	default_line_width.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value >= 0)
		Particle::lineWidth = value;

	// point size
	pointSize.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value > 0)
		Particle::pointSize = value;

	// camera settings
	if (camOverride.GetCheck())
	{
		double camVal[6];
		posX.GetWindowText(buffer, 64);
		camVal[0] = strtod(buffer, &temp);
		posY.GetWindowText(buffer, 64);
		camVal[1] = strtod(buffer, &temp);
		posZ.GetWindowText(buffer, 64);
		camVal[2] = strtod(buffer, &temp);

		lookX.GetWindowText(buffer, 64);
		camVal[3] = strtod(buffer, &temp);
		lookY.GetWindowText(buffer, 64);
		camVal[4] = strtod(buffer, &temp);
		lookZ.GetWindowText(buffer, 64);
		camVal[5] = strtod(buffer, &temp);

		cam->setInput(false);
		cam->setManualPosition(camVal[0], camVal[1], camVal[2], camVal[3], camVal[4], camVal[5]);
	}
	else
		cam->setInput(true);

	view->SetOption(SHOWTIME, showTime.GetCheck());
	view->SetOption(SHOWFPS, showFPS.GetCheck());

	if (up_vector_radio == X)
		cam->setUpVector(1.0, 0.0, 0.0);
	else if (up_vector_radio == Y)
		cam->setUpVector(0.0, 1.0, 0.0);
	else if (up_vector_radio == Z)
		cam->setUpVector(0.0, 0.0, 1.0);
	

	static float black[] = {0, 0, 0, 1};
	static float white[] = {1, 1, 1, 1};

	// Light 1
	triple lightpos;
	double bright;
	light1X.GetWindowText(buffer, 64);	lightpos.x = strtod(buffer, &temp);
	light1Y.GetWindowText(buffer, 64);	lightpos.y = strtod(buffer, &temp);
	light1Z.GetWindowText(buffer, 64);	lightpos.z = strtod(buffer, &temp);
	bright1.GetWindowText(buffer, 64);	bright = strtod(buffer, &temp);
	view->Scene()->SetLight(0, useLight1.GetCheck(), lightpos, bright, l1_point.GetCheck());

	float light1[] = {bright, bright, bright, 1.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light1);

	// Light 2
	light2X.GetWindowText(buffer, 64);	lightpos.x = strtod(buffer, &temp);
	light2Y.GetWindowText(buffer, 64);	lightpos.y = strtod(buffer, &temp);
	light2Z.GetWindowText(buffer, 64);	lightpos.z = strtod(buffer, &temp);
	bright2.GetWindowText(buffer, 64);	bright = strtod(buffer, &temp);
	view->Scene()->SetLight(1, useLight2.GetCheck(), lightpos, bright, l2_point.GetCheck());

	float light2[] = {bright, bright, bright, 1.0};
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light2);

	// Ambience
	ambience.GetWindowText(buffer, 64);
	bright = strtod(buffer, &temp);
	view->Scene()->ambience = bright;

	// Alpha
	alpha.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	if (value > 0)
		primary->SetAlpha(value);

	if (reverseYAxis.GetCheck())
		cam->setAxisFlip(true);
	else
		cam->setAxisFlip(false);

	if (camOrtho.GetCheck())
		cam->setProjection(CAM_ORTHO);
	else
	{
		if (cam->getProjection()==CAM_ORTHO)
			cam->setProjection(CAM_PERSPECTIVE);
	}

	// Axes
	scale_axes.GetWindowText(buffer, 64);
	value = strtod(buffer, &temp);
	view->Scene()->SetAxes(value, resize_axes.GetCheck());

	Particle::setup = false; // "dirty" flag
}

INT_PTR DialogScene::DoModal(Camera* _cam, ParticleSet* _primary, CChildView* _view)
{
	cam = _cam;
	primary = _primary;
	view = _view;

	return CDialog::DoModal();
}

void DialogScene::OnBnClickedApply()
{
	AfxGetApp()->m_pMainWnd->Invalidate(TRUE);
	//Invalidate(FALSE);
	SetValues();
	view->ForceRedraw();
}

void DialogScene::OnBnClickedButtonautoconfig()
{
	view->ui->AutoSceneSettings();
	LoadValues();
}

void DialogScene::OnBnClickedDeflightButton()
{
	view->Scene()->SetLight(0, true, triple(0.25, 0.25, 0.1), 1.0, false);
	view->Scene()->SetLight(1, true, triple(-.3, -.2, .075), 0.5, false);
	LoadValues();
}

void DialogScene::OnCaptureChanged(CWnd *pWnd)
{
	view->ForceRedraw();
	CDialog::OnCaptureChanged(pWnd);
}
