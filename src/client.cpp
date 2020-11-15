#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <errno.h>

#define SA struct sockaddr

using namespace std;

char s_message[2048];
char r_message[2048];
int sock;

void* receive_message(void* ptr) {
    while (1)
    {
        recv(sock,r_message,2048,0);
        puts(r_message);
        memset(r_message,0,sizeof(r_message));
    }
    return nullptr;
}

void* send_message(void *ptr) {
    while (1)
    {
        cout << "Send a message: ";
        cin >> s_message;
        send(sock,s_message,strlen(s_message), 0);
        memset(s_message,0,sizeof(s_message));
    }
    return nullptr;
}

int check_if_error(int returned_value, char *error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

void create_server_socket(int port) {

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

    cout << "Listening for requests on port " << port << endl;
}

/*
    Function to create a request to send to the server
    method: a string that is POST, GET, etc.
    headers: the fields that will help the server determine what to do.
    
    Note: headers is an unordered map which maps a certain field to a certain
    value.

*/
std::string build_request(std::string method, std::unordered_map headers) {
    std::string request = "";
    request += (method + " / HTTP 1.1\n";
    for(auto headers : header) {
        request += header.first + ":" + header.second + "\n";
    }
    return request;
}

void send_request(std::string request) {
    char* c_request;
    c_request = request.c_str();
    send(sock,c_request,strlen(c_request), 0);
}

int main() 
{
    int port = 4310;

    create_server_socket(port);

    pthread_t thread_send, thread_receive;

    pthread_create(&thread_send, NULL, send_message,NULL);
    pthread_create(&thread_receive, NULL, receive_message, NULL);

    pthread_join(thread_send,NULL);
    while (1);
    return 0;
}
