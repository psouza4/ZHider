#include "Compiler Settings.H"

#include <windows.h>
#include <stdio.h>
#include <commctrl.h>

#include "resource.h"
#include "TransparencyClass.H"
#include "Window__List.H"

// Symbols from 'UtilityFunctions.cpp':
extern VOID FlushMessages(HWND);
extern VOID CenterDialog(HWND);

// Symbols from 'TaskManager.cpp':
#ifdef WANT_DLL_INJECTION
extern HWND FindWindowToSendTo(VOID);
extern VOID UpdateTaskManagers_Add(HWND);
extern VOID UpdateTaskManagers_Remove(HWND);
extern VOID UpdateTaskManagers_RemoveAll(VOID);
extern VOID SynchronizeTaskManager(VOID);
#endif

// Global data:
BOOL						bHiddenOrNot = FALSE;
struct handle_list_tag *	list_of_handles = NULL;

#ifdef WANT_TRANSPARENCY
class HRT timer;
BOOL bMoving = FALSE;
time_t lastMoveUpdate = 0;
#endif

VOID RemoveSpecificWindow(HWND hWnd)
{
	struct handle_list_tag *temp, *temp2, *prev;

	if (!list_of_handles)
		return;

	if (list_of_handles->hWindow == hWnd)
	{
		temp = list_of_handles->next;
		free(list_of_handles);
		list_of_handles = temp;
#ifdef WANT_DLL_INJECTION
		UpdateTaskManagers_Remove(hWnd);
#endif
		return;
	}

	for (temp = list_of_handles; temp; temp = temp->next)
	{
		if (temp->hWindow == hWnd)
		{
			temp2 = temp->next;
			free(temp);
			prev->next = temp2;
#ifdef WANT_DLL_INJECTION
			UpdateTaskManagers_Remove(hWnd);
#endif
			return;
		}

		prev = temp;
	}

	return;
}

VOID RemoveDeadWindows(VOID)
{
	struct handle_list_tag *temp;

start_over:
	if (list_of_handles)
	{
		for (temp = list_of_handles; temp; temp = temp->next)
		{
			if (IsWindow(temp->hWindow) == FALSE)
			{
				RemoveSpecificWindow(temp->hWindow);
				goto start_over;
			}
		}
	}

#ifdef WANT_DLL_INJECTION
	SynchronizeTaskManager( );
#endif
}

VOID HideAllWindows(VOID)
{
	struct handle_list_tag *temp;

	if (!list_of_handles)
		return;

	for (temp = list_of_handles; temp; temp = temp->next)
	{
		ShowWindow(temp->hWindow, SW_HIDE);
#ifdef WANT_DLL_INJECTION
		UpdateTaskManagers_Add(temp->hWindow);
#endif
	}

	bHiddenOrNot = TRUE;

	return;
}

VOID UnHideAllWindows(VOID)
{
	struct handle_list_tag *temp;

	if (!list_of_handles)
		return;

	for (temp = list_of_handles; temp; temp = temp->next)
	{
		ShowWindow(temp->hWindow, SW_SHOWNA);
#ifdef WANT_DLL_INJECTION
		UpdateTaskManagers_Remove(temp->hWindow);
#endif
	}

	bHiddenOrNot = FALSE;

	return;
}

VOID RemoveAllWindows(VOID)
{
	struct handle_list_tag *temp;

here:
	if (list_of_handles)
	{
		temp = list_of_handles->next;
		ShowWindow(list_of_handles->hWindow, SW_SHOWNA);
		free(list_of_handles);
		list_of_handles = temp;
		goto here;
	}

#ifdef WANT_DLL_INJECTION
	UpdateTaskManagers_RemoveAll( );
#endif
	return;
}

