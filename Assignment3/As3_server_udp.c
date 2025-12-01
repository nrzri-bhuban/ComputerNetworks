#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUF_SIZE 2048

struct Fruit {
    char name[20];
    int quantity;
    time_t last_sold;
};

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buffer[BUF_SIZE];
    char response[BUF_SIZE];
    char temp[256];

    // Initialize Inventory
    struct Fruit shop[3] = {
        {"Apple", 50, 0},
        {"Orange", 50, 0},
        {"Banana", 50, 0}
    };

    // Store unique customer IDs
    char history[100][50]; 
    int unique_count = 0;

    // 1. Create UDP Socket (SOCK_DGRAM)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 2. Bind
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    printf("UDP Fruit Server listening on port %d...\n", PORT);

    while (1) {
        addr_len = sizeof(client_addr);
        
        // 3. Receive Packet 
        memset(buffer, 0, BUF_SIZE);
        if (recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len) < 0) {
            continue; 
        }

        // Create Customer ID string <IP:Port>
        char current_id[50];
        sprintf(current_id, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Request from: %s\n", current_id);

        char req_name[20];
        int req_qty;
        int found = 0;
        strcpy(response, ""); 

        if (sscanf(buffer, "%s %d", req_name, &req_qty) == 2) {
            for (int i = 0; i < 3; i++) {
                if (strcasecmp(shop[i].name, req_name) == 0) {
                    found = 1;
                    if (shop[i].quantity >= req_qty) {
                        shop[i].quantity -= req_qty;
                        shop[i].last_sold = time(NULL);
                        
                        // Add to unique history if new
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

        // 5. Append Stock Report (Same as before)
        strcat(response, "\n--- Stock Status ---\n");
        for(int i=0; i<3; i++) {
            long t = (long)shop[i].last_sold;
            sprintf(temp, "%s | Qty: %d | Last Sold: %ld\n", shop[i].name, shop[i].quantity, t);
            strcat(response, temp);
        }

        strcat(response, "\n--- Transactions ---\n");
        for(int i=0; i<unique_count; i++) {
            sprintf(temp, "%s\n", history[i]);
            strcat(response, temp);
        }

        sprintf(temp, "\nTotal Unique Customers: %d\n", unique_count);
        strcat(response, temp);

        // 6. Send Reply (sendto uses the client_addr we caught earlier)
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&client_addr, addr_len);
    }
    
    close(sockfd);
    return 0;
}
