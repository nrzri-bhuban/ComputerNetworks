#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 8080

// Globals
int all_clients[100];
int client_count = 0;


void write_log(char *msg) {
    FILE *fp = fopen("log.txt", "a");
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';
    fprintf(fp, "[%s] %s\n", time_str, msg);
    fclose(fp);
}

// Thread function to handle one client
void *client_handler(void *arg) {
    // We cast the argument back to an integer socket
    int sock = *((int*)arg);
    free(arg);
    
    char buffer[1024];
    int len;

    while ( (len = recv(sock, buffer, 1024, 0)) > 0 ) {
        buffer[len] = '\0';
        
        // 1. Show on Console
        printf("Client: %s\n", buffer);
        
        // 2. Write to Log File
        write_log(buffer);

        // 3. Broadcast to other clients
        for(int i=0; i < client_count; i++) {
            if(all_clients[i] != sock) {
                send(all_clients[i], buffer, strlen(buffer), 0);
            }
        }
    }
    close(sock);
    return NULL;
}

int main() {
    int sockfd, newfd;
    struct sockaddr_in server, client;
    pthread_t thread_id;
    int c = sizeof(struct sockaddr_in);

    // Setup Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    listen(sockfd, 3);

    printf("Server live on port %d. Waiting for connections...\n", PORT);

    while( (newfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        printf("Accepted\n");
        
        // Save client to list for broadcasting
        all_clients[client_count++] = newfd;

        // Allocate memory for the socket ID to pass it safely to the thread
        int *safe_sock = malloc(sizeof(int));
        *safe_sock = newfd;

        if( pthread_create( &thread_id, NULL, client_handler, (void*) safe_sock) < 0)
        {
            perror("Thread error");
            return 1;
        }
    }

    return 0;
}
