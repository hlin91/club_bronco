#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() 
{
    struct sockaddr_in server;
    int fd;

    int connection;

    char message[1024];
    fd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(8096);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); //This binds the client to localhost
    connect(fd, (struct sockaddr *)&server, sizeof(server)); //This connects the client to the server.
    while(1) {
    printf("Enter a message: ");
    fgets(message, 100, stdin);
    send(fd, message, strlen(message), 0);
    //An extra breaking condition can be added here (to terminate the while loop)
    }
    return 0;
}