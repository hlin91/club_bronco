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
#include <vector>
#include <thread>
#include <unordered_map>
#include <mutex>
#include "server.hpp"
#include "Parser.h"
#include "config.h"

#define M_SIZE 1024

#define SA struct sockaddr

/*
    The characters are represented almost entirely like headers, and the outside
    unordered_map exists for quick checking if a user exists in the world.
*/

int Server::check_if_error(int returned_value, std::string error_msg)
{
    if (returned_value < 0)
    {
        //Print the sent in error message and exit with the failure
        perror(error_msg.c_str());
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

int Server::create_server_socket(int port)
{

    struct sockaddr_in server_address;

    int opt = 1;

    //Clear the server address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;

    //Use the ip address and port from the config file
    if (NO_FRIENDS)
    {
        server_address.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS_LOCAL);

    }
    server_address.sin_port = htons(PORT);
    
    //Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //Check for error with socket
    Server::check_if_error(sock, "Error with socket");

    Server::check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "setsockopt");

    // Binding newly created socket to given IP and verification 
    int bind_val = bind(sock, (SA*)&server_address, sizeof(server_address));
    Server::check_if_error(bind_val, "Error with binding");

    // Attempt to make the socket (fd) a listening type socket
    Server::check_if_error(listen(sock, 10), "Could not make the socket a listening type socket");

    //message to declare running
    std::cout << "Listening for requests on port " << port << std::endl;
    if (NO_FRIENDS)
    {
        std::cout << "Running on localhost" << std::endl;
    }

    return sock;
}

void Server::addUser(std::unordered_map<std::string,std::string> key_and_values)
{
    std::cout << key_and_values["name"] << " has joined!" << std::endl;
    world_state[key_and_values["id"]] = key_and_values;
    for (auto kv : key_and_values)
    {
        std::cout << kv.first << " : " << kv.second << std::endl;
    }
    return;
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
    if (inMap(key_and_values,"message"))
    {
        return;
    }
    std::cout << "Updating " << key_and_values["name"] << std::endl;
    world_state_mutex.lock();
    std::unordered_map<std::string,std::string> user = world_state.at(key_and_values.at("id"));

    //For checking if the appropriate key is in the updating data
    std::string dancing = "dancing";
    std::string inputting = "inputting";
    std::string x = "xPos";
    std::string y = "yPos";

    bool isDancing;
    bool isInputting;

    if (Server::inMap(key_and_values,dancing)) {
        dancing = key_and_values[dancing];
        user["dancing"] = dancing;
        std::cout << user["name"] << " now dancing" << std::endl;
    }

    if (Server::inMap(key_and_values,inputting)) {
        inputting = key_and_values[inputting];
        user["inputting"] = inputting;
        std::cout << user["name"] << " now inputting" << std::endl;
    }

    if (Server::inMap(key_and_values,x)) {
        x = key_and_values[x];
        user["xPos"] = x;
        std::cout << user["name"] << " now moving" << std::endl;
    }

    if (Server::inMap(key_and_values,y)) {
        y = key_and_values[y];
        user["yPos"] = y;
        std::cout << user["name"] << " now moving" << std::endl;
    }

    if (Server::inMap(key_and_values,"time"))
    {
        user["time"] = key_and_values["time"];
    }

    //Replace the old user with the updated user
    world_state[key_and_values["id"]] = user;
    std::cout << key_and_values["id"] << " now updated:" << std::endl;
    for (auto thing : user)
    {
        std::cout << thing.first << " : " << thing.second << std::endl;
    }

    world_state_mutex.unlock();
}

void Server::updateOrAddUser(std::unordered_map<std::string,std::string> key_and_values)
{
    if (!Server::inMap(key_and_values,"name") || !Server::inMap(key_and_values,"id"))
    {
        return;
    }
    std::string charId = key_and_values["id"];
    if (world_state.find(charId) == world_state.end()) {
        Server::addUser(key_and_values);
    }
    else {
        Server::updateUser(key_and_values);
    }
}

