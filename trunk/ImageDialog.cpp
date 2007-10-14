// ImageDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "ImageDialog.h"
#include "MainFrm.h"


// ImageDialog dialog

IMPLEMENT_DYNAMIC(ImageDialog, CDialog)
ImageDialog::ImageDialog(PNGWriter* png, AVIWriter* avi, bool& saveall, bool& s_png, bool& s_avi, CWnd* pParent /* = NULL */)
 : CDialog(ImageDialog::IDD, pParent), aviwriter(avi), pngwriter(png), save(saveall), savePng(s_png), saveAvi(s_avi)
{
}

ImageDialog::~ImageDialog()
{
}

void ImageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, filePath);
	DDX_Control(pDX, IDC_EDIT2, fileNum);
	DDX_Control(pDX, IDC_CHECK1, saveAll);
	DDX_Control(pDX, IDC_CHECK2, dumpAscii);
	DDX_Control(pDX, IDC_AVIFILEEDIT, AVIFilename);
	DDX_Control(pDX, IDC_EDITFPS, FPS);
	DDX_Control(pDX, IDC_DIMCOMBO, AVIDims);
	DDX_Control(pDX, IDC_BUTTONCOMP, compressionButton);
	DDX_Control(pDX, IDC_CHECKPNG, savePNG);
	DDX_Control(pDX, IDC_CHECKAVI, saveAVI);
}


BEGIN_MESSAGE_MAP(ImageDialog, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_BROWSE2, OnBnClickedBrowseAVI)
	ON_CBN_SELCHANGE(IDC_DIMCOMBO, OnCbnSelchangeDimcombo)
	ON_BN_CLICKED(IDC_BUTTONCOMP, OnBnClickedCompression)
END_MESSAGE_MAP()


void ImageDialog::OnOK()
{
	char buffer[MAX_PATH];
	if (saveAll.GetCheck())
		save = true;
	else
		save = false;

	if (savePNG.GetCheck())
		savePng = true;
	else
		savePng = false;

	if (saveAVI.GetCheck())
		saveAvi = true;
	else
		saveAvi = false;

	if (dumpAscii.GetCheck())
		pngwriter->SetAscii(true);
	else
		pngwriter->SetAscii(false);

	fileNum.GetWindowText(buffer, MAX_PATH);
	pngwriter->SetFileNumber( atoi(buffer) );

	filePath.GetWindowText(buffer, MAX_PATH);
	path = "";
	path.assign(buffer);

	pngwriter->SetFileName( path );

	AfxGetApp()->WriteProfileString("Last Config", "lastSave", buffer);

	if (aviwriter->FileOpen()) // modify AVI settings (FPS/fname) if a valid file exists
	{
		SetAviOptions();
		if (!aviwriter->FileOpen())
			aviwriter->OpenFile();

		window_size* newsize = new window_size;
		newsize->w = aviwriter->GetWidth();
		newsize->h = aviwriter->GetHeight();
		GetParent()->PostMessage(UWM_CHANGESIZE, 0, (LPARAM)newsize);
	}
	CDialog::OnOK();
}

