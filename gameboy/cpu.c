/*
 * cpu.c
 *
 *  Created on: 6 juil. 2020
 *      Author: Guillaume Fouilleul
 */

#include "cpu.h"
#include "irq.h"
#include "mem.h"
#include "opcode.h"
#include "opcode_cb.h"

#include <stdio.h>

#include <SDL2/SDL.h>

// Exported to be use directly
struct cpu_t cpu;

static void cpu_print_state(uint8_t Opcode)
{
    if (true == cpu.prefix_cb)
        printf("%04X: %s\n", cpu.reg.PC, opcodeCbList[Opcode].pFuncName);
    else
        printf("%04X: %s\n", cpu.reg.PC, opcodeList[Opcode].pFuncName);
    printf("\tA = 0x%02X\tF = 0x%02X\tAF = 0x%04X\n", cpu.reg.A, cpu.reg.F, cpu.reg.AF);
    printf("\tB = 0x%02X\tC = 0x%02X\tBC = 0x%04X\n", cpu.reg.B, cpu.reg.C, cpu.reg.BC);
    printf("\tD = 0x%02X\tE = 0x%02X\tDE = 0x%04X\n", cpu.reg.D, cpu.reg.E, cpu.reg.DE);
    printf("\tH = 0x%02X\tL = 0x%02X\tHL = 0x%04X\n", cpu.reg.H, cpu.reg.L, cpu.reg.HL);
    printf("\tSP = 0x%04X\n", cpu.reg.SP);
    printf("\t");
    if (cpu.reg.Flags.Z)
        printf("Z ");
    else
        printf("- ");
    if (cpu.reg.Flags.N)
        printf("N ");
    else
        printf("- ");
    if (cpu.reg.Flags.H)
        printf("H ");
    else
        printf("- ");
    if (cpu.reg.Flags.C)
        printf("C\n");
    else
        printf("-\n");
}

void cpu_init(void)
{
    // Init ALU
    cpu.reg.AF = 0;
    cpu.reg.BC = 0;
    cpu.reg.DE = 0;
    cpu.reg.HL = 0;
    cpu.reg.SP = 0;
    cpu.reg.PC = 0;

    // Init Flags
    cpu.halted = false;
    cpu.cycle_counter = 1;
}

void cpu_exec(void)
{
    bool update_pc = false;

    cpu.cycle_counter--;

    if (0 == cpu.cycle_counter)
    {
        // Check for interrupt
        if (false == irq_check())
        {
            // Read opcode
            uint8_t opcode = mem_read_u8(cpu.reg.PC);

            // Execute opcode
            if (cpu.prefix_cb)
            {
#ifdef DEBUG
                cpu_print_state(opcode);
#endif
                cpu.prefix_cb = false;
                cpu.cycle_counter = opcodeCbList[opcode].func();
                update_pc = opcodeCbList[opcode].update_pc;
            }
            else
            {
#ifdef DEBUG
                cpu_print_state(opcode);
#endif
                cpu.cycle_counter = opcodeList[opcode].func();
                update_pc = opcodeList[opcode].update_pc;
            }

            // Update Program Counter
            if (update_pc)
                cpu.reg.PC += opcodeList[opcode].length;
        }
    }
}