// vigenere_server.c - Windows version
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8086
#define KEY "SECRET"

void decrypt(char *text, char *key) {
    int text_len = strlen(text);
    int key_len = strlen(key);
    int key_idx = 0;

    for (int i = 0; i < text_len; i++) {
        if (isalpha(text[i])) {
            int shift = toupper(key[key_idx % key_len]) - 'A';

            if (isupper(text[i])) {
                text[i] = ((text[i] - 'A' - shift + 26) % 26) + 'A';
            } else {
                text[i] = ((text[i] - 'a' - shift + 26) % 26) + 'a';
            }
            key_idx++;
        }
    }
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

    printf("Vigenere Cipher Server listening on port %d...\n", PORT);
    printf("Using key: %s\n", KEY);

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