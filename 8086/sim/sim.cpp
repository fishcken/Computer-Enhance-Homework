#include <stdlib.h>
#include <stdio.h>
#include "sim86_shared.h"

typedef enum
{
    Register_NULL,
	Register_A,
    Register_B,
    Register_C,
    Register_D,
	Register_SP,
	Register_BP,
	Register_SI,
	Register_DI,
    Register_ES,
    Register_CS,
    Register_SS,
    Register_DS,
} Register;

typedef struct
{
    u16 registers[8 + 4 + 1];
} Sim8086;

static u8 buffer[1024 * 1024];

char *register_name(register_access x)
{
    if (x.Count == 2)
    {
        switch (x.Index)
        {
            case Register_A:
            return "ax";
            case Register_B:
            return "bx";
            case Register_C:
            return "cx";
            case Register_D:
            return "dx";
            case Register_SP:
            return "sp";
            case Register_BP:
            return "bp";
            case Register_SI:
            return "si";
            case Register_DI:
            return "di";
            case Register_ES:
            return "es";
            case Register_CS:
            return "cs";
            case Register_SS:
            return "ss";
            case Register_DS:
            return "ds";
            default:
            return "<invalid>";
        }
    }
    else
    {
        if (x.Offset == 0)
        {
            switch (x.Index)
            {
                case Register_A:
                return "al";
                case Register_B:
                return "bl";
                case Register_C:
                return "cl";
                case Register_D:
	            return "dl";
                default:
                return "<invalid>";
            }
        }
        else
        {
            switch (x.Index)
            {
                case Register_A:
                return "ah";
                case Register_B:
                return "bh";
                case Register_C:
                return "ch";
                case Register_D:
	            return "dh";
                default:
                return "<invalid>";
            }
        }
    }
}

void dump_registers(Sim8086 *sim)
{
    puts("8086 register dump:");
    fprintf(stderr,
            "\tax = 0x%x\n" "\tbx = 0x%x\n"
            "\tcx = 0x%x\n" "\tdx = 0x%x\n"
            "\tsp = 0x%x\n" "\tbp = 0x%x\n"
            "\tsi = 0x%x\n" "\tdi = 0x%x\n"
            "\tes = 0x%x\n" "\tss = 0x%x\n"
            "\tds = 0x%x\n",
            sim->registers[Register_A], sim->registers[Register_B],
            sim->registers[Register_C], sim->registers[Register_D],
            sim->registers[Register_SP], sim->registers[Register_BP],
            sim->registers[Register_SI], sim->registers[Register_DI],
            sim->registers[Register_ES], sim->registers[Register_SS],
            sim->registers[Register_DS]);

}

u16 operand_getval(Sim8086 const *sim, instruction_operand operand)
{
    u16 result = 0;

    switch (operand.Type)
    {
        case Operand_Register:
        {
            register_access reg = operand.Register;
            result = (u16)(sim->registers[reg.Index] >> (reg.Offset * 8)) & (u16)~(0xffff << (reg.Count * 8));
        } break;
        case Operand_Immediate:
        {
            immediate imm = operand.Immediate;
            result = imm.Value;
        } break;
    }

    return result;
}

void run_program(Sim8086 *sim, u8 *program, size_t len)
{
    size_t offset = 0;
    while (offset < len)
    {
        instruction ins;
        u16 src_val = 0;
        u8 *dst = NULL;

        Sim86_Decode8086Instruction((u32)(len - offset), buffer + offset, &ins);

        if (! ins.Op)
            break;

        offset += ins.Size;

        src_val = operand_getval(sim, ins.Operands[1]);

        switch (ins.Operands[0].Type)
        {
            case Operand_Register:
            {
                register_access reg = ins.Operands[0].Register;

                dst = (u8 *)&sim->registers[reg.Index] + reg.Offset;
            } break;
        }

        switch (ins.Op)
        {
            case Op_mov:
            {
                if (ins.Flags & Inst_Wide)
                    *((u16 *)dst) = src_val;
                else
                    *((u8 *)dst) = (u8)src_val;
            }
        }
    }
}

int main(int argc, char **argv)
{
    FILE *istream = NULL;
    Sim8086 sim = {0};

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

    size_t len = fread(buffer, sizeof(u8), sizeof(buffer), istream);
    if (len == 0)
    {
        fprintf(stderr, "Error reading input");
    }

    run_program(&sim, buffer, len);
    dump_registers(&sim);

    return 0;
}