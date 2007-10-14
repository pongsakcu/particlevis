// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "PControlDialog.h"
#include "DialogNumInput.h"

#include "MainFrm.h"
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_FULL, OnViewFull)
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_HELP_CONTENTS, OnHelpContents)
	ON_COMMAND(ID_SETWINDOWTOSIZE_320X240, OnSetwindowtosize320x240)
	ON_COMMAND(ID_SETWINDOWSIZE_640X480, OnSetwindowsize640x480)
	ON_COMMAND(ID_SETWINDOWSIZE_800X600, OnSetwindowsize800x600)
	ON_COMMAND(ID_SETWINDOWSIZE_1024X768, OnSetwindowsize1024x768)
	ON_COMMAND(ID_SETWINDOWSIZE_1280X960, OnSetwindowsize1280x960)
	ON_MESSAGE(UWM_CHANGESIZE, OnSetWindowSize)
	ON_WM_SIZE()
	ON_COMMAND(ID_SETWINDOWTOSIZE_512X512, OnSetwindowtosize512x512)
	ON_COMMAND(ID_SETWINDOWTOSIZE_SETTOARBITARYSIZE, OnSetwindowtosizeSettoarbitarysize)
END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		// try again.  if we're going for AA, you have to do this.  (don't ask me. I just hack here.)
		if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
		{
			TRACE0("Failed to create view window\n");
			return -1;
		}
	}

	// Toolbar enabled!
	/*
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	*/
	/*if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{   // IDR_MAINFRAME
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);*/

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	/*cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU; Old-style*/

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU;

	//cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	
	cs.dwExStyle |= WS_EX_CLIENTEDGE;

	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnViewFull()
{
	if (!fullscreen)
	{
		fullscreen = true;
		CRect rcv;
		m_wndView.GetWindowRect(&rcv);

		// get frame rectangle
		GetWindowRect(oldFrame); // save for restore
		const CRect& rcf = oldFrame;				// frame rect

		// now compute new rect
		CRect rc(0,0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN));

		rc.left  += rcf.left  - rcv.left;
		rc.top   += rcf.top   - rcv.top;
		rc.right += rcf.right - rcv.right;
		rc.bottom+= rcf.bottom- rcv.bottom;

		// move frame!
		SetWindowPos(NULL, rc.left, rc.top,
			rc.Width(), rc.Height(), SWP_NOZORDER);
		//SetWindowPos(NULL, 100, 100, 640+12, 480+59, SWP_NOZORDER);

		m_wndView.SetMessage("Press Escape to exit fullscreen mode.");
	}
	else
	{
		const CRect& rc = oldFrame;
		if (rc != NULL)
		{
			SetWindowPos(NULL, rc.left, rc.top,
				rc.Width(), rc.Height(), SWP_NOZORDER);
			oldFrame.SetRectEmpty();
		}
		fullscreen = false;
	}
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (fullscreen)
	{
		CRect rc(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		rc.InflateRect(10, 50);
		CSize cs = rc.Size();

		lpMMI->ptMaxSize = CPoint(cs);
		lpMMI->ptMaxTrackSize = CPoint(cs);
	}
	else
	{
		CFrameWnd::OnGetMinMaxInfo(lpMMI);
	}
}

void CMainFrame::Restore()
{
	if (fullscreen)
		OnViewFull();
}
void CMainFrame::OnHelpContents()
{	
	string helppath = m_wndView.exePath + "help\\index.html";
	//AfxGetApp()->m_pszHelpFilePath = m_wndView.exePath + "PVisHelp.chm";
	//HtmlHelp(NULL, HH_DISPLAY_TOPIC);
	ShellExecute(NULL, "open", helppath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void CMainFrame::OnSetwindowtosize320x240()
{
	SetWindowSize(320, 240);
}

void CMainFrame::OnSetwindowtosize512x512()
{
	SetWindowSize(512, 512);
}
void CMainFrame::OnSetwindowsize640x480()
{
	SetWindowSize(640, 480);
}

void CMainFrame::OnSetwindowsize800x600()
{
	SetWindowSize(800, 600);
}

void CMainFrame::OnSetwindowsize1024x768()
{
	SetWindowSize(1024, 768);
}

void CMainFrame::OnSetwindowsize1280x960()
{
	SetWindowSize(1280, 960);
}

void CMainFrame::OnSetwindowtosizeSettoarbitarysize()
{
	int w, h;
	double width, height;
	DialogNumInput in_x("Window Width", width, this);
	in_x.DoModal();
	DialogNumInput in_y("Window Height", height, this);
	in_y.DoModal();
	w = (int)width;
	h = (int)height;
	if (w > 0 && h > 0)
		SetWindowSize(w, h);
}

void CMainFrame::SetWindowSize(int screen_w, int screen_h)
{
	CRect rcv;
	m_wndView.GetWindowRect(&rcv);

	// get frame rectangle
	GetWindowRect(oldFrame); // save for restore
	const CRect& rcf = oldFrame;				// frame rect

	// now compute new rect
	CRect rc(0,0, screen_w,	screen_h);
	rc.left  += rcf.left  - rcv.left;
	rc.top   += rcf.top   - rcv.top;
	rc.right += rcf.right - rcv.right;
	rc.bottom+= rcf.bottom- rcv.bottom;

	ShowWindow(SW_SHOWNORMAL);
	SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER);
}

LRESULT CMainFrame::OnSetWindowSize(WPARAM w, LPARAM l)
{
	// w is ignored
	window_size* ptr = (window_size*)l;
	SetWindowSize(ptr->w, ptr->h);
	delete ptr;
	return 0;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	if (nType == SIZE_MINIMIZED)
	{
		m_wndView.SetMinimized(true);
	}
	else
	{
		m_wndView.SetMinimized(false);
	}
}
