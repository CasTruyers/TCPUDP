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
void execution();
void cleanup();
void *receive();


int main()
{
    //INITIALISATION
    int internet_socket = initialisation();

    //EXECUTION
    execution(internet_socket);
    
    //CLEANUP
    cleanup(internet_socket);
    return 0;
}


//FUNCTIONS
int initialisation()
{
    //AddrInfo
    struct addrinfo address_setup;
    struct addrinfo *address_result;
    memset(&address_setup, 0, sizeof(address_setup));
    address_setup.ai_family = AF_UNSPEC;
    address_setup.ai_socktype = SOCK_STREAM;
    check(getaddrinfo("192.168.1.4","24022",&address_setup, &address_result), "addrInfo", 2, NULL);

    //Socket
    int internet_socket = -1;
    struct addrinfo *address_result_iterate = address_result;
    while(address_result_iterate != NULL)
    {
        internet_socket = socket(address_result_iterate->ai_family, address_result_iterate->ai_socktype, address_result_iterate->ai_protocol);
        if(internet_socket == -1) perror("Socket");
        else
        {
            //connect (3 way handshake) when succes, breaking the loop.
            if(check(connect(internet_socket, address_result_iterate->ai_addr, address_result_iterate->ai_addrlen), "connect", 0, NULL)) break;
        }
        address_result_iterate = address_result_iterate->ai_next;
    }
    
    freeaddrinfo(address_result);
    if(internet_socket == -1) {perror("Socket: no valid socket address found"); exit(1);}
    return internet_socket;
}

void execution(int internet_socket)
{
    char message[100];
    struct sockaddr_storage client_address;
    socklen_t client_address_length = sizeof(client_address);
    int* internet_socket_ptr = malloc(sizeof(int));
    *internet_socket_ptr = internet_socket;
    fflush(stdin);
    printf("\nWelcome to the chat room! \t\t(Enter '*close' to close your connection.)\n");

    pthread_t newThread;
    pthread_create(&newThread, NULL, receive, internet_socket_ptr);

    while(1)
    {
        scanf ("%[^\n]%*c", message);

        //Send
        int bytes_send = send(internet_socket, message, strlen(message), 0);
        if(bytes_send == -1){perror("send"); exit(3);}

        //check for close
        if( (strcmp(message, "*close") == 0) || (strcmp(message, "*Close") == 0) ) {cleanup(internet_socket); break;}

        //Receive
        
        fflush(stdin);
    }
}
void *receive(void *internet_socket_ptr)
{
    char buffer[1000];
    int internet_socket = *((int*)internet_socket_ptr);
    while(1)
    {
        int bytes_received  = recv(internet_socket, buffer, sizeof(buffer), 0);
        if(bytes_received == -1){perror("received"); return NULL;}
        else if(bytes_received == 0) {cleanup(internet_socket); printf("server offline\n"); exit(0);}
        else
        {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        }
    }
    return NULL;
}

//int check(int functionreturn, perror message, type of excecution, socket pointer when typeOfExecution == 1 otherwise 'NULL')
//return 0 when error, 1 when no error.
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

void cleanup(int internet_socket)
{
    //closing socket
    close(internet_socket);
}