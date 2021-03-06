/***************************************************************
  Student Name: Alan Benitez
  File Name: Server.cpp
  Project 2

  In this file I develop the responses from the client. Will add the new users to a .txt file and does the communication with the client
***************************************************************/
#include "Server.hpp"

Server::Server() {
    id = 0;
    usersActive = 0;
    addrLen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    cout << "bind done" << endl;
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }


    while(true) {
        printf("Waiting for incoming connection...\n");
        if((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrLen)) < 0){
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        id++;
        threads.push_back(std::thread(&Server::run, this, new_socket, id));


    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        if (threads[i].joinable())
            threads[i].join();
    }
}


void Server::run(int new_socket, int id){
    cout << "Connection accepted" << endl;
    cout << "Handler assigned" << endl;
    cout << "ID #" << id << endl;
    mainMenu(new_socket, id);

    while(loggedIn){
        memset(receivingBuff, 0, MAX);
        read(new_socket, receivingBuff, (size_t)MAX);
        if(strcmp(receivingBuff, "1") == 0){
            subscribe(new_socket, id);
        }else if(strcmp(receivingBuff, "2") == 0){
            unsubscribe(new_socket, id);
        }else if(strcmp(receivingBuff, "3") == 0){
            seeActiveUsers(new_socket, id);
        }else if(strcmp(receivingBuff, "4") == 0){
            sendMsgToAUser(new_socket, id);
        }else if(strcmp(receivingBuff, "5") == 0){
            sendMsgToALocation(new_socket, id);
        }else if(strcmp(receivingBuff, "6") == 0){
            seeLocations(new_socket, id);
        }else if(strcmp(receivingBuff, "7") == 0){
            seeLast10Msg(new_socket, id);
        }else if(strcmp(receivingBuff, "8") == 0){
            changePassword(new_socket, id);
        }else if(strcmp(receivingBuff, "9") == 0){
            loggedIn = false;
            for (size_t i = 0; i < users.size(); i++)
            {
                if (users[i].getId() == id)
                {
                    users.erase(users.begin() + i);
                    break;
                }
            }
            usersActive--;
            mainMenu(new_socket, id);
        }else{
            memset(sendingBuff, 0, MAX);
            string invalid = "Invalid choice, please try again";
            strcpy(sendingBuff, invalid.c_str());
            write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
            optionsWhenLoggedIn(new_socket);
        }
    }
}


bool Server::Login(int new_socket, int id) {
    memset(receivingBuff, 0, MAX);
    memset(sendingBuff, 0, MAX);

    string username, password;
    string uName = " Username:\n";

    strcpy(sendingBuff, uName.c_str());
    write(new_socket, sendingBuff, (int)MAX);
    memset(receivingBuff, 0, MAX);
    read(new_socket, receivingBuff, (size_t)MAX);

    username = receivingBuff;
    memset(sendingBuff, 0, MAX);
    string pWord = " Password:\n";
    strcpy(sendingBuff, pWord.c_str());
    write(new_socket, sendingBuff, (int)MAX);
    read(new_socket, receivingBuff, (size_t)MAX);  //lee lo que acabo de escribir el client
    password = receivingBuff;
    cout << "u: " << username << " p: "  << password << endl;

    if(checkLogin(username, password)){
        User userr(username, password, new_socket, id);
        users.push_back(userr);
        usersActive++;
        cout << "Users active: " << usersActive << endl;
        memset(sendingBuff, 0, MAX);
        string logged = "Successfully Logged In";
        strcpy(sendingBuff, logged.c_str());
        write(new_socket, sendingBuff, (int)MAX);
        return true;
    }else{
        return false;
    }

}


