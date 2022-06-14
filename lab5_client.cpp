#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <chrono>
#include <thread>
#include <string>

#define prot IPPROTO_TCP; 

int main()
{
    WSADATA wsaData;
    int result;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = prot;

    getaddrinfo("localhost", "27015", &hints, &addrResult);

    SOCKET ConnectSocket = INVALID_SOCKET;

    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

    connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);


    while (true) {
        std::cout << "Input your request:\n";
        std::string request;
        std::getline(std::cin, request);

        result = send(ConnectSocket, request.c_str(), (int)strlen(request.c_str()), 0);
        if (result == SOCKET_ERROR) return -1;

        std::cout << "Bytes sent: " << result << " bytes" << std::endl;
        char recvBuffer[512];
        ZeroMemory(recvBuffer, 512);
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            std::cout << "Receieved data:\n" << recvBuffer << std::endl;
        }
        else if (result == 0) {
            std::cout << "Connection closed" << std::endl;
            break;
        }
        else {
            std::cout << "Recv failed with error: " << result << std::endl;
            break;
        }
    }

    return 0;


}