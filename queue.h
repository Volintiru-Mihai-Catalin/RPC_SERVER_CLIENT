#ifndef QUEUE_H
#define QUEUE_H

/*
    Implementarea unor functionalitati pentru coada. Aceste functii vor fi
 implementate atat de client cat si de catre server
 */

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *rear;
} StringQueue;

void initializeQueue(StringQueue *queue);
int isEmpty(StringQueue *queue);
void enqueue(StringQueue *queue, char *str);
char* dequeue(StringQueue *queue);
void printQueue(StringQueue *queue);
void freeQueue(StringQueue *queue);

#endif