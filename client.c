#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"
#include "queue.h"

#define LINE_SIZE 100
#define REQUEST "REQUEST"
#define USER_NOT_FOUND "USER_NOT_FOUND"
#define REQUEST_DENIED "REQUEST_DENIED"
#define PERMISSION_DENIED "PERMISSION_DENIED"
#define TOKEN_EXPIRED "TOKEN_EXPIRED"
#define RESOURCE_NOT_FOUND "RESOURCE_NOT_FOUND"
#define OPERATION_NOT_PERMITTED "OPERATION_NOT_PERMITTED"
#define PERMISSION_GRANTED "PERMISSION_GRANTED"
#define READ "READ"
#define INSERT "INSERT"
#define MODIFY "MODIFY"
#define DELETE "DELETE"
#define EXECUTE "EXECUTE"

typedef struct client_input {
    char *username;
    char *action;
    char *option;
} input;

StringQueue *q;

struct user_token {
    char *username;
    char *res_token;
};

struct user_token user_list[100];
int user_token_size;

void initializeQueue(StringQueue *queue) {
    queue->front = queue->rear = NULL;
}

int isEmpty(StringQueue *queue) {
    return queue->front == NULL;
}

void enqueue(StringQueue *queue, char *str) {
    
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    
    if (str[strlen(str) - 1] == '\n') {
		newNode->data = strndup(str, strlen(str) - 1);
	} else {
		newNode->data = strdup(str);
	}

    if (newNode->data == NULL) {
        perror("Error allocating memory");
        free(newNode);
        exit(EXIT_FAILURE);
    }

    newNode->next = NULL;

    if (isEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

char* dequeue(StringQueue *queue) {
    if (isEmpty(queue)) {
        fprintf(stderr, "Error: Queue is empty\n");
        exit(EXIT_FAILURE);
    }

    char *data = queue->front->data;

    Node *temp = queue->front;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);

    return data;
}

void freeQueue(StringQueue *queue) {
    while (!isEmpty(queue)) {
        char *data = dequeue(queue);
        free(data);
    }
    free(queue);
}

void printQueue(StringQueue *queue) {
    Node *current = queue->front;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
}

/*
    Functie care se ocupa cu cititul comenzilor din fisierul client.in
 */
void read_clients_in(char *file_path, StringQueue *q) {
    FILE* client_fp = fopen(file_path, "r");
	if (client_fp == NULL) {
		fprintf(stderr, "%s\n", "error opening client_in file");
	}
	char line[LINE_SIZE];
	memset(line, 0, LINE_SIZE);
	while(fgets(line, sizeof(line), client_fp) != NULL) {
		enqueue(q, line);
		memset(line, 0, LINE_SIZE);
	}
}

/*
    Functie care se ocupa cu parsarea comenzilor clientului
 */
input * client_parser(char *str) {
    char *token;
    input * in = (input *)malloc(sizeof(input));

    token = strtok(str, ",");
    in->username = strdup(token);

    token = strtok(NULL, ",");
    in->action = strdup(token);

    token = strtok(NULL, ",");  
    in->option = strdup(token);

    return in;
}

void free_input(input *in) {
    free(in->username);
    free(in->action);
    free(in->option);
    free(in);
}

void add_user_to_list(char *username, char *resources_token) {
    user_list[user_token_size].username = strdup(username);
    user_list[user_token_size].res_token = strdup(resources_token);
    user_token_size++;
}

void update_user_in_list(char *username, char *resources_token) {
    for (int i = 0; i < user_token_size; i++) {
        if (strcmp(username, user_list[i].username) == 0) {
            free(user_list[i].res_token);
            user_list[i].res_token = strdup(resources_token);
            return;
        }
    }
    add_user_to_list(username, resources_token);
}

char *search_user_in_list(char *username) {
    for (int i = 0; i < user_token_size; i++) {
        if (strcmp(username, user_list[i].username) == 0) {
            return user_list[i].res_token;
        }
    }
    return "NO_USER_FOUND";
}

void start(char *host) {
    CLIENT *clnt;
    
    clnt = clnt_create(host, INTERFACE_PROG, INTERFACE_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    // Citim toate requesturile/actiunile si le executam
    while(!isEmpty(q)) {
        char *elem = dequeue(q);
        input *in = client_parser(elem);
        free(elem);
        if (strcmp(in->action, REQUEST) == 0) {
            struct user u;
            
            u.username = strdup(in->username);
            char **req_auth_token = request_authorization_1(u.username, clnt);
            if (req_auth_token == (char **) NULL) {
                clnt_perror(clnt, "REQUEST_AUTHORIZATION call failed\n!");
                exit(1);
            }       
            if (strcmp(*req_auth_token, USER_NOT_FOUND) == 0) {
                printf("%s\n", USER_NOT_FOUND);
                continue;
            }

            u.req_auth_token = strdup(*req_auth_token);
            char **validated = approve_request_1(u.req_auth_token, clnt);
            if (validated == (char **) NULL) {
                clnt_perror(clnt, "APPROVE_REQUEST call failed\n!");
                exit(1);
            }

            struct token_pair *t = request_access_token_1(u.username, u.req_auth_token, *validated, atoi(in->option), clnt);
            if (t == (struct token_pair *) NULL) {
                clnt_perror(clnt, "REQUEST_ACCESS_TOKEN call failed\n!");
                exit(1);
            }

            if (strcmp(t->resources_token, REQUEST_DENIED) == 0) {
                printf("%s\n", REQUEST_DENIED);
                continue;
            }

            update_user_in_list(u.username, t->resources_token);
            
            u.resources_token = strdup(t->resources_token);
            u.reg_token = strdup(t->reg_token);

            if(atoi(in->option) == 1) {
                printf("%s -> %s,%s\n", u.req_auth_token, u.resources_token, u.reg_token);
                
            } else {
                printf("%s -> %s\n", u.req_auth_token, u.resources_token);
            }
            
            free(u.username);
            free(u.req_auth_token);
            free(u.resources_token);
            free(u.reg_token);
        } else {
            struct status *result = validate_delegated_action_1(in->action, in->option, search_user_in_list(in->username), clnt);
            if (result == (struct status*) NULL) {
                clnt_perror(clnt, "VALIDATE_DELEGATED_ACTION call failed\n!");
                exit(1);
            }
            if (strcmp(result->pair.resources_token, TOKEN_EXPIRED) != 0 &&
                strcmp(result->pair.resources_token, RESOURCE_NOT_FOUND) != 0 &&
                strcmp(result->pair.resources_token, OPERATION_NOT_PERMITTED) != 0 &&
                strcmp(result->pair.resources_token, PERMISSION_GRANTED) != 0 &&
                strcmp(result->pair.resources_token, PERMISSION_DENIED) != 0 ) {

                update_user_in_list(in->username, result->pair.resources_token);
            }
            printf("%s\n", result->stat);
        }
        free_input(in);

    }

    clnt_destroy(clnt);
}

int main(int argc, char *argv[]) {

    char *host;
    q = (StringQueue *)malloc(sizeof(StringQueue));
	initializeQueue(q);

    host = argv[1];
    read_clients_in(argv[2], q);


	start(host);
    freeQueue(q);
	exit (0);
}