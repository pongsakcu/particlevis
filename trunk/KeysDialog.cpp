// KeysDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "KeysDialog.h"


// KeysDialog dialog

IMPLEMENT_DYNAMIC(KeysDialog, CDialog)
KeysDialog::KeysDialog(CWnd* pParent /*=NULL*/)
	: CDialog(KeysDialog::IDD, pParent)
{
}

KeysDialog::~KeysDialog()
{
}

void KeysDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(KeysDialog, CDialog)
END_MESSAGE_MAP()


// KeysDialog message handlers
