#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

using namespace std;

int create_server_socket(int port) {

    struct sockaddr_in server_address;

    int opt = 1;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    //Create the socket
    int sock = socket(AF_INET,
                        SOCK_STREAM,
                        0);

    //Check for error with socket
    if (sock < 0) {
        perror("Error with socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    // Attempt to make the socket (fd) a listening type socket
    if (listen(sock, 10) <= -1) {
        perror("Could not make the socket a listening type socket");
        exit(EXIT_FAILURE);
    }

    cout << "Listening for requests on port " << port << endl;

    return sock;
}

void* handle_client(void* client_ptr) {
    pthread_detach(pthread_self());

    int client = *((int*) client_ptr);


    //Request from the client
    char request[BUFSIZ + 1];
    bzero(request, sizeof(request));
    int bytes_read = recv(client, request, sizeof(request), 0); 
    if (bytes_read < 0)
    {
        perror("Error reading from client"); 
    }
    cout << request << endl;
    close(client);
    return 0;
}

int main() {
    int port = 4310;
    int socket;
    socket = create_server_socket(port);

    // Loop until calling accept returns -1 or lower value
    while (true) {
        int client = accept(socket, (struct sockaddr *)NULL, NULL);

        pthread_t tid;
        int flag = pthread_create(&tid, NULL, handle_client, &client);
        if (flag < 0) 
        {
            perror("Problem creating thread");
        }
    }

    return 0;
}