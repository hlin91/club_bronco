#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <thread>
#include <string>
#include <mutex>

class Client
{

private:
    char s_message[2048];
    std::mutex response_queue_mutex;
    std::list<std::string> response_queue;
    char r_message[2048];
    int sock;
    bool open_for_sending = true;
    bool open_for_receiving = true;
    int port = 4310;
    std::thread thread_receive;
    int client_id = -1;
    std::string client_name = "";

    int check_if_error(int returned_value, char *error_msg);
    void create_server_socket();
    void receive_response();
    int get_and_set_id();

public:
    Client(int p, std::string name);
    std::string build_request(std::string method, std::list<std::string> headers);
    void send_request(std::string request);
    std::string pop_response();
    int run();
};

#endif