
// WPicProgDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "WPicProg.h"
#include "WPicProgDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <picprog.h>


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWPicProgDlg dialog



CWPicProgDlg::CWPicProgDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WPICPROG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	AppPath = NULL;
	*lastCOM = 0;
	**files = 0;
}

void CWPicProgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cbComboBox);
	DDX_Control(pDX, IDC_COMBO2, m_FileName);
	DDX_Control(pDX, IDC_BUTTON1, bSelectFile);
	DDX_Control(pDX, IDC_LIST1, m_Messages);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	DDX_Control(pDX, IDC_CHIP_TYPE, m_chiptype);
}

BEGIN_MESSAGE_MAP(CWPicProgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CWPicProgDlg::OnClickedButton1)
//	ON_CBN_EDITCHANGE(IDC_COMBO1, &CWPicProgDlg::OnEditchangeCombo1)
	ON_CBN_SETFOCUS(IDC_COMBO1, &CWPicProgDlg::OnSetfocusCombo1)
//	ON_CBN_SELENDOK(IDC_COMBO1, &CWPicProgDlg::OnSelendokCombo1)
	ON_CBN_DROPDOWN(IDC_COMBO1, &CWPicProgDlg::OnDropdownCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CWPicProgDlg::OnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &CWPicProgDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWPicProgDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CWPicProgDlg::OnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON2, &CWPicProgDlg::OnClickedButton2)
//	ON_LBN_SELCHANGE(IDC_LIST1, &CWPicProgDlg::OnSelchangeList1)
	ON_MESSAGE(WM_MessageMessage, &CWPicProgDlg::OnMessagemessage)
	ON_MESSAGE(WM_CHIPTYPE, &CWPicProgDlg::OnChiptype)
	ON_MESSAGE(WM_PICRANGE, &CWPicProgDlg::OnPicrange)
	ON_MESSAGE(WM_PICPOINT, &CWPicProgDlg::OnPicpoint)
END_MESSAGE_MAP()


// CWPicProgDlg message handlers

BOOL CWPicProgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &AppPath) == S_OK)
	{
		readpars();
		FillList();
	}

	m_Messages.ResetContent();


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWPicProgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWPicProgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWPicProgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWPicProgDlg::OnClickedButton1()
{
	CFileDialog* celf=new CFileDialog(TRUE,NULL , files[m_FileName.GetCount()-1],0,_T("ELF Files (*.elf)|*.elf|All Files (*.*)|*.*||"), this);
	if (celf->DoModal() == IDOK)
	{
		addfile(celf->GetPathName());
	}
}

int CWPicProgDlg::FillList()
{
	m_cbComboBox.ResetContent();
	m_cbComboBox.Clear();
	TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	int ncoms = 0;
	for (int i = 1; i < 255; i++) // checking ports from COM0 to COM255
	{
		CString str;
		str.Format(_T("%d"), i);
		CString ComName = CString("COM") + CString(str); // converting to COM0, COM1, COM2

		DWORD test = QueryDosDevice(ComName, (LPWSTR)lpTargetPath, 5000);

		if (test != 0) m_cbComboBox.InsertString(0,ComName);
	}

	if (lastCOM[0] == 0)
		m_cbComboBox.GetLBText(0, lastCOM);
	int n = m_cbComboBox.FindStringExact(0, lastCOM);
	if (n != CB_ERR) m_cbComboBox.SetCurSel(n);

	return m_cbComboBox.GetCount();

}


int CWPicProgDlg::readpars()
{
	TCHAR* str;
	FILE* f = NULL;
	TCHAR file0[256] = { 0 };
	m_FileName.ResetContent();
	m_FileName.Clear();
	_tcscpy_s(file0, AppPath);
	_tcscat_s(file0, _T("\\WPicProg\\WPicProg.dat"));
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(file0, &FindFileData);
	if (handle != INVALID_HANDLE_VALUE) if (_tfopen_s(&f, file0, _T("r"))) f = NULL;
	if (f == NULL) return 0;
	if ((str = _fgetts(lastCOM, 9, f)) == NULL)
	{
		fclose(f);
		return -1;
	}
	str[_tcslen(str) - 1] = 0;
	for (int i = 0; i < 10; i++)
	{
		if (feof(f))
		{
			files[i][0] = 0;
			break;
		}
		if ((str = _fgetts(files[i], 255, f)) == NULL)
		{
			fclose(f);
			m_FileName.SetCurSel(0);
			return -1;
		}
		str[_tcslen(str) - 1] = 0;
		m_FileName.AddString(files[i]);
	}
	fclose(f);
	m_FileName.SetCurSel(0);
	return m_FileName.GetCount();
}

