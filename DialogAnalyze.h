#pragma once
#include "afxwin.h"
#include "GraphView.h"
#include "ParticleSet.h"

// DialogAnalyze dialog

class DialogAnalyze : public CDialog
{
	DECLARE_DYNAMIC(DialogAnalyze)

public:
	DialogAnalyze(CWnd* pParent, ParticleSet* set, int particle, GraphView* target);   // standard constructor
	virtual ~DialogAnalyze();

// Dialog Data
	enum { IDD = IDD_ANALYSIS_DIALOG };

protected:
	int selected_particle;
	GraphView* graph_target;
	ParticleSet* set;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit PID;
protected:
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	int radio_index;
};
