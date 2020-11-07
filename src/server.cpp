#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "utils.h"

using namespace std;

int create_server_socket(int port) {

    struct sockaddr_in server_address;
    
    int socket = socket(AF_INET,
                        SOCK_STREAM,
                        0);

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    cout << "Listening for requests on port " <<
}

void* handle_client(void* client_ptr) {
    pthread_detach(pthread_self());

    int client = *((int*) client_ptr);

    char buffer[BUFF_SIZE + 1];
    bzero(buffer, sizeof(buffer));
    int bytes_read = recv(client, buffer, sizeof(buffer), 0); 
    if (bytes_read < 0)
    { 
        error_msg("Problem with recv call", false);
    }
}

int main() {
    struct sockaddr_in server;
    int fd;
    int portNum = 4310;
    int opt = 1;

    int connection;

    char message[1024];

    // Populate socket address structure
    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);
    server.sin_addr.s_addr = INADDR_ANY;

    // Attempt to create a socket and assign it to fd
    if (fd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
        perror("Error with socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    // Attempt to assign the socket address to a socket (fd)
    if (bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Binding error\n");
        exit(EXIT_FAILURE);
    }

    // Attempt to make the socket (fd) a listening type socket
    if (listen(fd, 10) <= -1) {
        perror("Could not make the socket a listening type socket");
        exit(EXIT_FAILURE);
    }

    // Loop until calling accept returns -1 or lower value
    while (true) {
        int client = accept(fd, (struct sockaddr *)NULL, NULL);

        pthread_t tid;
        int flag = pthread_create(&tid, NULL, handle_client, &client);
        if (flag < 0) 
        {
            perror("Problem creating thread");
        }
    }
}