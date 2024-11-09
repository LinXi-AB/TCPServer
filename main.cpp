#include <stdio.h>  
#include <winsock2.h>  
#include <ws2tcpip.h>  // for inet_ntop

#pragma comment(lib, "ws2_32.lib")  

int main(int argc, char* argv[])
{
    // 初始化 Winsock
    WORD sockVersion = MAKEWORD(2, 2);// WORD是一个十六位无符号整数，MAKEWORD将两个八位整数组合为16位，在此处用来表示socket的版本号2.2
    WSADATA wsaData;// 按F12查看结构体定义，在WSAStartup中接受信息，如socket版本等
    if (WSAStartup(sockVersion, &wsaData) != 0)// 初始化winsock使得可以使用网络功能，WSA是Windows Sockets API的缩写  
    {
        printf("WSAStartup failed!\n");
        return 1;
    }

    // 创建套接字
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// IPv4地址族, TCP
    if (slisten == INVALID_SOCKET)
    {
        printf("socket creation failed! Error code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 绑定 IP 和端口, bind
    sockaddr_in sin;
    sin.sin_family = AF_INET;// 地址族为IPv4
    sin.sin_port = htons(8888);// 端口号为8888，0~65535，htons表示host to network short，主机字节序转化为网络字节序  
    sin.sin_addr.S_un.S_addr = INADDR_ANY;// 设置IP地址，INADDR_ANY即0.0.0.0表示监听所有IP地址
    if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)// 将套接字与地址和端口绑定
    {
        printf("bind failed! Error code: %d\n", WSAGetLastError());
        closesocket(slisten);
        WSACleanup();
        return 1;
    }

    // 开始监听, listen
    if (listen(slisten, 5) == SOCKET_ERROR)
    {
        printf("listen failed! Error code: %d\n", WSAGetLastError());
        closesocket(slisten);
        WSACleanup();
        return 1;
    }

    // 循环接收数据
    SOCKET sClient;
    sockaddr_in remoteAddr;// 客户端地址信息
    int nAddrlen = sizeof(remoteAddr);
    char clientIP[INET_ADDRSTRLEN];  // 用来存储客户端的 IP 地址
    while (true)
    {
        printf("Waiting for a connection...\n");

        // 等待并接受客户端连接, accept
        sClient = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);// 阻塞，等待连接，返回绑定了地址的已连接的套接字
        if (sClient == INVALID_SOCKET)
        {
            printf("accept failed! Error code: %d\n", WSAGetLastError());
            continue; // 如果 accept 失败，继续等待新的连接
        }

        // 使用 inet_ntop 来获取客户端的 IP 地址
        if (inet_ntop(AF_INET, &remoteAddr.sin_addr, clientIP, sizeof(clientIP)) == NULL)
        {
            printf("inet_ntop failed! Error code: %d\n", WSAGetLastError());
        }
        else
        {
            printf("Connection received from: %s \n", clientIP);
        }

        // 接收数据
        char revData[255];// 存储从客户端接收到的数据
        int ret = recv(sClient, revData, sizeof(revData) - 1, 0);
        if (ret > 0)
        {
            revData[ret] = 0x00; // 确保字符串结尾
            printf("Received from client: %s\n", revData);
        }
        else if (ret == 0)
        {
            printf("Connection closed by client.\n");
        }
        else
        {
            printf("recv failed! Error code: %d\n", WSAGetLastError());
        }

        // 发送数据
        const char* sendData = "Hello, TCP client!\n";
        int sendResult = send(sClient, sendData, strlen(sendData), 0);
        if (sendResult == SOCKET_ERROR)
        {
            printf("send failed! Error code: %d\n", WSAGetLastError());
        }

        // 关闭与客户端的连接
        closesocket(sClient);
    }

    // 关闭监听 socket 并清理 Winsock
    closesocket(slisten);
    WSACleanup();// 清理winsock，winsock初始化对应
    return 0;
}
