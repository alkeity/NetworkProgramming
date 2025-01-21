#include<Windows.h>
#include<CommCtrl.h>
#include<iostream>
#include<format>
#include<string>
#include"resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcSubnets(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID InitLVColumn(LVCOLUMN column, LPSTR text, INT subitem);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CONST INT SIZE = 256;
	DWORD ipAddress, ipMask = 0;
	CHAR sz_buffer[SIZE];
	HWND hIp = GetDlgItem(hwnd, IDC_IPADDRESS);
	HWND hMask = GetDlgItem(hwnd, IDC_IPADDRESS_MASK);
	HWND hPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hUpDown = GetDlgItem(hwnd, IDC_SPIN);
		SendMessage(hUpDown, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
		SendMessage(hIp, IPM_SETADDRESS, 0, 0xc0a864c8);
		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);
	}
	break;
	case WM_COMMAND:
	{
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
			break;
		}
		case IDC_BUTTON_SUBNETS:
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_SUBNETS), hwnd, (DLGPROC)DlgProcSubnets, 0);
			break;
		case ID_OK:
		{
			SendMessage(hIp, IPM_GETADDRESS, 0, (LPARAM)&ipAddress);
			SendMessage(hMask, IPM_GETADDRESS, 0, (LPARAM)&ipMask);
			SendMessage(hPrefix, WM_GETTEXT, SIZE, (LPARAM)sz_buffer);
			DWORD dwNetworkAddress = ipAddress & ipMask;
			DWORD dwBroadcastAddress = dwNetworkAddress | ~ipMask;
			int hosts = pow(2, 32 - atoi(sz_buffer)) - 2;

			std::string sz_info = std::format(
				"Network address: {0}.{1}.{2}.{3}\nBroadcast address: {0}.{1}.{2}.{4}\nMax number of hosts: {5}",
				FIRST_IPADDRESS(dwNetworkAddress), SECOND_IPADDRESS(dwNetworkAddress), THIRD_IPADDRESS(dwNetworkAddress),
				FOURTH_IPADDRESS(dwNetworkAddress), FOURTH_IPADDRESS(dwBroadcastAddress), hosts
			);
			SendMessage(GetDlgItem(hwnd, IDC_STATIC_INFO_TEXT), WM_SETTEXT, 0, (LPARAM)sz_info.c_str());
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

BOOL CALLBACK DlgProcSubnets(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hList = GetDlgItem(hwnd, IDC_LIST_SUBNETS);
	static LVCOLUMN lvcNetworkAddress;
	static LVCOLUMN lvcBroadcastAddress;
	static LVCOLUMN lvcNumberOfAddresses;
	static LVCOLUMN lvcNumberOfHosts;
	/*InitLVColumn(&lvcNetworkAddress, (LPSTR)"Network Address");
	InitLVColumn(&lvcBroadcastAddress, (LPSTR)"Broadcast Address");
	InitLVColumn(&lvcNumberOfAddresses, (LPSTR)"Number of IP addresses");
	InitLVColumn(&lvcNumberOfHosts, (LPSTR)"Number of hosts");*/

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvcNetworkAddress);
		SendMessage(hList, LVM_INSERTCOLUMN, 1, (LPARAM)&lvcBroadcastAddress);
		SendMessage(hList, LVM_INSERTCOLUMN, 2, (LPARAM)&lvcNumberOfAddresses);
		SendMessage(hList, LVM_INSERTCOLUMN, 3, (LPARAM)&lvcNumberOfHosts);
	break;
	case WM_COMMAND:
	break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
	}
	return FALSE;
}

VOID InitLVColumn(LPLVCOLUMN column, LPSTR text, INT subitem)
{
	ZeroMemory(column, sizeof(LVCOLUMN));
	column->mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column->cx = 50;
	column->pszText = text;
	column->iSubItem = subitem;
	column->fmt = LVCFMT_LEFT;
}
