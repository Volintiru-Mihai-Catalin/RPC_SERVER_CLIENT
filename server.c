#include "interface.h"
#include "queue.h"
#include "token.h"

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

extern int users_size;
extern char **users;

extern int res_size;
extern char **resources;

extern StringQueue *q;

extern int token_life;

extern struct user *users_list;

/* 
    Functie prin care se parseaza linia de permisiuni primita din fisierul
 de permisiuni
 */
void parse_perms(char *permissions, int user_number) {
    
    char *token1 = strtok(permissions, ",");
    char *token2 = strtok(NULL, ",");
    while(token1 != NULL) {
        
        for (int index = 0; index < res_size; index++) {
            if (strcmp(users_list[user_number].res[index].res_type, token1) == 0) {
                users_list[user_number].res[index].perms = strdup(token2);
                index = res_size;
            }
        }
        token1 = strtok(NULL, ",");
        token2 = strtok(NULL, ",");
    }
}

/* 
    Functionalitate prin care se elibereaza campurile aferente unui utilizator
 din baza de date
 */
void free_user_data(int index, int option) {
    if (!option) {
        if (users_list[index].req_auth_token != NULL) {
            free(users_list[index].req_auth_token);
            users_list[index].req_auth_token = NULL;
        }
    } 
    if (users_list[index].resources_token != NULL)  {
        free(users_list[index].resources_token);
        users_list[index].resources_token = NULL;
    }
    if (! option) {
        if (users_list[index].reg_token != NULL) {
            free(users_list[index].reg_token);
            users_list[index].reg_token = NULL;
        }
    }
    if (!option) {
        for (int index1 = 0; index1 < res_size; index1++) {
            if (users_list[index].res[index1].perms != NULL) {
                free(users_list[index].res[index1].perms);
                users_list[index].res[index1].perms = NULL;
            }
        }
    }
    users_list[index].val_time = 0;
}

/*
    Functie ce verifica permisiunile pe o anumita resursa
 */
int check_perm(char *perm_str, char* action) {

    if (perm_str == NULL) {
        return 0;
    }

    char c;
    if (strcmp(action, READ) == 0) {
        c = 'R';
    } else if (strcmp(action, INSERT) == 0) {
        c = 'I';
    } else if (strcmp(action, MODIFY) == 0) {
        c = 'M';
    } else if (strcmp(action, DELETE) == 0) {
        c = 'D';
    } else if (strcmp(action, EXECUTE) == 0) {
        c = 'X';
    }

    for (int index = 0; index < strlen(action); index++) {
        if (perm_str[index] == c) {
            return 1;
        }
    }

    return 0;
}

/*
    Functie care regenereaza tokenii de access si refresh in urma unei operatii de REFRESH
 */
void refresh_tokens_operation(int index) {
    free_user_data(index, 1);
    
    users_list[index].resources_token = strdup(generate_access_token(users_list[index].reg_token));
    users_list[index].reg_token = strdup(generate_access_token(users_list[index].resources_token));
    users_list[index].val_time = token_life;
    printf("BEGIN %s AUTHZ REFRESH\n", users_list[index].username);
    printf("  AccessToken = %s\n", users_list[index].resources_token);
    printf("  RefreshToken = %s\n", users_list[index].reg_token);
}

/*
    Function that checks if a resource is valid
 */
int check_valid_res_type(char *res_type) {
    for (int index1 = 0; index1 < res_size; index1++) {
        if (strcmp(resources[index1], res_type) == 0) {
            return 1;
        }
    }
    
    return 0;
}

extern  char ** request_authorization_1_svc(char *username, struct svc_req *rqstp) {    
    static char *result;

    printf("BEGIN %s AUTHZ\n", username);

    if (result != 0) {
        free(result);
    }

    for (int index = 0; index < users_size; index++) {
        if (strcmp(users[index], username) == 0) {
            result = generate_access_token(username);
            printf("  RequestToken = %s\n", result);
            return &result;
        }
    }
    result = strdup(USER_NOT_FOUND);
    return &result;
}

extern char **approve_request_1_svc(char *request_auth_token, struct svc_req *rqstp) {
    static char *result;

    if (result != 0) {
        free(result);
    }

    result = dequeue(q); 

    return &result;
}