VOID HideThisWindow(HWND hWindow)
{
	struct handle_list_tag *temp;

#ifdef WANT_DLL_INJECTION
	UpdateTaskManagers_Add(hWindow);
#endif

	RemoveSpecificWindow(hWindow);
	ShowWindow(hWindow, SW_HIDE);

	if (list_of_handles)
	{
		temp = (struct handle_list_tag *)malloc(sizeof(struct handle_list_tag));
		temp->next = list_of_handles;
		temp->hWindow = hWindow;
		list_of_handles = temp;
	}
	else
	{
		list_of_handles = (struct handle_list_tag *)malloc(sizeof(struct handle_list_tag));
		list_of_handles->next = NULL;
		list_of_handles->hWindow = hWindow;
	}

	bHiddenOrNot = TRUE;

	return;
}

VOID HideThisWindow(VOID)
{
	HWND hWindow = GetForegroundWindow( );
	struct handle_list_tag *temp;

	if (hWindow == NULL)
		return;

#ifdef WANT_DLL_INJECTION
	UpdateTaskManagers_Add(hWindow);
#endif

	RemoveSpecificWindow(hWindow);
	ShowWindow(hWindow, SW_HIDE);

	if (list_of_handles)
	{
		temp = (struct handle_list_tag *)malloc(sizeof(struct handle_list_tag));
		temp->next = list_of_handles;
		temp->hWindow = hWindow;
		list_of_handles = temp;
	}
	else
	{
		list_of_handles = (struct handle_list_tag *)malloc(sizeof(struct handle_list_tag));
		list_of_handles->next = NULL;
		list_of_handles->hWindow = hWindow;
	}

	bHiddenOrNot = TRUE;

	return;
}

DWORD dwWhichOneIsSelected(HWND hWndListView)
{
	signed int uCount;

	uCount = (signed int)SendMessage(hWndListView, LVM_GETITEMCOUNT, 0, 0) - 1;

	while (uCount >= 0)
	{
		if (SendMessage(hWndListView, LVM_GETITEMSTATE, (WPARAM)uCount, (LPARAM)LVIS_SELECTED) == LVIS_SELECTED)
			return uCount;

		uCount--;
	}

	return 0;
}

VOID SetCurrentSelection(HWND hWndListView, DWORD dwNewIndex)
{
	LV_ITEM litem;
	if (SendMessage(hWndListView, LVM_GETITEM, (WPARAM)dwNewIndex, (LPARAM)&litem) == FALSE)
		return;
	litem.state |= LVIS_SELECTED;
	litem.stateMask = LVIS_SELECTED;
	SendMessage(hWndListView, LVM_SETITEMSTATE, (WPARAM)dwNewIndex, (LPARAM)&litem);
	return;
}

BOOL bIsWindowVisibleByByIndex(DWORD dwWhich)
{
	struct handle_list_tag *temp;

	if (dwWhich == -1)
		return FALSE;

	if (list_of_handles)
	{
		dwWhich++;

		for (temp = list_of_handles; temp; temp = temp->next)
		{
			if (--dwWhich == 0)
			{
				return (GetWindowLong(temp->hWindow, GWL_STYLE) & WS_VISIBLE) ? TRUE : FALSE;
			}
		}
	}

	return FALSE;
}

VOID UnhideSpecificByIndex(HWND hWnd)
{
	DWORD dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
	struct handle_list_tag *temp;

	if (dwWhich == -1)
		return;

	if (list_of_handles)
	{
		dwWhich++;

		for (temp = list_of_handles; temp; temp = temp->next)
		{
			if (--dwWhich == 0)
			{
				ShowWindow(temp->hWindow, SW_SHOWNA);
#ifdef WANT_DLL_INJECTION
				UpdateTaskManagers_Remove(temp->hWindow);
#endif
				return;
			}
		}
	}

	return;
}

VOID CloseSpecificByIndex(HWND hWnd)
{
	DWORD dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
	struct handle_list_tag *temp;

	if (dwWhich == -1)
		return;

	if (list_of_handles)
	{
		dwWhich++;

		for (temp = list_of_handles; temp; temp = temp->next)
		{
			if (--dwWhich == 0)
			{
#ifdef WANT_DLL_INJECTION
				UpdateTaskManagers_Remove(temp->hWindow);
#endif
				CloseWindow(temp->hWindow);
				::Sleep(150);
				DestroyWindow(temp->hWindow);
				::Sleep(50);
				return;
			}
		}
	}

	return;
}

