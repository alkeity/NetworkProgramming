#include<Windows.h>
#include<CommCtrl.h>
#include"resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD CreateMask(DWORD ipAddress);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hUpDown = GetDlgItem(hwnd, IDC_SPIN);
		SendMessage(hUpDown, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_OK:
		{
			HWND hIp = GetDlgItem(hwnd, IDC_IPADDRESS);
			HWND hMask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);
			if (SendMessage(hIp, IPM_ISBLANK, 0, 0))
			{
				SendMessage(hIp, IPM_SETADDRESS, 0, MAKEIPADDRESS(0, 0, 0, 0));
				SendMessage(hMask, IPM_SETADDRESS, 0, MAKEIPADDRESS(0, 0, 0, 0));
			}
			else
			{
				DWORD ipAddress;
				SendMessage(hIp, IPM_GETADDRESS, 0, (LPARAM)(LPWORD)&ipAddress);
				SendMessage(hMask, IPM_SETADDRESS, 0, CreateMask(ipAddress));
			}
			break;
		}
		default: break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
	}
	return FALSE;
}

DWORD CreateMask(DWORD ipAddress)
{
	DWORD ipMask = MAKEIPADDRESS(
		FIRST_IPADDRESS(ipAddress) == 0 ? 0 : 255,
		SECOND_IPADDRESS(ipAddress) == 0 ? 0 : 255,
		THIRD_IPADDRESS(ipAddress) == 0 ? 0 : 255,
		FOURTH_IPADDRESS(ipAddress) == 0 ? 0 : 255);
	return ipMask;
}