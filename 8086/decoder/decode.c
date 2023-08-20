#include <stdio.h>
#include <inttypes.h>

#define sign_extend(x) ((i16)((i8)(x)))

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

char const *op_ext[] =
{
    [0b000] = "add",
    [0b101] = "sub",
    [0b111] = "cmp",
};

char const *conditional_jumps[] =
{
    [0b0000] = "jo",
    [0b0001] = "jno",
    [0b0010] = "jb",
    [0b0011] = "jnb",
    [0b0100] = "je",
    [0b0101] = "jne",
    [0b0110] = "jbe",
    [0b0111] = "jnbe",
    [0b1000] = "js",
    [0b1001] = "jns",
    [0b1010] = "jp",
    [0b1011] = "jnp",
    [0b1100] = "jl",
    [0b1101] = "jnl",
    [0b1110] = "jle",
    [0b1111] = "jnle",
};

void read_exp(char *exp_buf, u8 modregrm, u8 width_mask, FILE *istream)
{
    u8 mod = modregrm & (0b11 << 6);
    u8 rm  = modregrm & 0b111;

    switch (mod)
    {
        case 0b00 << 6:
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
        case 0b01 << 6:
        {
            i16 value = sign_extend(getc(istream));

            if (value >= 0)
                sprintf(exp_buf, "[%s + %hd]", expressions[rm], value);
            else
                sprintf(exp_buf, "[%s - %hd]", expressions[rm], (i16)(value * -1));
        } break;
        case 0b10 << 6:
        {
            i16 value = (i16)getc(istream);
            value |= (i16)getc(istream) << 8;

            if (value >= 0)
                sprintf(exp_buf, "[%s + %hd]", expressions[rm], value);
            else
                sprintf(exp_buf, "[%s - %hd]", expressions[rm], (i16)(value * -1));
        } break;
        case 0b11 << 6:
        {
            sprintf(exp_buf, "%s", registers[rm | width_mask]);
        }
    }
}

void read_data_sw(char *data_buf, u8 sw, FILE *istream)
{
    i16 data = (i16)getc(istream);

    switch (sw)
    {
        case 0b01:
        {
            data |= (i16)getc(istream) << 8;
            sprintf(data_buf, "word %hd", data);
        } break;
        case 0b11:
        {
            data = sign_extend(data);
            sprintf(data_buf, "word %hd", data);
        } break;
        case 0b00: case 0b10:
        sprintf(data_buf, "byte %hd", data);
    }
}

void read_data_w(char *data_buf, u8 w, FILE *istream)
{
    i16 data = (i16)getc(istream);
    if (w)
    {
        data |= (i16)getc(istream) << 8;
        sprintf(data_buf, "word %hd", data);
    }
    else
    {
        sprintf(data_buf, "byte %hd", sign_extend(data));
    }
}

