#pragma once
#include "afxwin.h"
#include "ParticleSet.h"


// BoundsDialog dialog

class BoundsDialog : public CDialog
{
	DECLARE_DYNAMIC(BoundsDialog)

public:
	BoundsDialog(ParticleSet* primary, double *bounds, CWnd* pParent = NULL);   // standard constructor
	virtual ~BoundsDialog();

// Dialog Data
	enum { IDD = IDD_BOUNDSDIALOG };

protected:
	ParticleSet* prim;
	double* initialBounds;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadValues();

	DECLARE_MESSAGE_MAP()
public:
	CEdit XMin;
	CEdit XMax;
	CEdit YMin;
	CEdit YMax;
	CEdit ZMin;
	CEdit ZMax;
	CButton enableX;
	CButton enableY;
	CButton enableZ;
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	CButton transBound;
	CButton markRegion;
};
