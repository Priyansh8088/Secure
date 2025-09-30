// hill_server.c - Windows version
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8082

// Key matrix [[3,3],[2,5]] with determinant 9, inverse exists
int key[2][2] = {{3, 3}, {2, 5}};
int inv_key[2][2] = {{15, 17}, {20, 9}}; // Modular inverse of key matrix

void decrypt(char *text) {
    int len = strlen(text);
    char result[1024];
    int idx = 0;

    for (int i = 0; i < len; i += 2) {
        if (i + 1 >= len) break;

        int c1 = text[i] - 'A';
        int c2 = text[i + 1] - 'A';

        int p1 = (inv_key[0][0] * c1 + inv_key[0][1] * c2) % 26;
        int p2 = (inv_key[1][0] * c1 + inv_key[1][1] * c2) % 26;

        if (p1 < 0) p1 += 26;
        if (p2 < 0) p2 += 26;

        result[idx++] = p1 + 'A';
        result[idx++] = p2 + 'A';
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

    printf("Hill Cipher Server listening on port %d...\n", PORT);

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