#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8082

int key[2][2] = {{3, 3}, {2, 5}};

void encrypt(char *text) {
    int len = strlen(text);
    char result[1024];
    int idx = 0;

    char clean[1024];
    int clean_idx = 0;
    for (int i = 0; i < len; i++) {
        if (isalpha(text[i])) {
            clean[clean_idx++] = toupper(text[i]);
        }
    }
    clean[clean_idx] = '\0';

    if (clean_idx % 2 != 0) {
        clean[clean_idx++] = 'X';
        clean[clean_idx] = '\0';
    }

    len = clean_idx;

    for (int i = 0; i < len; i += 2) {
        int p1 = clean[i] - 'A';
        int p2 = clean[i + 1] - 'A';

        int c1 = (key[0][0] * p1 + key[0][1] * p2) % 26;
        int c2 = (key[1][0] * p1 + key[1][1] * p2) % 26;

        result[idx++] = c1 + 'A';
        result[idx++] = c2 + 'A';
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

    encrypt(buffer);
    printf("Encrypted: %s\n", buffer);

    send(sock, buffer, strlen(buffer), 0);

    closesocket(sock);
    WSACleanup();
    return 0;
}