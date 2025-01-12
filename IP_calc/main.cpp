#include<Windows.h>
#include<CommCtrl.h>
#include<iostream>
#include"resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CONST INT SIZE = 256;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hUpDown = GetDlgItem(hwnd, IDC_SPIN);
		SendMessage(hUpDown, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);
	}
	break;
	case WM_COMMAND:
	{
		DWORD ipAddress, ipMask = 0;
		CHAR sz_buffer[SIZE];
		HWND hIp = GetDlgItem(hwnd, IDC_IPADDRESS);
		HWND hMask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);
		HWND hPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIp, IPM_GETADDRESS, 0, (LPARAM)&ipAddress);
				BYTE first = FIRST_IPADDRESS(ipAddress);
				if (first < 128) ipMask = MAKEIPADDRESS(255, 0, 0, 0);
				else if (first < 192) ipMask = MAKEIPADDRESS(255, 255, 0, 0);
				else if (first < 256) ipMask = MAKEIPADDRESS(255, 255, 255, 0);
				SendMessage(hMask, IPM_SETADDRESS, 0, ipMask);
				int count = 1;
				for (; ipMask <<= 1; count++);
				_itoa_s(count, sz_buffer, 10);
				SendMessage(hPrefix, WM_SETTEXT, 0, (LPARAM)sz_buffer);
			}
			break;
		case IDC_IPADDRESS_MASK:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hMask, IPM_GETADDRESS, 0, (LPARAM)&ipMask);
				int count = 1;
				for (; ipMask <<= 1; count++);
				_itoa_s(count, sz_buffer, 10);
				SendMessage(hPrefix, WM_SETTEXT, 0, (LPARAM)sz_buffer);
			}
			break;
		case IDC_EDIT_PREFIX:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hPrefix, WM_GETTEXT, SIZE, (LPARAM)sz_buffer);
				UINT dwMask = UINT_MAX;
				DWORD dwPrefix = atoi(sz_buffer);
				dwMask <<= (32 - dwPrefix);
				if (dwPrefix != 0) SendMessage(hMask, IPM_SETADDRESS, 0, dwMask);
			}
		}
		case ID_OK:
		{
			CHAR sz_info[SIZE] = "Здесь будет информация о сети";
			SendMessage(GetDlgItem(hwnd, IDC_STATIC_INFO_TEXT), WM_SETTEXT, 0, (LPARAM)sz_info);
			break;
		}
		case ID_CANCEL: EndDialog(hwnd, 0); break;
		default: break;
		}
	}
		break;
	case WM_CLOSE:
		//FreeConsole();
		EndDialog(hwnd, 0);
	}
	return FALSE;
}
