#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

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
        return -1;
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
    while (connection = accept(fd, (struct sockaddr *)NULL, NULL)) {
        int pid;

        // Fork child process to service connection
        if ((pid = fork()) == 0) {
            while (recv(connection, message, 1024, 0) > 0) {
                cout << message;
                if (strcmp(message,"exit") == 0) {
                    close(fd);
                    return 0;
                }
                memset(&message, 0, sizeof(message));
            }
            exit(0);
        }
    }
}