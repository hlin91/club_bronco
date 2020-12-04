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
#include <ctime>
#include <unordered_map>
#include "config.h"

#define SA struct sockaddr
#define MBOX_CHAR_W = 45;
#define M_SIZE 1024

#define IP_ADDRESS "172.88.76.72"

/*
    Constructor for the client that takes in the
    desired port number and the name of this client
    p: desired port
    n: desired name
*/
Client::Client(int p, std::string n)
{
    std::cout << "Starting client..." << std::endl;
    Client::port = p;
    Client::name = n;
}

/*
    Sets the name of this client object
    s: new name for client
*/
void Client::setName(std::string s)
{
    std::cout << "Setting name to " << s << std::endl;
    Client::name = s;
}

/*
    returns an unordered_map of the id, name, and generated time of
    this request. Other headers can be added depending on the desired
    outcome.
*/
std::unordered_map<std::string,std::string> Client::getDefaultHeaders()
{
    std::unordered_map<std::string,std::string> headers;
    headers["id"] = Client::getId();
    headers["name"] = Client::getName();

    //Calculate the current system time.
    std::time_t seconds;
    std::time(&seconds);
    ss << seconds;
    std::string seconds_str = ss.str();
    
    //Add the time into the headers
    headers["time"] = seconds_str;
    ss.str(std::string());

    return headers;
}

/*
    Get the initial world state from the server
    others: a reference passed by the game client that will be populated
            by the client
*/
int Client::getWorldState(std::unordered_map<unsigned int, Character>& others)
{
    //Send an initial request to the world state.
    sendInitialWSRequest();

    //Sleep because the server needs some time to respond
    std::this_thread::sleep_for (std::chrono::milliseconds(83));
    std::cout << "getting world state..." << std::endl;

    //get a string from the response queue
    std::string resp = Client::pop_response();

    //instantiate the headers that will hold the information in resp
    std::unordered_map<std::string,std::string> response_headers;
    //Process the entire queue
    while (resp.compare("") != 0) {
        response_headers = Client::processResponse(resp);
        executeResponse(others,response_headers);
        resp = Client::pop_response();
    }
    //Return the client's ID, a requirement of the API provided to the Game Client
    return std::stoi(Client::getId());
}

/*
    Function much like getWorldState but this is specifically for polling the server
    after the initial handshake
    others: unordered map of the other players that will be mutated by the client
    messages: current messages of the game client, client will update this list
              with any messages sent since the last update
    MAX_MESSAGES: an integer that the client will use in order to determine if some
                  older messages should be tossed.
*/
void Client::pollState(std::unordered_map<unsigned int, Character>& others, std::deque<std::string>& messages, const unsigned int MAX_MESSAGES)
{
    //Tell server to give information
    sendWSRequest();
    std::this_thread::sleep_for (std::chrono::milliseconds(83));
    //Process each response
    std::unordered_map<std::string,std::string> response_headers;
    std::string resp = Client::pop_response();
    while (resp.compare("") != 0) {
        response_headers = Client::processResponse(resp);
        //Check if this has a message within
        if (response_headers.find("message") != response_headers.end())
        {
            executeMessage(messages,response_headers,MAX_MESSAGES);
        }
        else
        {
            executeResponse(others,response_headers);
        }
        resp = Client::pop_response();
    }

}

/*
    Function to check if a response from the server is valid, and if it's
    valid, determine if a character needs to be updated or added.
    others: current information of the other users
    response_headers: A demodulated request that is no longer a long string
                      but instead is an unordered map for easy access
*/
void Client::executeResponse(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string>& response_headers)
{
    //Check if these response headers are even valid.
    if (!fieldInMap(response_headers,"name") || !fieldInMap(response_headers,"id"))
    {
        return;
    }
    //Check if this user exists
    if (others.find(std::stoi(response_headers["id"])) == others.end())
    {
        //Add them if they don't exist
        Client::addCharacter(others,response_headers);
    }
    else
    {
        //update them if they do exist
        Client::updateCharacter(others,response_headers);
    }
}

/*
    Method to take a message and put it into messages
    messages: current messages that the game client has
    response_headers: information about and including the
                      message to be displayed
    MAX_MESSAGES: maximum number of messages allowed in messages
*/
void Client::executeMessage(std::deque<std::string>& messages, std::unordered_map<std::string,std::string>& response_headers, const unsigned int MAX_MESSAGES)
{
    std::cout << "Receiving message..." << std::endl;
    //Construct the message
    std::string m = response_headers["name"] + ": " + response_headers["message"];
    //Cut the message if necessary
    messages.push_back(m.substr(0,MSG_LENGTH));
    //If there's still some message left...
    if (m.size() > MSG_LENGTH)
    {
        //Put the rest of the message in the deque
        messages.push_back(m.substr(MSG_LENGTH));
    }
    //Pop the oldest messages while the MAX_MESSAGES
    //is breached
    while (messages.size() > MAX_MESSAGES)
    {
        messages.pop_front();
    }
}

