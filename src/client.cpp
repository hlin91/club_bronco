#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

int main() 
{
    struct sockaddr_in server;
    int fd;
    int portNum = argv;

    int connection;

    char message[1024];

    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);
    //server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    fd = socket(AF_INET, SOCK_STREAM,0);

    if (fd == -1) {
        cout << "Socket creation error\n";
        perror("Socket creation error");
        return -1;
    }


    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); //This binds the client to localhost
    if (connect(fd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        cout << "connection failed\n";
        perror("Connection failed");
        return -1;
    } //This connects the client to the server.

    while(1) {
        cout << "Enter a message: ";
        fgets(message, 100, stdin);
        send(fd, message, strlen(message), 0);
        if (strcmp(message,"exit") == 0) {
            close(fd);
            return 0;
        }
        memset(&message,0,sizeof(message));
    //An extra breaking condition can be added here (to terminate the while loop)
    }
    return 0;
}
