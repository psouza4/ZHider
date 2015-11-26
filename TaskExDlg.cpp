
#include "stdafx.h"
#include "TaskExApp.h"
#include "TaskExDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern HWND chWnd;

#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 777)

TaskExDlg::TaskExDlg(CWnd* pParent)
	: CDialog(TaskExDlg::IDD, pParent)
{
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void TaskExDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TaskExDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_TRAY_ICON_NOTIFY_MESSAGE, OnTrayNotify)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL TaskExDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nidIconData.cbSize			= sizeof(NOTIFYICONDATA);
	m_nidIconData.hWnd				= 0;
	m_nidIconData.uID				= 1;
	m_nidIconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
	m_nidIconData.hIcon				= 0;
	m_nidIconData.szTip[0]			= 0;	
	m_nidIconData.uFlags			= NIF_MESSAGE | NIF_ICON;
	m_nidIconData.hIcon = m_hIcon;

	m_nidIconData.hWnd = this->m_hWnd;
	m_nidIconData.uID = 1;

	Shell_NotifyIcon(NIM_ADD,&m_nidIconData);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	DWORD tid;
	CreateThread(NULL, 0, GetTaskManagerThread, this, 0, &tid);
	return TRUE;
}

void TaskExDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR TaskExDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void TaskExDlg::OnBnClickedOk()
{
	if(m_nidIconData.hWnd && m_nidIconData.uID>0)
	{
		Shell_NotifyIcon(NIM_DELETE,&m_nidIconData);
	}

	OnOK();
}

BOOL bInstalled = FALSE;

void TaskExDlg::Install(HWND hWnd, DWORD pid)
{
	m_taskManagers.insert(pid);

	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
									FALSE, pid);	

	if (hProcess != NULL)
	{
		HANDLE hThread;
		char   szLibPath [_MAX_PATH];
		void*  pLibRemote = 0;
		DWORD  hLibModule = 0;

		HMODULE hKernel32 = ::GetModuleHandle("Kernel32");

//		if( !::GetSystemDirectory(szLibPath, _MAX_PATH))
//			return;

//		strcat(szLibPath, "\\InstallTaskHook.dll");
		strcpy(szLibPath, "TaskExHook.dll");

		pLibRemote = ::VirtualAllocEx( hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE );
		
		if( pLibRemote == NULL )
			return;

		::WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath,sizeof(szLibPath),NULL);

//		::MessageBox(NULL, "Attemping DLL injection...", "Installing...", 64);

		hThread = ::CreateRemoteThread( hProcess, NULL, 0,	
						(LPTHREAD_START_ROUTINE) ::GetProcAddress(hKernel32,"LoadLibraryA"), 
						pLibRemote, 0, NULL );

		if( hThread != NULL )
		{
//			::MessageBox(NULL, "Success!", "Installed!", 64);
			::SendMessage(chWnd, 0x7053, (WPARAM)0, (LPARAM)0);
			bInstalled = TRUE;			
			::WaitForSingleObject( hThread, INFINITE );
			::GetExitCodeThread( hThread, &hLibModule );
			::CloseHandle( hThread );
		}
//		else
//		{
//			::MessageBox(NULL, "Error!", "Error", 16);
//		}


	}
}

BOOL CALLBACK TaskExDlg::EnumProc(HWND hWnd, LPARAM p)
{
	char szTitle[24];

	::GetWindowText(hWnd, szTitle, sizeof(szTitle));

	if (strcmp(szTitle, "Windows Task Manager") == 0)
	{
		TaskExDlg *This = (TaskExDlg *)p;
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);

		if (This->m_taskManagers.find(pid) == This->m_taskManagers.end())
		{
			This->Install(hWnd, pid);
		}
	}

	return TRUE;
}

DWORD WINAPI TaskExDlg::GetTaskManagerThread(LPVOID p)
{
	while(true)
	{
		if (bInstalled == FALSE)
		{
			EnumWindows(EnumProc, (LPARAM)p);
			::Sleep(100);
		}
		else
		{
			::Sleep(100);
		}
	}

	return 0;
}

BOOL isVisible = TRUE;

LRESULT TaskExDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam) 
{ 
    UINT uID; 
    UINT uMsg; 
 
    uID = (UINT) wParam; 
    uMsg = (UINT) lParam; 
 
	if (uID != 1)
		return 0;
	
	CPoint pt;	

    switch (uMsg ) 
	{ 
	case WM_LBUTTONDBLCLK:
		ShowWindow(!isVisible);
		isVisible = !isVisible;
		break;
    } 
     return 0; 
 } 


void TaskExDlg::StartItUp(void)
{
	DWORD tid;
	CreateThread(NULL, 0, GetTaskManagerThread, this, 0, &tid);
}
