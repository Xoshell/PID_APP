#include <iostream>
#include <thread>
#include <vector>
#include <ctime>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#define PORT 12345
#define BUFFER_SIZE 1024

void handleClient(SOCKET clientSocket)
{
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // nul-terminate
        // získať aktuálny čas
        std::time_t t = std::time(nullptr);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char timestamp[64];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &tm);

        std::string filename = std::string(timestamp) + ".json";

        std::ofstream file(filename);
        if (file.is_open()) {
            file << buffer;
            file.close();
            std::cout << "Saved client data to " << filename << std::endl;
        }
    }

    closesocket(clientSocket);
}

int main()
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket.\n";
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        return 1;
    }

    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    std::vector<std::thread> threads;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket != INVALID_SOCKET) {
            threads.emplace_back(handleClient, clientSocket);
        }
    }

    // join threads (v praxi sa tu nedostaneme, lebo server beží donekonečna)
    for (auto &t : threads) t.join();

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
