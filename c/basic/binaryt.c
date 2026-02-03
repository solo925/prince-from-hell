// i byte = 8 bits
#include <stdio.h>

int sizes() {
    unsigned char byte = 0b10101010; // 1 byte (8 bits)
    unsigned short two_bytes = 0b1010101010101010; // 2 bytes (16 bits)
    unsigned int four_bytes = 0b10101010101010101010101010101010; // 4 bytes (32 bits)
    unsigned long long eight_bytes = 0b1010101010101010101010101010101010101010101010101010101010101010; // 8 bytes (64 bits)

    return 0;
}

int operations() {
    unsigned char a = 0b1100; // 12 in decimal
    unsigned char b = 0b1010; // 10 in decimal

    unsigned char and_result = a & b; // Bitwise AND
    unsigned char or_result = a | b;  // Bitwise OR
    unsigned char xor_result = a ^ b; // Bitwise XOR
    unsigned char not_result = ~a;     // Bitwise NOT

    unsigned char left_shift = a << 2; // Left shift by 2
    unsigned char right_shift = a >> 2; // Right shift by 2

    return 0;
}

int sample(){
    int n = 13;
    for(int i = 0;i<=n;i++){
        int bit = (n >> i)&1;
        printf("%d", bit);
    }
}

int main() {
    printf("sizes\n");
    sizes();
    printf("operations\n");
    operations();
    printf("sample\n");
    sample();
    printf("\n");
    return 0;
}




