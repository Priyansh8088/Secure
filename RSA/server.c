#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

#define PORT 8080
#define P 61
#define Q 53
#define E 17

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int mod_inverse(int e, int phi) {
    for (int d = 1; d < phi; d++) {
        if ((e * d) % phi == 1)
            return d;
    }
    return -1;
}

long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}

void decrypt(char *cipher, char *plain, int d, int n) {
    int len = strlen(cipher) / 4;
    for (int i = 0; i < len; i++) {
        char hex[5];
        strncpy(hex, cipher + i * 4, 4);
        hex[4] = '\0';
        int encrypted = (int)strtol(hex, NULL, 16);
        plain[i] = (char)mod_pow(encrypted, d, n);
    }
    plain[len] = '\0';
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[4096] = {0};
    char decrypted[1024] = {0};

    int n = P * Q;
    int phi = (P - 1) * (Q - 1);
    int d = mod_inverse(E, phi);

    printf("Private Key (d, n): (%d, %d)\n", d, n);

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
    printf("Server listening...\n");

    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed\n");
        return 1;
    }

    int valread = recv(new_socket, buffer, sizeof(buffer), 0);
    buffer[valread] = '\0';
    printf("Encrypted text: %s\n", buffer);

    decrypt(buffer, decrypted, d, n);
    printf("Decrypted text: %s\n", decrypted);

    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}