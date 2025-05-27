#include "SocketTCP.h"

SocketTCP::SocketTCP() {
    serverSocket = -1;
    port = -1;
    servAddr = sockaddr_in();

}

bool SocketTCP::setup(int port)
{
    this->port = port;
    int iResult;

    if (serverSocket == -1) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cout << "[TCPSOCKET] Could not create socket" << endl;
            cout << "[TCPSOCKET] Error. " << WSAGetLastError() << endl;
            return 0;
        }
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    cout << "Server open on port: " << ntohs(servAddr.sin_port) << endl;

    setopt_REUSEADDR(); // SO_REUSEADDR

    iResult = bind(serverSocket, (const struct sockaddr*)&servAddr, sizeof(servAddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        close();
        WSACleanup();
        return 1;
    }
    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        close();
        WSACleanup();
        return 1;
    }
    return 1;
}

int SocketTCP::start(DataBase &db) {
    string response;
    string request;
    while (1) {
        response = "";
        clientSocket = accept(serverSocket, (struct sockaddr*)&cliAddr, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue;
        }
        cout << "Client " << string(inet_ntoa((in_addr)cliAddr.sin_addr)) << ":" << cliAddr.sin_port << " connected." << endl;
        string request;
        if (recvMessage(request) <= 0) {
            break;
        }
        cout << request << endl;
        if (request.find("SELECT") != std::string::npos) {
            cout << "SEL handle" << endl;
            auto v = db.find(request);
            if (v.size() > 0) {
                cout << "FIND RESULTS: " << endl;
                for (auto str : v) {
                    cout << "FIND DATA " << str << endl;
                }
                response = v[0];
            }
            else {
                cout << "NO RESULTS FIND" << endl;
                response = "NO RESULTS FIND";
            }

        }
        else if (request.find("UPDATE") != std::string::npos) {
            cout << "UPDATE handle" << endl;
            if (db.update(request)) {
                response = "UPDATE SUCC";
            }
            else {
                response = "UPDATE ERR";
            }
        }
        else {
            cout << "Error sql is incorrect" << endl;
            response = "ERROR";
        }

        if (sendMessage(response) <= 0) {
            break;
        }
        
        closesocket(clientSocket);
        cout << "Client disconnected" << endl << endl;
    }
    return 1;
}

int SocketTCP::recvMessage(string &msg) {
    char buff[1024] = {0};
    int r = Receive(buff);
    msg = string(buff);
    return 1;
}


int SocketTCP::sendMessage(string &msg) {
    char buff[1024] = {0};
    for (int i = 0; i < msg.size(); ++i) {
        buff[i] = msg[i];
    }
    buff[msg.size()] = '\0';

    int s = Send(buff,msg.size()+1);
    
    return 1;
}


int SocketTCP::Send(char data[], size_t data_size)
{
    int s;
    if ((s = send(clientSocket, data, data_size, 0)) < 0) {
        cout << "[TCPSOCKET] Could not send data" << WSAGetLastError() << endl;
    }
    else {
        cout << "[TCPSOCKET] Bytes send: " << s << endl;
        cout << "[TCPSOCKET] Send data:" << string(data) << endl;
    }
    return s;
}

int SocketTCP::Receive(char data[])
{
    int r = 0;
    if ((r = recv(clientSocket, data, 1024, 0)) < 0) {
        cout << "[TCPSOCKET] Could not recv data. " << WSAGetLastError() << endl;
    }
    else {
        cout << "[TCPSOCKET] Bytes recv: " << r << endl;
        // qWarning() << "[TCPSOCKET] Receive data: " << string(data);
    }
    return r;

}

void SocketTCP::close() {
    closesocket(serverSocket);
}

bool SocketTCP::setopt_REUSEADDR() {
    int yes = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes)) < 0) {
        cout << "Error. Set SO_REUSEADDR.";
        return 0;
    }
    else {
        return 1;
    }
}