int main(int argc, char **argv)
{
    FILE *istream;

    if (argc == 2)
    {
        istream = fopen(argv[1], "rb");
        if (istream == NULL)
        {
            fprintf(stderr, "Error opening file `%s`\n", argv[0]);
            return 1;
        }
    }
    else
    {
        istream = stdin;
    }

    char exp_buf[64];
    char data_buf[64];

    puts("bits 16\n");
    for (;;)
    {
        int byte = getc(istream);
        if (byte == EOF) return 0;

        u8 opcode = (u8)byte;

        switch (opcode)
        {
            case 0b10001000 ... 0b10001011:
            // NOTE(chitato): MOV Register/memory to/from register
            {
                u8 modregrm = getc(istream);

                u8 d   = opcode & 0b10;
                u8 w   = opcode & 0b1;
                u8 reg = (modregrm & 0b00111000) >> 3;

                u8 width_mask = w << 3;
                read_exp(exp_buf, modregrm, width_mask, istream);

                if (d)
                    printf("mov %s, %s\n", registers[reg | width_mask], exp_buf);
                else
                    printf("mov %s, %s\n", exp_buf, registers[reg | width_mask]);
            } break;
            case 0b10110000 ... 0b10111111:
            // NOTE(chitato): MOV Immediate to register
            {
                u8 w = (opcode & 0b1000);
                read_data_w(data_buf, w, istream);

                printf("mov %s, %s\n", registers[opcode & 0b1111], data_buf);
            } break;
            case 0b11000110: case 0b11000111:
            // NOTE(chitato): MOV Immediate to register/memory
            {
                u8 modregrm = (u8)getc(istream);

                u8 w = (opcode & 1);

                read_exp(exp_buf, modregrm, w << 3, istream);
                read_data_w(data_buf, w, istream);

                printf("mov %s, word %s\n", exp_buf, data_buf);
            } break;
            case 0b10100000: case 0b10100001:
            // NOTE(chitato): MOV Memory to accumulator
            {
                u16 addr = (u16)getc(istream);
                addr |= (u16)getc(istream) << 8;

                printf("mov ax, [%hu]\n", addr);
            } break;
            case 0b10100010: case 0b10100011:
            // NOTE(chitato): MOV Accumulator to memory
            {
                u16 addr = (u16)getc(istream);
                addr |= (u16)getc(istream) << 8;

                printf("mov [%hu], ax\n", addr);
            } break;
            case 0b00000000 ... 0b00000011:
            // NOTE(chitato): ADD Reg/memory with register to either
            {
                u8 modregrm = (u8)getc(istream);

                u8 d = opcode & 0b10;
                u8 w = opcode & 0b1;
                u8 reg = (modregrm & 0b00111000) >> 3;

                u8 width_mask = w << 3;
                read_exp(exp_buf, modregrm, width_mask, istream);

                if (d)
                    printf("add %s, %s\n", registers[reg | width_mask], exp_buf);
                else
                    printf("add %s, %s\n", exp_buf, registers[reg | width_mask]);
            } break;
            case 0b00000100: case 0b00000101:
            // NOTE(chitato): ADD Immediate to accumulator
            {
                u8 width_mask = (opcode & 1) << 3;
                read_data_w(data_buf, width_mask, istream);
                printf("add %s, %s\n", registers[width_mask], data_buf);
            } break;
            case 0b00101000 ... 0b00101011:
            // NOTE(chitato): SUB Reg/memory and register to either
            {
                u8 modregrm = (u8)getc(istream);

                u8 d = opcode & 0b10;
                u8 w = opcode & 0b1;
                u8 reg = (modregrm & 0b00111000) >> 3;

                u8 width_mask = w << 3;
                read_exp(exp_buf, modregrm, width_mask, istream);

                if (d)
                    printf("sub %s, %s\n", registers[reg | width_mask], exp_buf);
                else
                    printf("sub %s, %s\n", exp_buf, registers[reg | width_mask]);
            } break;
            case 0b00101100: case 0b00101101:
            // NOTE(chitato): SUB Immediate from accumulator
            {
                u8 width_mask = (opcode & 1) << 3;
                read_data_w(data_buf, width_mask, istream);
                printf("sub %s, %s\n", registers[width_mask], data_buf);
            } break;
            case 0b00111000 ... 0b00111011:
            // NOTE(chitato): CMP Reg/memory and register
            {
                u8 modregrm = (u8)getc(istream);

                u8 d = opcode & 0b10;
                u8 w = opcode & 0b1;
                u8 reg = (modregrm & 0b00111000) >> 3;

                u8 width_mask = w << 3;
                read_exp(exp_buf, modregrm, width_mask, istream);

                if (d)
                    printf("cmp %s, %s\n", registers[reg | width_mask], exp_buf);
                else
                    printf("cmp %s, %s\n", exp_buf, registers[reg | width_mask]);
            } break;
            case 0b00111100: case 0b00111101:
            // NOTE(chitato): CMP Immediate with accumulator
            {
                u8 width_mask = (opcode & 1) << 3;
                read_data_w(data_buf, width_mask, istream);
                printf("cmp %s, %s\n", registers[width_mask], data_buf);
            } break;
            case 0b10000000 ... 0b10000011:
            {
                u8 modoprm = (u8)getc(istream);
                u8 op = (modoprm & 0b00111000) >> 3;

                u8 width_mask = (opcode & 1) << 3;

                read_exp(exp_buf, modoprm, width_mask, istream);
                read_data_sw(data_buf, opcode & 0b11, istream);

                printf("%s %s, %s\n", op_ext[op], exp_buf, data_buf);
            } break;
            case 0b01110000 ... 0b01111111:
            {
                u8 index = opcode & 0b1111;
                i8 data = (i8)getc(istream);

                printf("%s $%+d\n", conditional_jumps[index], (int)data+2); // weird nasm stuff
            } break;
            case 0b11100000 ... 0b11100011:
            {
                char const *loops[4] = {
                    [0b00] = "loopnz",
                    [0b01] = "loopz",
                    [0b10] = "loop",
                    [0b11] = "jcxz",
                };

                u8 index = opcode & 0b11;
                i8 data = (i8)getc(istream);

                printf("%s $%+d\n", loops[index], (int)data+2); // weird nasm stuff
            } break;
            default:
            printf("Instruction `0x%x` is not implemented!\n", (unsigned int)opcode);
            return 1;
        }
    }

    return 0;
}
