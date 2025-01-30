#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>

using std::cin;
using std::cout;
using std::endl;

#define DEFAULT_PORT "27015"
#define BUFFER_SIZE 1500
#define MAX_CONNECTIONS 5

#pragma comment(lib, "Ws2_32.lib")

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

SOCKET ClientSockets[MAX_CONNECTIONS];
void HandleConnections(LPVOID lParam);

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
	//HandleConnections(ListenSocket);

	HANDLE hThreadArray[MAX_CONNECTIONS];
	DWORD dwThreadIDArray[MAX_CONNECTIONS];
	int ClientIDs[MAX_CONNECTIONS]{};
	int numberOfClients = 0;
	while (true)
	{
		if (numberOfClients < MAX_CONNECTIONS)
		{
			ClientIDs[numberOfClients] = numberOfClients;

			ClientSockets[numberOfClients] = accept(ListenSocket, NULL, NULL);
			if (ClientSockets[numberOfClients] == INVALID_SOCKET)
			{
				cout << "Accept failed with error " << WSAGetLastError() << endl;
				//closesocket(ListenSocket);
				//WSACleanup();
				return;
			}

			hThreadArray[numberOfClients] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleConnections, &ClientIDs[numberOfClients], 0, &dwThreadIDArray[numberOfClients]);
			numberOfClients++;
			/*if (hThreadArray[threadAmount] == NULL)
			{
				ExitProcess(3);
			}*/
		}
		else
		{
			return;
		}
	}

	WSACleanup();
	system("PAUSE");
}

void HandleConnections(LPVOID lParam)
{
	cout << "enter" << endl;
	int id = *((int*)lParam);
	cout << id << endl;

	SOCKADDR clientSocket;
	int namelen = INET6_ADDRSTRLEN;
	char szClientName[INET6_ADDRSTRLEN]{};
	getpeername(ClientSockets[id], &clientSocket, &namelen);

	cout << "Client data: " << ClientSocketData(clientSocket).getClientAddress(szClientName) << endl;

	// 6. Receive / send data
	char recvbuffer[BUFFER_SIZE]{};
	int received = 0;

	do
	{
		ZeroMemory(recvbuffer, BUFFER_SIZE);
		received = recv(ClientSockets[id], recvbuffer, BUFFER_SIZE, 0);
		cout << received << endl;
		if (received > 0)
		{
			cout << "Bytes received from " << szClientName << ": " << received << " Message: " << recvbuffer << endl;
			int iSendResult = send(ClientSockets[id], recvbuffer, received, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Sending data failed with error " << WSAGetLastError() << endl;
				closesocket(ClientSockets[id]);
				WSACleanup();
				return;
			}
			cout << "Bytes sent: " << iSendResult << endl;
		}
		else if (received == 0) cout << "Connection closing...\n";
		else
		{
			cout << "Receive failed with error " << WSAGetLastError() << endl;
			closesocket(ClientSockets[id]);
		}
	} while (received > 0);

	// 7. disconnect
	cout << "disconnecting\n";
	int iResult = shutdown(ClientSockets[id], SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown failed with error " << WSAGetLastError() << endl;
	}
	closesocket(ClientSockets[id]);
	cout << "exit\n";
}