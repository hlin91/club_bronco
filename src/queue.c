#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

node_t* head = NULL;
node_t* tail = NULL;

/*
    Adds client_socket to the end of the queue.
    Returns true if client_socket is sucessfully enqueued.
    client_socket: Integer pointer to add to the end of the queue.
*/
bool enqueue(int *client_socket) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->next = NULL;

    if (!tail) 
    {
        head = new_node;
    } 
    else 
    {
        tail->next = new_node;
    }

    tail = new_node;
    return true;
}

/*
    Removes first client_socket on the head.
    Returns NULL if queue is empty, else return the removed_client_socket pointer instead.
*/
int* dequeue() {
    if (!head) {
        return NULL;
    } 
    else 
    {
        int *removed_client_socket = head->client_socket;
        node_t *temp = head;
        head = head->next;

        if (!head) 
        {
            tail = NULL;
        }

        free(temp);
        return removed_client_socket;
    }
}