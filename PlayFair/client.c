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

    used['J' - 'A'] = 1;

    for (int i = 0; key[i] != '\0'; i++) {
        char c = toupper(key[i]);
        if (c >= 'A' && c <= 'Z' && !used[c - 'A']) {
            if (c == 'J') c = 'I';
            matrix[idx / 5][idx % 5] = c;
            used[c - 'A'] = 1;
            idx++;
        }
    }

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

void encrypt(char *text) {
    char clean[1024];
    int clean_idx = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        if (isalpha(text[i])) {
            char c = toupper(text[i]);
            if (c == 'J') c = 'I';

            if (clean_idx > 0 && clean[clean_idx - 1] == c) {
                clean[clean_idx++] = 'X';
            }
            clean[clean_idx++] = c;
        }
    }

    if (clean_idx % 2 != 0) {
        clean[clean_idx++] = 'X';
    }
    clean[clean_idx] = '\0';

    char result[1024];
    int idx = 0;

    for (int i = 0; i < clean_idx; i += 2) {
        int r1, c1, r2, c2;
        find_position(clean[i], &r1, &c1);
        find_position(clean[i + 1], &r2, &c2);

        if (r1 == r2) {
            result[idx++] = matrix[r1][(c1 + 1) % 5];
            result[idx++] = matrix[r2][(c2 + 1) % 5];
        } else if (c1 == c2) {
            result[idx++] = matrix[(r1 + 1) % 5][c1];
            result[idx++] = matrix[(r2 + 1) % 5][c2];
        } else {
            result[idx++] = matrix[r1][c2];
            result[idx++] = matrix[r2][c1];
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

    create_matrix(KEY);

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