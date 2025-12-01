#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

// Thread to receive messages from server
void *receive_messages(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];
    int len;
    while( (len = recv(sock, buffer, 1024, 0)) > 0 ) {
        buffer[len] = '\0';
        printf("New Message: %s\n", buffer);
    }
    return NULL;
}

int main() {
    int sock;
    struct sockaddr_in server;
    char message[1024];
    pthread_t recv_thread;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("10.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to Chat Server.\n");

    // Start thread to listen for incoming messages
    pthread_create(&recv_thread, NULL, receive_messages, (void*) &sock);

    // Main loop to send messages
    while(1) {
        fgets(message, 1024, stdin);
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}
