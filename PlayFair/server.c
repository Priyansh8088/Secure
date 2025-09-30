// playfair_server.c - Windows version
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8084
#define KEY "MONARCHY"

char matrix[5][5];

void create_matrix(char *key) {
    int used[26] = {0};
    int idx = 0;

    used['J' - 'A'] = 1; // Treat I and J as same

    // Add key characters
    for (int i = 0; key[i] != '\0'; i++) {
        char c = toupper(key[i]);
        if (c >= 'A' && c <= 'Z' && !used[c - 'A']) {
            if (c == 'J') c = 'I';
            matrix[idx / 5][idx % 5] = c;
            used[c - 'A'] = 1;
            idx++;
        }
    }

    // Add remaining letters
    for (char c = 'A'; c <= 'Z'; c++) {
        if (!used[c - 'A']) {
            if (c == 'J') continue;
            matrix[idx / 5][idx % 5] = c;
            idx++;
        }
    }
}

void find_position(char c, int *row, int *col) {
    if (c == 'J') c = 'I';
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (matrix[i][j] == c) {
                *row = i;
                *col = j;
                return;
            }
        }
    }
}

void decrypt(char *text) {
    int len = strlen(text);
    char result[1024];
    int idx = 0;

    for (int i = 0; i < len; i += 2) {
        if (i + 1 >= len) break;

        int r1, c1, r2, c2;
        find_position(text[i], &r1, &c1);
        find_position(text[i + 1], &r2, &c2);

        if (r1 == r2) {
            // Same row - move left
            result[idx++] = matrix[r1][(c1 + 4) % 5];
            result[idx++] = matrix[r2][(c2 + 4) % 5];
        } else if (c1 == c2) {
            // Same column - move up
            result[idx++] = matrix[(r1 + 4) % 5][c1];
            result[idx++] = matrix[(r2 + 4) % 5][c2];
        } else {
            // Rectangle - swap columns
            result[idx++] = matrix[r1][c2];
            result[idx++] = matrix[r2][c1];
        }
    }
    result[idx] = '\0';
    strcpy(text, result);
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    create_matrix(KEY);

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

    printf("Playfair Cipher Server listening on port %d...\n", PORT);

    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed\n");
        return 1;
    }

    int valread = recv(new_socket, buffer, sizeof(buffer), 0);
    buffer[valread] = '\0';

    printf("Encrypted text: %s\n", buffer);
    decrypt(buffer);
    printf("Decrypted text: %s\n", buffer);

    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}