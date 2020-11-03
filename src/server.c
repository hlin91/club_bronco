#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> 

int main() {
    struct sockaddr_in server;
    int fd;

    int connection;

    char message[1024];

    server.sin_family = AF_INET;
    server.sin_port = htons(8096);
    server.sin_addr.s_addr = INADDR_ANY;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    bind(fd, (struct sockaddr *)&server, sizeof(server));

    listen(fd,10);

    while (connection = accept(fd, (struct sockaddr *)NULL, NULL)) {
        int pid;
        if ((pid = fork()) == 0) {
            while (recv(connection, message, 1024, 0) > 0) {
                printf(message);
            }
            exit(0);
        }
    }
}