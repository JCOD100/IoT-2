#include <iostream>
#include "SocketTCP.h"
#include <locale.h>
#include "sqlite3.h"
#include <string.h>
using namespace std;
int main()
{
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    SocketTCP server;
    //DataBase db("C:/Users/mikha/Desktop/","people.db");
    DataBase db("C:/Users/Ivan/Desktop/", "people.db");
    server.setup(1234);
    server.start(db);
    return 0;
}