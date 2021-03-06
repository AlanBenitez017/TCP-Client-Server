/***************************************************************
  Student Name: Alan Benitez
  File Name: main.cpp
  Project 2

  This is the main file of the client where I do the connections with the appropriate ports and send and receive messages from the client
***************************************************************/

#include "Client.hpp"

void sendMsg(int client_fd);
void receiveMsg(int client_fd);

int main(){

    Client c;
    string ipAddress;
    do{
        cout << "Enter hostname: ";
        cin >> ipAddress;
        if(ipAddress != "localhost"){
            cout << "Can't find the hostname, try again" << endl;
        }
    }while(ipAddress != "localhost");
    ipAddress = "127.0.0.1";

    if((c.client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Could not make socket !!!");
        exit(EXIT_FAILURE);
    } else{
        cout << "Socket created" << endl;
    }

    memset(c.address.sin_zero, '\0', sizeof c.address.sin_zero);
    c.address.sin_family = AF_INET;
    c.address.sin_port = htons(PORT);
    c.address.sin_addr.s_addr = inet_addr((char*)ipAddress.c_str());

    if(connect(c.client_fd, (struct sockaddr*)&c.address, sizeof(c.address)) < 0){
        perror("Failed to bind !!!");
        close(c.client_fd);
        exit(EXIT_FAILURE);
    } else{
        cout << "localhost resolved to " << ipAddress << endl << "Connected" << endl << endl;
    }

    memset(c.sendingBuff, '\0', sizeof c.sendingBuff);
    memset(c.receivingBuff, '\0', sizeof c.receivingBuff);

    read(c.client_fd,c.receivingBuff, (size_t) MAX);
    cout << c.receivingBuff << endl;
    memset(c.sendingBuff,0,MAX);
    cin.getline(c.sendingBuff, MAX);
    write(c.client_fd, c.sendingBuff, (size_t)MAX);

    thread t1(receiveMsg, c.client_fd);
    thread t2(sendMsg, c.client_fd);

    c.threadReceiving = move(t1);
    c.threadSending = move(t2);


    if (c.threadReceiving.joinable())
        c.threadReceiving.join();

    if (c.threadSending.joinable())
        c.threadSending.join();


    return 0;
}

void receiveMsg(int client_fd) {
    Client c;
    while(true){
        memset(c.receivingBuff, 0, MAX);
        cin.getline(c.receivingBuff, MAX);
        write(client_fd, c.receivingBuff, (size_t)MAX);
        if(strcmp(c.receivingBuff, "exit") == 0){
            c.exitClient = true;
            c.threadReceiving.detach();
            close(client_fd);
            return;
        }
    }
}

void sendMsg(int client_fd) {
    Client c;
    while(true){
        if(c.exitClient){
            c.threadSending.detach();
            return;
        }
        memset(c.sendingBuff, 0, MAX);
        int bytesReceived = read(client_fd, c.sendingBuff, (size_t)MAX);
        if(bytesReceived <= 0){
            continue;
        }
        cout << c.sendingBuff << endl;
        fflush(stdout);
    }
}