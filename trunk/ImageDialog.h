#pragma once
#include "afxwin.h"
#include <string>
#include "AVIWriter.h"
#include "PNGWriter.h"

using namespace std;


// ImageDialog dialog

class ImageDialog : public CDialog
{
	DECLARE_DYNAMIC(ImageDialog)

public:
	ImageDialog(PNGWriter* png, AVIWriter* avi, bool& saveall, bool& s_png, bool& s_avi, CWnd* pParent = NULL);
	~ImageDialog();

// Dialog Data
	enum { IDD = IDD_DIALOGIMAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	DECLARE_MESSAGE_MAP()
public:
	CEdit filePath;
	CEdit fileNum;
	CButton saveAll;
protected:
	virtual void OnOK();
	bool &save;
	bool &saveAvi;
	bool &savePng;
	//bool &ascii;
	//int &num;
	string path;

	PNGWriter* pngwriter;
	AVIWriter* aviwriter;

public:
	virtual BOOL OnInitDialog();
	void SetAviOptions();
	afx_msg void OnBnClickedBrowse();
	CButton dumpAscii;
	CEdit AVIFilename;
	afx_msg void OnBnClickedBrowseAVI();
	CEdit FPS;
	CComboBox AVIDims;
	CButton compressionButton;
	afx_msg void OnCbnSelchangeDimcombo();
	afx_msg void OnBnClickedCompression();
	CButton savePNG;
	CButton saveAVI;
};