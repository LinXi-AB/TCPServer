#include <stdio.h>  
#include <winsock2.h>  
#include <ws2tcpip.h>  // for inet_ntop

#pragma comment(lib, "ws2_32.lib")  

int main(int argc, char* argv[])
{
    // ��ʼ�� Winsock
    WORD sockVersion = MAKEWORD(2, 2);// WORD��һ��ʮ��λ�޷���������MAKEWORD��������λ�������Ϊ16λ���ڴ˴�������ʾsocket�İ汾��2.2
    WSADATA wsaData;// ��F12�鿴�ṹ�嶨�壬��WSAStartup�н�����Ϣ����socket�汾��
    if (WSAStartup(sockVersion, &wsaData) != 0)// ��ʼ��winsockʹ�ÿ���ʹ�����繦�ܣ�WSA��Windows Sockets API����д  
    {
        printf("WSAStartup failed!\n");
        return 1;
    }

    // �����׽���
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// IPv4��ַ��, TCP
    if (slisten == INVALID_SOCKET)
    {
        printf("socket creation failed! Error code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // �� IP �Ͷ˿�, bind
    sockaddr_in sin;
    sin.sin_family = AF_INET;// ��ַ��ΪIPv4
    sin.sin_port = htons(8888);// �˿ں�Ϊ8888��0~65535��htons��ʾhost to network short�������ֽ���ת��Ϊ�����ֽ���  
    sin.sin_addr.S_un.S_addr = INADDR_ANY;// ����IP��ַ��INADDR_ANY��0.0.0.0��ʾ��������IP��ַ
    if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)// ���׽������ַ�Ͷ˿ڰ�
    {
        printf("bind failed! Error code: %d\n", WSAGetLastError());
        closesocket(slisten);
        WSACleanup();
        return 1;
    }

    // ��ʼ����, listen
    if (listen(slisten, 5) == SOCKET_ERROR)
    {
        printf("listen failed! Error code: %d\n", WSAGetLastError());
        closesocket(slisten);
        WSACleanup();
        return 1;
    }

    // ѭ����������
    SOCKET sClient;
    sockaddr_in remoteAddr;// �ͻ��˵�ַ��Ϣ
    int nAddrlen = sizeof(remoteAddr);
    char clientIP[INET_ADDRSTRLEN];  // �����洢�ͻ��˵� IP ��ַ
    while (true)
    {
        printf("Waiting for a connection...\n");

        // �ȴ������ܿͻ�������, accept
        sClient = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);// �������ȴ����ӣ����ذ��˵�ַ�������ӵ��׽���
        if (sClient == INVALID_SOCKET)
        {
            printf("accept failed! Error code: %d\n", WSAGetLastError());
            continue; // ��� accept ʧ�ܣ������ȴ��µ�����
        }

        // ʹ�� inet_ntop ����ȡ�ͻ��˵� IP ��ַ
        if (inet_ntop(AF_INET, &remoteAddr.sin_addr, clientIP, sizeof(clientIP)) == NULL)
        {
            printf("inet_ntop failed! Error code: %d\n", WSAGetLastError());
        }
        else
        {
            printf("Connection received from: %s \n", clientIP);
        }

        // ��������
        char revData[255];// �洢�ӿͻ��˽��յ�������
        int ret = recv(sClient, revData, sizeof(revData) - 1, 0);
        if (ret > 0)
        {
            revData[ret] = 0x00; // ȷ���ַ�����β
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

        // ��������
        const char* sendData = "Hello, TCP client!\n";
        int sendResult = send(sClient, sendData, strlen(sendData), 0);
        if (sendResult == SOCKET_ERROR)
        {
            printf("send failed! Error code: %d\n", WSAGetLastError());
        }

        // �ر���ͻ��˵�����
        closesocket(sClient);
    }

    // �رռ��� socket ������ Winsock
    closesocket(slisten);
    WSACleanup();// ����winsock��winsock��ʼ����Ӧ
    return 0;
}
