#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>

#define PORT 8080

int IP[] = {58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7};
int FP[] = {40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25};
int E[] = {32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,32,1};
int P[] = {16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25};
int PC1[] = {57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35,27,19,11,3,60,52,44,36,63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,28,20,12,4};
int PC2[] = {14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,16,7,27,20,13,2,41,52,31,37,47,55,30,40,51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32};
int shifts[] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

uint64_t key = 0x133457799BBCDFF1ULL;

void permute(uint8_t *out, uint8_t *in, int *table, int n) {
    for (int i = 0; i < n; i++)
        out[i] = in[table[i] - 1];
}

void left_shift(uint8_t *data, int len, int shifts) {
    for (int s = 0; s < shifts; s++) {
        uint8_t temp = data[0];
        for (int i = 0; i < len - 1; i++)
            data[i] = data[i + 1];
        data[len - 1] = temp;
    }
}

void generate_keys(uint64_t key, uint8_t keys[16][48]) {
    uint8_t key_bits[64], pc1[56], C[28], D[28], CD[56], pc2[48];
    for (int i = 0; i < 64; i++)
        key_bits[63 - i] = (key >> i) & 1;
    permute(pc1, key_bits, PC1, 56);
    memcpy(C, pc1, 28);
    memcpy(D, pc1 + 28, 28);
    for (int round = 0; round < 16; round++) {
        left_shift(C, 28, shifts[round]);
        left_shift(D, 28, shifts[round]);
        memcpy(CD, C, 28);
        memcpy(CD + 28, D, 28);
        permute(keys[round], CD, PC2, 48);
    }
}

void des_decrypt(uint64_t cipher, uint64_t *plain, uint8_t keys[16][48]) {
    uint8_t cipher_bits[64], ip[64], L[32], R[32], temp[32];
    for (int i = 0; i < 64; i++)
        cipher_bits[63 - i] = (cipher >> i) & 1;
    permute(ip, cipher_bits, IP, 64);
    memcpy(L, ip, 32);
    memcpy(R, ip + 32, 32);
    for (int round = 15; round >= 0; round--) {
        memcpy(temp, R, 32);
        uint8_t expanded[48], xored[48];
        permute(expanded, R, E, 48);
        for (int i = 0; i < 48; i++)
            xored[i] = expanded[i] ^ keys[round][i];
        uint8_t permuted[32];
        for (int i = 0; i < 32; i++)
            permuted[i] = xored[i];
        uint8_t p_out[32];
        permute(p_out, permuted, P, 32);
        for (int i = 0; i < 32; i++)
            R[i] = L[i] ^ p_out[i];
        memcpy(L, temp, 32);
    }
    uint8_t combined[64], fp[64];
    memcpy(combined, R, 32);
    memcpy(combined + 32, L, 32);
    permute(fp, combined, FP, 64);
    *plain = 0;
    for (int i = 0; i < 64; i++)
        *plain |= ((uint64_t)fp[63 - i]) << i;
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    uint8_t keys[16][48];

    generate_keys(key, keys);

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

    int blocks = strlen(buffer) / 16;
    printf("Decrypted text: ");
    for (int i = 0; i < blocks; i++) {
        char hex[17];
        strncpy(hex, buffer + i * 16, 16);
        hex[16] = '\0';
        uint64_t cipher_block = strtoull(hex, NULL, 16);
        uint64_t plain_block;
        des_decrypt(cipher_block, &plain_block, keys);
        for (int j = 7; j >= 0; j--) {
            char c = (plain_block >> (j * 8)) & 0xFF;
            if (c != 0) printf("%c", c);
        }
    }
    printf("\n");

    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}