#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <list>
#include <iterator>
#include "Parser.h"
#include <vector>
#include <thread>
#include <unordered_map>
#include "game_cli.cpp"
#include "olcPixelGameEngine/olcPixelGameEngine.h"

#define SA struct sockaddr

using namespace std;

static std::unordered_map<int, Character> world_state;

static int num_clients = 0;

bool process_request(char* s) {
    char req[1024];
    char *headers[MAX_ARGS];
    char message[1024];
    unsigned int numHeaders;
    parseRequest(s,req,headers,message,numHeaders);
    
}

int check_if_error(int returned_value, char *error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

int create_server_socket(int port)
{

    struct sockaddr_in server_address;

    int opt = 1;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    //Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //Check for error with socket
    check_if_error(sock, "Error with socket");

    // Binding newly created socket to given IP and verification 
    if ((bind(sock, (SA*)&server_address, sizeof(server_address))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 

    check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");

    // Attempt to make the socket (fd) a listening type socket
    check_if_error(listen(sock, 10), "Could not make the socket a listening type socket");

    cout << "Listening for requests on port " << port << endl;

    return sock;
}

void handle_client(int client_ptr)
{

    cout << "Handling client " << client_ptr << endl;
    int client_id = client_ptr;

    //Receive from the client their name
    char request[BUFSIZ + 1];
    bzero(request, sizeof(request));
    int bytes_read = recv(client_id,request,sizeof(request),0);
    std::string name = std::string str(request);
    bzero(request, sizeof(request));

    //Write to this client only and give them their client_id;
    char *client_id_to_send = (char*)&client_id;
    int client_id_size = sizeof(client_id);
    int rc;
    rc = write(client_id,client_id_to_send,client_id_size);

    //Create this character
    //Send them the world state
    //Add them to the world

    std::string reqString;

    //Request from the client
    while (1) {
        bzero(request, sizeof(request));
        bytes_read = recv(client_id, request, sizeof(request), 0);
        check_if_error(bytes_read, "Error reading from client");
        
        char response[BUFSIZ +1];
        bzero(response,sizeof(response));
        int bytes_written = 0;
        process_request(request);
        reqString = std::string str(request);
        for (const auto& c : clients)
        {
            bytes_written = write( c, response, sizeof(response) );
        }
    }
    close(client);
}

void process_request(char* request)
{
    char *req;
    char **headers;
    char *message;
    unsigned int numHeaders = 0;
    parseRequest(request,req,headers,message,numHeaders);
    std::unordered_map<std::string,std::string> key_and_values;
    for (int i = 0; i < numHeaders; i++) {
        char *key;
        char *value;
        parseHeader(headers,key,value);
        //Add each header name and value into the unordered_map
        key_and_values.insert(std::make_pair(std::string s(key), std::string s(value)));
    }
    //Key and values should now hold all the "important" values of the character that was sent
    updateOrAddUser(key_and_values);
}

void updateOrAddUser(std::unordered_map<std::string,std::string> key_and_values)
{
    std::string charId = key_and_values["id"];
    if (world_state.find(id) == m.end()) {
        addUser(key_and_values);
    }
    else {
        updateUser(key_and_values);
    }
}

void addUser(std::unordered_map<std::string,std::string> key_and_values)
{

}

/*
    Function to determine if a given parameter is in this map made by the user
*/
bool inMap(std::unordered_map<std::string,std::string> key_and_values, std::string key) {
    if (key_and_values.find(key) == m.end()) {
        return false;
    }
    return true;
}

void updateUser(std::unordered_map<std::string,std::string> key_and_values)
{
    std::string charId = key_and_values.at("id");
    //Get the user to be updated
    Character myChar = world_state[charId];

    std::string dancing = "dancing";
    std::string inputting = "inputting";
    std::string x = "xPos";
    std::string y = "yPos";

    bool isDancing;
    bool isInputting;

    int xInt;
    int yInt;

    if (inMap(key_and_values,dancing)) {
        dancing = key_and_values[dancing];
        istringstream(dancing) >> isDancing;
    }

    if (inMap(key_and_values,inputting)) {
        inputting = key_and_values[inputting];
        istringstream(inputting) >> isInputting;
    }

    if (inMap(key_and_values,x)) {
        x = key_and_values[x];
        xInt = std::stoi (x,nullptr);
    }

    if (inMap(key_and_values,y)) {
        y = key_and_values[y];
        yInt = std::stoi(y,nullptr);
    }
    



}

int main()
{
    int port = 4310;
    int socket;
    socket = create_server_socket(port);
    int client;

    while (true) {
        client = accept(socket, (struct sockaddr *)NULL, NULL);
        cout << "Connected to client!" << endl;
        
        std::thread tid;
        tid = std::thread(handle_client,std::ref(client));
        tid.detach();
    }

    return 0;
}