void Server::Register(int new_socket){
    mtx.lock();
    memset(receivingBuff, 0, MAX);
    memset(sendingBuff, 0, MAX);

    string username, password;
    string uName = " Username:\n";

    strcpy(sendingBuff, uName.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
    memset(receivingBuff, 0, MAX);   //might delete it later, already freeing it when calling this function
    read(new_socket, receivingBuff, (size_t)MAX);  //lee lo que acabo de escribir el client

    username = receivingBuff;
    memset(sendingBuff, 0, MAX);
    string pWord = " Password:\n";
    strcpy(sendingBuff, pWord.c_str());
    write(new_socket, sendingBuff, (int)MAX);
    read(new_socket, receivingBuff, (size_t)MAX);  //lee lo que acabo de escribir el client
    password = receivingBuff;
    cout << "u: " << username << " p: "  << password << endl; //just to check, sends to the server terminal

    ofstream usersFile;
    usersFile.open("users.txt", fstream::app);
    if (usersFile.is_open())
    {
        usersFile << username << "\t" << password << endl;
        usersFile.close();
    }
    else{
        cout << "Unable to open file";
    }
    memset(sendingBuff, 0, MAX);
    string success = "Successfully Registered";
    strcpy(sendingBuff, success.c_str());
    write(new_socket, sendingBuff, (int)MAX);
    mtx.unlock();
}

void Server::changePassword(int new_socket, int id) {
    mtx.lock();
    memset(sendingBuff, 0, MAX);
    string pWord = " Please insert new password:\n";
    strcpy(sendingBuff, pWord.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer

    memset(receivingBuff, 0, MAX);
    read(new_socket, receivingBuff, (size_t)MAX);  //lee lo que acabo de escribir el client
    string newPassword = receivingBuff;

    string uName;
    for (unsigned i = 0; i < users.size(); i++)
    {
        if (users[i].getId() == id)
        {
            uName = users[i].getUsername();
            users[i].setPassword(newPassword);
            break;
        }
    }
    string username;
    string password;
    string inLine;

    ifstream usersFile;
    fstream tempFile;
    usersFile.open("users.txt", ios::in);
    tempFile.open("temp.txt", ios_base::app | ios::out | ios::in);
    if (usersFile.is_open() && tempFile.is_open()) {
        while (getline(usersFile, inLine)) {
            stringstream ss(inLine);
            ss >> username;
            ss >> password;
            string newLine = username;
            newLine += " ";
            if (username == uName){
                newLine += newPassword;
            }else{
                newLine += password;
            }
            newLine += "\n";

            tempFile << newLine;

        }
    }

    usersFile.close();
    tempFile.close();
    rename("temp.txt", "users.txt");

    memset(sendingBuff, 0, MAX);
    pWord = " Password changed successfully\n";
    strcpy(sendingBuff, pWord.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
    optionsWhenLoggedIn(new_socket);
    mtx.unlock();
}

void Server::mainMenu(int new_socket, int id) {
    memset(sendingBuff, 0, MAX);
    string firstOptions = "Welcome!\n  Press 1 to Login\n  Press 2 to Register\n  Type \'exit\' to Quit\n";
    strcpy(sendingBuff, firstOptions.c_str());
    write(new_socket, sendingBuff, (int)MAX);

    while(loggedIn == false){
        memset(receivingBuff, 0, MAX);
        read(new_socket, receivingBuff, (size_t)MAX);
        if(strcmp(receivingBuff, "exit") == 0){
            //close(new_socket);
            exit(EXIT_SUCCESS);
        }
        if(strcmp(receivingBuff, "1") == 0){
            if(Login(new_socket, id)){
                optionsWhenLoggedIn(new_socket);
                loggedIn = true;
            }else{
                string notFound = "Could not find the account";
                strcpy(sendingBuff, notFound.c_str());
                write(new_socket, sendingBuff, (int)MAX);
                mainMenu(new_socket, id);
            }
        }
        if(strcmp(receivingBuff, "2") == 0){
            Register(new_socket);
            mainMenu(new_socket, id);
        }
    }
}

bool Server::checkLogin(string username, string password) {
    string uName, pWord;
    ifstream usersFile;
    usersFile.open("users.txt");
    if (usersFile.is_open())
    {
        usersFile >> uName >> pWord;
        while (!usersFile.eof()) { // keep reading until end-of-file
            if(uName == username && pWord == password){
                return true;
            }
            usersFile >> uName >> pWord; // sets EOF flag if no value found
        }
        usersFile.close();
    }
    return false;
}

void Server::optionsWhenLoggedIn(int new_socket) {
    memset(sendingBuff, 0, MAX);
    string options;
    options = " 1. Subscribe to a location\n";
    options += " 2. Unsubscribe to a location\n";
    options += " 3. See the online users\n";
    options += " 4. Send a message to a user\n";
    options += " 5. Send a group message to a location\n";
    options += " 6. See all the locations that the client has subscribed to\n";
    options += " 7. See the last 10 message received\n";
    options += " 8. Change password\n";
    options += " 9. Quit\n";
    strcpy(sendingBuff, options.c_str());
    write(new_socket, sendingBuff, (int)MAX);

}

void Server::subscribe(int new_socket, int id){
    memset(sendingBuff, 0, MAX);
    string locToSub = "Please enter a location to subscribe";
    strcpy(sendingBuff, locToSub.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer

    memset(receivingBuff, 0, MAX);   //might delete it later, already deleted it when calling this function
    read(new_socket, receivingBuff, (size_t)MAX);  //lee lo que acabo de escribir el client

    string location = receivingBuff;

    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            users[i].subscribe(location);
        }
    }

    memset(sendingBuff, 0, MAX);
    locToSub = "Subscribed successfully";
    strcpy(sendingBuff, locToSub.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
    optionsWhenLoggedIn(new_socket);
}


void Server::unsubscribe(int new_socket, int id){
    memset(sendingBuff, 0, MAX);
    string showLocations;
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            showLocations += users[i].seeLocations();
        }
    }
    strcpy(sendingBuff, showLocations.c_str());
    write(new_socket, sendingBuff, (int)MAX);

    memset(sendingBuff, 0, MAX);
    string locToSub = "Please enter a location to unsubscribe";
    strcpy(sendingBuff, locToSub.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer

    memset(receivingBuff, 0, MAX);   //might delete it later, already deleted it when calling this function
    read(new_socket, receivingBuff, (size_t)MAX);  //lee lo que acabo de escribir el client

    string location = receivingBuff;
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            if(users[i].unsubscribe(location)){
                memset(sendingBuff, 0, MAX);
                locToSub = "Unsubscribed successfully";
                strcpy(sendingBuff, locToSub.c_str());
                write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
                optionsWhenLoggedIn(new_socket);
            }else{
                memset(sendingBuff, 0, MAX);
                locToSub = "Could not find the location";
                strcpy(sendingBuff, locToSub.c_str());
                write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
                optionsWhenLoggedIn(new_socket);
            }
        }
    }
    /*if(u.unsubscribe(location)){
        memset(sendingBuff, 0, MAX);
        locToSub = "Unsubscribed successfully";
        strcpy(sendingBuff, locToSub.c_str());
        write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
        optionsWhenLoggedIn(new_socket);
    }else{
        memset(sendingBuff, 0, MAX);
        locToSub = "Could not find the location";
        strcpy(sendingBuff, locToSub.c_str());
        write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
        optionsWhenLoggedIn(new_socket);
    }*/
}

