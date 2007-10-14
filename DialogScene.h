/*

DialogScene

	Allows modification of scene constants used during rendering:
	Camera Values
	ParticleSet Limits
	Lights (future)

*/
#pragma once
#include "afxwin.h"
#include "ParticleSet.h"
#include "Camera.h"
#include "ChildView.h"

// DialogScene dialog

class DialogScene : public CDialog
{
	DECLARE_DYNAMIC(DialogScene)

public:
	DialogScene(CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogScene();

// Dialog Data
	enum { IDD = IDD_DIALOGSCENE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadValues();
	void SetValues();
	Camera* cam;
	ParticleSet* primary;
	CChildView* view;

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	CEdit angle;
	CEdit zmin;
	CEdit zmax;
	CEdit distance;
	CEdit max_velocity;
	CEdit max_r_velocity;
	CEdit max_vector;
	CEdit path_length;
	virtual INT_PTR DoModal(Camera* _cam, ParticleSet* _primary, CChildView* _view);
	CButton camOverride;
	CEdit posX;
	CEdit posY;
	CEdit posZ;
	CEdit lookX;
	CEdit lookY;
	CEdit lookZ;
	afx_msg void OnBnClickedApply();
	CEdit alpha;
	CEdit light1X;
	CEdit light1Y;
	CEdit light1Z;
	CButton useLight1;
	CEdit light2X;
	CEdit light2Y;
	CEdit light2Z;
	CButton useLight2;
	CEdit bright1;
	CEdit bright2;
	CButton showTime;
	CButton showFPS;
	CButton reverseYAxis;
	CButton camOrtho;
	afx_msg void OnBnClickedButtonautoconfig();
	CEdit max_rvector;
	CEdit max_vectorfield;
	CEdit maxFieldVector;
	afx_msg void OnBnClickedDeflightButton();
	CButton l1_point;
	CButton l2_point;
	int up_vector_radio;
	CEdit ambience;
	CEdit default_line_width;
	CEdit pointSize;
	CButton resize_axes;
	CEdit scale_axes;
	afx_msg void OnCaptureChanged(CWnd *pWnd);
};
