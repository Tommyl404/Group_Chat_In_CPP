#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <map>

#include <winsock2.h>
#include <WS2tcpip.h>
#include "useful_funcs.h"

#pragma comment(lib, "ws2_32.lib")

std::string userName;

/*THIS CODE WAS MADE BY TOMMY LEVI.*/

int main()
{
    
    // Initialize Winsock.
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData); //version 2.2
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return result;
    }

    // Create a socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        //If failed to create the socket.
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket to a local address and port.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(5000);

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
        //If the bind for the listenSocket failed for some reason.
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Set the socket to listen for incoming connections.
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        //If the ListenSocket failed to become a listener.
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Listening for incoming connections on port 5000..." << std::endl;

    // Run the server on a separate thread
    std::thread serverThread([&]()
        {
            std::string clientChatId;
            while (true)
            {
                // Accept an incoming connection
                sockaddr_in clientAddr;
                int clientAddrSize = sizeof(clientAddr);
                SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
                if (clientSocket == INVALID_SOCKET)
                {
                    std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
                    break;
                }

                std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

                // Start a new thread to receive messages from the client
                std::thread clientThread([&](SOCKET clientSocket,std::string id) //id to touch the clients room only.
                    {
                        //here will be the start screen.
                        send(clientSocket, STARTSCREEN.c_str(), STARTSCREEN.length() + 1, 0);
                        while (true)
                        {
                            char buffStart[BUFFER_SIZE];
                            int bytesReceived = recv(clientSocket, buffStart, BUFFER_SIZE, 0);
                            if (bytesReceived == SOCKET_ERROR)
                            {
                                std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
                                break;
                            }
                            if (bytesReceived == 0)
                            {
                                // Client disconnected
                                break;
                            }
                            if (strcmp(buffStart, "$create$") == 0)
                            {
                                // buffer starts with "$create$"
                                clientChatId = createChatRoom(clientSocket);
                                break;
                            }
                            if (strcmp(buffStart, "$login$") == 0)
                            {
                                // buffer starts with "$login$"
                                clientChatId = loginToChatRoom(clientSocket);
                                if (clientChatId == "a" || clientChatId == "b" || clientChatId == "c" || clientChatId == "d")
                                {
                                    closesocket(clientSocket);
                                    std::cout << "Disconnect connection " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
                                }
                                break;

                            }
                            if (strcmp(buffStart, "$exit$") == 0)
                            {
                                // buffer is with "$exit$"
                                closesocket(clientSocket);
                                std::cout << "Disconnect connection " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
                            }
                        }
                        send(clientSocket, ASK_NAME.c_str(), ASK_NAME.length() + 1, 0); //ask for the user's name
                        while (true)
                        {
                            char buffer[BUFFER_SIZE];
                            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                            if (bytesReceived == SOCKET_ERROR)
                            {
                                std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
                                break;
                            }
                            if (bytesReceived == 0)
                            {
                                send(clientSocket, SMARTASS.c_str(), SMARTASS.length() + 1, 0);
                                userName = generate_random_string();
                                break;
                            }
                            if (bytesReceived > 0)
                            {
                                std::string str(buffer);
                                userName = str;
                                break;
                            }
                        }


                        std::string userText = userName + ">";

                        while (true)
                        {
                            // Receive a message from the client
                            char buffer[BUFFER_SIZE];
                            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                            if (bytesReceived == SOCKET_ERROR)
                            {
                                std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
                                break;
                            }
                            if (bytesReceived == 0)
                            {
                                // Client disconnected
                                break;
                            }
                            if (strcmp(buffer, "$exit$") == 0)
                            {
                                // buffer is with "$exit$"
                                std::cout << "Disconnect connection " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
                                closesocket(clientSocket);
                                break;
                                
                            }
                            std::string str(buffer);
                            std::string msg_with_username = userText + str;
                            if (clientSocket) // Send the message to all other clients in the room
                            {
                                std::lock_guard<std::mutex> lock(*chatRooms[clientChatId].clientListMutex);
                                for (auto& client : chatRooms[clientChatId].clients)
                                {
                                    if (client != clientSocket)
                                    {
                                        send(client, msg_with_username.c_str(), msg_with_username.length() + 1, 0);
                                    }
                                }
                            }
                            
                        }

                        // Remove the client from the list of connected clients in the room
                        std::lock_guard<std::mutex> lock(*chatRooms[clientChatId].clientListMutex);
                        chatRooms[clientChatId].clients.erase(std::remove(chatRooms[clientChatId].clients.begin(), chatRooms[clientChatId].clients.end(), clientSocket), chatRooms[clientChatId].clients.end());
                        std::cout << "Client disconnected" << std::endl;

                        // Close the socket
                        closesocket(clientSocket);

                    }, clientSocket,clientChatId);
                    clientThread.detach();
            }
        });

    serverThread.join();

    // Clean up
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
