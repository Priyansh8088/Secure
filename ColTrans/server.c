// columnar_server.c - Windows version
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8081
#define KEY "KEY"

void decrypt(char *text, char *key) {
    int len = strlen(text);
    int keylen = strlen(key);
    int cols = keylen;
    int rows = (len + cols - 1) / cols;

    int order[10];
    char sorted_key[10];
    strcpy(sorted_key, key);

    // Create column order based on key
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

    char grid[20][10] = {0};
    int idx = 0;

    // Fill grid column by column in sorted order
    for (int col = 0; col < cols; col++) {
        int actual_col = 0;
        for (int i = 0; i < cols; i++) {
            if (order[i] == col) {
                actual_col = i;
                break;
            }
        }
        for (int row = 0; row < rows && idx < len; row++) {
            grid[row][actual_col] = text[idx++];
        }
    }

    // Read row by row
    idx = 0;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (grid[row][col] != 0) {
                text[idx++] = grid[row][col];
            }
        }
    }
    text[idx] = '\0';
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    WSAStartup(MAKEWORD(2,2), &wsaData);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Columnar Transposition Server listening on port %d...\n", PORT);

    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed\n");
        return 1;
    }

    int valread = recv(new_socket, buffer, sizeof(buffer), 0);
    buffer[valread] = '\0';

    printf("Encrypted text: %s\n", buffer);
    decrypt(buffer, KEY);
    printf("Decrypted text: %s\n", buffer);

    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}