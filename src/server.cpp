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
#include <signal.h>
#include <unordered_map>
#include <mutex>
#include "server.hpp"
#include "Parser.h"
#include "config.h"

#define M_SIZE 1024

#define SA struct sockaddr

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

/*
    Function to create a server socket and bind it.
    port: integer representation of desired port.
*/
int Server::create_server_socket(int port)
{
    signal(SIGPIPE, SIG_IGN);
    //Declare sockaddr_in struct for use with socket
    struct sockaddr_in server_address;

    int opt = 1;

    //Clear the server address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;

    //Check the flag from config.h to use local host or not
    if (NO_FRIENDS)
    {
        server_address.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS_LOCAL);

    }
    //Establish the port
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
    //Let user know if they're on local host or not
    if (NO_FRIENDS)
    {
        std::cout << "Running on localhost" << std::endl;
    }
    //Otherwise print their ip
    else {
        std::cout << "Running on " << IP_ADDRESS_LOCAL << std::endl;
    }

    return sock;
}

/*
    Function to add a user to the server
    key_and_values: unordered_map of user attributes
*/
void Server::addUser(std::unordered_map<std::string,std::string> key_and_values)
{
    std::cout << key_and_values["name"] << " has joined!" << std::endl;
    //Add them directly to the world_state
    world_state[key_and_values["id"]] = key_and_values;
    //Print their attributes (name, id, location, etc.)
    for (auto kv : key_and_values)
    {
        std::cout << kv.first << " : " << kv.second << std::endl;
    }
    return;
}

/*
    Function to determine if a given parameter is in this map made by the user
    key_and_values: unordered_map received from a client
    key: key string to check and see if its in the unordered_map
*/
bool Server::inMap(std::unordered_map<std::string,std::string> key_and_values, std::string key) {
    if (key_and_values.find(key) == key_and_values.end()) {
        return false;
    }
    return true;
}

/*
    Function to update a user
    key_and_values: values to update in the user
*/
void Server::updateUser(std::unordered_map<std::string,std::string> key_and_values)
{
    std::cout << "Updating " << key_and_values["name"] << std::endl;

    //For checking if the appropriate key is in the updating data
    std::string dancing = "dancing";
    std::string inputting = "inputting";
    std::string x = "xPos";
    std::string y = "yPos";

    bool isDancing;
    bool isInputting;

    //Lock the world state for this update
    world_state_mutex.lock();
    std::unordered_map<std::string,std::string> user = world_state.at(key_and_values.at("id"));

    //Update the user's dancing if necessary
    if (Server::inMap(key_and_values,dancing)) {
        dancing = key_and_values[dancing];
        user["dancing"] = dancing;
        std::cout << user["name"] << " now dancing" << std::endl;
    }

    //Update the user's inputting if necessary
    if (Server::inMap(key_and_values,inputting)) {
        inputting = key_and_values[inputting];
        user["inputting"] = inputting;
        std::cout << user["name"] << " now inputting" << std::endl;
    }

    //Update the user's xPos if necessary
    if (Server::inMap(key_and_values,x)) {
        x = key_and_values[x];
        user["xPos"] = x;
        std::cout << user["name"] << " now moving" << std::endl;
    }

    //Update the user's yPos if necessary
    if (Server::inMap(key_and_values,y)) {
        y = key_and_values[y];
        user["yPos"] = y;
    }

    //Update the user's timestamp if it's there
    if (Server::inMap(key_and_values,"time"))
    {
        user["time"] = key_and_values["time"];
    }

    //Replace the old user with the updated user
    world_state[key_and_values["id"]] = user;
    std::cout << key_and_values["id"] << " now updated:" << std::endl;

    //unlock the world_state_mutex
    world_state_mutex.unlock();
}

/*
    Function to add or update a user into the server's worldstate
    key_and_values: unordered_map of information about the user to be updated
    or created
*/
void Server::updateOrAddUser(std::unordered_map<std::string,std::string> key_and_values)
{
    //If this request has no name or id, can't process it.
    if (!Server::inMap(key_and_values,"name") || !Server::inMap(key_and_values,"id"))
    {
        return;
    }
    //Get the character's id
    std::string charId = key_and_values["id"];
    //If they're not in the world yet, add them!
    if (world_state.find(charId) == world_state.end()) {
        Server::addUser(key_and_values);
    }
    //Otherwise, add them into the world.
    else {
        Server::updateUser(key_and_values);
    }
}

