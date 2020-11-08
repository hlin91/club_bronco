#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

using namespace std;

int check_if_error(int returned_value, char *error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

int create_server_socket(int port) {

    struct sockaddr_in server_address;

    int opt = 1;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    //Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //Check for error with socket
    check_if_error(sock, "Error with socket");

    check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");

    // Attempt to make the socket (fd) a listening type socket
    check_if_error(listen(sock, 10), "Could not make the socket a listening type socket");

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
    check_if_error(bytes_read, "Error reading from client");

    cout << request << endl;
    close(client);
    return 0;
}

int main() {
    int port = 4310;
    int socket;
    socket = create_server_socket(port);

    while (true) {
        int client = accept(socket, (struct sockaddr *)NULL, NULL);

        pthread_t tid;
        int flag = pthread_create(&tid, NULL, handle_client, &client);
        check_if_error(flag, "Problem creating thread");
    }

    return 0;
}