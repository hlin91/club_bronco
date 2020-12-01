#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <errno.h>
#include <thread>
#include <list>
#include <iterator>
#include <vector>
#include <pthread.h>
#include <string>
#include <mutex>
#include "client.hpp"
#include "Parser.h"
#include <chrono>
#include <unordered_map>

#define SA struct sockaddr

Client::Client(int p, std::string n)
{
    std::cout << "Starting client..." << std::endl;
    Client::port = p;
    Client::name = n;
}

void Client::setName(std::string s)
{
    std::cout << "Setting name to " << s << std::endl;
    Client::name = s;
}

std::unordered_map<std::string,std::string> Client::getDefaultHeaders()
{
    std::unordered_map<std::string,std::string> headers;
    headers["id"] = Client::getId();
    headers["name"] = Client::getName();
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);
    //TODO: account for timestamp
    return headers;
}

/*
    NOTE: This function is only called after a successful handshake,
    hence the popping that takes place without a server request.
*/
int Client::getWorldState(std::unordered_map<unsigned int, Character>& others)
{
    std::cout << "getting world state..." << std::endl;
    std::string resp = Client::pop_response();
    std::unordered_map<std::string,std::string> response_headers;
    //Process the entire queue
    while (resp.compare("") != 0) {
        response_headers = Client::processResponse(resp);
        executeResponse(others,response_headers);
        resp = Client::pop_response();
    }
    return std::stoi(Client::getId());
}

void Client::pollState(std::unordered_map<unsigned int, Character>& others, std::deque<std::string>& messages, const unsigned int MAX_MESSAGES)
{
    //Tell server to give information
    sendWSRequest();
    std::this_thread::sleep_for (std::chrono::seconds(1));
    //Process each response
    std::unordered_map<std::string,std::string> response_headers;
    std::string resp = Client::pop_response();
    while (resp.compare("") != 0) {
        response_headers = Client::processResponse(resp);
        //Check if this has a message within
        if (response_headers.find("message") != response_headers.end())
        {
            executeMessage(messages, response_headers,MAX_MESSAGES);
        }
        else
        {
            executeResponse(others,response_headers);
        }
        resp = Client::pop_response();
    }

}

void Client::executeResponse(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string>& response_headers)
{
    //Check if this user exists
    if (others.find(std::stoi(response_headers["id"])) == others.end())
    {
        Client::addCharacter(others,response_headers);
    }
    else
    {
        Client::updateCharacter(others,response_headers);
    }
}

void Client::executeMessage(std::deque<std::string>& messages, std::unordered_map<std::string,std::string>& response_headers, const unsigned int MAX_MESSAGES)
{
    std::string user_message = response_headers["name"] + ": " + response_headers["message"];
    if (messages.size() >= MAX_MESSAGES)
    {
        messages.pop_front();
    }
    messages.push_back(user_message);
}

void Client::updateCharacter(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string> response_headers)
{
    //Get the character out of the others using the id (remember, id is a string, gonna have to convert to int!)
    others[std::stoi(response_headers["id"])];
    if (fieldInMap(response_headers,"xPos") && fieldInMap(response_headers,"yPos"))
    {
        float xLoc = std::stof(response_headers["xPos"],nullptr);
        float yLoc = std::stof(response_headers["yPos"],nullptr);
        others[std::stoi(response_headers["id"])].move(xLoc,yLoc);
    }
    if (fieldInMap(response_headers,"dancing"))
    {
        std::istringstream(response_headers["dancing"]) >> others[std::stoi(response_headers["id"])].dancing;
    }
    if (fieldInMap(response_headers,"inputting"))
    {
        std::istringstream(response_headers["inputting"]) >> others[std::stoi(response_headers["id"])].inputting;
    }
    return;
}

bool Client::fieldInMap(std::unordered_map<std::string,std::string>& key_and_values, std::string key) {
    if (key_and_values.find(key) == key_and_values.end()) {
        return false;
    }
    return true;
}

void Client::addCharacter(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string>& response_headers)
{
    //Go through the response headers for the relevant information
    unsigned int cid = std::stoi(response_headers["id"],nullptr);
    std::string cname = response_headers["name"];
    float xVal = std::stof(response_headers["xPos"],nullptr);
    float yVal = std::stof(response_headers["yPos"],nullptr);
    bool isDancing;
    bool isInputting;
    std::istringstream(response_headers["dancing"]) >> isDancing;
    std::istringstream(response_headers["inputting"]) >> isInputting;

    Character c = Character(cname,cid,xVal,yVal,xVal,yVal);
    c.inputting = isInputting;
    c.dancing = isDancing;

    others.insert(std::make_pair(cid,c));

    return;
}

/*
    Function to demodulate a string response into its headers
*/
std::unordered_map<std::string, std::string> Client::processResponse(std::string response) {
    std::cout << "Processing response from server..." << std::endl;
    char req[1024];
    char *headers[12];
    char message[1024];
    unsigned int numHeaders = 0;
    parseRequest(&response[0],req,headers,message,&numHeaders);

    std::unordered_map<std::string,std::string> key_and_values;
    for (int i = 0; i < numHeaders; i++) {
        char key[1024];
        char value[1024];
        parseHeader(headers[i],key,value);
        key_and_values.insert(std::make_pair(std::string(key), std::string(value)));
    }
    return key_and_values;
}