VOID HideSpecificByIndex(HWND hWnd)
{
	DWORD dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
	struct handle_list_tag *temp;

	if (dwWhich == -1)
		return;

	if (list_of_handles)
	{
		dwWhich++;

		for (temp = list_of_handles; temp; temp = temp->next)
		{
			if (--dwWhich == 0)
			{
#ifdef WANT_DLL_INJECTION
				UpdateTaskManagers_Add(temp->hWindow);
#endif
				ShowWindow(temp->hWindow, SW_HIDE);
				return;
			}
		}
	}

	return;
}

VOID RemoveSpecificByIndex(HWND hWnd)
{
	DWORD dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
	struct handle_list_tag *temp;

	if (dwWhich == -1)
		return;

	if (list_of_handles)
	{
		dwWhich++;

		for (temp = list_of_handles; temp; temp = temp->next)
		{
			if (--dwWhich == 0)
			{
				RemoveSpecificWindow(temp->hWindow);
				return;
			}
		}
	}

	return;
}

VOID AddDataToListView(HWND hWndListView, LPTSTR szWindowValue, LPTSTR szHiddenState, LPTSTR szName, LPTSTR szClass)
{
	UINT uCount;
	LV_ITEM litem;

	uCount = SendMessage(hWndListView, LVM_GETITEMCOUNT, 0, 0);

	litem.mask = LVIF_TEXT;
	litem.iItem = uCount;
	litem.iSubItem = 0;
	litem.pszText = szWindowValue;
	litem.state = 0;
	litem.stateMask = 0;
	litem.cchTextMax = 0;
	litem.iImage = 0;
	litem.lParam = 0;
	SendMessage(hWndListView, LVM_INSERTITEM, (WPARAM)uCount, (LPARAM)&litem);

	litem.iSubItem = 1;
	litem.pszText = szHiddenState;
	SendMessage(hWndListView, LVM_SETITEM, (WPARAM)uCount, (LPARAM)&litem);

	litem.iSubItem = 2;
	litem.pszText = szName;
	SendMessage(hWndListView, LVM_SETITEM, (WPARAM)uCount, (LPARAM)&litem);

	litem.iSubItem = 3;
	litem.pszText = szClass;
	SendMessage(hWndListView, LVM_SETITEM, (WPARAM)uCount, (LPARAM)&litem);
}

VOID ShowList(HWND hWnd)
{
	struct handle_list_tag *temp;
	CHAR szTemp1[1024];
	CHAR szTemp2[1024];
	CHAR szTemp3[1024];
	CHAR szTemp4[1024];

	RemoveDeadWindows( );

	EnableWindow(hWnd, FALSE);
	SendMessage(hWnd, LVM_DELETEALLITEMS, 0, 0);
	AddDataToListView(hWnd, "", "", "", "");
	AddDataToListView(hWnd, "", "Refreshing...", "", "");
	FlushMessages(GetParent(hWnd));
	SendMessage(hWnd, LVM_DELETEALLITEMS, 0, 0);

	if (list_of_handles)
	{
		for (temp = list_of_handles; temp; temp = temp->next)
		{
			sprintf(szTemp1, "%08x", (DWORD)temp->hWindow);
			sprintf(szTemp2, "%s", (GetWindowLong(temp->hWindow, GWL_STYLE) & WS_VISIBLE) ? "visible" : "hidden");
			memset(szTemp3, 0, 1024); SendMessage(temp->hWindow, WM_GETTEXT, (WPARAM)800, (LPARAM)szTemp3);
			memset(szTemp4, 0, 1024); GetClassName(temp->hWindow, szTemp4, 800);
			AddDataToListView(hWnd, szTemp1, szTemp2, szTemp3, szTemp4);
		}
	}


	EnableWindow(hWnd, TRUE);
	return;
}

