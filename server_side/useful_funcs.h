#pragma once

#include <iostream>
#include <string>
#include <random>
#include <cctype>
#include <mutex>
#include <vector>
#include <map>
#include <utility>

#include <winsock2.h>
#include <WS2tcpip.h>

std::string CREATE = "What will be the password? :";
std::string WISEGUYAY = "So no password? Sorry buddy, but you get the password: TESTER\n next time, try not to be a smartass.\n Chat room id is: ";
std::string SEND_ID_MSG = "Chat room id is: ";
std::string WHATS_THE_ID = "Chat room's id? (5 char.) :";
std::string WHATS_THE_PASSWORD = "Chat room's password?: ";
std::string ERROR_ID_DONT_EXIST = "No chat room have this id.";
std::string ERROR_WORNG_PASSWORD = "Wrong password (three times this message and you are DONE!)";
std::string DISCONNECTMSG = "Thank you and goodbye";
std::string STARTSCREEN = "Wellcome to Tommy's Chat App! \n what would you like to do today?\n $login$ to login to a chatroom (fail to login and you will be DC from the server.)\n $create$ to create a chatroom\n $exit$ to exit the app (don't worry, you can type this commend after too, it's still works)\n";
std::string ASK_NAME = "What is your name? : ";
std::string SMARTASS = "Well if you don't have a name you will be gibrish mate.\n";

const int BUFFER_SIZE = 1024;
std::string loginToChatRoom(SOCKET clientSock);



/*THIS CODE WAS MADE BY TOMMY LEVI.*/





//make a struct for clients.
struct ChatRoom
{
    // A vector to store the sockets of clients in the chat room
    std::vector<SOCKET> clients;

    // A pointer of mutex to protect the client list
    std::mutex* clientListMutex;

    std::string password;

    std::string id;

    ChatRoom() {
        this->clientListMutex = new std::mutex; //to make the mutex
    }

    ChatRoom(std::string input_password, std::string input_id)
    {
        this->clientListMutex = new std::mutex; //to make the mutex
        password = input_password;
        id = input_id;
    }
};

// A map to store the chat rooms
std::map<std::string, ChatRoom> chatRooms;//string is the hash.



std::string generate_random_string() {
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(0, 51); // 0 to 51 (52 characters in total)

    std::string str;
    for (int i = 0; i < 5; i++) {
        int random_number = dis(gen);
        char c = (random_number < 26) ? 'A' + random_number : 'a' + (random_number - 26);
        str += c;
    }
    return str; //This will give me a random 5 cher string that we will use for the hash as the id.
}

std::string createChatRoom(SOCKET clientSock)
{
    send(clientSock, CREATE.c_str(), CREATE.length() + 1, 0);
    int flag = 0; //will turn to 1 when we got a password.
    while (flag == 0)
    {
        char buffer[BUFFER_SIZE];
        int  bytesReceived = recv(clientSock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            return "a"; //socket error
        }
        if (bytesReceived == 0)
        {
            std::string id = generate_random_string();
            std::string msg = WISEGUYAY + id;
            // Client is trying to not enter password\DC.
            send(clientSock, msg.c_str(), msg.length() + 1, 0);

            chatRooms.insert(std::make_pair(id, ChatRoom("TESTER", id)));
            std::lock_guard<std::mutex> lock(*chatRooms[id].clientListMutex);
            chatRooms[id].clients.push_back(clientSock);
            std::cout << "room id: " << id << " has password: TESTER" << std::endl;
            return id;
        }
        else if(bytesReceived > 0)
        {
            std::string id = generate_random_string();
            std::string msg = SEND_ID_MSG + id;
            send(clientSock, msg.c_str(), msg.length() + 1, 0);

            std::string str(buffer); //pasword input
            chatRooms.insert(std::make_pair(id, ChatRoom(buffer,id)));
            std::lock_guard<std::mutex> lock(*chatRooms[id].clientListMutex);
            chatRooms[id].clients.push_back(clientSock);
            std::cout << "room id:" << id << " has password:" << buffer << std::endl;
            return id;
        }

    }
}

std::string loginToChatRoom(SOCKET clientSock)
{
    int failedAtId = 0;
    send(clientSock, WHATS_THE_ID.c_str(), WHATS_THE_ID.length() + 1, 0);
    while (true)
    {
        char buffer[BUFFER_SIZE];
        int  bytesReceived = recv(clientSock, buffer, BUFFER_SIZE, 0);
        
        if (bytesReceived == SOCKET_ERROR)
        {
            
            return "a";
        }
        if (bytesReceived == 0)
        {
           
            return "b";
        }
        if (bytesReceived > 0)
        {
            std::string buffer_to_string_id(buffer);
            if (chatRooms.find(buffer_to_string_id) != chatRooms.end())
            {
                // key exists, ask for password
                send(clientSock, WHATS_THE_PASSWORD.c_str(), WHATS_THE_PASSWORD.length() + 1, 0);
                int failedAttempts = 0;
                while (failedAttempts < 3)
                {
                    char bufferLoopTwo[BUFFER_SIZE];
                    int  bytesReceived = recv(clientSock, bufferLoopTwo, BUFFER_SIZE, 0);
                    if (bytesReceived == SOCKET_ERROR)
                    {
                        return "a";
                    }
                    if (bytesReceived >= 0)
                    {
                        std::string inputPassword(bufferLoopTwo);
                        if (chatRooms[buffer_to_string_id].password != inputPassword) //wrong password.
                        {
                            failedAttempts++;
                        }
                        else if(chatRooms[buffer_to_string_id].password == inputPassword)// right password.
                        {
                            std::string welcome = "welcome to chat room with id: ";
                            std::string msgwelcome = welcome + buffer_to_string_id;
                            send(clientSock, msgwelcome.c_str(), msgwelcome.length() + 1, 0);
                            std::lock_guard<std::mutex>lock(*chatRooms[buffer_to_string_id].clientListMutex);
                            chatRooms[buffer_to_string_id].clients.push_back(clientSock);
                            return buffer_to_string_id;
                        }
                    }
                }
                return "d"; //failed all 3 attempts
            }
            else
            {
                if (failedAtId < 3)
                {
                    
                    std::cout << "user failed in vaild id with input: " << buffer_to_string_id <<" for the" << failedAtId <<" time." << std::endl;
                    send(clientSock, ERROR_ID_DONT_EXIST.c_str(), ERROR_ID_DONT_EXIST.length() + 1, 0);
                    failedAtId++;
                }
                else
                {
                    std::cout << buffer_to_string_id << std::endl;
                    // key does not exist
                    send(clientSock, ERROR_ID_DONT_EXIST.c_str(), ERROR_ID_DONT_EXIST.length() + 1, 0);
                    return "c"; //error with id.
                }
            }

        }


    }
    






}