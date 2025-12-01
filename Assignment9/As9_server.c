/*
 * server.c - TCP File Transfer Server
 * Usage: ./server <filename_to_serve>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h> 

#define PORT 9090
#define BUFFER_SIZE 4096
#define FILE_TO_RECEIVE "server_recvd_file" // Default name for incoming files

void die(const char *msg) {
    perror(msg);
    exit(1);
}

// Modified: Accepts filename as argument
void send_file(int client_sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("[-] Error in reading file to send");
        return;
    }

    // 1. Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    printf("[*] Sending file '%s' (Size: %ld bytes)...\n", filename, file_size);

    // 2. Send file size first
    send(client_sock, &file_size, sizeof(file_size), 0);

    // 3. Send file content
    char buffer[BUFFER_SIZE];
    clock_t start = clock(); 
    
    while (1) {
        int bytes_read = fread(buffer, 1, BUFFER_SIZE, fp);
        if (bytes_read > 0) {
            if (send(client_sock, buffer, bytes_read, 0) == -1) {
                die("[-] Error in sending file");
            }
        }
        if (bytes_read < BUFFER_SIZE) {
            if (feof(fp)) break;
            if (ferror(fp)) die("[-] Error reading file");
        }
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("[+] Sent successfully.\n");
    printf("    Transfer Time (Tx): %f seconds\n", time_taken);
    
    fclose(fp);
}

void receive_file(int client_sock) {
    char save_name[50];
    sprintf(save_name, "%ld_%s", time(NULL), FILE_TO_RECEIVE);

    FILE *fp = fopen(save_name, "wb");
    if (fp == NULL) {
        perror("[-] Error opening file for write");
        return;
    }

    long file_size;
    if (recv(client_sock, &file_size, sizeof(file_size), 0) <= 0) {
        printf("[-] Client disconnected or error receiving size.\n");
        fclose(fp);
        return;
    }
    printf("[*] Incoming upload. Expecting: %ld bytes\n", file_size);

    char buffer[BUFFER_SIZE];
    long total_received = 0;
    int bytes_received;
    
    clock_t start = clock();

    while (total_received < file_size) {
        bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) break;

        fwrite(buffer, 1, bytes_received, fp);
        total_received += bytes_received;
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("[+] File received and saved as '%s'.\n", save_name);
    printf("    Transfer Time (Rx): %f seconds\n", time_taken);

    fclose(fp);
}

int main(int argc, char *argv[]) {
    int listen_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // --- ARGUMENT CHECK ---
    if (argc != 2) {
        printf("Usage: %s <file_to_serve_to_client>\n", argv[0]);
        printf("Example: %s my_big_data.txt\n", argv[0]);
        exit(1);
    }
    char *file_to_send = argv[1];

    // Check if the file actually exists before starting server
    if (access(file_to_send, F_OK) == -1) {
        printf("[-] Error: File '%s' not found. Create it first.\n", file_to_send);
        exit(1);
    }

    // 1. Socket Setup
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) die("socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int reuse = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        die("bind");

    if (listen(listen_sock, 5) < 0) die("listen");

    printf("Server listening on port %d...\n", PORT);
    printf("Ready to serve file: %s\n", file_to_send);

    // 2. Main Loop
    while(1) {
        printf("\nWaiting for connection...\n");
        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) die("accept");

        printf("[+] Connected to client.\n");

        // --- STEP A: Server sends the specific file ---
        printf("\n--- Phase 1: Sending '%s' to Client ---\n", file_to_send);
        send_file(client_sock, file_to_send);

        // --- STEP B: Server receives whatever client uploads ---
        printf("\n--- Phase 2: Receiving File from Client ---\n");
        receive_file(client_sock);

        close(client_sock);
        printf("[+] Connection closed.\n");
    }

    close(listen_sock);
    return 0;
}
