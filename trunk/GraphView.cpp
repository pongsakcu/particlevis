#include ".\graphview.h"
#include "WGLFont.h"

GraphView::GraphView(double font_size /* = 0.05 */) :
current_frame(0), 
init(false),
kill(false),
scale(1),
fontsize(font_size)
{
}

GraphView::~GraphView(void)
{
}
BEGIN_MESSAGE_MAP(GraphView, GLWnd)
	//ON_WM_MOVE()
	//ON_WM_MOVING()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

bool GraphView::Draw(void)
{
	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated((float)(tick%360), 0, 0, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
		glVertex3d(-1, -1, 0);
		glVertex3d(1, 1, 0);
		glVertex3d(-1, 1, 0);
		glVertex3d(1, -1, 0);
	glEnd();*/
	//thisgraph->SetPosition(0.1, 0.1, 0.8, 0.8);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (init)
		thisgraph->Draw(current_frame);

	return true;
}

void GraphView::GLInit()
{
	GLWnd::GLInit();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	WGLFont* mister_font = new WGLFont(pDC->GetSafeHdc());
	mister_font->MakeFont("Verdana", fontsize, true);
	thisgraph = new GraphDisplay(mister_font);
	thisgraph->SetPosition(0.1, 0.1, 0.8, 0.8);
	init = true;
}
void GraphView::OnClose()
{
	kill = true;
	ShowWindow(SW_HIDE);
}

void GraphView::OnLButtonDown(UINT nFlags, CPoint point)
{
	dragging = true;
	last_button = 1;

	if (thisgraph->IsZoomed())
		thisgraph->ZoomOut();
	else
		thisgraph->ZoomIn(point.x, point.y);
}

void GraphView::OnRButtonDown(UINT nFlags, CPoint point)
{
	dragging = true;
	last_button = 2;
	thisgraph->ZoomOut();
}

void GraphView::OnLButtonUp(UINT nFlags, CPoint point)
{
	//ReleaseCapture();
	dragging = false;
}

void GraphView::OnRButtonUp(UINT nFlags, CPoint point)
{
	//ReleaseCapture();
	dragging = false;
}

void GraphView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (dragging)
	{
		if (last_button==1 && thisgraph->IsZoomed())
			thisgraph->ZoomIn(point.x, point.y);
	}
}

void GraphView::LoadParticle(Particle* p, vector<double>::const_iterator times,  int datatype)
{
	if (p == NULL)// || times == NULL)
		return;
	
	int max_frame = p->GetFrames();
	std::vector<float> timepoints, datapoints;
	float data;
	for (int i = 0; i < max_frame; i++)
	{
		float t = *times;
		times++;
		switch(datatype)
		{
		case X:
			data = p->GetPosition(i).x;
			break;
		case Y:
			data = p->GetPosition(i).y;
			break;
		case Z:	
			data = p->GetPosition(i).z;
			break;
		case X+3:
			data = p->PeekVector(i, POSVELOCITY).x;
			break;
		case Y+3:
			data = p->PeekVector(i, POSVELOCITY).y;
			break;
		case Z+3:
			data = p->PeekVector(i, POSVELOCITY).z;
			break;
		case X+6:
			data = p->PeekVector(i, ROTVELOCITY).x;
			break;
		case Y+6:
			data = p->PeekVector(i, ROTVELOCITY).y;
			break;
		case Z+6:
			data = p->PeekVector(i, ROTVELOCITY).z;
			break;
		}
		timepoints.push_back(t);
		datapoints.push_back(data);
	}
	thisgraph->LoadData(timepoints, datapoints);

	char buf[64];
	sprintf(buf, "PID %d ", p->GetPID());
	thisgraph->x_label = "Time";
	thisgraph->y_label.assign(buf);
	switch(datatype)
	{
	case X:
		thisgraph->y_label += "X Coordinate";
		break;
	case Y:
		thisgraph->y_label += "Y Coordinate";
		break;
	case Z:	
		thisgraph->y_label += "Z Coordinate";
		break;
	case X+3:
		thisgraph->y_label += "X Velocity";
		break;
	case Y+3:
		thisgraph->y_label += "Y Velocity";
		break;
	case Z+3:
		thisgraph->y_label += "Z Velocity";
		break;
	case X+6:
		thisgraph->y_label += "Eta-Dot";
		break;
	case Y+6:
		thisgraph->y_label += "Xi-Dot";
		break;
	case Z+6:
		thisgraph->y_label += "Lambda-Dot";
		break;
	}
}