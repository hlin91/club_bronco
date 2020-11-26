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
#include "game_cli.cpp"
#include <chrono>

#define SA struct sockaddr

Client::Client(int p, std::string name)
{
    port = p;
    client_name = name;

}

std::unordered_map<std::string,std::string> getDefaultHeaders() {
    std::unordered_map<std::string,std::string> headers;
    headers["id"] = client_id;
    headers["name"] = client_name;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);
    return headers;
}

void Client::sendMovement(float xPos,float yPos)
{
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
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
    while (open_for_receiving)
    {
        recv(sock,r_message,sizeof(r_message),0);
        if (r_message[0] == '\0') {
            continue;
        }
        else
        {
            response_queue_mutex.lock();
            response_queue.push_back(r_message);
            response_queue_mutex.unlock();
            bzero(&r_message,sizeof(r_message));
        }

    }
}

int Client::get_and_set_id() {
    send_request(client_name);
    recv(sock,r_message,sizeof(r_message,0),0);
    std::string id_num(r_message);
    client_id = std::stoi (r_message, nullptr);
    bzero(&r_message,sizeof(r_message));
}

std::string Client::build_request(std::string method, std::unordered_map<std::string, std::string> headers)
{ 

    std::string request = "";
    request += (method + " / HTTP/1.1\n");

    for (auto it = headers.begin(); it != headers.end(); it++) // identifier "header" is undefined
    {
        request += it->first->c_str();
        request += ":";
        request += it->second->c_str();
        request += "\n";
    }
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
    std::string oldest_response;

    response_queue_mutex.lock();

    oldest_response.assign(response_queue.front());
    response_queue.pop_front();

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
    std::cout << "My id is: " + client_id << std::endl;
    thread_receive = std::thread(&Client::receive_response, this);
    thread_receive.detach();
    return 0;
}

int main()
{
    Client myClient(4310, "Johnny");
    myClient.run();
}