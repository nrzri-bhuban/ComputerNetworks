// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define PI 3.14159265

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Scientific Calculator Server listening on port %d...\n", PORT);

    while (1) {
        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, 
                        (struct sockaddr *)&client_addr, &addr_len);
        buffer[n] = '\0';
        
        printf("Client requested: %s\n", buffer);

        char op[10];
        float num1, num2, result = 0.0;
        int args_parsed;
        char response[BUFFER_SIZE];

        // Format: "OP NUM1" or "OP NUM1 NUM2"
        // Example: "sin 90" or "add 5 10"
        args_parsed = sscanf(buffer, "%s %f %f", op, &num1, &num2);

        if (strcmp(op, "sin") == 0) {
            // Convert degrees to radians
            result = sin(num1 * (PI / 180.0)); 
        } else if (strcmp(op, "cos") == 0) {
            result = cos(num1 * (PI / 180.0));
        } else if (strcmp(op, "tan") == 0) {
            result = tan(num1 * (PI / 180.0));
        } else if (strcmp(op, "inv") == 0 || strcmp(op, "1/x") == 0) {
            if (num1 != 0) result = 1.0 / num1;
            else strcpy(response, "Error: Div by Zero");
        } else if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) {
            result = num1 + num2;
        } else if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) {
            result = num1 - num2;
        } else if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) {
            result = num1 * num2;
        } else if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
            if (num2 != 0) result = num1 / num2;
            else strcpy(response, "Error: Div by Zero");
        } else {
            strcpy(response, "Error: Invalid Operation");
        }

        // If response wasn't an error message, format the float result
        if (strlen(response) == 0 || response[0] != 'E') {
            sprintf(response, "Result: %.4f", result);
        }

        sendto(sockfd, (const char *)response, strlen(response), MSG_CONFIRM, 
               (const struct sockaddr *)&client_addr, addr_len);
        
        printf("Sent to client: %s\n", response);
        // Reset response buffer
        memset(response, 0, sizeof(response));
    }
    return 0;
}
