/*
 * memory.c
 *
 *  Created on: 6 juil. 2020
 *      Author: Guillaume Fouilleul
 */

#include "mem.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MEM_CARTRIDGE_ROM_BANK_MAX      128 // 128 * 16 kiB = 2MiB
#define MEM_CARTRIDGE_RAM_BANK_MAX      16  // 16 * 8 kiB = 128kiB

#define MEM_SRAM_SIZE                   8192 // 8 kiB
#define MEM_VRAM_SIZE                   8192 // 8 kiB
#define MEM_OAM_RAM_SIZE                160  // 40 Spites
#define MEM_HRAM_SIZE                   128
#define MEM_IO_PORTS_SIZE               128

struct memory_map_t
{
    uint8_t *pBootReg; // BOOT register 0xFF50
    uint8_t *pBootROM; // BootROM

    // Array of pointers on cartridge ROM Banks
    uint8_t *aCartridgeROMBank[MEM_CARTRIDGE_ROM_BANK_MAX];

    // Mapped Banks
    uint8_t ROMIndex; // [0x4000 - 0x8000]
    uint8_t RAMIndex; // [0xA000 - 0xC000]

    // On board RAM
    uint8_t SRAM[MEM_SRAM_SIZE];
    uint8_t VRAM[MEM_VRAM_SIZE];
    uint8_t OAM_RAM[MEM_OAM_RAM_SIZE];
    uint8_t HRAM[MEM_HRAM_SIZE];
    uint8_t IOPorts[MEM_IO_PORTS_SIZE];

} mem;


// IO Ports map
bool aIOPortsMap[MEM_IO_PORTS_SIZE] =
{
  //   00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F
     true,  true,  true, false,  true,  true,  true,  true, false, false, false, false, false, false, false,  true, // 0xFF00
     true,  true,  true,  true,  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, // 0xFF10
     true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false, false, false, false, // 0xFF20
     true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, // 0xFF30
     true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, // 0xFF40
     true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 0xFF50
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 0xFF60
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 0xFF70
};


static void* mem_translation(uint16_t Addr)
{
    if ((Addr < 0x100) && ((*mem.pBootReg & 0x01) == 0))
    {
        return &mem.pBootROM[Addr];
    }

	if (Addr < 0x4000) // ROM Bank #0
		return &mem.aCartridgeROMBank[0][Addr];

	if (Addr < 0x8000) // Mapped ROM Bank
		return &mem.aCartridgeROMBank[mem.ROMIndex][Addr - 0x4000];

	if (Addr < 0xA000) // VRAM
		return &mem.VRAM[Addr - 0x8000];

	if (Addr < 0xC000) // Mapped RAM Bank
		return NULL; //&mem.pMappedRAMBank[Addr - 0xA000];

	if (Addr < 0xE000) // SRAM
		return &mem.SRAM[Addr - 0xC000];

	if (Addr < 0xFE00) // Echo of SRAM
		return &mem.SRAM[Addr - 0xE000];

	if (Addr < 0xFEA0) // OAM RAM
		return &mem.OAM_RAM[Addr - 0xFE00];

	if (Addr < 0xFF00) // Empty
		return NULL;

	if (Addr < 0xFF80) // IO Ports
	{
	    if (aIOPortsMap[Addr -  0xFF00] == true)
            return &mem.OAM_RAM[Addr - 0xFF00];
	    return NULL;
	}

	return &mem.HRAM[Addr - 0xFF80];
}

void mem_init()
{
    // Init BootROM location
    mem.pBootROM = (uint8_t *) 0x08100000;
    mem.pBootReg = mem_get_register(BOOT);

    // Init Cartridge ROM banks location
    memset(mem.aCartridgeROMBank, 0, MEM_CARTRIDGE_ROM_BANK_MAX * sizeof(uint8_t *));
    mem.aCartridgeROMBank[0] = (uint8_t *) 0x08110000;
    mem.aCartridgeROMBank[1] = (uint8_t *) 0x08118000;

    // Map memory
    mem.ROMIndex = 1;
    mem.RAMIndex = 0;
}

uint8_t mem_read_u8(uint16_t Addr)
{
    uint8_t *pU8 = (uint8_t *) mem_translation(Addr);
    if (NULL == pU8)
        return 0xFF;
    else
        return *pU8;
}

int8_t mem_read_s8(uint16_t Addr)
{
    int8_t *pS8 = (int8_t *) mem_translation(Addr);
    if (NULL == pS8)
        return 0xFF;
    else
        return *pS8;
}

uint16_t mem_read_u16(uint16_t Addr)
{
    uint16_t *pU16 = (uint16_t *) mem_translation(Addr);
    if (NULL == pU16)
        return 0xFFFF;
    else
        return *pU16;
}

void mem_write_u8(uint16_t Addr, uint8_t Value)
{
    uint8_t *pU8 = (uint8_t *) mem_translation(Addr);
    if (NULL != pU8)
        *pU8 = Value;
}

void mem_write_u16(uint16_t Addr, uint16_t Value)
{
    uint16_t *pU16 = (uint16_t *) mem_translation(Addr);
    if (NULL != pU16)
        *pU16 = Value;
}

uint8_t* mem_get_register(enum IOPorts_reg reg)
{
    switch (reg)
    {
        case JOYPAD:
            return &mem.IOPorts[0x00];
        case SERIAL:
            return &mem.IOPorts[0x01];
        case TIMER:
            return &mem.IOPorts[0x04];
        case SOUND:
            return &mem.IOPorts[0x10];
        case PPU:
            return &mem.IOPorts[0x40];
        case IF:
            return &mem.IOPorts[0x0F];
        case IE:
            return &mem.IOPorts[0xFF];
        case BOOT:
            return &mem.IOPorts[0x50];
        default:
            return NULL;
    }
}

uint8_t* mem_get_oam_ram(void)
{
    return &mem.OAM_RAM[0];
}

uint8_t* mem_get_vram(void)
{
    return &mem.VRAM[0];
}

void mem_set_bootrom(uint8_t *pBootROM)
{
    mem.pBootROM = pBootROM;
}

void mem_set_gamerom(uint8_t *pGameROM, uint8_t index)
{
    mem.aCartridgeROMBank[index] = pGameROM;
}