extern struct token_pair * request_access_token_1_svc(char* username, char *request_auth_token, char *permissions, int auto_refresh, struct svc_req *rqstp) {
    static token_pair result;

    if (result.resources_token != NULL) {
        free(result.resources_token);
    }
    if (result.reg_token != NULL) {
        free(result.reg_token);
    }

    if (permissions[0] == '*') {
        result.resources_token = strdup(REQUEST_DENIED);
        result.reg_token = strdup(REQUEST_DENIED);
    } else {
        result.resources_token = generate_access_token(request_auth_token);
        if (auto_refresh) {
            result.reg_token = generate_access_token(result.resources_token);
        } else {
            result.reg_token = strdup("NO_TOKEN");
        }
        result.val_time = token_life;

        for (int index = 0; index < users_size; index++) {
            if (strcmp(users_list[index].username, username) == 0) {
                free_user_data(index, 0);
                
                users_list[index].req_auth_token = strdup(request_auth_token);
                users_list[index].resources_token = strdup(result.resources_token);
                users_list[index].reg_token = strdup(result.reg_token);   
                users_list[index].automatic_refresh = auto_refresh;
                users_list[index].val_time = token_life;
                parse_perms(permissions, index);
            }
        }
        printf("  AccessToken = %s\n", result.resources_token);
        if (auto_refresh) {
            printf("  RefreshToken = %s\n", result.reg_token);
        }
    }

    return &result;
}

struct status * validate_delegated_action_1_svc(char *action, char *res_type, char *resources_token, struct svc_req *rqstp) {
    static status result;
    int is_refreshed = 0;
    if (result.stat != NULL) {
        free(result.stat);
    }
    if (result.pair.resources_token != NULL) {
        free(result.pair.resources_token);
    }
    if (result.pair.reg_token != NULL) {
        free(result.pair.reg_token);
    }

    for (int index = 0; index < users_size; index++) {
        if (users_list[index].resources_token != NULL) {
            if (strcmp(users_list[index].resources_token, resources_token) == 0) {

                if (users_list[index].val_time < 1) {
                    if (users_list[index].automatic_refresh == 0) {
                        printf("DENY (%s,%s,,0)\n", action, res_type);
                        result.stat = strdup(TOKEN_EXPIRED);
                        result.pair.reg_token = strdup(TOKEN_EXPIRED);
                        result.pair.resources_token = strdup(TOKEN_EXPIRED);
                        return &result;
                    } else {
                        refresh_tokens_operation(index);
                        result.pair.reg_token = strdup(users_list[index].reg_token);
                        result.pair.resources_token = strdup(users_list[index].resources_token);
                        is_refreshed = 1;
                    }
                }

                if (!check_valid_res_type(res_type)) {
                    users_list[index].val_time--;
                    printf("DENY (%s,%s,%s,%d)\n", action, res_type, users_list[index].resources_token, users_list[index].val_time);
                    result.stat = strdup(RESOURCE_NOT_FOUND);
                    if (!is_refreshed) {
                        result.pair.reg_token = strdup(RESOURCE_NOT_FOUND);
                        result.pair.resources_token = strdup(RESOURCE_NOT_FOUND);
                    }
                    return &result;
                }

                for (int index1 = 0; index1 < res_size; index1++) {
                    if (strcmp(users_list[index].res[index1].res_type, res_type) == 0) {
                        users_list[index].val_time--;
                        if (!check_perm(users_list[index].res[index1].perms, action)) {
                            printf("DENY (%s,%s,%s,%d)\n", action, res_type, users_list[index].resources_token, users_list[index].val_time);
                            result.stat = strdup(OPERATION_NOT_PERMITTED);
                            if (!is_refreshed) {
                                result.pair.reg_token = strdup(OPERATION_NOT_PERMITTED);
                                result.pair.resources_token = strdup(OPERATION_NOT_PERMITTED);
                            }
                            return &result;
                        } else {
                            printf("PERMIT (%s,%s,%s,%d)\n", action, res_type, users_list[index].resources_token, users_list[index].val_time);
                            result.stat = strdup(PERMISSION_GRANTED);
                            if (!is_refreshed) {
                                result.pair.reg_token = strdup(PERMISSION_GRANTED);
                                result.pair.resources_token = strdup(PERMISSION_GRANTED);
                            }
                            return &result;
                        }
                    }
                }
            }
        }
    }

    printf("DENY (%s,%s,,0)\n", action, res_type);
    result.stat = strdup(PERMISSION_DENIED);
    result.pair.reg_token = strdup(PERMISSION_DENIED);
    result.pair.resources_token = strdup(PERMISSION_DENIED);

    return &result;
}
