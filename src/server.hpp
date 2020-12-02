#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <unordered_map>
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
#include <mutex>


class Server
{   

public:
    int port;
    int servSock;
    std::unordered_map<std::string, std::unordered_map<std::string,std::string> > world_state;
    std::mutex world_state_mutex;

    Server(int p);

    int check_if_error(int returned_value, std::string error_msg);
    int run();
    int create_server_socket(int port);
    void addUser(std::unordered_map<std::string,std::string> key_and_values);
    bool inMap(std::unordered_map<std::string,std::string> key_and_values, std::string key);
    void updateUser(std::unordered_map<std::string,std::string> key_and_values);
    void updateOrAddUser(std::unordered_map<std::string,std::string> key_and_values);
    void echo_message_to_world(char* request, int client_id);
    void process_request(char* request, int client_id);
    void send_world_state(int client_id,std::unordered_map<std::string, std::string>&);
    void handle_client(int client_ptr);
    std::string build_request(std::string method, std::unordered_map<std::string, std::string> headers);
};

#endif