/*
    Function to update a character in the game client world_state
    others: Characters of the invoking game client's world
    response_headers: The information about a character to be updated
*/
void Client::updateCharacter(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string> response_headers)
{
    //check if this response from the server is even valid
    if (!fieldInMap(response_headers,"name") || !fieldInMap(response_headers,"id"))
    {
        return;
    }

    //Exit the character if they want to exit
    if (fieldInMap(response_headers,"exit"))
    {
        others.erase(std::stoi(response_headers["id"]));

    }
    
    //If they sent new location data, move the character
    if (fieldInMap(response_headers,"xPos") && fieldInMap(response_headers,"yPos"))
    {
        float xLoc = std::stof(response_headers["xPos"],nullptr);
        float yLoc = std::stof(response_headers["yPos"],nullptr);
        others[std::stoi(response_headers["id"])].move(xLoc,yLoc);
    }
    //If the character has changed dancing, then update that
    if (fieldInMap(response_headers,"dancing"))
    {
        std::istringstream(response_headers["dancing"]) >> others[std::stoi(response_headers["id"])].dancing;
    }
    //If the character has changed inputting, then update that
    if (fieldInMap(response_headers,"inputting"))
    {
        std::istringstream(response_headers["inputting"]) >> others[std::stoi(response_headers["id"])].inputting;
    }

    return;
}

/*
    Utility function for determining if a key is in an unordered_map
    key_and_values: unordered_map that will be tested
    key: the key that is being checked to see if it is in the map
*/
bool Client::fieldInMap(std::unordered_map<std::string,std::string>& key_and_values, std::string key) {
    if (key_and_values.find(key) == key_and_values.end()) {
        return false;
    }
    return true;
}

/*
    Function to add a character
    others: world_state passed in from game client
    response_headers: information from server as an unordered map
*/
void Client::addCharacter(std::unordered_map<unsigned int, Character>& others, std::unordered_map<std::string,std::string>& response_headers)
{
    //Go through the response headers for the relevant information
    unsigned int cid = std::stoi(response_headers["id"],nullptr);
    std::string cname = response_headers["name"];
    float xVal = std::stof(response_headers["xPos"],nullptr);
    float yVal = std::stof(response_headers["yPos"],nullptr);

    //Get dancing and inputting information
    bool isDancing;
    bool isInputting;
    std::istringstream(response_headers["dancing"]) >> isDancing;
    std::istringstream(response_headers["inputting"]) >> isInputting;

    //Create the character
    Character c = Character(cname,cid,xVal,yVal,xVal,yVal);
    c.inputting = isInputting;
    c.dancing = isDancing;

    //Add them to the world_state
    others.insert(std::make_pair(cid,c));

    return;
}

/*
    Function to demodulate a string response into its headers
    response: string from server that needs to be demodulated into an
              unordered_map
*/
std::unordered_map<std::string, std::string> Client::processResponse(std::string response) {
    //cstring to save the request
    char req[M_SIZE];
    //vector to save the headers
    std::vector<std::string> headers;
    //cstring to save the message
    char message[M_SIZE];
    //Demodulate using the Parser
    parseResponse(&response[0],req,headers,message);

    std::unordered_map<std::string,std::string> key_and_values;

    //Parse each header and add it to the unordered_map
    for (int i = 0; i < headers.size(); i++) {
        char key[M_SIZE];
        char value[M_SIZE];
        parseHeader(headers[i].c_str(),key,value);
        key_and_values.insert(std::make_pair(std::string(key), std::string(value)));
    }
    //Empty the headers
    headers.clear();
    return key_and_values;
}

/*
    Function to send a world state request to the server

    NOTE: A "generic" GET request is assumed to be a world
    state request.
*/
void Client::sendWSRequest()
{
    //Build a default request
    std::string method = "GET";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    std::string request = build_request(method,headers);
    send_request(request);
}

/*
    Like the sendWSRequest but specify that this user is new
    to the server
*/
void Client::sendInitialWSRequest()
{
    std::string method = "GET";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    //Server will check for this later, that the request is initial
    headers["initial"] = "1";
    std::string request = build_request(method,headers);
    send_request(request);
}

/*
    Send a request to the server with all of the user's information
    xPos: current x location
    yPos: current y location
    isDancing: bool to tell if user is dancing
    isInputting: bool to tell if user is inputting
*/
void Client::sendInitial(float xPos = 0, float yPos = 0, bool isDancing = false, bool isInputting = false)
{
    //Make the default HTTP request
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    //Add headers for location
    std::string x_string = std::to_string(xPos);
    std::string y_string = std::to_string(yPos);
    headers["xPos"] = x_string;
    headers["yPos"] = y_string;

    //Add header for inputting
    std::string i_string = std::to_string(isInputting);
    headers["inputting"] = i_string;

    //Add header for dancing
    std::string d_string = std::to_string(isDancing);
    headers["dancing"] = d_string;

    //Build the request and send it
    std::string request = build_request(method,headers);
    send_request(request);
}

/*
    Send a request to the server to move the client
    xPos: desired x location
    yPos: desired y location
*/
void Client::sendMovement(float xPos,float yPos)
{
    //Build a default POST request
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    //Change float location to string
    std::string x_string = std::to_string(xPos);
    std::string y_string = std::to_string(yPos);

    //Put these strings into the unordered_map
    headers["xPos"] = x_string;
    headers["yPos"] = y_string;

    //Build and send request
    std::string request = build_request(method,headers);
    send_request(request);
}