void Client::sendWSRequest()
{
    std::cout << "Sending request for world state" << std::endl;
    std::string method = "GET";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    std::string request = build_request(method,headers);
    std::cout << request <<std::endl;
    send_request(request);
}

void Client::sendInitial(float xPos = 0, float yPos = 0, bool isDancing = false, bool isInputting = false)
{
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    std::string x_string = std::to_string(xPos);
    std::string y_string = std::to_string(yPos);
    headers["xPos"] = x_string;
    headers["yPos"] = y_string;

    std::string i_string = std::to_string(isInputting);
    headers["inputting"] = i_string;

    std::string d_string = std::to_string(isDancing);
    headers["dancing"] = d_string;

    std::string request = build_request(method,headers);
    send_request(request);
}

void Client::sendMovement(float xPos,float yPos)
{
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    std::string x_string = std::to_string(xPos);
    std::string y_string = std::to_string(yPos);

    headers["xPos"] = x_string;
    headers["yPos"] = y_string;

    std::string request = build_request(method,headers);
    send_request(request);
}

void Client::sendInputting(bool i) {
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers;
    std::string i_string = std::to_string(i);
    headers["inputting"] = i_string;

    std::string request = build_request(method, headers);
    send_request(request);
}

void Client::sendDancing(bool d) {
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    std::string d_string = std::to_string(d);
    headers["dancing"] = d_string;

    std::string request = build_request(method, headers);
    send_request(request);
}

void Client::sendMessage(std::string message)
{
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    headers["message"] = message;
    std::string request = build_request(method, headers);
    send_request(request);
}

void Client::sendExit()
{
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    headers["exit"] = "true";
    std::string request = build_request(method,headers);
    send_request(request);
}

int Client::check_if_error(int returned_value, char *error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

void Client::create_server_socket()
{
    struct sockaddr_in server_address;

    int opt = 1;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    //Create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    //Check for error with socket
    check_if_error(sock, "Error with socket");

    check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");

    //Connect to server
    connect(sock, (SA*)&server_address,sizeof(server_address));
}

void Client::receive_response() 
{
    std::cout << "Now receiving from server..." << std::endl;
    
    int bytes_read = 0;

    while (open_for_receiving)
    {
        bytes_read = recv(sock,r_message,1024,0);
        while (bytes_read < 10){
            bytes_read = recv(sock,r_message,1024,0);
        }
        
        if (r_message[0] == '\0') {
            bzero(&r_message,sizeof(r_message));
            continue;
        }
        else
        {
            std::cout << "getting info from server..." << std::endl << std::endl;
            std::cout << "B***" << std::endl << r_message << "***E" << std::endl;
            response_queue_mutex.lock();
            response_queue.push_back(std::string(r_message));
            response_queue_mutex.unlock();
            bzero(&r_message,sizeof(r_message));
        }

    }
}

void Client::setId(std::string i) {
    Client::id = i;
}

std::string Client::getName() {
    return name;
}

std::string Client::getId() {
    return id;
}

int Client::get_and_set_id() {
    std::cout << "sending name to server" << std::endl;
    send_request(Client::name);
    std::cout << "receiving id from server" << std::endl;
    recv(sock,r_message,sizeof(r_message),0);
    Client::setId(std::string(r_message));
    std::cout << "id of this client is: " << Client::getId() << std::endl;
    bzero(&r_message,sizeof(r_message));
    return std::stoi(Client::getId());
}

std::string Client::build_request(std::string method, std::unordered_map<std::string, std::string> headers)
{ 
    std::cout << "Sending " << method << " request" << std::endl;
    std::string request = "";
    request += (method + " / HTTP/1.1\n");

    for (auto it = headers.begin(); it != headers.end(); it++) // identifier "header" is undefined
    {
        request += (it->first).c_str();
        request += ":";
        request += (it->second).c_str();
        request += "\n";
    }
    request += "\n";
    return request;
}

void Client::send_request(std::string request)
{
    char c_request[2048];
    strcpy(c_request, request.c_str());
    send(sock,c_request,strlen(c_request), 0);

}

std::string Client::pop_response()
{   
    std::cout << "popping response from response queue" << std::endl;
    std::string oldest_response = "";

    response_queue_mutex.lock();

    if (!response_queue.empty())
    {
        oldest_response = response_queue.front();
        response_queue.pop_front();
    }

    response_queue_mutex.unlock();
    return oldest_response;
}

/*

    Function to start receiving from the server

*/
int Client::run()
{
    create_server_socket();
    get_and_set_id();
    thread_receive = std::thread(&Client::receive_response, this);
    thread_receive.detach();
    return 0;
}

// int main()
// {
//     Client myClient(4310, "Johnny");
//     myClient.run();
//     //myClient.sendInitial();
// }
