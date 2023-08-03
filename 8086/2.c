#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

typedef uint8_t u8;
typedef uint16_t u16;

typedef int8_t i8;
typedef int16_t i16;

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

char const *expressions[] =
{
    [0b000] = "bx + si",
    [0b001] = "bx + di",
    [0b010] = "bp + si",
    [0b011] = "bp + di",
    [0b100] = "si",
    [0b101] = "di",
    [0b110] = "bp",
    [0b111] = "bx",
};

int main(int argc, char **argv)
{
    FILE *istream;

    if (argc == 2)
    {
        istream = fopen(argv[1], "rb");
        if (istream == NULL)
        {
            fprintf(stderr, "Error opening file `%s`\n", argv[0]);
            exit(1);
        }
    }
    else if (argc == 1)
    {
        istream = stdin;
    }

    puts("bits 16\n");
    for (;;)
    {
        int byte = getc(istream);
        if (byte == EOF) return 0;

        u8 byte1 = (u8)byte;

        switch (byte1)
        {
        case 0b10001000: case 0b10001001: case 0b10001010: case 0b10001011:
            // NOTE(chitato): MOV Register/memory to/from register
            {
                u8 byte2 = getc(istream);
                char exp_buf[64];

                u8 d   = (byte1 & 0b10) >> 1;
                u8 w   = (byte1 & 0b1);
                u8 mod = (byte2 & 0b11000000) >> 6;
                u8 reg = (byte2 & 0b00111000) >> 3;
                u8 rm  = (byte2 & 0b00000111);

                switch (mod)
                {
                case 0b00:
                    {
                        if (rm == 0b110)
                        {
                            u16 value = (u16)getc(istream);
                            value |= (u16)getc(istream) << 8;

                            sprintf(exp_buf, "[%hu]", value);
                        }
                        else
                        {
                            sprintf(exp_buf, "[%s]", expressions[rm]);
                        }
                    }
                    break;
                case 0b01:
                    {
                        // this is a sign extension
                        i16 value = (i16)((i8)getc(istream));

                        if (value >= 0)
                            sprintf(exp_buf, "[%s + %hd]", expressions[rm], value);
                        else
                            sprintf(exp_buf, "[%s - %hd]", expressions[rm], value * -1);
                    } break;
                case 0b10:
                    {
                        i16 value = (i16)(getc(istream));
                        value |= (i16)getc(istream) << 8;

                        if (value >= 0)
                            sprintf(exp_buf, "[%s + %hd]", expressions[rm], value);
                        else
                            sprintf(exp_buf, "[%s - %hd]", expressions[rm], value * -1);
                    } break;
                case 0b11:
                    {
                        u8 word = 0b1000 * w;

                        sprintf(exp_buf, "%s", registers[rm | word]);
                    } break;
                }

                if (d)
                    printf("mov %s, %s\n", registers[reg | (0b1000 * w)], exp_buf);
                else
                    printf("mov %s, %s\n", exp_buf, registers[reg | (0b1000 * w)]);
            } break;
        case 0b10110000: case 0b10110001: case 0b10110010: case 0b10110011:
        case 0b10110100: case 0b10110101: case 0b10110110: case 0b10110111:
        case 0b10111000: case 0b10111001: case 0b10111010: case 0b10111011:
        case 0b10111100: case 0b10111101: case 0b10111110: case 0b10111111:
            // NOTE(chitato): MOV Immediate to register
            {
                u8 w = (byte1 & 0b1000) >> 3;
                u16 value = (u16)getc(istream);

                if (w)
                {
                    value |= (u16)getc(istream) << 8;
                    printf("mov %s, word %hu\n", registers[byte1 & 0b1111], value);
                }
                else
                {
                    printf("mov %s, byte %hu\n", registers[byte1 & 0b1111], value);
                }
            } break;
        case 0b11000110: case 0b11000111:
            // NOTE(chitato): MOV Immediate to register/memory
            {
                u8 byte2 = (u8)getc(istream);

                u8 w = (byte1 & 1);
                u8 mod = (byte2 & 0b11000000) >> 6;
                u8 rm  = (byte2 & 0b00000111);

                char exp_buf[64];

                switch (mod)
                {
                case 0b00:
                    {
                        if (rm == 0b110)
                        {
                            u16 value = (u16)getc(istream);
                            value |= (u16)getc(istream) << 8;

                            sprintf(exp_buf, "[%hu]", value);
                        }
                        else
                        {
                            sprintf(exp_buf, "[%s]", expressions[rm]);
                        }
                    }
                    break;
                case 0b01:
                    {
                        i16 value = (i16)((i8)getc(istream));

                        if (value >= 0)
                        {
                            sprintf(exp_buf, "[%s + %hd]", expressions[rm], value);
                        }
                        else
                        {
                            sprintf(exp_buf, "[%s - %hd]", expressions[rm], value * -1);
                        }
                    } break;
                case 0b10:
                    {
                        i16 value = (i16)(getc(istream));
                        value |= (i16)getc(istream) << 8;

                        if (value >= 0)
                        {
                            sprintf(exp_buf, "[%s + %hd]", expressions[rm], value);
                        }
                        else
                        {
                            sprintf(exp_buf, "[%s - %hd]", expressions[rm], value * -1);
                        }
                    } break;
                case 0b11:
                    {
                        u8 word = 0b1000 * w;

                        sprintf(exp_buf, "%s", registers[rm | word]);
                    } break;
                }

                u16 data = (u16)getc(istream);
                if (w)
                {
                    data |= (u16)getc(istream) << 8;
                    printf("mov %s, word %hu\n", exp_buf, data);
                }
                else
                {
                    printf("mov %s, byte %hu\n", exp_buf, data);
                }
            } break;
        case 0b10100000: case 0b10100001:
            // NOTE(chitato): MOV Memory to accumulator
            {
                u8 w = byte1 & 1;

                u16 addr = (u16)getc(istream);
                addr |= (u16)getc(istream) << 8;

                printf("mov ax, [%hu]\n", addr);
            } break;
        case 0b10100010: case 0b10100011:
            // NOTE(chitato): MOV Accumulator to memory
            {
                u8 w = byte1 & 1;

                u16 addr = (u16)getc(istream);
                addr |= (u16)getc(istream) << 8;

                printf("mov [%hu], ax\n", addr);
            } break;
        default:
            printf("Instruction is not implemented!\n");
        }
    }
}