/*
    Alert the server that the game client is inputting
    i: bool that is true when user is inputting
*/
void Client::sendInputting(bool i) {
    //Build default request
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    //Put inputting flag into unordered_map
    std::string i_string = std::to_string(i);
    headers["inputting"] = i_string;

    //Send request
    std::string request = build_request(method, headers);
    send_request(request);
}

/*
    Alert the server that the game client is dancing
    d: bool that is true when user is dancing
*/
void Client::sendDancing(bool d) {
    //Build default request
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    //Put dancing flag into unordered_map
    std::string d_string = std::to_string(d);
    headers["dancing"] = d_string;

    //build and send request
    std::string request = build_request(method, headers);
    send_request(request);
}

/*
    Send a message to the server
    message: message to be sent
*/
void Client::sendMessage(std::string message)
{
    std::string method = "POST";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();
    headers["message"] = message;
    std::string request = build_request(method, headers);
    send_request(request);
}

/*
    Tell the server that the client is leaving
*/
void Client::sendExit()
{
    //Tell the server goodbye
    Client::sendMessage("Goodbye!");

    //Build the default DELETE request
    std::string method = "DELETE";
    std::unordered_map<std::string,std::string> headers = getDefaultHeaders();

    //Put an exit header into the headers
    headers["exit"] = "1";
    //Build and send request
    std::string request = build_request(method,headers);
    send_request(request);
}

/*
    Function to check for an error and exit if so.
    returned_value: the value that determines error
    error_msg: message to be printed to the user
*/
int Client::check_if_error(int returned_value, char *error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

/*
    Function to create the server socket
*/
void Client::create_server_socket()
{
    //Declare struct for the server
    struct sockaddr_in server_address;
    int opt = 1;

    //clear out this server_address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;

    //Check to see if localhost will be used
    if (NO_FRIENDS)
    {
        server_address.sin_addr.s_addr = INADDR_ANY;

    }
    //Otherwise, use the IP_ADDRESS in config.h
    else
    {
        server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    }
    //Declare the port. Port comes from client construction
    server_address.sin_port = htons(port);
    
    //Create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    //Check for error with socket
    check_if_error(sock, "Error with socket");

    check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "setsockopt");

    //Connect to server
    connect(sock, (SA*)&server_address,sizeof(server_address));
}

/*
    Function to receive responses from the server
*/
void Client::receive_response() 
{
    std::cout << "Now receiving from server..." << std::endl;
    
    int bytes_read = 0;

    //Continually receive from the server
    while (open_for_receiving)
    {
        bytes_read = recv(sock,r_message,M_SIZE,0);
        //If very few bytes were received...
        while (bytes_read < 10){
            //Read again
            bytes_read = recv(sock,r_message,M_SIZE,0);
        }
        
        //Empty message? exit this iteration, into the next
        if (r_message[0] == '\0') {
            bzero(&r_message,sizeof(r_message));
            continue;
        }
        //If a legitimate response is received...
        else
        {
            //Lock the response_queue_mutex and add the response
            response_queue_mutex.lock();
            response_queue.push_back(std::string(r_message));
            response_queue_mutex.unlock();
            //Reset the r_message buffer
            bzero(&r_message,sizeof(r_message));
        }

    }
}
/*
    Set the client's id
    i: the desired id to be set
*/
void Client::setId(std::string i) {
    Client::id = i;
}

/*
    Get the client's name
*/
std::string Client::getName() {
    return name;
}

/*
    Get the client's id
*/
std::string Client::getId() {
    return id;
}

/*
    Function to handshake with the Server
*/
int Client::get_and_set_id() {

    //Tell the server the Client's name
    std::cout << "sending name to server" << std::endl;
    send_request(Client::name);

    //Get the id from the server
    std::cout << "receiving id from server" << std::endl;
    recv(sock,r_message,sizeof(r_message),0);

    //Set that id
    Client::setId(std::string(r_message));
    std::cout << "id of this client is: " << Client::getId() << std::endl;
    bzero(&r_message,sizeof(r_message));

    //Return the gotten id
    return std::stoi(Client::getId());
}

/*
    Function to build an HTTP request
    method: the method of the request (GET, POST, DELETE), etc.
    headers: the headers of the request
*/
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

/*
    Function to send a request to the server
    request: request as a string to send
*/
void Client::send_request(std::string request)
{
    send(sock,&request[0],M_SIZE, 0);
}

/*
    Function to get a response from the response queue

*/
std::string Client::pop_response()
{   
    //Instantiate the response
    std::string oldest_response = "";

    //Lock the response queue
    response_queue_mutex.lock();

    //If there's something in there, pop from the front
    if (!response_queue.empty())
    {
        std::cout << "popping response from response queue" << std::endl;
        oldest_response = response_queue.front();
        response_queue.pop_front();
    }
    response_queue_mutex.unlock();
    //Return that response that may just be "".
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
