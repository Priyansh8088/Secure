// columnar_client.c - Windows version
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8081
#define KEY "KEY"

void encrypt(char *text, char *key) {
    int len = strlen(text);
    int keylen = strlen(key);
    int rows = (len + keylen - 1) / keylen;

    char grid[20][10] = {0};
    int idx = 0;

    // Fill grid row by row
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < keylen && idx < len; col++) {
            grid[row][col] = text[idx++];
        }
    }

    // Create column order based on alphabetical key
    int order[10];
    char sorted_key[10];
    strcpy(sorted_key, key);

    for (int i = 0; i < keylen; i++) {
        int min_idx = i;
        for (int j = i + 1; j < keylen; j++) {
            if (sorted_key[j] < sorted_key[min_idx]) {
                char temp = sorted_key[min_idx];
                sorted_key[min_idx] = sorted_key[j];
                sorted_key[j] = temp;

                int t = (i == min_idx) ? j : min_idx;
                min_idx = (i == min_idx) ? min_idx : i;
            }
        }
    }

    for (int i = 0; i < keylen; i++) {
        for (int j = 0; j < keylen; j++) {
            if (key[i] == sorted_key[j]) {
                order[i] = j;
                sorted_key[j] = 0;
                break;
            }
        }
    }

    // Read column by column in sorted order
    idx = 0;
    for (int col = 0; col < keylen; col++) {
        int actual_col = 0;
        for (int i = 0; i < keylen; i++) {
            if (order[i] == col) {
                actual_col = i;
                break;
            }
        }
        for (int row = 0; row < rows; row++) {
            if (grid[row][actual_col] != 0) {
                text[idx++] = grid[row][actual_col];
            }
        }
    }
    text[idx] = '\0';
}

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    WSAStartup(MAKEWORD(2,2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return 1;
    }

    printf("Enter a message: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    encrypt(buffer, KEY);
    printf("Encrypted: %s\n", buffer);

    send(sock, buffer, strlen(buffer), 0);

    closesocket(sock);
    WSACleanup();
    return 0;
}