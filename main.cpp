#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <string>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")

class TCPServer
{
public:
	TCPServer();
	~TCPServer();
	bool initWinSock();
	bool createSocket();
	bool bindSocket(const char* ip, uint16_t port);
	bool listenForConnections(int backlog = 5);
	bool acceptConnection();
	bool receiveData();
	bool sendData(const std::string& data);
	void closeConnection();

private:
	SOCKET listenSocket;
	SOCKET clientSocket;
	sockaddr_in serverAddr;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	std::string receivedData;
};

TCPServer::TCPServer() : listenSocket(INVALID_SOCKET), clientSocket(INVALID_SOCKET)
{
	memset(&serverAddr, 0, sizeof(serverAddr));
	memset(&clientAddr, 0, sizeof(clientAddr));
}

TCPServer::~TCPServer()
{
	closeConnection();
	WSACleanup();
}

bool TCPServer::initWinSock()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		printf("WSAStartup failed!\n");
		return false;
	}
	return true;
}

bool TCPServer::createSocket()
{
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Socket creation failed! Error code: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool TCPServer::bindSocket(const char* ip, uint16_t port)
{
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = (ip == nullptr) ? INADDR_ANY : inet_pton(AF_INET, ip, &serverAddr.sin_addr);

	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf("Bind failed! Error code: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool TCPServer::listenForConnections(int backlog)
{
	if (listen(listenSocket, backlog) == SOCKET_ERROR)
	{
		printf("Listen failed! Error code: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool TCPServer::acceptConnection()
{
	int clientAddrLen = sizeof(clientAddr);
	clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Accept failed! Error code: %d\n", WSAGetLastError());
		return false;
	}

	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)) == NULL)
	{
		printf("inet_ntop failed! Error code: %d\n", WSAGetLastError());
		return false;
	}

	printf("Connection received from: %s\n", clientIP);
	return true;
}

bool TCPServer::receiveData()
{
	char buffer[512];
	int ret = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (ret > 0)
	{
		buffer[ret] = 0;
		receivedData = buffer;
		printf("Received from client: %s\n", receivedData.c_str());
		return true;
	}
	else if (ret == 0)
	{
		printf("Connection closed by client.\n");
		return false;
	}
	else
	{
		printf("Recv failed! Error code: %d\n", WSAGetLastError());
		return false;
	}
}

bool TCPServer::sendData(const std::string& data)
{
	int sendResult = send(clientSocket, data.c_str(), data.length(), 0);
	if (sendResult == SOCKET_ERROR)
	{
		printf("Send failed! Error code: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void TCPServer::closeConnection()
{
	if (clientSocket != INVALID_SOCKET)
	{
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}
	if (listenSocket != INVALID_SOCKET)
	{
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}
}

int main(int argc, char* argv[])
{
	TCPServer server;

	if (!server.initWinSock())
		return 1;

	if (!server.createSocket())
		return 1;

	if (!server.bindSocket(nullptr, 8888))// nullptr表示监听0.0.0.0
		return 1;

	if (!server.listenForConnections())// 将socket转化为监听状态，使得服务端准备与客户端准备连接
		return 1;

	while (true)
	{
		printf("Waiting for a connection...\n");

		if (!server.acceptConnection())
			continue;

		if (!server.receiveData())// 返回false表示连接已失效
			continue;

		const std::string sendData = "Hello, TCP client!\n";
		if (!server.sendData(sendData))
			continue;
	}

	return 0;
}
