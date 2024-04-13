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

struct vote_info {
    char entity[MAX_ENTITIES][ENTITY_SIZE];
    int votes[MAX_ENTITIES];
};

int main()
{
    int vote_req;
    vote_req = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(vote_req < 0){
        perror("vote_req");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));

    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if(bind(vote_req, (struct sockaddr* )&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(vote_req);
        exit(EXIT_FAILURE);
    }
    printf("Binded.\n");

    struct vote_info* Vote_Info_Table;
    int shm_id = shmget(IPC_PRIVATE, sizeof(struct vote_info), 0777|IPC_CREAT ); 
    Vote_Info_Table = (struct vote_info *) shmat(shm_id, 0, 0);

    int *table_size;
    int i_shm_id = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT ); 
    table_size = (int*) shmat(i_shm_id, 0, 0);
    *table_size = 0;

    listen(vote_req,5);

    int new_sock_fd; int print = 1;
    while(1)
    {
        if(print) 
        {
            printf("\n\nAdminstrator: \n");
            printf("1. Print list\n");
            printf("2. Add Entity\n");
            printf("3. Delete Entity\n");
            printf("4. Exit\n");
            print = 0;
        }

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO,&read_fds);
        FD_SET(vote_req,&read_fds);

        struct timeval timeout;
        timeout.tv_sec = 5 ;
        timeout.tv_usec = 0;

        int max_fd = STDIN_FILENO;
        if( vote_req > max_fd) max_fd = vote_req;

        int count = select(max_fd+1, &read_fds, NULL, NULL, &timeout);

        if(count < 0)
        {
            // perror("select");
            close(vote_req);
            exit(EXIT_FAILURE);
        }
        if(FD_ISSET(STDIN_FILENO, &read_fds))
        {
            // adminstrator user
            int option;
            scanf("%d", &option);
            // printf("Adminstrator.\n");
            // printf("option: %d\n", option);
            if(option == 1)
            {
                int size = *table_size;
                int i;
                printf("\nEntity\t\t\t\t Votes\n");
                for(i=0; i<size; i++)
                {
                    printf("%s\t\t\t\t: %d\n", Vote_Info_Table->entity[i], Vote_Info_Table->votes[i]);
                }
                printf("___________________________________________________\n");
            }
            else if(option == 2)
            {
                printf("Enter Entity name: ");
                char input[ENTITY_SIZE];
                scanf("%s", input);
                // printf("input: %s\n", input);
                input[strlen(input)] = '\0';
                int i, present = 0, size = (*table_size);
                // printf("size: %d\n", size);
                for(i=0; i< size; i++)
                {
                	if(strlen(input) == strlen(Vote_Info_Table->entity[i])) {
                    if(strncmp(Vote_Info_Table->entity[i], input, strlen(input)) == 0 )
                    {
                        present = 1;
                        break;
                    }
                	}
                }
                if( present == 0) {
                strncpy( Vote_Info_Table->entity[size], input , strlen(input));
                Vote_Info_Table->votes[size] = 0 ;
                (*table_size) = size + 1 ;
                // printf("updated table size: %d\n", *table_size);
                printf("Added succesfully.\n");
                }
                else if(present)
                {
                    printf("index: %d\n", i);
                    printf("Already present.\n");
                }
            }
            else if(option == 3)
            {
                printf("Enter Entity name to delete: ");
                char input[ENTITY_SIZE];
                scanf("%s", input);
                // printf("input: %s\n", input);
                input[strlen(input)] = '\0';
                int i, present = 0, size = (*table_size);
                // printf("size: %d\n", size);
                for(i=0; i< size; i++)
                {
                	if(strlen(input) == strlen(Vote_Info_Table->entity[i])) {
                    if(strncmp(Vote_Info_Table->entity[i], input, strlen(input)) == 0 )
                    {
                        present = 1; int j;
                        for(j = i+1; j<size; j++)
                        {
                            strcpy(Vote_Info_Table->entity[j-1], Vote_Info_Table->entity[j]);
                            Vote_Info_Table->votes[j-1] =  Vote_Info_Table->votes[j]; 
                        }
                        (*table_size) = size - 1 ;
                        break;
                    }
                	}
                }
                if( present == 0) {
                    printf("Entity %s Not present.\n", input);
                }
            }
            else if(option == 4)
            {
                close(vote_req);
                break;
            }
            else 
            {
                printf("select proper option (1,2,3) .\n");
            }
            print = 1;
        }
        if(FD_ISSET(vote_req, &read_fds))
        {
            // client connection.
            socklen_t client_len = sizeof(client_addr);
            new_sock_fd = accept( vote_req, (struct sockaddr *)&client_addr, &client_len);
            if(new_sock_fd < 0)
            {
                perror("accept");
                close(vote_req);
                exit(EXIT_FAILURE);
            }

            if(fork() == 0)
            {
                close(vote_req);

                int i;
                // handle connection. 
                int size = *table_size ;
                if( send(new_sock_fd, &size, sizeof(int),0) < 0)
                {
                    perror("send");
                    close(new_sock_fd);
                    exit(EXIT_FAILURE);
                }
                if( send(new_sock_fd, Vote_Info_Table->entity, sizeof(Vote_Info_Table->entity),0) < 0)
                {
                    perror("send");
                    close(new_sock_fd);
                    exit(EXIT_FAILURE);
                }
                char buffer[1024]; int bytes;
                memset(&buffer,0,sizeof(buffer));
                if( ( bytes = recv(new_sock_fd, &buffer, sizeof(buffer),0)) < 0 )
                {
                    perror("recv");
                    close(new_sock_fd);
                    exit(EXIT_FAILURE);
                }
                buffer[bytes] = '\0';
                if(strncmp(buffer,"-ERR",4) == 0)
                {
                    close(new_sock_fd);
                    continue;
                }
                size = *table_size ;
                int done = 0;
                for(i=0; i<size; i++)
                {
                    if(bytes == strlen(Vote_Info_Table->entity[i])) {
                    if(strncmp(Vote_Info_Table->entity[i], buffer, bytes) == 0 )
                    {
                        Vote_Info_Table->votes[i]++;
                        memset(&buffer,0,sizeof(buffer));
                        strcpy(buffer,"Vote registered succesfully.\n");
                         if( send(new_sock_fd, &buffer, sizeof(buffer),0) < 0)
                            {
                                perror("send");
                                close(new_sock_fd);
                                exit(EXIT_FAILURE);
                            }
                        done = 1;
                        break;
                    }
                    }
                }
                if(done == 0)
                {
                    memset(&buffer,0,sizeof(buffer));
                        strcpy(buffer,"Problem in voting. Try again later.\n");
                         if( send(new_sock_fd, &buffer, sizeof(buffer),0) < 0)
                            {
                                perror("send");
                                close(new_sock_fd);
                                exit(EXIT_FAILURE);
                            }
                }
                close(new_sock_fd);
            }
            close(new_sock_fd);
        }
    }
    shmdt(&shm_id);
    shmdt(&i_shm_id);
    shmctl(shm_id,IPC_RMID,0);
    shmctl(i_shm_id,IPC_RMID,0);

    return 0;
}
