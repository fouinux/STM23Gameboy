/*
 * core.h
 *
 *  Created on: 6 juil. 2020
 *      Author: Guillaume Fouilleul
 */

#ifndef INC_GAMEBOY_CPU_CORE_H_
#define INC_GAMEBOY_CPU_CORE_H_

#include <stdint.h>

struct core_reg_t
{
	union
	{
		struct
		{
			uint8_t A; // Accumulator
			union
			{
				uint8_t F; // Flags
				struct
				{
					uint8_t Z: 1; // Zero Flag
					uint8_t N: 1; // Subtract Flag
					uint8_t H: 1; // Half Carry Flag
					uint8_t C: 1; // Carry Flag
					uint8_t  : 4;
				} Flags;
			};
		};
		uint16_t AF;
	};

	union
	{
		struct
		{
			uint8_t B;
			uint8_t C;
		};
		uint16_t BC;
	};

	union
	{
		struct
		{
			uint8_t D;
			uint8_t E;
		};
		uint16_t DE;
	};

	union
	{
		struct
		{
			uint8_t H;
			uint8_t L;
		};
		uint16_t HL;
	};
	uint16_t SP; // Stack Pointer
	uint16_t PC; // Program Counter
};

extern struct core_reg_t core_reg;

#endif /* INC_GAMEBOY_CPU_CORE_H_ */