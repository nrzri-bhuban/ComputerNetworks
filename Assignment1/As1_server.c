#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/socket.h> 

#define PORT 8080
#define buff 1024

int main(){
    int socket_fd, new_socket;
    socklen_t sock_len;
    struct sockaddr_in server, client;
    char Buffer[buff] = {0}; // Initialize buffer to zero
    
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0); //TCP
    if (socket_fd == -1) {
          perror("Socket creation failed");
          exit(1);
    }
        
    // Define server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    // Bind socket
    if(bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("Binding Error");
        exit(1);
    }
    
    // Listen for connections
    listen(socket_fd, 5);
    printf("Server is listening on port %d...\n", PORT);
    
    sock_len = sizeof(client);
    new_socket = accept(socket_fd, (struct sockaddr*)&client, &sock_len);
    if (new_socket < 0) {
          perror("Accept failed");
          exit(1);
    }

    // Read message from client
    read(new_socket, Buffer, buff);
    printf("Client says: %s\n", Buffer);
    
    char *response = "Hello"; 
    
    write(new_socket, response, strlen(response));
    printf("Response sent: %s\n", response);
    
    close(socket_fd);
    close(new_socket);
    return 0;
}
