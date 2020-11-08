#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

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
    if (sock < 0) {
        perror("Error with socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    // Attempt to make the socket (fd) a listening type socket
    if (listen(sock, 10) < 0) {
        perror("Could not make the socket a listening type socket");
        exit(EXIT_FAILURE);
    }

    cout << "Listening for requests on port " << port << endl;

    return sock;
}

int main() 
{
    int port = 4310;
    int socket = create_server_socket(port);
    char message[1024];

    while(1) {
        cout << "Enter a message: ";
        fgets(message, 100, stdin);
        send(socket, message, strlen(message), 0);
        if (strcmp(message,"exit") == 0) {
            close(socket);
            return 0;
        }
        memset(&message,0,sizeof(message));
        //An extra breaking condition can be added here (to terminate the while loop)
    }
    return 0;
}
