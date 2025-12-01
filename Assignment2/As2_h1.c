#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUF_SIZE 2048

// (i) Record format: Name, Quantity, Timestamp
struct Fruit {
    char name[20];
    int quantity;
    time_t last_sold;
};

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[BUF_SIZE];
    char response[BUF_SIZE];
    char temp[256]; // Temporary string builder

    // Initialize Inventory
    struct Fruit shop[3] = {
        {"Apple", 50, 0},
        {"Orange", 50, 0},
        {"Banana", 50, 0}
    };

    // Store unique customer IDs (v)
    char history[100][50]; 
    int unique_count = 0;

    // Create Socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // Avoid "Address already in use" error
    int reuse = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_sock, 5);
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        // (ii) Accept connection (one at a time)
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        
        // Create Customer ID string <IP:Port>
        char current_id[50];
        sprintf(current_id, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Client connected: %s\n", current_id);

        // Handle client requests
        while (recv(client_sock, buffer, BUF_SIZE, 0) > 0) {
            char req_name[20];
            int req_qty;
            int found = 0;
            
            // Clear response buffer
            strcpy(response, ""); 

            // Parse input: "Apple 5"
            if (sscanf(buffer, "%s %d", req_name, &req_qty) == 2) {
                
                // Search for fruit
                for (int i = 0; i < 3; i++) {
                    if (strcasecmp(shop[i].name, req_name) == 0) {
                        found = 1;
                        // (iv) Check quantity
                        if (shop[i].quantity >= req_qty) {
                            // (iii) Update stock and timestamp
                            shop[i].quantity -= req_qty;
                            shop[i].last_sold = time(NULL);
                            
                            // (v) Add to unique history if new
                            int is_new = 1;
                            for(int k=0; k<unique_count; k++) {
                                if(strcmp(history[k], current_id) == 0) is_new = 0;
                            }
                            if(is_new) {
                                strcpy(history[unique_count], current_id);
                                unique_count++;
                            }
                            sprintf(temp, "SUCCESS: Bought %d %s(s)\n", req_qty, shop[i].name);
                        } else {
                            sprintf(temp, "REGRET: Only %d %s(s) available.\n", shop[i].quantity, shop[i].name);
                        }
                        strcat(response, temp);
                        break;
                    }
                }
                if (!found) strcat(response, "ERROR: Fruit not found.\n");

            } else {
                strcat(response, "ERROR: Send format 'FruitName Quantity'\n");
            }

            // --- Append Report to Response ---
            
            // 1. Current Stock
            strcat(response, "\n--- Stock Status ---\n");
            for(int i=0; i<3; i++) {
                // If never sold, show "0", else show timestamp
                long t = (long)shop[i].last_sold;
                sprintf(temp, "%s | Qty: %d | Last Sold: %ld\n", shop[i].name, shop[i].quantity, t);
                strcat(response, temp);
            }

            // 2. Customer List (v)
            strcat(response, "\n--- Transactions ---\n");
            for(int i=0; i<unique_count; i++) {
                sprintf(temp, "%s\n", history[i]);
                strcat(response, temp);
            }

            // 3. Total Unique Count (vi)
            sprintf(temp, "\nTotal Unique Customers: %d\n", unique_count);
            strcat(response, temp);

            send(client_sock, response, strlen(response), 0);
            memset(buffer, 0, BUF_SIZE); // Reset buffer
        }
        
        close(client_sock);
        printf("Client disconnected.\n");
    }
    
    close(server_sock);
    return 0;
}