/*
    Function to echo a message to the whole world
    request: message request
    cid: id of the client who sent this message
*/
void Server::echo_message_to_world(char* request, int cid) {
    std::cout << "Echoing a message request..." << std::endl;
    int bytes_written = 0;
    //Go through all the clients in the world
    for (auto c : world_state) {
        //Only send this message to people who AREN'T the user.
        if (c.first.compare(std::to_string(cid)) != 0)
        {
            bytes_written = send(std::stoi(c.first,nullptr), request, M_SIZE,0);
            //Close the signal with a client on a bad send
            if (bytes_written < 0)
            {
                close(std::stoi(c.first));
            }
        }
    }
    return;
}

/*
    Function to process a request from a client
    request: cstring of the request
    client_id: file descriptor/id of the client who made the
               request.
*/
void Server::process_request(char* request, int client_id)
{
    //cstring to hold the request type
    char req[M_SIZE];
    //vector to hold the headers
    std::vector<std::string> headers;
    //cstring to hold the message
    char message[M_SIZE];
    //Invoke the parser
    parseRequest(request,req,headers,message);

    //Declare the container for the headers
    std::unordered_map<std::string,std::string> key_and_values;

    for (int i = 0; i < headers.size(); i++) {
        //Key of parsed header
        char key[M_SIZE];
        //Value of parsed header
        char value[M_SIZE];
        //Parse each header line
        parseHeader(headers[i].c_str(),key,value);
        //Add each header name and value into the unordered_map
        key_and_values.insert(std::make_pair(std::string(key), std::string(value)));
    }
    //Clear the header
    headers.clear();

    //Maybe the user only wants the world state?
    if (std::string(req).find("GET") != std::string::npos) {
        send_world_state(client_id, key_and_values);
    }
    //If the user has a message, send it to the world
    else if (key_and_values.find("message") != key_and_values.end()) {
        Server::echo_message_to_world(request,client_id);
    }
    //If the user wants to leave, have them leave.
    else if (std::string(req).find("DELETE") != std::string::npos &&
             key_and_values.find("exit") != key_and_values.end())
    {
        Server::exit_character(key_and_values,client_id);
    }
    //Check if it's a standard HTTP POST request.
    else if (std::string(req).find("POST") != std::string::npos) {
        Server::updateOrAddUser(key_and_values);
    }
    return;
}

/*
    Remove a character from the server
    key_and_values: Character to be removed
    client_id: file descriptor of that client
*/
void Server::exit_character(std::unordered_map<std::string,std::string> key_and_values, int client_id)
{
    world_state.erase(key_and_values["id"]);
    close(client_id);
}

/*
    Send the world state to a client
    client_id: file descriptor/id of client to send to
    key_and_values: the worldstate request that was sent 
                    (to determine if a request is initial)
*/
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

/*
    Function to handle a client
    client_ptr: the file descriptor for the client to be handled
*/
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
        //If there's an error, stop receiving from the client
        if (bytes_read < 0)
        {
            break;
        }
        //If the request is legitimate then process it
        if (std::to_string(request[0]).compare("") != 0) {
            Server::process_request(request, client_id);
        }
        bzero(request, sizeof(request));

    }
    close(client_id);
}
/*
    Function to build an HTTP request
    method: the method of the request (GET, POST, DELETE), etc.
    headers: the headers of the request
*/
std::string Server::build_request(std::string method, std::unordered_map<std::string, std::string> headers)
{ 

    std::string request = "";
    request += (method + " / HTTP/1.1\n");

    for (auto it = headers.begin(); it != headers.end(); it++)
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

/*
    Function to run the server
*/
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

/*
    Main method for the server that makes use of the two public functions
*/
int main() 
{
    Server myServer(PORT);
    myServer.run();
}
