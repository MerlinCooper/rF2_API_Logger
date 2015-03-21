

#pragma once
#include <resource.h>
#include <afxwin.h>
#include "afxdialogex.h"

// RaceControlDlg dialog

class RaceControlDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RaceControlDlg)

public:
	RaceControlDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~RaceControlDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
