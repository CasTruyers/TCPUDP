#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int check();
int initialisation();
int connection();
void* execution();
void cleanup();

int clientSockets[10];


int main()
{
    //INITIALISATION
    int listen_socket = initialisation();

    //CONNECTION
    int client_socket = connection(listen_socket);

    //EXCEXUTION
    execution(client_socket);
    
    //Cleanup
    close(listen_socket);
    return 0;
}


//FUNCTIONS
int initialisation()
{
    //1.1
    struct addrinfo address_setup;
    struct addrinfo *address_result;
    memset(&address_setup, 0, sizeof(address_setup));
    address_setup.ai_family = AF_UNSPEC;
    address_setup.ai_socktype = SOCK_STREAM;
    address_setup.ai_flags = AI_PASSIVE;
    check(getaddrinfo(NULL ,"24022",&address_setup, &address_result), "addrInfo", 2, NULL);

    //SOCKET
    int listen_socket = -1;
    struct addrinfo *address_result_iterate = address_result;
    while(address_result_iterate != NULL)
    {
        listen_socket = socket(address_result_iterate->ai_family, address_result_iterate->ai_socktype, address_result_iterate->ai_protocol);
        if(listen_socket == -1) perror("Socket");
        else
        {
            //BIND 
            if(check(bind(listen_socket, address_result_iterate->ai_addr, address_result_iterate->ai_addrlen), "Bind", 2, NULL))
            {
                //LISTEN
                if(check(listen(listen_socket, 1), "Listen", 1, &listen_socket)) {printf("\nListening...\n\n"); break;} 
            }
        }
        address_result_iterate = address_result_iterate->ai_next;
    }
    
    freeaddrinfo(address_result);

    if(listen_socket == -1) {perror("Socket: no valid socket address found"); exit(1);}

    return listen_socket;
}


int connection(int listen_socket)
{
    struct sockaddr_storage client_addres;
    socklen_t client_address_length = sizeof client_addres;
    pthread_t newUser;
    int *client_socket_ptr = malloc(3*sizeof(int));
    int usersJoined = 0, usersLeft = 0, amount; //amount of users joined and users left.

    client_socket_ptr[1] = usersJoined;
    client_socket_ptr[2] = usersLeft;
    //Connecting users
    while(1)
    {
        int client_socket = accept(listen_socket, (struct sockaddr *) &client_addres, &client_address_length);
        if(client_socket == -1){perror("accept"); close(listen_socket); exit(3);}
        else
        {
            client_socket_ptr[0] = client_socket;
            pthread_create(&newUser, NULL, execution, client_socket_ptr);
        }
    }
}


void* execution(void* client_socket_ptr)
{
    int *userData = ((int*) client_socket_ptr);
    int userSocket = userData[0];
    int sendtoSocket;
    userData[1] += 1;
    int userNumber = (userData[1] - userData[2]);
    int activeUsers;
    clientSockets[userNumber-1] = userSocket;

    printf("\t|User joined, %d active|\n\n", userData[1]-userData[2]);
    char message[1000];

    struct sockaddr_storage client_address;
    socklen_t client_address_length = sizeof(client_address);

    while(1)
    {
        //RECEIVE
        int bytes_received  = recv(userSocket, message, sizeof(message)-1, 0);
        if(bytes_received == -1){perror("received"); return NULL;}
        else message[bytes_received] = '\0';

        //check for close
        if( (strcmp(message, "*close") == 0) || (strcmp(message, "*Close") == 0) || (bytes_received == 0) )
        {
            //shutdown connection
            check(shutdown(userSocket, SHUT_RD), "Shutdown", 0, NULL);

            //close connection
            check(close(userSocket), "Close", 0, NULL);
            break;
        }

        //print received message
        printf("received: %s\n", message);

        activeUsers = userData[1]-userData[2]; 

        //SEND to all active users
        for(int i=0; i<activeUsers; i++)
        {
            sendtoSocket = clientSockets[i];
            if(sendtoSocket == userSocket) continue;
            else
            {
                int bytes_send = send(sendtoSocket, message, strlen(message)+1, 1);
                if(bytes_send == -1){perror("send");}
                else printf("send user%d: %s\n", i+1, message);
            }
        }
    }
    userData[2] += 1;
    printf("\t|User left, %d active|\n", userData[1]-userData[2]);

    return NULL;
}


void closeClient(int client_socket)
{
    //shutdown connection
    //check(shutdown(client_socket, SHUT_RD), "Shutdown", 0, NULL);
    
    //close connection
    check(close(client_socket), "Close", 0, NULL);
    printf("Closed connection\n");
}

int check(int i, char message[], int type, int *socketPtr)
{
    if (i == -1)
    {
        if(type == 0) perror(message);
        else if(type == 1)
        {
            perror(message);
            close(*socketPtr);
        }
        else if(type == 2)
        {
            perror(message);
            exit(0);
        }
        return 0;
    }
    return 1;
}