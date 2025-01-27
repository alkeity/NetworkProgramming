#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>

using std::cin;
using std::cout;
using std::endl;

#define DEFAULT_PORT "27015"
#define BUFFER_SIZE 1500

#pragma comment(lib, "Ws2_32.lib")


int __cdecl main()
{
	setlocale(LC_ALL, "");
	// 1. init WinSock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		cout << "WinSock init failed with error " << iResult << endl;
		return 1;
	}

	// 2. create socket
	SOCKET ConnectSocket = INVALID_SOCKET;

	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hInst;

	ZeroMemory(&hInst, sizeof(hInst));
	hInst.ai_family = AF_UNSPEC;
	hInst.ai_socktype = SOCK_STREAM;
	hInst.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hInst, &result);

	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error " << iResult << endl;
		WSACleanup();
		return 1;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET)
		{
			cout << "Socket creation error " << WSAGetLastError() << endl;
			WSACleanup();
			return 1;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Socket error during connection.\n";
			closesocket(ConnectSocket);
			continue;
		}
		break;
	}

	/*SOCKET ConnectSocket = socket(hInst.ai_family, hInst.ai_socktype, hInst.ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		cout << "Socket creation error " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}*/

	// 3. server connection
	/*iResult = connect(ConnectSocket, hInst.ai_addr, hInst.ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Socket error during connection.\n";
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}*/

	freeaddrinfo(result); // not needed anymore

	if (ConnectSocket == INVALID_SOCKET)
	{
		cout << "Unable to connect to server.\n";
		WSACleanup();
		return 1;
	}

	// 4. send / receive data
	//const char sendbuffer[256] = "hello server!!";
	char userbuffer[256]{};
	bool exit = false;
	
	do
	{
		ZeroMemory(userbuffer, sizeof(userbuffer));
		cout << "Enter message: ";
		cin.getline(userbuffer, 256);
		char recvbuffer[BUFFER_SIZE]{};

		iResult = send(ConnectSocket, userbuffer, strlen(userbuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Sending data failed with error " << WSAGetLastError() << endl;
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		if (strcmp(userbuffer, "exit") == 0)
		{
			cout << "exit started with msg " << userbuffer << endl;
			iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR)
			{
				cout << "Shutdown failed with error " << WSAGetLastError() << endl;
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			exit = true;
		}

		int received = 0;

		received = recv(ConnectSocket, recvbuffer, BUFFER_SIZE, 0);

		if (received > 0) cout << "Bytes received: " << received << " Message: " << recvbuffer << endl;
		else if (received == 0) cout << "Connection closed.\n";
		else cout << "Receive failed with error " << WSAGetLastError() << endl;
	} while (!exit);

	// 5. close connection
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown failed with error " << WSAGetLastError() << endl;
	}
	closesocket(ConnectSocket);
	WSACleanup();

	system("PAUSE");

	return 0;
}

