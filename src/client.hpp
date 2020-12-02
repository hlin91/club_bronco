#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <thread>
#include <string>
#include <mutex>
#include <deque>
#include <unordered_map>
#include "character.cpp"
#include <sstream>

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
    std::stringstream ss;

    int check_if_error(int returned_value, char *error_msg);
    void create_server_socket();
    void receive_response();
    int get_and_set_id();
    std::unordered_map<std::string,std::string> getDefaultHeaders();
    std::string pop_response();
    void setId(std::string i);
    std::string getName();
    std::string getId();
    std::string build_request(std::string method, std::unordered_map<std::string, std::string> headers);
    void send_request(std::string request);
    bool fieldInMap(std::unordered_map<std::string,std::string>&,std::string);
    void executeResponse(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string>& response_headers);
    void executeMessage(std::deque<std::string>& messages, std::unordered_map<std::string,std::string>& response_headers,const unsigned int MAX_MESSAGES);
    void updateCharacter(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string> response_headers);
    void addCharacter(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string>& response_headers);
    std::unordered_map<std::string, std::string> processResponse(std::string);


    std::string name;
    std::string id;

public:
    Client(int p, std::string n);

    void pollState(std::unordered_map<unsigned int, Character>&, std::deque<std::string>&, const unsigned int);
    int getWorldState(std::unordered_map<unsigned int, Character>&);
    void sendMessage(std::string message);
    void sendMovement(float, float);
    void sendInputting(bool);
    void sendDancing(bool);
    void sendInitial(float,float,bool,bool);
    void sendWSRequest();
    int run();
    void sendExit();
    void setName(std::string);
};

#endif