// railfence_client.c - Windows version
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8085
#define RAILS 3

void encrypt(char *text, int rails) {
    int len = strlen(text);
    if (len == 0) return;

    char grid[100][1024];
    for (int i = 0; i < rails; i++) {
        for (int j = 0; j < len; j++) {
            grid[i][j] = '\0';
        }
    }

    // Fill grid in zigzag pattern
    int row = 0, dir = 1;
    for (int col = 0; col < len; col++) {
        grid[row][col] = text[col];
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }

    // Read row by row
    char result[1024];
    int idx = 0;
    for (int i = 0; i < rails; i++) {
        for (int j = 0; j < len; j++) {
            if (grid[i][j] != '\0') {
                result[idx++] = grid[i][j];
            }
        }
    }
    result[idx] = '\0';
    strcpy(text, result);
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

    encrypt(buffer, RAILS);
    printf("Encrypted: %s\n", buffer);

    send(sock, buffer, strlen(buffer), 0);

    closesocket(sock);
    WSACleanup();
    return 0;
}