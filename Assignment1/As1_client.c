#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define port 8080
#define buff 1024

int main(){
    int socket_fd;
    struct sockaddr_in server;
    char Buffer[buff] = {0}; // Initialize buffer to zero
    
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
          perror("Socket creation failed");
          exit(1);
    }

    // Define server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("10.0.0.1"); 
    server.sin_port = htons(port);
    
    // Connect to server
    if(connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("Connection error");
        exit(1);
    }
        
    printf("Connected to server.\n");
    
    char *message = "Hi";
    printf("Sending: %s\n", message);
    
    write(socket_fd, message, strlen(message));
    
    read(socket_fd, Buffer, buff);
    printf("Server says: %s\n", Buffer);
    
    close(socket_fd);
    return 0;
}
