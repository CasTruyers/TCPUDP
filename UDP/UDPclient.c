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
void check();

int main()
{
    char ip[] = "127.0.0.1";
    unsigned int amount;

    struct sockaddr* address = NULL;
    socklen_t *address_length = 0;

    //INITIALISATION
    int internet_socket = initialisation(&address, &address_length, ip);

    printf("Amount: ");
    scanf("%d", &amount);
    printf("\n%d packets\n", amount);

    //EXECUTION
    execution(internet_socket, address, address_length, amount);

    cleanup(internet_socket, address);
    
    return 0;
}


//FUNCTIONS
int initialisation(struct sockaddr** address, socklen_t *address_length, char *ip)
{
    //1.1
    struct addrinfo address_setup;
    struct addrinfo *address_result;
    memset(&address_setup, 0, sizeof(address_setup));
    address_setup.ai_family = AF_UNSPEC;
    address_setup.ai_socktype = SOCK_DGRAM;
    int getAddrReturn = getaddrinfo(ip,"24021",&address_setup, &address_result);
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
            *address_length = address_result->ai_addrlen;
            *address = (struct sockaddr *) malloc(address_result->ai_addrlen);
            memcpy(*address, address_result->ai_addr, address_result->ai_addrlen);
        }
        address_result_iterate = address_result->ai_next;
    }
    
    freeaddrinfo(address_result);

    if(internet_socket == -1) {perror("Socket"); exit(1);}

    return internet_socket;
}


void execution(int internet_socket, struct sockaddr *address, socklen_t address_length, int amount)
{
    //sending amount of packages to send back
    int bytes_send = sendto(internet_socket, &amount, sizeof(amount), 0, address, address_length);
    if(bytes_send == -1){perror("sendto"); exit(3);}

    //Securing package arrival
    int bytes_arrived;
    int bytes_received = recvfrom(internet_socket, &bytes_arrived, sizeof(bytes_arrived), 0, address, &address_length);
    if(bytes_received == -1){perror("received"); exit(4);}
    else printf("Packet:%d, Bytes send: %d, bytes arrived: %d",1, bytes_send, bytes_arrived);

    if(bytes_arrived == bytes_send) printf("\tPackage secured!\n");
    else printf("\tpackage loss!\n");

    //Receiving x packages
    char buffer[1000];
    for(int i=1; i<amount; i++)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received  = recvfrom(internet_socket, buffer, sizeof(buffer)-1, 0, address, &address_length);
        //printf("\nBytes received: %d\n", bytes_received);
        if(bytes_received == -1){perror("received"); exit(4);}
        else
        {
            buffer[bytes_received] = '\0';
            printf("Packet:%d, Bytes: %d, ASCII: %s\n",i+1,bytes_received, buffer);
        }
    }
}


void cleanup(int internet_socket, struct sockaddr *address)
{
    free(address);
    close(internet_socket);
}
