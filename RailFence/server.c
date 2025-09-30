#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8085
#define RAILS 3

void decrypt(char *text, int rails) {
    int len = strlen(text);
    if (len == 0) return;

    char grid[100][1024];
    for (int i = 0; i < rails; i++) {
        for (int j = 0; j < len; j++) {
            grid[i][j] = '\0';
        }
    }


    int row = 0, dir = 1;
    for (int col = 0; col < len; col++) {
        grid[row][col] = '*';
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }


    int idx = 0;
    for (int i = 0; i < rails; i++) {
        for (int j = 0; j < len; j++) {
            if (grid[i][j] == '*' && idx < len) {
                grid[i][j] = text[idx++];
            }
        }
    }


    char result[1024];
    idx = 0;
    row = 0;
    dir = 1;
    for (int col = 0; col < len; col++) {
        result[idx++] = grid[row][col];
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
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

    printf("Rail Fence Cipher Server listening on port %d...\n", PORT);

    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed\n");
        return 1;
    }

    int valread = recv(new_socket, buffer, sizeof(buffer), 0);
    buffer[valread] = '\0';

    printf("Encrypted text: %s\n", buffer);
    decrypt(buffer, RAILS);
    printf("Decrypted text: %s\n", buffer);

    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}