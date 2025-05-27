#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#include <string>
#include <iostream>
#include "DataBase.h"
#include "sqlite3.h"

using namespace std;



class SocketTCP
{
    int serverSocket;
    int clientSocket;
    sockaddr_in servAddr;
    sockaddr_in cliAddr;
    int port;
public:
    
    SocketTCP();

    bool setup(int port);

    int start(DataBase &db);

    int sendMessage(string &msg);
    int recvMessage(string &msg);

    void close();

private:

    int Send(char data[], size_t data_size);

    int Receive(char data[]);

    bool setopt_REUSEADDR();
};

#endif