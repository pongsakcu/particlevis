// PControlDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "PControlDialog.h"
#include "ChildView.h"
#include ".\pcontroldialog.h"


// PControlDialog dialog

IMPLEMENT_DYNAMIC(PControlDialog, CDialog)
PControlDialog::PControlDialog(CChildView* ref, CWnd* parent)
	: CDialog(PControlDialog::IDD, parent)
	, recording(false)
{
	main = ref;
}

PControlDialog::~PControlDialog()
{
}

void PControlDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITPOS, EditPosition);
	DDX_Control(pDX, IDC_EDITSPEED, EditSpeed);
	DDX_Control(pDX, IDC_SLIDERSPEED, SpeedSlider);
	DDX_Control(pDX, IDC_SLIDERPOS, PosSlider);
	DDX_Control(pDX, IDC_BUTTONSTART, StartStopButton);
	DDX_Control(pDX, IDC_CHECK1, playReverse);
	DDX_Control(pDX, IDC_CHECK2, playRepeat);
}


BEGIN_MESSAGE_MAP(PControlDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTONSTART, OnBnClickedButtonstart)
	ON_BN_CLICKED(IDC_BUTTONFWD, OnBnClickedButtonfwd)
	ON_BN_CLICKED(IDC_BUTTONPREV, OnBnClickedButtonprev)
	ON_BN_CLICKED(IDC_BUTTONLAST, OnBnClickedButtonlast)
	ON_BN_CLICKED(IDC_BUTTONFIRST, OnBnClickedButtonfirst)
	ON_BN_CLICKED(IDC_BUTTONMARK, OnBnClickedButtonmark)
	ON_BN_CLICKED(IDC_BUTTONSAVE, OnBnClickedButtonmark2)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_CONTROLLER_BACK, OnControllerBack)
END_MESSAGE_MAP()


// PControlDialog message handlers

void PControlDialog::OnBnClickedButtonstart()
{
	bool stopped = main->StartStop();
/*	if (stopped)
		StartStopButton.SetWindowText("Start");
	else
		StartStopButton.SetWindowText("Stop");*/
	if (main->GetOption(RECORD))
		main->SetMessage("Press 'ESC' or 'C' to halt recording.");

	UpdateScrollBars();
}

void PControlDialog::OnBnClickedButtonfwd()
{
	if (!main->IsStopped())
		main->StartStop();
	main->ModifyCurrentFrame(1);
	UpdateScrollBars();
}

void PControlDialog::OnBnClickedButtonprev()
{
	if (!main->IsStopped())
		main->StartStop();
	main->ModifyCurrentFrame(-1);
	UpdateScrollBars();
}

void PControlDialog::OnBnClickedButtonlast()
{
	main->ModifyCurrentFrame(main->primary->GetMaxFrames());
	UpdateScrollBars();
}

void PControlDialog::OnBnClickedButtonfirst()
{
	main->ModifyCurrentFrame(-1*main->primary->GetMaxFrames());	
	UpdateScrollBars();
}

void PControlDialog::OnBnClickedButtonmark()
{
	main->SetState(PARTPICK);
}

void PControlDialog::OnBnClickedButtonmark2()
{
	main->SaveImage();
}

void PControlDialog::OnCancel()
{
	CDialog::OnCancel();
}

void PControlDialog::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete(this);
}

void PControlDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{	
	if ((CSliderCtrl*)pScrollBar == &SpeedSlider)
	{
		int speed = SpeedSlider.GetPos();
		main->SetSpeed((double)(100-speed));
	}
	else if ((CSliderCtrl*)pScrollBar == &PosSlider)
	{
		int pos = PosSlider.GetPos();
		main->SeekToFrame(pos);
	}

	//CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateScrollBars();
}

void PControlDialog::UpdateScrollBars()
{
	char buffer[64];
	int speed = SpeedSlider.GetPos();
	if (speed==0)
	{
		UpdateText(EditSpeed, "Unlimited");
		//EditSpeed.SetWindowText("Unlimited");
	}
	else
	{
		//cs.Format("%d frames/second", 100-speed);
		sprintf(buffer, "%d frames/second", 100-speed);
		UpdateText(EditSpeed, buffer);
		//EditSpeed.SetWindowText(buffer);
	}
    sprintf(buffer, "Frame %d/%d", main->GetFrame(), max(0, main->primary->GetMaxFrames()-1));
	UpdateText(EditPosition, buffer);
	//EditPosition.SetWindowText(buffer);
	PosSlider.SetRange(0, main->primary->GetMaxFrames()-1, true);
	PosSlider.SetPos(main->GetFrame());

	if (main->IsStopped())
	{
		if (!main->GetOption(RECORD))
			UpdateText(StartStopButton, "Start");
			//StartStopButton.SetWindowText("Start");
		else
			UpdateText(StartStopButton, "Record");
			//StartStopButton.SetWindowText("Record");			
	}
	else
		UpdateText(StartStopButton, "Stop");
		//StartStopButton.SetWindowText("Stop");
}

void PControlDialog::UpdateText(CWnd& control, CString newtext)
{
	CString oldtext;
	control.GetWindowText(oldtext);
	if (oldtext != newtext)
		control.SetWindowText(newtext);
}

void PControlDialog::UpdatePosition(int pos)
{
	//PosSlider.SetPos(pos);
}

BOOL PControlDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SpeedSlider.SetRange(0, 99);
	SpeedSlider.SetPos(0);

	PosSlider.SetRange(0, 100);
	PosSlider.SetPos(100);

	UpdateScrollBars();

	if (main->GetOption(RUNBACKWARDS)) playReverse.SetCheck(BST_CHECKED);
	else playReverse.SetCheck(BST_UNCHECKED);

	if (main->GetOption(RUNLOOP)) playRepeat.SetCheck(BST_CHECKED);
	else playRepeat.SetCheck(BST_UNCHECKED);

	//m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_CONTROLLER));
	return TRUE;  // return TRUE unless you set the focus to a control
}
void PControlDialog::OnBnClickedCheck1() // Reverse
{
	if (playReverse.GetCheck())
		main->SetOption(RUNBACKWARDS, true);
	else
		main->SetOption(RUNBACKWARDS, false);
}

void PControlDialog::OnBnClickedCheck2() // Repeat
{
	if (playRepeat.GetCheck())
		main->SetOption(RUNLOOP, true);
	else
		main->SetOption(RUNLOOP, false);
}

void PControlDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	main->OnKeyDown(nChar, nRepCnt, nFlags);
}

void PControlDialog::OnControllerBack()
{
	OnBnClickedButtonprev();
}

BOOL PControlDialog::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message
					<= WM_KEYLAST)
	//if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		//return TRUE;
		//main->OnKeyDown(pMsg->message, 0, 0);
	{
		main->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
