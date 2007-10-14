#pragma once
#include "GLPlanarSlices.h"
#include "afxwin.h"

// DialogSlices dialog

class DialogSlices : public CDialog
{
	DECLARE_DYNAMIC(DialogSlices)

public:
	DialogSlices(GLPlanarSlices* slice, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogSlices();

// Dialog Data
	enum { IDD = IDD_DIALOGSLICES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	GLPlanarSlices* slices;
public:
	afx_msg void OnBnClickedButtontexel();
	CButton useDummy;
	CButton useLocal;
	CButton useLocalAlpha;
	CButton addBlend;
	CEdit max_alpha;
	CEdit slice_size;
	CEdit num_slices;
	CEdit origin_X;
	CEdit origin_Y;
	CEdit origin_Z;	
	CEdit rot_X;
	CEdit rot_Y;
	CEdit rot_Z;
	CEdit scale_X;
	CEdit scale_Y;
	CEdit scale_Z;
};
