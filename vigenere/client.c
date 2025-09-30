#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8086
#define KEY "SECRET"

void encrypt(char *text, char *key) {
    int text_len = strlen(text);
    int key_len = strlen(key);
    int key_idx = 0;

    for (int i = 0; i < text_len; i++) {
        if (isalpha(text[i])) {
            int shift = toupper(key[key_idx % key_len]) - 'A';

            if (isupper(text[i])) {
                text[i] = ((text[i] - 'A' + shift) % 26) + 'A';
            } else {
                text[i] = ((text[i] - 'a' + shift) % 26) + 'a';
            }
            key_idx++;
        }
    }
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

    printf("Using key: %s\n", KEY);
    encrypt(buffer, KEY);
    printf("Encrypted: %s\n", buffer);

    send(sock, buffer, strlen(buffer), 0);

    closesocket(sock);
    WSACleanup();
    return 0;
}