BOOL ImageDialog::OnInitDialog()
{
	char buffer[MAX_PATH];
	CDialog::OnInitDialog();

	AVIDims.AddString("320 x 240");
	AVIDims.AddString("320 x 480");
	AVIDims.AddString("640 x 480");
	AVIDims.AddString("800 x 600");
	AVIDims.AddString("1024 x 768");

	// PNG Options
	sprintf(buffer, "%d", pngwriter->GetFileNumber());
	fileNum.SetWindowText(buffer);
	filePath.SetWindowText(pngwriter->GetFileName().c_str());

	if (pngwriter->GetAscii())
		dumpAscii.SetCheck(1);
	else
		dumpAscii.SetCheck(0);


	// AVI Options

	sprintf(buffer, "%d", aviwriter->GetFPS());
	FPS.SetWindowText(buffer);

	if (aviwriter->FileOpen())
		AVIFilename.SetWindowText(aviwriter->GetFileName().c_str());
	else
		AVIFilename.SetWindowText("No current file");

	int wdim = aviwriter->GetWidth();
	switch (wdim)
	{
	case 320:
		if (aviwriter->GetHeight()==240)
			AVIDims.SelectString(-1, "320 x 240");
		if (aviwriter->GetHeight()==480)
			AVIDims.SelectString(-1, "320 x 480");
		break;
	case 640:
		AVIDims.SelectString(-1, "640 x 480");
		break;
	case 800:
		AVIDims.SelectString(-1, "800 x 600");
		break;
	case 1024:
		AVIDims.SelectString(-1, "1024 x 768");
		break;
	}

	if (aviwriter->FileOpen())
		compressionButton.EnableWindow(TRUE);
	else
		compressionButton.EnableWindow(FALSE);

	// Global options

	if (save)
		saveAll.SetCheck(1);
	else
		saveAll.SetCheck(0);

	if (savePng)
		savePNG.SetCheck(BST_CHECKED);
	else
		savePNG.SetCheck(BST_UNCHECKED);

	if (saveAvi)
		saveAVI.SetCheck(BST_CHECKED);
	else
		saveAVI.SetCheck(BST_UNCHECKED);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void ImageDialog::OnBnClickedBrowse()
{
	CWinApp* theApp = AfxGetApp();

	char buffer[MAX_PATH];
	string lastSave;
	lastSave = theApp->GetProfileString("Last Config", "lastSave");
	int trim = lastSave.rfind("\\");
	lastSave = lastSave.substr(0, trim);

	buffer[0] = '\0';
	OPENFILENAME browse;
	ZeroMemory(&browse, sizeof(browse));
	browse.lStructSize = sizeof(browse);
	browse.hwndOwner = this->m_hWnd;	
	browse.lpstrFile = buffer;
	browse.nMaxFile = sizeof(buffer);
	browse.lpstrFilter = "All Files\0*.*\0";
	browse.nFilterIndex = 1;
	browse.lpstrTitle = "Select Save Path/Filename";
	browse.lpstrFileTitle = 0;
	browse.lpstrInitialDir = lastSave.c_str();
	browse.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&browse) != 0)
	{
		string file(buffer);
		pngwriter->SetFileName(file);
		filePath.SetWindowText(pngwriter->GetFileName().c_str());
	}
}

void ImageDialog::SetAviOptions()
{
	char buffer[MAX_PATH];
	FPS.GetWindowText(buffer, MAX_PATH);
	aviwriter->SetFrameRate(atoi(buffer));

	AVIFilename.GetWindowText(buffer, MAX_PATH);
	string file(buffer);
	aviwriter->SetFileName(file);
}

void ImageDialog::OnBnClickedBrowseAVI()
{
	CWinApp* theApp = AfxGetApp();

	char buffer[MAX_PATH];
	string lastSave;
	lastSave = theApp->GetProfileString("Last Config", "lastSave");
	int trim = lastSave.rfind("\\");
	lastSave = lastSave.substr(0, trim);

	buffer[0] = '\0';
	OPENFILENAME browse;
	ZeroMemory(&browse, sizeof(browse));
	browse.lStructSize = sizeof(browse);
	browse.hwndOwner = this->m_hWnd;	
	browse.lpstrFile = buffer;
	browse.nMaxFile = sizeof(buffer);
	browse.lpstrFilter = "AVI Files\0*.avi\0";
	browse.nFilterIndex = 1;
	browse.lpstrTitle = "Save to AVI File";
	browse.lpstrFileTitle = 0;
	browse.lpstrInitialDir = lastSave.c_str();
	browse.lpstrDefExt = "avi";
	browse.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&browse) != 0)
	{
		AVIFilename.SetWindowText(buffer);
		SetAviOptions();
		aviwriter->OpenFile();
		compressionButton.EnableWindow(TRUE);
	}
}

void ImageDialog::OnCbnSelchangeDimcombo()
{
	int selection = AVIDims.GetCurSel();
	switch (selection)
	{
	case 0:
		aviwriter->SetSize(320, 240);
		break;
	case 1:
		aviwriter->SetSize(320, 480);
		break;
	case 2:
		aviwriter->SetSize(640, 480);
		break;
	case 3:
		aviwriter->SetSize(800, 600);
		break;
	case 4:
		aviwriter->SetSize(1024, 768);
		break;
	case LB_ERR:
		// crap
		break;
	}
	aviwriter->CloseFile();
	AVIFilename.SetWindowText("No current file");
}

void ImageDialog::OnBnClickedCompression()
{
	if (aviwriter->FileOpen())
	{
		aviwriter->SetCompression();
	}
}
