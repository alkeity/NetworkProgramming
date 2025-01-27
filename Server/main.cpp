#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>

using std::cin;
using std::cout;
using std::endl;

#define DEFAULT_PORT "27015"
#define BUFFER_SIZE 1500

#pragma comment(lib, "Ws2_32.lib")

void HandleConnections(SOCKET ListenSocket);

union ClientSocketData
{
	SOCKADDR clientSocket;

	ClientSocketData(SOCKADDR clientSocket)
	{
		this->clientSocket = clientSocket;
	}

	char* getClientAddress(char* szClientName)
	{
		char szClientPort[BUFFER_SIZE]{};
		switch (clientSocket.sa_family) // check for IPv4 or IPv6
		{
		case AF_INET:
		{
			SOCKADDR_IN* clientInfo = reinterpret_cast<SOCKADDR_IN*>(&clientSocket);
			inet_ntop(AF_INET, &clientInfo->sin_addr, szClientName, INET6_ADDRSTRLEN);
			_itoa_s(clientInfo->sin_port, szClientPort, 10);
			break;
		}
		case AF_INET6:
		{
			SOCKADDR_IN6* clientInfo = reinterpret_cast<SOCKADDR_IN6*>(&clientSocket);
			inet_ntop(AF_INET, &clientInfo->sin6_addr, szClientName, INET6_ADDRSTRLEN);
			_itoa_s(clientInfo->sin6_port, szClientPort, 10);
			break;
		}
		default:
			break;
		}
		strcat_s(szClientName, BUFFER_SIZE, ":");
		strcat_s(szClientName, BUFFER_SIZE, szClientPort);
		return szClientName;
	}
};

void main()
{
	setlocale(LC_ALL, "");
	// 1. init WinSock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (iResult != 0)
	{
		cout << "WinSock init failed with error " << iResult << endl;
		return;
	}

	// 2. create socket
	addrinfo* result = NULL;
	addrinfo hInst;

	// get socket address
	ZeroMemory(&hInst, sizeof(hInst));
	hInst.ai_family = AF_INET;
	hInst.ai_socktype = SOCK_STREAM;
	hInst.ai_protocol = IPPROTO_TCP;
	hInst.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hInst, &result);

	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error " << iResult << endl;
		WSACleanup();
		return;
	}

	// create socket
	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "Socket error " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	cout << "Server started on TCP port " << DEFAULT_PORT << endl;

	// 3. bind socket to port
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Bind failed with error " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	freeaddrinfo(result); // not needed anymore

	// 4. listen
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Listen failed with error " << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleConnections, &ListenSocket, 0, 0);
	HandleConnections(ListenSocket);

	WSACleanup();
	system("PAUSE");
}

void HandleConnections(SOCKET ListenSocket)
{
	while (true)
	{
		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			cout << "Accept failed with error " << WSAGetLastError() << endl;
			//closesocket(ListenSocket);
			//WSACleanup();
			return;
		}

		SOCKADDR clientSocket;
		int namelen = sizeof(clientSocket);
		char szClientName[INET6_ADDRSTRLEN]{};
		ZeroMemory(&clientSocket, sizeof(clientSocket));
		//getsockname(ClientSocket, &clientSocket, &namelen);
		getpeername(ClientSocket, &clientSocket, &namelen);

		cout << "Client data: " << ClientSocketData(clientSocket).getClientAddress(szClientName) << endl;

		// 6. Receive / send data
		char recvbuffer[BUFFER_SIZE]{};
		int received = 0;

		do
		{
			received = recv(ClientSocket, recvbuffer, BUFFER_SIZE, 0);
			cout << received << endl;
			if (received > 0)
			{
				cout << "Bytes received from " << szClientName << ": " << received << " Message: " << recvbuffer << endl;
				int iSendResult = send(ClientSocket, recvbuffer, received, 0);
				if (iSendResult == SOCKET_ERROR)
				{
					cout << "Sending data failed with error " << WSAGetLastError() << endl;
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				cout << "Bytes sent: " << iSendResult << endl;
				ZeroMemory(recvbuffer, sizeof(recvbuffer));
			}
			else if (received == 0) cout << "Connection closing...\n";
			else
			{
				cout << "Receive failed with error " << WSAGetLastError() << endl;
				closesocket(ClientSocket);
			}
		} while (received > 0);

		// 7. disconnect
		int iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Shutdown failed with error " << WSAGetLastError() << endl;
		}
		closesocket(ClientSocket);
	}
}