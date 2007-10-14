// ParticleVis.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "MainFrm.h"
#include "PControlDialog.h"
#include ".\particlevis.h"
#include "PControlDialog.h"
#include <afxadv.h>
#include <math.h>
#include "RGBGradient.h"
#include "GraphView.h"
#include "UIProcessor.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CParticleVisApp

BEGIN_MESSAGE_MAP(CParticleVisApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_SUSPENDDRAWINGPROCESSES, OnFileSuspenddrawingprocesses)
END_MESSAGE_MAP()

int particle_type = 1;
CSemaphore lock;

// CParticleVisApp construction

CParticleVisApp::CParticleVisApp()
{
	suspendDraw = false;
}


// The one and only CParticleVisApp object

CParticleVisApp theApp;

// CParticleVisApp initialization

BOOL CParticleVisApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	SetRegistryKey(_T("ParticleVis"));

	m_pszHelpFilePath = new char[128];
	strcpy((char*)m_pszHelpFilePath, "PVisHelp.chm");

	// check for stereo window mode
	string args(m_lpCmdLine);
	static const basic_string <char>::size_type npos = -1;
	if (args.find("-stereo") != npos)
		GLWnd::stereoMode = true;

	if (args.find("-8x") != npos)
		GLWnd::useAA = 8;

	if (args.find("-4x") != npos)
		GLWnd::useAA = 4;


	// create main frame
	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);
	// The one and only window has been initialized, so show and update it

	if (strlen(m_lpCmdLine) == 0)
	{
		pFrame->ShowWindow(SW_SHOWMAXIMIZED);
		pFrame->UpdateWindow();
	}

	PControlDialog* controller;
    controller = new PControlDialog(&(pFrame->m_wndView), pFrame);
	//controller = new PControlDialog(&(pFrame->m_wndView), &(pFrame->m_wndView));
	controller->Create(PControlDialog::IDD);
	if (strlen(m_lpCmdLine) == 0)
		controller->ShowWindow(SW_SHOW);

	pFrame->m_wndView.SetController(controller);
	CMenu* menu = new CMenu();
	menu->Attach(pFrame->m_hMenuDefault);
	pFrame->m_wndView.LoadCMenu(menu);
	pFrame->m_wndView.UpdateMenu();
	pFrame->m_wndView.DragAcceptFiles();

	LoadStdProfileSettings(5); // Load standard INI file options (including MRU)
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

	UIProcessor proc(&(pFrame->m_wndView), pFrame->m_wndView.primary);

	//pFrame->m_wndView.ProcessArgs(m_lpCmdLine); // if there are args, execute batch mode
	proc.ProcessArgs(m_lpCmdLine);
	if (strlen(m_lpCmdLine) != 0)
	{
		pFrame->ShowWindow(SW_SHOWMAXIMIZED);
		pFrame->UpdateWindow();
		controller->ShowWindow(SW_SHOW);
	}
	pFrame->m_wndView.SetFocus();


	_set_SSE2_enable(1);
	return TRUE;
}


// CParticleVisApp message handlers



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CParticleVisApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CParticleVisApp message handlers


CDocument* CParticleVisApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	string convert(lpszFileName);
	CMainFrame *ptr = (CMainFrame*)m_pMainWnd;
	ptr->m_wndView.LoadFile(convert.c_str());
	return (CDocument*)true;
}

int CParticleVisApp::Run()
{
	ASSERT_VALID(this);
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;

	//MSG msg;
	int nFrames = 0;
	double fps = 0, time = 0;
	CMainFrame *ptr = (CMainFrame*)m_pMainWnd;
	CChildView *view = &(ptr->m_wndView);
	PControlDialog *control = view->controller;

	WINDOWPLACEMENT isShown;
	vector<GraphView*>::iterator sub_iter;

	/*while (true)	// primary message loop
	{
		// Process all messages
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				if (!PreTranslateMessage(&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				_CrtDumpMemoryLeaks();
				return TRUE;
			}
		}*/

	bool swap = true;
	for (;;)
	{
		// phase2: pump messages while available
		
		do
		{
			// pump message, but quit on WM_QUIT
			if (!PumpMessage())			
				return ExitInstance();

			// reset "no idle" state after pumping "normal" message
			/*if (IsIdleMessage(&(pState->m_msgCur)))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}*/

		} while (::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE));


		// phase1: check to see if we can do idle work
		/*while (bIdle &&
			!::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			// call OnIdle while in bIdle state
			if (!OnIdle(lIdleCount++))
				bIdle = FALSE; // assume "no idle" state
		}*/
		//if (view->redraw)


		// draw latest frame
		//if (view && !suspendDraw && !view->IsHidden())//!view->IsMinimized())
		
		while (!::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE) &&
			view && !suspendDraw && !view->IsHidden())
		{
			if (!wglMakeCurrent(view->pDC->m_hDC, view->hRC)) // Activate context
				return ExitInstance();
			//if (true) // Actual GL routines
			
			{
				if (swap)
					if (!SwapBuffers(view->pDC->m_hDC)) // Swap back-buffer (if present)
						return ExitInstance();
				swap = view->Draw();
			}
			if (subwindow_stack.size() > 0)
			{
				for (sub_iter = subwindow_stack.begin(); sub_iter != subwindow_stack.end(); sub_iter++)
				{
					if ((*sub_iter)->IsDead())
					{
						delete (*sub_iter);
						sub_iter = subwindow_stack.erase(sub_iter);
						//sub_iter = subwindow_stack.end() - 1;
						if (sub_iter == subwindow_stack.end())
							break;
					}
					else
					{
						if (wglMakeCurrent((*sub_iter)->pDC->m_hDC, (*sub_iter)->hRC))
						{
							(*sub_iter)->SetFrame(view->GetFrame());
							if ((*sub_iter)->Draw())
							{
								if (!SwapBuffers((*sub_iter)->pDC->m_hDC))
									return ExitInstance();
							}
						}
					}

				}
			}
			//view->Invalidate(FALSE);			
		}
		view->living = true;
	}
}

void CParticleVisApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
	CWinApp::AddToRecentFileList(lpszPathName);
	m_pRecentFileList->WriteList();
}

void CParticleVisApp::OnFileSuspenddrawingprocesses()
{
	CMainFrame *ptr = (CMainFrame*)m_pMainWnd;
	CChildView *view = &(ptr->m_wndView);

	suspendDraw = !suspendDraw;
	view->SetOption(SUSPEND, suspendDraw);
	view->UpdateMenu();

	/*if (suspendDraw)
		this->SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
	else
		this->SetThreadPriority(THREAD_PRIORITY_NORMAL);*/
}
