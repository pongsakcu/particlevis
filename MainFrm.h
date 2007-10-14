// MainFrm.h : interface of the CMainFrame class
//

#define UWM_CHANGESIZE (WM_APP + 1)

struct window_size
{
	int w;
	int h;
};

#pragma once

#include "ChildView.h"
class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CChildView    m_wndView;

	LRESULT OnSetWindowSize(WPARAM, LPARAM);

protected:  // control bar embedded members
	CToolBar    m_wndToolBar;

	bool fullscreen;
	CRect oldFrame;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:
	void SetWindowSize(int screen_w, int screen_h);
	void Restore();
	afx_msg void OnViewFull();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	
	afx_msg void OnHelpContents();
	afx_msg void OnSetwindowsize640x480();
	afx_msg void OnSetwindowsize800x600();
	afx_msg void OnSetwindowsize1024x768();
	afx_msg void OnSetwindowsize1280x960();
	afx_msg void OnSetwindowtosize320x240();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetwindowtosize512x512();
	afx_msg void OnSetwindowtosizeSettoarbitarysize();
};


