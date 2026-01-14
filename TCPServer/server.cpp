#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "json.hpp"  // nlohmann/json, header-only: https://github.com/nlohmann/json

#pragma comment(lib, "Ws2_32.lib")

using json = nlohmann::json;

int main() {
    // Inicializácia Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Cannot create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(12345);

    if (bind(listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server running on port 12345...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        char buffer[4096] = {0};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            try {
                json j = json::parse(buffer);

                // vytvorenie nazvu suboru podla casu
                std::time_t t = std::time(nullptr);
                std::tm tm;

                #ifdef _MSC_VER
                    localtime_s(&tm, &t); // MSVC
                #else
                    tm = *std::localtime(&t); // MinGW / GCC
                #endif

                char filename[100];
                std::strftime(filename, sizeof(filename), "pid_%Y-%m-%d_%H-%M-%S.json", &tm);

                std::ofstream file(filename);
                file << j.dump(4); // zapis pekne naformátovaný JSON
                file.close();

                std::cout << "Received and saved: " << filename << "\n";
            } catch (...) {
                std::cerr << "Error parsing JSON\n";
            }
        }
        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
