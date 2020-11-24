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

#define SA struct sockaddr

struct ServerCharacter
{
    int id = -1;
    std::string name; //Name of character;
    int xpos; //x position of the character;
    int ypos;
    bool inputting;
    bool dancing;
};



Client::Client(int p, std::string name)
{
    port = p;
    client_name = name;
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
    recv(sock,r_message,sizeof(r_message,0),0);
    std::string id_num(r_message);
    client_id = std::stoi (r_message, nullptr);
    bzero(&r_message,sizeof(r_message));
}

std::string Client::build_request(std::string method, std::list<std::string> headers) // argument list for class template "std::unordered_map" is missing
{ 
    std::list<std::string>::const_iterator it;

    std::string request = "";
    request += (method + " / HTTP/1.1\n");

    request += "ID: " + client_id;
    request += "Name: " + client_name;
    for (it = headers.begin(); it != headers.end(); ++it) // identifier "header" is undefined
    {
        request += it->c_str();
        request += ": ";
        ++it;
        request += it->c_str();
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