void Server::seeActiveUsers(int new_socket, int id){
    memset(sendingBuff, 0, MAX);
    string activeUserss;
    for(int i = 0; i < users.size(); i++){
        activeUserss += to_string(i) + ": " + users[i].getUsername() + "\n";
    }
    strcpy(sendingBuff, activeUserss.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
    optionsWhenLoggedIn(new_socket);

}

void Server::seeLocations(int new_socket, int id){
    memset(sendingBuff, 0, MAX);
    string locations = "";
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            locations += users[i].seeLocations();
        }
    }
    strcpy(sendingBuff, locations.c_str());
    write(new_socket, sendingBuff, (int)MAX);  //enviando el buffer
    optionsWhenLoggedIn(new_socket);

}

void Server::sendMsgToAUser(int new_socket, int id){        //SOS :(
    mtx.lock();
    memset(sendingBuff, 0, MAX);
    string prompt = "Please choose what user you would like to send the message\n";
    for(int i = 0; i < users.size(); i++){
        prompt += to_string(i) + ": " + users[i].getUsername() + "\n"; //imprime los users conectadps
    }
    strcpy(sendingBuff, prompt.c_str());
    write(new_socket, sendingBuff, (int)MAX); //envio AL CLIENTE los users coenctados

    memset(receivingBuff, 0, MAX);
    read(new_socket, receivingBuff, (size_t)MAX);  //recibe RESPUESTA DEL MISMO USUARIO, con ell nombre del usuario
    string userToSendTo = receivingBuff;

    memset(sendingBuff, 0, MAX);
    string msg = "Enter the message: ";
    strcpy(sendingBuff, msg.c_str());
    write(new_socket, sendingBuff, (int)MAX);   //ENVIA AL CLIENTE "ENTER THE MESSAGE"
    memset(receivingBuff, 0, MAX);
    read(new_socket, receivingBuff, (size_t)MAX);   //LEE LO QUE EL MISMO CLIENTE ESCRIBIO

    memset(sendingBuff, 0, MAX);
    string name = "Message from: ";
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            name += users[i].getUsername();
        }
    }
    strcpy(sendingBuff, name.c_str());
    //write(new_socket, sendingBuff, (int)MAX);   //ENVIA AL CLIENTE "ENTER THE MESSAGE"

    for(int i = 0; i < users.size(); i++){
        if(users[i].getUsername() == userToSendTo){
            users[i].addMsg(receivingBuff);
            write(users[i].getNewSocket(), sendingBuff, (int)MAX);
            write(users[i].getNewSocket(), receivingBuff, (int)MAX);
            break;
        }
    }
    //write(new_socket, sendingBuff, (int)MAX);



    /*for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
                 //adding to the vector of msgs
        }
    }*/
    optionsWhenLoggedIn(new_socket);
    mtx.unlock();
}

