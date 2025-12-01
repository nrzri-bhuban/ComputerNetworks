/*
 * client.c - TCP File Transfer Client
 * Usage: ./client <server_ip> <file_to_upload>
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
#define FILE_TO_SAVE_AS "client_downloaded_file" // Default name for download

void die(const char *msg) {
    perror(msg);
    exit(1);
}

void receive_file(int sock) {
    // Generate a unique name based on time to prevent overwriting
    char save_name[50];
    sprintf(save_name, "%ld_%s", time(NULL), FILE_TO_SAVE_AS);

    FILE *fp = fopen(save_name, "wb");
    if (fp == NULL) {
        perror("[-] Error opening file for write");
        return;
    }

    long file_size;
    if (recv(sock, &file_size, sizeof(file_size), 0) <= 0)
        die("[-] Connection lost receiving size");
        
    printf("[*] Downloading file. Size: %ld bytes\n", file_size);

    char buffer[BUFFER_SIZE];
    long total_received = 0;
    int bytes_received;

    clock_t start = clock();

    while (total_received < file_size) {
        bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) break;

        fwrite(buffer, 1, bytes_received, fp);
        total_received += bytes_received;
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("[+] Download complete. Saved as '%s'.\n", save_name);
    printf("    Transfer Time (Rx): %f seconds\n", time_taken);
    
    fclose(fp);
}

// Modified: Accepts filename as argument
void send_file(int sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("[-] Error reading file to upload");
        return;
    }

    // 1. Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    printf("[*] Uploading '%s' (Size: %ld bytes)...\n", filename, file_size);

    // 2. Send file size
    send(sock, &file_size, sizeof(file_size), 0);

    // 3. Send file content
    char buffer[BUFFER_SIZE];
    clock_t start = clock();

    while (1) {
        int bytes_read = fread(buffer, 1, BUFFER_SIZE, fp);
        if (bytes_read > 0) {
            if (send(sock, buffer, bytes_read, 0) == -1) {
                die("[-] Error sending file");
            }
        }
        if (bytes_read < BUFFER_SIZE) {
            if (feof(fp)) break;
            if (ferror(fp)) die("[-] Error reading file");
        }
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("[+] Upload complete.\n");
    printf("    Transfer Time (Tx): %f seconds\n", time_taken);

    fclose(fp);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;

    // --- ARGUMENT CHECK ---
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <file_to_upload>\n", argv[0]);
        fprintf(stderr, "Example: %s 10.0.0.1 homework.txt\n", argv[0]);
        exit(1);
    }
    
    char *server_ip = argv[1];
    char *file_to_upload = argv[2];

    // Check if upload file exists
    if (access(file_to_upload, F_OK) == -1) {
        printf("[-] Error: File '%s' not found. Cannot upload.\n", file_to_upload);
        exit(1);
    }

    // 1. Create Socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) die("socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        die("inet_pton");
    }

    // 2. Connect
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        die("connect");
    }
    printf("Connected to server at %s.\n", server_ip);

    // --- STEP A: Client Download (Server Sends whatever it was started with) ---
    printf("\n--- Phase 1: Downloading from Server ---\n");
    receive_file(sock);

    // --- STEP B: Client Upload (Client Sends argument file) ---
    printf("\n--- Phase 2: Uploading '%s' to Server ---\n", file_to_upload);
    send_file(sock, file_to_upload);

    // 3. Close
    close(sock);
    printf("\n[+] Connection closed.\n");

    return 0;
}
