#include <thread>
#include "BindedSymbolMap.h"

//the function that will update the map in seperate thread
void *BindedSymbolMap::startUpdatesRoutine(void *symbolMap) {
    BindedSymbolMap *symap = (BindedSymbolMap*) symbolMap;
    if (symap != nullptr) {
        //while this thread should be active try and get info from client
        while (symap->isUpdatesActive()) {
            symap->updateTable();
            symap->waitBetweenUpdates();
        }
        // close the thread
        pthread_exit(nullptr);
    }
    return nullptr;
}

//the function that opening the data server incharge of updating the map
void BindedSymbolMap::openDataServer(int port, int frequency) {

    //open the sevrer and get flight gear as client
    openServerAndGetClient(port);

    //get one pulse of information to set the table
    updateTable();
    //make it true
    *updatesThreadActive = true;
    // start the updates thread
    if (pthread_create(&serverThread, nullptr, startUpdatesRoutine, this))
        throw "Error: failed creating updates pthred";

}

void BindedSymbolMap::openServerAndGetClient(int port) {

    //open the server and get the flightgear client

    Address serverAddress;
    socklen_t servAddrLen = sizeof(serverAddress);
    // create socket point
    updatesServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (updatesServerSocket == FAILED)
        throw "Error: socket opening failed";
    /* Initialize socket structure */
    bzero((char *) &serverAddress, sizeof(serverAddress));
    //address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    //bind the host adress using bind
    if (bind(updatesServerSocket, (struct sockaddr *) &serverAddress, servAddrLen) == FAILED)
        throw "Error: socket binding failed";
    //start listening to incoming connections
    listen(updatesServerSocket, MAX_CONNEDTED_CLIENTS);
    //main routine loop of collecting clients
    //define the client sockets structure
    Address clientAdress;
    socklen_t cliAddrLen = sizeof(clientAdress);
    //look for clients
    updatesSocket = accept(updatesServerSocket, (struct sockaddr *) &clientAdress, &cliAddrLen);
    if (updatesSocket == FAILED)
        throw "Error: client socket failed";
}

void BindedSymbolMap::updateTable() {
    if (pthread_mutex_lock(&updatsMutex) <= FAILED)  //make sure two threads wont update at once and cause resource race
        throw runtime_error("lock failed: ");

    vector<string> lines;
    //read from flightgear server the values
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    if (read(updatesSocket, buffer, BUFFER_SIZE) != FAILED) {
        string packet = string(buffer);
        lines = split(packet, ',');
        if (lines.size() != paths.size()) {
            // if its not equal (happens when debugging and few packets are coming at once)
            packet = getInnerString('\n', packet, '\n');
            lines = split(packet, ',');
        }
        if (lines.size() == paths.size()) {
            for (int i = 0; i < paths.size(); i++) {   //if its adress of variable
                (*symbolMap)[paths[i]] = new LocalValue((double) stof(lines[i]));
            }
        }
    }

    if (pthread_mutex_unlock(&updatsMutex) <= FAILED)
        throw runtime_error("unlock failed: ");
}

void BindedSymbolMap::waitBetweenUpdates() {
    if (updatesFrequency != 0)
        this_thread::sleep_for(chrono::milliseconds(1000 / updatesFrequency));
}

//the function that connects to the flightgear as a client;
void BindedSymbolMap::connect(const string &ip, int port) {
    //connect to server and update:
    struct sockaddr_in serverAddress;
    socklen_t addressLen = sizeof(serverAddress);
    struct hostent *server = gethostbyname(ip.c_str());
    if (server == nullptr)
        throw "Error: failed finding the host";
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);
    //create socket to connect client to server
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == FAILED)
        throw "Error: failed to open client socket";
    if (::connect(clientSocket, (struct sockaddr *) &serverAddress, addressLen) == FAILED)
        throw "Error: client failed to connect to server";
    connectedAsClient = true;
}

