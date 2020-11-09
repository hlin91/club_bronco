#ifndef QUEUE_H_
#define QUEUE_H_

/*
    Struct for each node in the queue.
    node: Struct pointer to another struct node
    client_socket: Integer pointer to client_socket
*/
struct node {
    struct node* next;
    int *client_socket;
};
typedef struct node node_t;

#endif