void Server::echo_message_to_world(char* request, int cid) {
    std::cout << "Echoing a message request..." << std::endl;
    int bytes_written = 0;
    //TODO: should the client that made this message request also receive it from
    //The server?
    for (auto c : world_state) {
        //Only send this message to people who AREN'T the user.
        if (c.first.compare(std::to_string(cid)) != 0)
        {
            //TODO check if client is still valid somehow?
            bytes_written = send(std::stoi(c.first,nullptr), request, M_SIZE,MSG_NOSIGNAL);
        }
    }
    return;
}

void Server::process_request(char* request, int client_id)
{
    char req[M_SIZE];
    std::vector<std::string> headers;
    char message[M_SIZE];
    parseRequest(request,req,headers,message);

    std::unordered_map<std::string,std::string> key_and_values;
    for (int i = 0; i < headers.size(); i++) {
        char key[M_SIZE];
        char value[M_SIZE];
        parseHeader(headers[i].c_str(),key,value);
        //Add each header name and value into the unordered_map
        key_and_values.insert(std::make_pair(std::string(key), std::string(value)));
    }
    headers.clear();

    //Maybe the user only wants the world state?
    if (std::string(req).find("GET") != std::string::npos) {
        send_world_state(client_id, key_and_values);
    }
    else if (key_and_values.find("message") != key_and_values.end()) {
        Server::echo_message_to_world(request,client_id);
    }
    else if (key_and_values.find("exit") != key_and_values.end())
    {
        Server::exit_character(key_and_values,client_id);
        Server::echo_message_to_world(request,client_id);
    }
    else {
        Server::updateOrAddUser(key_and_values);
    }
    return;
}

void Server::exit_character(std::unordered_map<std::string,std::string> key_and_values, int client_id)
{
    close(client_id);
    world_state.erase(key_and_values["id"]);
}

void Server::send_world_state(int client_id, std::unordered_map<std::string, std::string>& key_and_values) {
    bool sending = false;
    std::string user_serialization;
    unsigned int reqTimeStamp = 0;
    unsigned int charTimeStamp = 0;
    for (auto kv : world_state) {
        //Don't send a user their own information!
        if (std::stoi(kv.first,nullptr) != client_id)
        {
            //Check for correct timing
            reqTimeStamp = std::stoi(key_and_values["time"]);
            charTimeStamp = std::stoi(kv.second["time"]);

            //Only compare timestamp if this is not an initialrequest
            if (reqTimeStamp > charTimeStamp && !inMap(key_and_values, "initial"))
            {
                continue;
            }

            //Serialize a user into a POST request
            user_serialization = Server::build_request("POST",kv.second);
            //Write this user serialization to the user who requested it.
            write(client_id,&user_serialization[0],M_SIZE);
            sending = true;
        }
    }
}

void Server::handle_client(int client_ptr)
{
    int client_id = client_ptr;

    //Instantiate and clear a buffer
    char request[M_SIZE];
    bzero(request, sizeof(request));
    //Read the client's name into the buffer
    int bytes_read = recv(client_id,request,M_SIZE,0);
    //Save into name string
    std::string name = std::string(request);
    std::cout << "Handling Client name: " << name << std::endl;
    bzero(request, sizeof(request));

    //Write to this client only and give them their client_id;
    std::string client_id_str = std::to_string(client_id);
    int rc;
    std::cout << "Sending id to " << name << std::endl;
    rc = write(client_id,&client_id_str[0],M_SIZE);


    while (1) {
        bzero(request, sizeof(request));
        bytes_read = recv(client_id, request, M_SIZE, 0);
        if (bytes_read < 0)
        {
            break;
        }
        if (std::to_string(request[0]).compare("") != 0) {
            Server::process_request(request, client_id);
        }
        bzero(request, sizeof(request));

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
    request += "\n!";
    return request;
}

Server::Server(int p)
{
    Server::port = p;
}

int Server::run()
{
    servSock = Server::create_server_socket(PORT);
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
    Server myServer(PORT);
    myServer.run();
}