BOOL CALLBACK ListCBProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD dwWhich;
	LV_COLUMN ldata;

	switch (message)
	{
		case WM_NOTIFY:
			if (GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2) == ((NMHDR FAR *)lParam)->hwndFrom)
			{
				if (((NMHDR FAR *)lParam)->code == NM_DBLCLK)
				{
					if (list_of_handles != NULL)
					{
						dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
						if (bIsWindowVisibleByByIndex(dwWhich) == TRUE)
							HideSpecificByIndex(hWnd);
						else
							UnhideSpecificByIndex(hWnd);
						ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
						SetCurrentSelection(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), dwWhich);
					}
				}
			}
			break;

		case WM_DESTROY:
		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_CLOSE_CLOSEWINDOWSILENTLY:
					CloseSpecificByIndex(hWnd);
					RemoveSpecificByIndex(hWnd);
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					break;

				case ID_UNHIDE_UNHIDE:
					dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					UnhideSpecificByIndex(hWnd);
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					SetCurrentSelection(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), dwWhich);
					break;

				case ID_UNHIDE_UNHIDEALL:
					dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					UnHideAllWindows( );
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					SetCurrentSelection(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), dwWhich);
					break;

				case ID_UNHIDE_UNHIDEANDREMOVE:
					UnhideSpecificByIndex(hWnd);
					RemoveSpecificByIndex(hWnd);
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					break;

				case ID_UNHIDE_UNHIDEANDREMOVEALL:
					UnHideAllWindows( );
					RemoveAllWindows( );
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					break;

				case ID_HIDE_HIDE:
					dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					HideSpecificByIndex(hWnd);
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					SetCurrentSelection(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), dwWhich);
					break;

				case ID_HIDE_HIDEALL:
					dwWhich = dwWhichOneIsSelected(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					HideAllWindows( );
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					SetCurrentSelection(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), dwWhich);
					break;

				case ID_REMOVE_REMOVE:
					UnhideSpecificByIndex(hWnd);
					RemoveSpecificByIndex(hWnd);
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					break;

				case ID_REMOVE_REMOVEALL:
					UnHideAllWindows( );
					RemoveAllWindows( );
					ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));
					break;

				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd, TRUE);
					break;
			}
			break;
#ifdef WANT_TRANSPARENCY
		case WM_TIMER:
			if (timer.end( ) > 100)
			{
				bMoving = FALSE;
				trWindow::trWindow(hWnd).trans(100);
				KillTimer(hWnd, 43505);
			}
			break;

		case WM_MOVING:
			trWindow::trWindow(hWnd).trans(50);
			timer.start( );
			if (bMoving == FALSE)
			{
				bMoving = TRUE;
				SetTimer(hWnd, 43505, 100, NULL);
			}
			break;
#endif
		case WM_INITDIALOG:
			CenterDialog(hWnd);
			SetFocus(GetDlgItem(hWnd, IDOK));

			ListView_SetExtendedListViewStyle(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

			ldata.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
			ldata.fmt = LVCFMT_LEFT;
			ldata.cx = 70;
			ldata.pszText = "Window ID";
			SendMessage(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&ldata);
			ldata.cchTextMax = 0;
			ldata.iSubItem = 0;

			ldata.cx = 54;
			ldata.pszText = "Visibility";
			SendMessage(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), LVM_INSERTCOLUMN, (WPARAM)1, (LPARAM)&ldata);

			ldata.cx = 252;
			ldata.pszText = "Name";
			SendMessage(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), LVM_INSERTCOLUMN, (WPARAM)2, (LPARAM)&ldata);

			ldata.cx = 80;
			ldata.pszText = "Class";
			SendMessage(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2), LVM_INSERTCOLUMN, (WPARAM)3, (LPARAM)&ldata);

			ShowList(GetDlgItem(hWnd, IDC_HIDDEN_WINDOWS2));

			break;
	}

	return (FALSE);
}
