
// WPicProgDlg.h : header file
//
#pragma once

#define WM_MessageMessage	WM_USER+1
#define WM_CHIPTYPE			WM_USER+2
#define WM_PICRANGE			WM_USER+3
#define WM_PICPOINT			WM_USER+4

// CWPicProgDlg dialog
class CWPicProgDlg : public CDialogEx
{
// Construction
public:
	CWPicProgDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WPICPROG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_FileName;
	CComboBox m_cbComboBox;
	CButton bSelectFile;
	CListBox m_Messages;
	CProgressCtrl m_Progress;
	CEdit uid[4];
private:
	TCHAR lastCOM[10];
	TCHAR uid_str[4][128];
	unsigned long uid_l[4];
	int uid_err[4];
	TCHAR files[10][256];
	TCHAR* AppPath;
	int FillList();
	int readpars();
	int writepars();
	int addfile(CString filePath);
	void FillEdit(void);
public:
//	afx_msg void OnEditchangeCombo1();
	afx_msg void OnSetfocusCombo1();
//	afx_msg void OnSelendokCombo1();
	afx_msg void OnDropdownCombo1();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSelchangeCombo2();
	afx_msg void OnClickedButton1();
	afx_msg void OnClickedButton2();
	afx_msg void OnEditChange1();
	afx_msg void OnEditChange2();
	afx_msg void OnEditChange3();
	afx_msg void OnEditChange4();
	afx_msg void OnEditChange(int edit_num);
	//	void PutMessage(LPCTSTR message);
//	afx_msg void OnSelchangeList1();
protected:
	afx_msg LRESULT OnMessagemessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChiptype(WPARAM wParam, LPARAM lParam);
public:
	CStatic m_chiptype;
protected:
	afx_msg LRESULT OnPicrange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPicpoint(WPARAM wParam, LPARAM lParam);
};
