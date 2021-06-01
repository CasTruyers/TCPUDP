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

int initialisation();
void execution();
void cleanup();


int main()
{
    char pakket[] = "Een pakket";
    char input;
    int c;
    int internet_socket;

    //INITIALISATION
    internet_socket = initialisation();

    for(;;)
    {
        //EXECUTION
        execution(internet_socket, pakket);

        printf("Keep server running(y/n): ");
        for(;;)
        {
            scanf("%c", &input);
            if(input == 'n' || input == 'N') exit(0);
            else if(input == 'y' || input == 'Y') break;
            else printf("\nPlease enter 'y'(yes, keep server running) OR 'n'(No, exit server)\n");
        }
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    
    //CLEANUP
    cleanup(internet_socket);
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
    address_setup.ai_socktype = SOCK_DGRAM;
    address_setup.ai_flags = AI_PASSIVE;
    int getAddrReturn = getaddrinfo(NULL ,"24021",&address_setup, &address_result);
    if(getAddrReturn) {perror("addrInfo"); free(address_result); exit(0);}

    //1.2
    int internet_socket = -1;
    struct addrinfo *address_result_iterate = address_result;
    while(address_result_iterate != NULL)
    {
        internet_socket = socket(address_result_iterate->ai_family, address_result_iterate->ai_socktype, address_result_iterate->ai_protocol);
        if(internet_socket == -1) perror("Socket");
        else
        {
            int bind_return = bind(internet_socket, address_result_iterate->ai_addr, address_result_iterate->ai_addrlen);
            if(bind_return == -1)
            {
                close(internet_socket);
                perror("bind");
            }
            else break;
        }
        address_result_iterate = address_result->ai_next;
    }
    
    freeaddrinfo(address_result);

    if(internet_socket == -1) {perror("Socket"); exit(1);}

    return internet_socket;
}


void execution(int internet_socket, char *pakket)
{
    //RECEIVE
    struct sockaddr_storage client_address;
    socklen_t client_address_length = sizeof(client_address);
    unsigned int aantal=0;
    int bytes_received  = recvfrom(internet_socket, &aantal, sizeof(aantal), 0, (struct sockaddr*)&client_address, &client_address_length);
    if(bytes_received == -1){perror("received"); exit(4);}
    else
    {  
        printf("\nreceived: %d\n", aantal);
    }

    sendto(internet_socket, &bytes_received, sizeof(bytes_received), 0, (struct sockaddr*)&client_address, client_address_length);

    //SEND
    for(int i=1; i<aantal; i++)
    {
        int bytes_send = sendto(internet_socket, pakket, strlen(pakket), 0, (struct sockaddr*)&client_address, client_address_length);
        //printf("\nBytes send: %d\n", bytes_send);
        if(bytes_send == -1){perror("sendto"); exit(3);}
        printf("transmitted: %s\n", pakket);
    }
}


void cleanup(int internet_socket)
{
    close(internet_socket);
}