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
#include <mutex>
#include "server.hpp"

#define SA struct sockaddr

/*
    The characters are represented almost entirely like headers, and the outside
    unordered_map exists for quick checking if a user exists in the world.
*/

int Server::check_if_error(int returned_value, std::string error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg.c_str());
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

int Server::create_server_socket(int port)
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
    Server::check_if_error(sock, "Error with socket");

    // Binding newly created socket to given IP and verification 
    if ((bind(sock, (SA*)&server_address, sizeof(server_address))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 

    Server::check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");

    // Attempt to make the socket (fd) a listening type socket
    Server::check_if_error(listen(sock, 10), "Could not make the socket a listening type socket");

    std::cout << "Listening for requests on port " << port << std::endl;

    return sock;
}

void Server::addUser(std::unordered_map<std::string,std::string> key_and_values)
{
    world_state.insert(std::make_pair(key_and_values["id"],key_and_values));
}

/*
    Function to determine if a given parameter is in this map made by the user
*/
bool Server::inMap(std::unordered_map<std::string,std::string> key_and_values, std::string key) {
    if (key_and_values.find(key) == key_and_values.end()) {
        return false;
    }
    return true;
}

void Server::updateUser(std::unordered_map<std::string,std::string> key_and_values)
{
    //This is the user that is going to be updated
    std::unordered_map<std::string,std::string> user = world_state.at(key_and_values.at("id"));

    //For checking if the appropriate key is in the updating data
    std::string dancing = "dancing";
    std::string inputting = "inputting";
    std::string x = "xPos";
    std::string y = "yPos";

    bool isDancing;
    bool isInputting;

    int xInt;
    int yInt;

    if (Server::inMap(key_and_values,dancing)) {
        dancing = key_and_values[dancing];
        user["dancing"] = dancing;
    }

    if (Server::inMap(key_and_values,inputting)) {
        inputting = key_and_values[inputting];
        user["inputting"] = inputting;
    }

    if (Server::inMap(key_and_values,x)) {
        x = key_and_values[x];
        user["xPos"] = x;
    }

    if (Server::inMap(key_and_values,y)) {
        y = key_and_values[y];
        user["yPos"] = y;
    }
}

void Server::updateOrAddUser(std::unordered_map<std::string,std::string> key_and_values)
{
    std::string charId = key_and_values["id"];
    if (world_state.find(charId) == world_state.end()) {
        Server::addUser(key_and_values);
    }
    else {
        Server::updateUser(key_and_values);
    }
}

void Server::echo_message_to_world(char* request) {
    int bytes_written = 0;
    //TODO: should the client that made this message request also receive it from
    //The server?
    for (auto c : world_state) {
        bytes_written = write(std::stoi(c.first,nullptr), request, sizeof(request));
    }
}

void Server::process_request(char* request)
{
    char *req;
    char **headers;
    char *message;
    unsigned int numHeaders = 0;
    parseRequest(request,req,headers,message,&numHeaders);
    std::unordered_map<std::string,std::string> key_and_values;
    for (int i = 0; i < numHeaders; i++) {
        char *key;
        char *value;
        parseHeader(headers[i],key,value);
        //Add each header name and value into the unordered_map
        key_and_values.insert(std::make_pair(std::string(key), std::string(value)));
    }
    //Key and values should now hold all the "important" values of the character that was sent
    if (key_and_values.find("message") != key_and_values.end()) {
        //This is a message! Process it as such
        Server::echo_message_to_world(request);
    }
    else {
        Server::updateOrAddUser(key_and_values);
    }
}

void Server::send_world_state(int client_id) {

    std::string user_serialization;
    for (auto kv : world_state) {
        //Build a post request for the client wherein the info is a user from the world_state
        //TODO: should clients be receiving information about their own positioning?
        //If not, then do a quick check on kv.first to make sure that it is not equal to the
        //client id.
        user_serialization = Server::build_request("POST",kv.second);
        write(client_id,user_serialization.c_str(),sizeof(user_serialization.c_str()));
    }
}

void Server::handle_client(int client_ptr)
{

    std::cout << "Handling client " << client_ptr << std::endl;
    int client_id = client_ptr;

    //Receive from the client their name: That will be the first thing sent
    char request[BUFSIZ + 1];
    bzero(request, sizeof(request));
    int bytes_read = recv(client_id,request,sizeof(request),0);
    std::string name = std::string(request);
    bzero(request, sizeof(request));

    //Write to this client only and give them their client_id;
    char *client_id_to_send = (char*)&client_id;
    int client_id_size = sizeof(client_id);
    int rc;
    rc = write(client_id,client_id_to_send,client_id_size);

    //Send them all the "characters"
    Server::send_world_state(client_id);

    //Create this user and add them to the map
    std::unordered_map<std::string, std::string> user_map;
    user_map["name"] = name;
    user_map["id"] = std::to_string(client_id);
    user_map["xPos"] = "0";
    user_map["yPos"] = "0";
    user_map["dancing"] = "0";
    user_map["inputting"] = "0";

    world_state.insert(std::make_pair(std::to_string(client_id),user_map));

    std::string reqString;

    //Request from the client
    while (1) {
        bzero(request, sizeof(request));
        bytes_read = recv(client_id, request, sizeof(request), 0);
        check_if_error(bytes_read, "Error reading from client");
        
        char response[BUFSIZ +1];
        bzero(response,sizeof(response));
        int bytes_written = 0;
        Server::process_request(request);
        reqString = std::string(request);
    }
    close(client_id);
}

std::string Server::build_request(std::string method, std::unordered_map<std::string, std::string> headers)
{ 

    std::string request = "";
    request += (method + " / HTTP/1.1\n");

    for (auto it = headers.begin(); it != headers.end(); it++) // identifier "header" is undefined
    {
        request += (it->first).c_str();
        request += ":";
        request += (it->second).c_str();
        request += "\n";
    }
    return request;
}

Server::Server(int p)
{
    Server::port = p;
}

int Server::run()
{
    servSock = Server::create_server_socket(port);
    int client;

    while (true) {
        client = accept(servSock, (struct sockaddr *)NULL, NULL);
        std::cout << "Connected to client!" << std::endl;
        
        std::thread tid;
        tid = std::thread(&Server::handle_client,this,std::ref(client));
        tid.detach();
    }

    return 0;
}

int main() 
{
    Server myServer(4310);
    myServer.run();
}