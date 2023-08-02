#include <inttypes.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;

#define d_bit ((u8)0b10)
#define w_bit ((u8)0b1)

char const *registers[16] =
{
    [0b0000] = "al",
    [0b0001] = "cl",
    [0b0010] = "dl",
    [0b0011] = "bl",
    [0b0100] = "ah",
    [0b0101] = "ch",
    [0b0110] = "dh",
    [0b0111] = "bh",

    [0b1000] = "ax",
    [0b1001] = "cx",
    [0b1010] = "dx",
    [0b1011] = "bx",
    [0b1100] = "sp",
    [0b1101] = "bp",
    [0b1110] = "si",
    [0b1111] = "di",
};

void decode_mov(u8 first_byte, u8 second_byte)
{
    u8 dst, src;

    if (first_byte & d_bit)
    {
        dst = (second_byte >> 3) & 0b111;
        src = second_byte & 0b111;
    }
    else
    {
        src = (second_byte >> 3) & 0b111;
        dst = second_byte & 0b111;
    }

    u8 w = 0b1000 * (first_byte & w_bit);
    printf("mov %s, %s\n", registers[dst | w], registers[src | w]);
}

int main()
{
    puts("bits 16\n");
    for (;;)
    {
        int byte1 = getchar();
        int byte2 = getchar();
        if (byte1 == EOF) return 0;

        decode_mov((u8)byte1, (u8)byte2);
    }
}
