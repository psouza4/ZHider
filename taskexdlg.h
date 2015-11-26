// TaskExDlg.h : header file
//

#pragma once

#include <set>
using std::set;

// TaskExDlg dialog
class TaskExDlg : public CDialog
{
// Construction
public:
	TaskExDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TASKEXAPP_DIALOG };

	void StartItUp(void);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA	m_nidIconData;
	set<DWORD> m_taskManagers;

	static DWORD WINAPI GetTaskManagerThread(LPVOID p);
	static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM p);

	void Install(HWND hWnd, DWORD pid);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
