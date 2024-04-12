// TIRZAH GRACE YARRANNA - 21CS10071

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 30000
#define MAX_ENTITIES 10
#define ENTITY_SIZE 200


int main()
{
    int vote_req;
    vote_req = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(vote_req < 0){
        perror("vote_req");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);


    if( connect(vote_req, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(vote_req);
        exit(EXIT_FAILURE);
    }
    printf("connected.\n");
    int size;
    if(recv(vote_req,&size,sizeof(int),0 ) < 0) {
        perror("recv");
        close(vote_req);
        exit(EXIT_FAILURE);
    }

    char buffer[ENTITY_SIZE];
    char entity[10][ENTITY_SIZE];
    if(recv(vote_req,&entity,sizeof(entity),0 ) < 0) {
        perror("recv");
        close(vote_req);
        exit(EXIT_FAILURE);
    }
    int i;
    for(i= 0; i<size; i++)
    {
        printf("%d : %s\n",i,entity[i]);
    }
    printf("Choose an option ( [0,%d) ) to vote, or outside of this region to exit.\n",size);
    int option;
    scanf("%d", &option);
    if(option >= 0 && option < size) {

    memset(&buffer,0,sizeof(buffer));
    strcpy(buffer,entity[option]);
    if(send(vote_req,&buffer,sizeof(buffer),0 ) < 0) {
        perror("send");
        close(vote_req);
        exit(EXIT_FAILURE);
    }

    memset(&buffer,0,sizeof(buffer));
    if(recv(vote_req,&buffer,sizeof(buffer),0 ) < 0) {
        perror("recv");
        close(vote_req);
        exit(EXIT_FAILURE);
    }
    printf("received: %s\n", buffer);
    }
    else 
    {
    memset(&buffer,0,sizeof(buffer));
    strcpy(buffer,"-EXIT\n");
    if(send(vote_req,&buffer,sizeof(buffer),0 ) < 0) {
        perror("send");
        close(vote_req);
        exit(EXIT_FAILURE);
    }
    }
    close(vote_req);
    printf("Exiting\n");
    return 0;
}
