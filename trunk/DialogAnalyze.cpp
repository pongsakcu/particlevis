// DialogAnalyze.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogAnalyze.h"
#include ".\dialoganalyze.h"


// DialogAnalyze dialog

IMPLEMENT_DYNAMIC(DialogAnalyze, CDialog)
DialogAnalyze::DialogAnalyze(CWnd* pParent /*=NULL*/, ParticleSet* pset, int particle, GraphView* target)
	: CDialog(DialogAnalyze::IDD, pParent),
	selected_particle(particle),
	graph_target(target),
	set(pset),
	radio_index(0)
{
}

DialogAnalyze::~DialogAnalyze()
{
}

void DialogAnalyze::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PID, PID);
	DDX_Radio(pDX, IDC_RADIO1, radio_index);
}


BEGIN_MESSAGE_MAP(DialogAnalyze, CDialog)
END_MESSAGE_MAP()


// DialogAnalyze message handlers

void DialogAnalyze::OnOK()
{
	UpdateData();
	int particle;
	char buffer[128];
	PID.GetWindowText(buffer, 128);
	particle = atoi(buffer);
	Particle* p = set->GetParticle(particle);	
	if (p)
		graph_target->LoadParticle(p, set->GetTime(), radio_index);
	CDialog::OnOK();
}

BOOL DialogAnalyze::OnInitDialog()
{
	CDialog::OnInitDialog();
	char buffer[128];
	sprintf(buffer, "%d", selected_particle);
	PID.SetWindowText(buffer);	
	return TRUE;
}
