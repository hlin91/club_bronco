#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <thread>
#include <string>
#include <mutex>
#include <deque>
#include <unordered_map>
#include "game_cli.cpp"

class Client
{

private:
    char s_message[2048];
    std::mutex response_queue_mutex;
    std::deque<std::string> response_queue;
    char r_message[2048];
    int sock;
    bool open_for_sending = true;
    bool open_for_receiving = true;
    int port = 4310;
    std::thread thread_receive;

    int check_if_error(int returned_value, char *error_msg);
    void create_server_socket();
    void receive_response();
    int get_and_set_id();

    std::string name;
    std::string id;

public:
    Client(int p, std::string n);
    void setId(std::string i);
    std::string getName();
    std::string getId();
    std::string build_request(std::string method, std::unordered_map<std::string, std::string> headers);
    void send_request(std::string request);
    std::unordered_map<std::string, std::string> Client::processResponse(std::string response)

    std::unordered_map<std::string,std::string> getDefaultHeaders();
    void pollState(std::unordered_map<unsigned int, Character>&, std::deque<std::string>&);
    int getWorldState(std::unordered_map<unsigned int, Character>&);
    void sendMessage(std::string message);
    void sendMovement(float, float);
    void sendInputting(bool);
    void sendDancing(bool);
    void sendInitial(float,float,bool,bool);
    std::string pop_response();
    int run();
    void sendExit();
};

#endif