int CWPicProgDlg::writepars()
{
	FILE* f = NULL;
	TCHAR nl = '\n';
	TCHAR file0[256] = { 0 };
	_tcscpy_s(file0, AppPath);
	_tcscat_s(file0, _T("\\WPicProg"));
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(file0, &FindFileData);
	if (handle == INVALID_HANDLE_VALUE)
	{
		if (!CreateDirectory(file0, NULL)) return -1;
	}
	_tcscpy_s(file0, AppPath);
	_tcscat_s(file0, _T("\\WPicProg\\WPicProg.dat"));
	if (_tfopen_s(&f, file0, _T("w"))) return -1;
	if (f == NULL) return -1;
	if (_fputts(lastCOM, f) < 0) return -1;
	if (_fputtc(nl, f) < 0) return -1;
	for (int i = 0; i<m_FileName.GetCount();i++)
	{
		m_FileName.GetLBText(i, files[i]);
		if (_fputts(files[i], f) < 0) return -1;
		if (_fputtc(nl, f) < 0) return -1;
	}
	fclose(f);
	return m_FileName.GetCount();
}

int CWPicProgDlg::addfile(CString filePath)
{
	int n = 0;
	if ((n=m_FileName.FindStringExact(0,(LPCTSTR)filePath)) != CB_ERR)
	{
		m_FileName.DeleteString(n);
	}
	m_FileName.InsertString(0,(LPCTSTR)filePath);
	m_FileName.SetCurSel(0);
	return m_FileName.GetCount();
}



//void CWPicProgDlg::OnEditchangeCombo1()
//{
//	// TODO: Add your control notification handler code here
//}


void CWPicProgDlg::OnSetfocusCombo1()
{
	FillList();
}


//void CWPicProgDlg::OnSelendokCombo1()
//{
//	// TODO: Add your control notification handler code here
//}


void CWPicProgDlg::OnDropdownCombo1()
{
	FillList();
}


void CWPicProgDlg::OnSelchangeCombo1()
{
	int n = m_cbComboBox.GetCurSel();
	m_cbComboBox.GetLBText(n, lastCOM);
}


void CWPicProgDlg::OnBnClickedOk()
{
	writepars();
	CDialogEx::OnOK();
}


void CWPicProgDlg::OnBnClickedCancel()
{
	writepars();
	CDialogEx::OnCancel();
}


void CWPicProgDlg::OnSelchangeCombo2()
{
	TCHAR file1[256];
	int n = m_FileName.GetCurSel();
	m_FileName.GetLBText(n,file1);
	m_FileName.DeleteString(n);
	m_FileName.InsertString(0,(LPCTSTR)file1);
	m_FileName.SetCurSel(0);
}


void CWPicProgDlg::OnClickedButton2()
{
	m_Messages.ResetContent();
	m_Messages.RedrawWindow();
	picprog* p=new picprog(lastCOM, files[0], this->m_hWnd);
	if (p->proc())
	{
		::MessageBox(NULL, _T("Success"), _T("End of program"), MB_OK | MB_ICONINFORMATION);
	}
	delete p;
}


afx_msg LRESULT CWPicProgDlg::OnMessagemessage(WPARAM wParam, LPARAM lParam)
{
	int n=m_Messages.AddString((LPCTSTR)lParam);
	if (n != LB_ERR && n != LB_ERRSPACE)	m_Messages.SetCurSel(n);
	return 0;
}


afx_msg LRESULT CWPicProgDlg::OnChiptype(WPARAM wParam, LPARAM lParam)
{
	m_chiptype.SetWindowText((LPCTSTR)lParam);
	return 0;
}


afx_msg LRESULT CWPicProgDlg::OnPicrange(WPARAM wParam, LPARAM lParam)
{
	m_Progress.SetRange(0, (short)wParam);
	m_Progress.SetPos(0);
	return 0;
}


afx_msg LRESULT CWPicProgDlg::OnPicpoint(WPARAM wParam, LPARAM lParam)
{
	m_Progress.SetPos((int)wParam);
	return 0;
}
