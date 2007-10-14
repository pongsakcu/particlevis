// DialogRandom.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogRandom.h"
#include "PRandomGenerator.h"


// DialogRandom dialog

IMPLEMENT_DYNAMIC(DialogRandom, CDialog)
DialogRandom::DialogRandom(ParticleSet* target, CWnd* pParent /*=NULL*/)
	: CDialog(DialogRandom::IDD, pParent), primary(target)
{
}

DialogRandom::~DialogRandom()
{
}

void DialogRandom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PNUMBER, PNumber);
	DDX_Control(pDX, IDC_NORMALCHECK, Normal);
}


BEGIN_MESSAGE_MAP(DialogRandom, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


void DialogRandom::OnBnClickedOk()
{
	int pnumber = 1000;
	bool normal = (bool)Normal.GetCheck();

	char buffer[128];
	PNumber.GetWindowText(buffer, 128);
	pnumber = (int)strtod(buffer, NULL);
	if (pnumber < 0) pnumber = 0;

	PRandomGenerator pr;
	pr.GenerateRandomFrame(primary, pnumber, normal);

	OnOK();
}
