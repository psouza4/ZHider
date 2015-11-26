// TaskExApp.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource2.h"		// main symbols


// TaskExApp:
// See TaskExApp.cpp for the implementation of this class
//

class TaskExApp : public CWinApp
{
public:
	TaskExApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern TaskExApp theApp;