void Server::sendMsgToALocation(int new_socket, int id) {

    memset(sendingBuff, 0, MAX);
    string locations = "Please choose what location you would like to send the message\n";
    //string locations = "";
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id) {
            locations += users[i].seeLocations();
        }
    }
    strcpy(sendingBuff, locations.c_str());
    write(new_socket, sendingBuff, (int)MAX);

    memset(receivingBuff, 0, MAX);
    read(new_socket, receivingBuff, (size_t)MAX);  //recibe RESPUESTA DEL MISMO USUARIO, con ell nombre del usuario
    string locationToSendTo = receivingBuff;

    memset(sendingBuff, 0, MAX);
    string msg = "Enter the message: ";
    strcpy(sendingBuff, msg.c_str());
    write(new_socket, sendingBuff, (int)MAX);   //ENVIA AL CLIENTE "ENTER THE MESSAGE"
    memset(receivingBuff, 0, MAX);
    read(new_socket, receivingBuff, (size_t)MAX);   //LEE LO QUE EL MISMO CLIENTE ESCRIBIO

    memset(sendingBuff, 0, MAX);
    string name = "Message from: ";
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            name += users[i].getUsername();
        }
    }
    strcpy(sendingBuff, name.c_str());

    for(int i = 0; i < users.size(); i++) {
        for(int j = 0; j < users[i].getLocations().size(); j++){
            if(users[i].getLocations().at(j) == locationToSendTo && users[i].getNewSocket() != new_socket) {
                users[i].addMsg(receivingBuff);
                write(users[i].getNewSocket(), sendingBuff, (int)MAX);
                write(users[i].getNewSocket(), receivingBuff, (int) MAX);
            }
        }
    }
    optionsWhenLoggedIn(new_socket);

}

void Server::seeLast10Msg(int new_socket, int id){
    memset(sendingBuff, 0, MAX);
    string msgs;
    for(int i = 0; i < users.size(); i++){
        if(users[i].getId() == id){
            msgs += users[i].seeLast10Msg();   //actually showing last 10 msgs sent, gotta change it
        }
    }
    strcpy(sendingBuff, msgs.c_str());
    write(new_socket, sendingBuff, (int)MAX);
    optionsWhenLoggedIn(new_socket);
}
