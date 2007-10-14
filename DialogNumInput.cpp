// DialogNumInput.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogNumInput.h"

// DialogNumInput dialog

IMPLEMENT_DYNAMIC(DialogNumInput, CDialog)
DialogNumInput::DialogNumInput(string _title, double& _datatarget, CWnd* pParent /*=NULL*/)
	: CDialog(DialogNumInput::IDD, pParent), data(_datatarget)
{
	title = _title;
}

DialogNumInput::~DialogNumInput()
{
}

void DialogNumInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITNUM, input);
	DDX_Control(pDX, IDC_EDITTEXT, title_box);
}

BEGIN_MESSAGE_MAP(DialogNumInput, CDialog)
END_MESSAGE_MAP()


// DialogNumInput message handlers

void DialogNumInput::OnOK()
{
	char buffer[256];
	input.GetWindowText(buffer, 256);
	data = strtod(buffer, NULL);
	CDialog::OnOK();
}

BOOL DialogNumInput::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetWindowText(title.c_str());
	char buffer[255];
	sprintf(buffer, "Input %s:", title.c_str());
	title_box.SetWindowText(buffer);

	return TRUE; 
}
