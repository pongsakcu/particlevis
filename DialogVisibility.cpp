// DialogVisibility.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "ParticleSet.h"
#include "DialogVisibility.h"
#include "ChildView.h"

// DialogVisibility dialog

IMPLEMENT_DYNAMIC(DialogVisibility, CDialog)
DialogVisibility::DialogVisibility(CChildView* pParent, ParticleSet * pset)
: CDialog(DialogVisibility::IDD, pParent),
parent(pParent),
set(pset),
visible_set(FALSE)
{
}

DialogVisibility::~DialogVisibility()
{
}

void DialogVisibility::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_HIDE, hide_selected);
	DDX_Control(pDX, IDC_CHECK_MARK, mark_selected);
	DDX_Control(pDX, IDC_CHECK_TRANS, flag_selected);
	DDX_Radio(pDX, IDC_RADIO_USEVIS, visible_set);
	DDX_Control(pDX, IDC_RADIO_USEVIS, select_visible);
}


BEGIN_MESSAGE_MAP(DialogVisibility, CDialog)
	ON_BN_CLICKED(IDC_BUTTONSELECT, OnBnClickedButtonSelect)
	ON_BN_CLICKED(IDC_BUTTONINVERT, OnBnClickedButtonInvert)
	ON_BN_CLICKED(IDC_BUTTONRESET, OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_CHECK_HIDE, OnBnClickedCheckHide)
	ON_BN_CLICKED(IDC_CHECK_MARK, OnBnClickedCheckMark)
	ON_BN_CLICKED(IDC_CHECK_TRANS, OnBnClickedCheckTrans)
	ON_BN_CLICKED(IDC_RADIO_USEVIS, OnBnClickedRadioUsevis)
	ON_BN_CLICKED(IDC_RADIO_USEOCC, OnBnClickedRadioUseocc)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButtonHelp)
	ON_BN_CLICKED(IDC_BUTTONSELECT2, OnBnClickedButtonselect2)
END_MESSAGE_MAP()


// DialogVisibility message handlers

// Make visibility selection: simply enable the flags in the ParticleSet
void DialogVisibility::OnBnClickedButtonSelect()
{
	ParticleSet::useQueries = true;
	Particle::setup = false;
}

// Make visibility selection on selected region (mouse-drag box)
void DialogVisibility::OnBnClickedButtonselect2()
{
	parent->SetState(SELECTION);
}

// Invert current selection
void DialogVisibility::OnBnClickedButtonInvert()
{
	set->InvertSelection();	
}

// Clear all current marking
void DialogVisibility::OnBnClickedButtonReset()
{
	set->UnMarkAll();
	set->ClearColor();
}

// An marking option was changed, update currentQuery object
void DialogVisibility::Refresh(void)
{
	if (ParticleSet::currentQuery.mark)
		mark_selected.SetCheck(BST_CHECKED);
	else
		mark_selected.SetCheck(BST_UNCHECKED);

	if (ParticleSet::currentQuery.hide)
		hide_selected.SetCheck(BST_CHECKED);
	else
		hide_selected.SetCheck(BST_UNCHECKED);

	if (ParticleSet::currentQuery.flag_notrans)
		flag_selected.SetCheck(BST_CHECKED);
	else
		flag_selected.SetCheck(BST_UNCHECKED);

	if (ParticleSet::currentQuery.occluded_set)
		visible_set = true;
	else
		visible_set = false;
}

// Callbacks for check/radio events

void DialogVisibility::OnBnClickedCheckHide()
{
	if (hide_selected.GetCheck())
		ParticleSet::currentQuery.hide = true;
	else
		ParticleSet::currentQuery.hide = false;
	Refresh();
}

void DialogVisibility::OnBnClickedCheckMark()
{
	if (mark_selected.GetCheck())
		ParticleSet::currentQuery.mark = true;
	else
		ParticleSet::currentQuery.mark = false;
	Refresh();
}

void DialogVisibility::OnBnClickedCheckTrans()
{
	if (flag_selected.GetCheck())
		ParticleSet::currentQuery.flag_notrans = true;
	else
		ParticleSet::currentQuery.flag_notrans = false;
	Refresh();
}

void DialogVisibility::OnBnClickedRadioUsevis()
{
	if (select_visible.GetCheck())
		ParticleSet::currentQuery.occluded_set = false;
	else
		ParticleSet::currentQuery.occluded_set = true;
	Refresh();
}

void DialogVisibility::OnBnClickedRadioUseocc()
{	
	if (select_visible.GetCheck())
		ParticleSet::currentQuery.occluded_set = false;	
	else
		ParticleSet::currentQuery.occluded_set = true;
	Refresh();
}

BOOL DialogVisibility::OnInitDialog()
{
	CDialog::OnInitDialog();
	Refresh();
	return TRUE;
}



// The (very long) function call for the help button
void DialogVisibility::OnBnClickedButtonHelp()
{
	AfxMessageBox("The visibility selector will apply marking actions to all visible (or occluded) particles.\nCheck off what marking actions to take and press the 'perform visibility selection' button to sample the current scene.",
		MB_OK|MB_ICONQUESTION);
}
