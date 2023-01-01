#include <iostream>
#include <string>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

constexpr int BUFFER_SIZE = 1024;

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData); //version 2.2
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Create a socket for connecting to the server
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

   
    // Set the server address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); //using 127.0.0.1 just to broadcase and making my life easier :)

    // Connect to the server
    result = connect(connectSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR)
    {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
            // Close the socket and clean up Winsock
            closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the server" << std::endl;

    

    // Start a thread to receive messages from the server
    std::thread receiveThread([&]()
        {
            
            while (true)
            {
                // Receive a message from the server
                char buffer[BUFFER_SIZE];
                int bytesReceived = recv(connectSocket, buffer, BUFFER_SIZE, 0);
                if (bytesReceived == SOCKET_ERROR)
                {
                    std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
                    break;
                }
                if (bytesReceived == 0)
                {
                    break;
                }
                // Print the message to the console
                std::cout << buffer << std::endl;
            }
        });
    // Read messages from the console and send them to the server
    std::string message;
    std::string userNameAndMessage;

    while (std::getline(std::cin, message))
    {
        if (!message.empty())
        {
            if (message == "$create$") //see if the msg is the commend
            {
                //for the create chatroom in serverside
                send(connectSocket, message.c_str(), message.length() + 1, 0);
                continue;
            }
            else if (message == "$login$") //see if the msg is the commend
            {
                //for the login chatroom in serverside
                send(connectSocket, message.c_str(), message.length() + 1, 0);
                continue;
            }
            else if (message == "$exit$") //see if the msg is the commend
            {
                //for the exit chatroom in serverside
                send(connectSocket, message.c_str(), message.length() + 1, 0);
                Sleep(300);
                break;
            }
            userNameAndMessage =  message;
            // Send the message to the server
            int bytesSent = send(connectSocket, userNameAndMessage.c_str(), userNameAndMessage.length() + 1, 0);
            if (bytesSent == SOCKET_ERROR)
            {
                std::cerr << "send failed: " << WSAGetLastError() << std::endl;
                break;
            }
        }
    }

    // Wait for the receive thread to finish
    receiveThread.join();

    // Close the socket and clean up Winsock
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}

