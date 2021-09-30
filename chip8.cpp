#include "chip8.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

void empty_log(const char* fmt, ...)
{

}

void Chip8::reset()
{
	clear_memory();
	clear_screen();
	clear_keys();

	// set the program counter to the entry point
	prog_count = CHIP8_WORK_MEM_START;
	index_reg = 0;

	// clear registers
	for (int i = 0; i < CHIP8_TOTAL_V_REGS; i++)
	{
		v_reg[i] = 0x00;
	}

	// clear stack
	stack_pointer = 0;
	for (int i = 0; i < CHIP8_STACK_SIZE; i++)
	{
		stack[i] = 0x00;
	}

	// load font set
	for (int i = 0; i < CHIP8_FONTSET_SIZE; i++)
	{
		memory[CHIP8_FONTSET_MEM_START + i] = chip8_fontset[i];
	}

	// copy the rom into memory, we keep the rom because
	// all of memory is work ram, so the 'rom' could be modified at runtime
	for (int i = CHIP8_WORK_MEM_START; i < CHIP8_TOTAL_MEMSIZE; i++)
	{
		memory[i] = romcopy[i];
	}
}

void Chip8::clear_memory()
{
	for (int i = CHIP8_WORK_MEM_START; i < CHIP8_TOTAL_MEMSIZE; i++)
	{
		memory[i] = 0x00;
	}
}

void Chip8::clear_screen()
{
	for (int i = 0; i < CHIP8_GRAPHICSMEM_TOTAL; i++)
	{
		screen_buf[i] = 0x00;
	}
}

void Chip8::clear_keys()
{
	for (int i = 0; i < CHIP8_INPUT_KEYS; i++)
	{
		keys[i] = 0x00;
	}
}

Chip8::Chip8()
{
	// so there's always a function here
	log = &empty_log;

	// set default config
	ambig_8XY6_8XYE_VY_mode = AMBIG_8XY6_DEFAULT;
	ambig_BNNN_mode = AMBIG_BNNN_ADD_VX;
	ambig_FX55_FX65_mode = AMBIG_FX55_FX65_DEFAULT;
}

Chip8::~Chip8()
{
}

void Chip8::Init()
{
}

void Chip8::Tick(long long sys_ticks)
{
	system_ticks = sys_ticks;
	// Fetch opcode
	fetch_opcode();

	// Execute opcode
	execute_opcode();

	// Update timers (should tick at 60hz)
	if (delay_timer > 0) delay_timer--;
	if (sound_timer > 0) sound_timer--;
	if (sound_timer > 0)
	{
		// beep!
	}

	user_keypressed = false;
}

uint8_t * Chip8::GetScreenBuf()
{
	return screen_buf;
}

void Chip8::Reset()
{
	reset();
}

bool Chip8::LoadROM(uint8_t * rom, long size)
{
	int charp = 0;

	if (rom != nullptr && size > 0 && size <= CHIP8_TOTAL_MEMSIZE - 0x200)
	{
		for (int i = 0; i < size; i++)
		{
			romcopy[i + 0x200] = rom[i];
			printf("%X ", rom[i]);
			charp++;
			if (charp == 16)
			{
				printf("\n");
				charp = 0;
			}
		}
	}
	else
	{
		printf("Chip8 ROM was null or size [%i] was 0 or bigger than available space.", size);
		return false;
	}

	return true;
}

bool Chip8::LoadROMFromFile(const char * filename)
{
	std::streampos begin;
	std::streampos end;
	unsigned char buffer [1024*16];//wayyy big
	for (int i = 0; i < 1024 * 16; i++) { buffer[i] = 0; }
	bool success = true;

	FILE * fp = fopen(filename, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long filesize = ftell(fp);
		rewind(fp);
		fseek(fp, 0, SEEK_SET);
		int r = fread(buffer, sizeof(unsigned char), filesize, fp);
		fclose(fp);
		
		success = LoadROM((uint8_t*)buffer, filesize);
	}
	else
	{
		printf("Chip8 could not load file: %s", filename);
		success = false;
	}

	reset();

	return success;
}

void Chip8::SetLogFunc(chip8_log_func func)
{
	log = func;
}

void Chip8::SetKey(uint8_t key, bool pressed)
{
	if (key >= 0 && key < 16)
	{
		keys[key] = pressed;

		if (pressed)
		{
			user_keypressed = true;
			last_keypressed = key;
		}
	}
}

long Chip8::GetMemorySize()
{
	return CHIP8_TOTAL_MEMSIZE;
}

uint8_t * Chip8::GetMemory()
{
	return memory;
}

uint8_t * Chip8::GetVRegs()
{
	return v_reg;
}

uint8_t * Chip8::GetKeys()
{
	return keys;
}

uint8_t Chip8::GetSoundTimer()
{
	return sound_timer;
}

uint8_t Chip8::GetDelayTimer()
{
	return delay_timer;
}

short Chip8::GetIndexRef()
{
	return index_reg;
}

short Chip8::GetProgCount()
{
	return prog_count;
}

bool Chip8::GetConfig_8XY6_8XYE_VY_mode()
{
	return ambig_8XY6_8XYE_VY_mode;
}

void Chip8::SetConfig_8XY6_8XYE_VY_mode(bool mode)
{
	ambig_8XY6_8XYE_VY_mode = mode;
}

bool Chip8::GetConfig_BNNN_ADD_mode()
{
	return ambig_BNNN_mode;
}

void Chip8::SetConfig_BNNN_ADD_mode(bool mode)
{
	ambig_BNNN_mode = mode;
}

bool Chip8::GetConfig_FX55_FX65_VY_mode()
{
	return ambig_FX55_FX65_mode;
}

void Chip8::SetConfig_FX55_FX65_VY_mode(bool mode)
{
	ambig_FX55_FX65_mode = mode;
}

void Chip8::fetch_opcode()
{
	last_opcode = opcode;

	uint8_t byte1 = memory[prog_count];
	uint8_t byte2 = memory[prog_count + 1];

	// merge the two opcodes by shifting the first
	//  byte left by 8 so it occupies the first byte
	//  of the unsigned short
	opcode = byte1 << 8 | byte2;
}

void Chip8::execute_opcode()
{
	uint8_t n0 = get_nibble_0(opcode);
	uint8_t n1 = get_nibble_1(opcode);
	uint8_t n2 = get_nibble_2(opcode);
	uint8_t n3 = get_nibble_3(opcode);

	short NNN = get_nibbles123(opcode);
	uint8_t NN = get_nibbles23(opcode);
	uint8_t N = n3;
	uint8_t X = n1;
	uint8_t Y = n2;

	int temp1 = 0;
	int temp2 = 0;
	int temp3 = 0;

	log("Executing: 0x%X\n", opcode);
	log("Split: 0x%X 0x%X 0x%X 0x%X\n", n0, n1, n2, n3);
	log("NNN: 0x%X\n", (short)NNN);
	log("NN : 0x%X\n", NN);
	log("N  : 0x%X\n", N);
	log("X  : 0x%X\n", X);
	log("Y  : 0x%X\n", Y);
	log("\n", Y);
	
	if (opcode == 0x0)
	{
		log("Unknown or unimplemented opcode 0x%X\n", opcode);
		log("Previous opcode was 0x%X\n", last_opcode);
	}

	// check the left-most 2 bytes for the opcode in most cases
	switch (n0) // check the first nibble only
	{
		//case: 0x0NNN: not needed for emulation

	case 0x0: // 0x00..

		switch (n3)
		{
			case 0x0: // 0x00E0 - Clears the screen
				clear_screen();
				break;
			case 0xE: // 0x00EE - returns from a subroutine
				// pop the stack pointer? and move prog_counter back to it
				stack_pointer--;
				prog_count = stack[stack_pointer];
				break;
			default:
				break;
				log("Unknown or unimplemented opcode 0x%X\n", opcode);
		}

		break;

	case 0x1: // 0x1NNN - GOTO NNN
		prog_count = NNN; //clear off the top nibble
		prog_count -= 2;
		break;

	case 0x2: // 0x2NNN - Calls subroutine at NNN
		stack[stack_pointer] = prog_count;
		stack_pointer++;
		prog_count = NNN;
		prog_count -= 2;

		break;

	case 0x3: // 0x3XNN - Skip next instruction if VX equals NN
		if (v_reg[X] == NN)
		{
			prog_count += 2;
		}
		break;

	case 0x4: // 0x4XNN - Skip next instruction is VX not equal to NN
		if (v_reg[X] != NN)
		{
			prog_count += 2;
		}
		break;

	case 0x5: // 0x4XY0 - Skip next instruction if VX equal to VY
		if (v_reg[X] == v_reg[Y])
		{
			prog_count += 2;
		}
		break;

	case 0x6: // 0x6XNN - Set VX to NN
		v_reg[X] = NN;
		break;

	case 0x7: // 0x7XNN - Adds NN to VX (does not set carry flag!)
		v_reg[X] += NN;
		break;

	case 0x8: // 0x8... math operators I think

		switch (n3) // check the right-most nibble
		{
			case 0x0: // 0x8XY0 - Sets VX to value of VY
				v_reg[X] = v_reg[Y];
				break;
			case 0x1: // 0x8XY1 - Sets VX to VX bitwise OR'ed with VY
				v_reg[X] = v_reg[X] | v_reg[Y];
				break;
			case 0x2: // 0x8XY2 - Sets VX to VX bitwise ANDed with VY
				v_reg[X] = v_reg[X] & v_reg[Y];
				break;
			case 0x3: // 0x8XY3 - Sets VX to VX XORed with VY
				v_reg[X] = v_reg[X] ^ v_reg[Y];
				break;
			case 0x4: // 0x8XY4 - Adds VY to VX, VF set to 1 when there's a carry
				temp1 = v_reg[X];
				v_reg[X] += v_reg[Y];
				v_reg[15] = (v_reg[X] < temp1) ? 1 : 0;
				break;
			case 0x5: // 0x8XY5 - VY is subtracted from VX, VF is set 0 when borrow, 1 when not
				v_reg[15] = (v_reg[X] > v_reg[Y]) ? 1 : 0;
				v_reg[X] -= v_reg[Y];

				break;
			case 0x6: // 0x8XY6 - Stores least significant bit of VX in VF then shifts VX right by 1
				// temp1 == lsb
				if (ambig_8XY6_8XYE_VY_mode == AMBIG_8XY6_SHIFTMODE_SET_VX_TO_VY)
				{
					v_reg[X] = v_reg[Y];
				}

				temp1 = v_reg[X] & 0x00000001;
				v_reg[15] = temp1;
				v_reg[X] = v_reg[X] >> 1;
			
				break;
			case 0x7: // 0x8XY7 - Sets VX to VY minus VX, VF is set 0 when there's a borrow, 1 when not
				v_reg[15] = (v_reg[Y] < v_reg[X]) ? 0 : 1;
				v_reg[X] = v_reg[Y] - v_reg[X];
				break;
			case 0xE: // 0x8XYE - Stores the most significant bit of VX in VF and then shifts VX left by 1
				// temp1 == msb
				if (ambig_8XY6_8XYE_VY_mode == AMBIG_8XY6_SHIFTMODE_SET_VX_TO_VY)
				{
					v_reg[X] = v_reg[Y];
				}

				temp1 = (v_reg[X] >> 7);
				v_reg[15] = temp1;
				v_reg[X] = v_reg[X] << 1; 
				break;
			default:
				log("Unknown or unimplemented opcode 0x%X\n", opcode);
				break;
		}

		break;

	case 0x9: // 0x9XY0 - Skips the next instruction if VX does not equal VY
		if (v_reg[X] != v_reg[Y])
		{
			prog_count += 2;
		}
		break;

	case 0xA: // 0xANNN - Sets index_counter to the address NNN
		index_reg = NNN;
		break;

	case 0xB: // 0xBNNN - Jumps to address NNN + V0

		if (ambig_BNNN_mode == AMBIG_BNNN_ADD_V0)
		{
			prog_count = NNN + v_reg[0];
		}
		else
		{
			prog_count = NNN + v_reg[X];
		}

		prog_count -= 2;
		break;

	case 0xC: // 0xCXNN - Sets VX to result of bitwise AND op on a random number (0-255) and NN (rand() & NN)
		// temp1 == rand
		temp1 = system_ticks % 256; // not rand at all really, but hey
		v_reg[X] = temp1 & NN;
		break;

	case 0xD: // 0xDXYN - Draw a sprite at coordinate (VX, VY), width 8 pixels, height N pixels
				// read as bit-coded starting from memory location index_counter, index_counter does not change
				// VF is set to 1 if any screen pixels are flipped from SET to UNSET when draw, 0 if not
	{
		uint8_t sprite = 0;
		uint8_t bit1 = 0;
		uint8_t oldBit = 0;

		v_reg[15] = 0;
		for (short py = 0; py < N; py++)
		{
			sprite = memory[index_reg + py];
			for (short px = 0; px < 8; px++)
			{
				bit1 = sprite & (0x80 >> px);
				int gfx_idx = ((short)v_reg[X] + px) + ((short)(v_reg[Y] + py) * 64);
				oldBit = (screen_buf[gfx_idx]);
				
				if (bit1 && (screen_buf[gfx_idx]))
				{
					// if it was unset, set VF
					v_reg[15] = 1;
				}

				screen_buf[gfx_idx] ^= bit1;
			}
		}

		break;
	}
	case 0xE: 
		switch (n3)
		{
			case 0xE: // 0xEX9E - Skips next instruction if the key stored in VX is pressed
				// temp 1 == key num
				temp1 = v_reg[X];
				if (temp1 <= 15 && keys[temp1])
				{
					prog_count += 2;
				}
				break;
			case 0x1: // 0xEXA1 - Skips next instruction if the key stored in VX is not pressed
				temp1 = v_reg[X];
				if (temp1 <= 15 && !keys[temp1])
				{
					prog_count += 2;
				}
				break;
			default:
				log("Unknown or unimplemented opcode 0x%X\n", opcode);
				break;
		}

		break;

	case 0xF: // all 0xFX..
		switch ((NN))
		{
			case 0x07: // 0xFX07 - Sets VX to the value of the delay timer
				v_reg[X] = delay_timer;
				break;
			case 0x0A: // 0xFX0A - A key press is awaited and then stored in VX (Blocking operation!)
				if (user_keypressed)
				{
					v_reg[X] = last_keypressed;
				}
				else
				{
					prog_count -= 2;
				}
				break;
			case 0x15: // 0xFX15 - Sets delay_timer to VX
				delay_timer = v_reg[X];
				break;
			case 0x18: // 0xFX18 - Sets sound timer to VX
				sound_timer = v_reg[X];
				break;
			case 0x1E: // 0xFX1E - Adds VX to index_counter, VF is not affected
				index_reg += v_reg[X];
				break;
			case 0x29: // 0xFX29 - Sets index_counter to location of the sprite for the character in VX
							// sprite means font sprite for the CHAR 0-F
				// temp1 == character num
				temp1 = v_reg[X];
				if (temp1 <= 15)
				{
					index_reg = CHIP8_FONTSET_MEM_START + (16 * temp1);
				}
				
				break;
			case 0x33: // 0xFX33 - Stores the binary-coded decimel representation of VX, with the most significant
								// three digits at the address in index_counter, the middle digit at index+1, and the least
								// significant digit at index+2
								// or: take the decimal representation of VX, place the hundreds digit in memory 
								// at location in I, the tens digit at location I+1, and the ones digit at location I+2
				temp1 = v_reg[X];
				temp1 = v_reg[X] / 100;
				temp2 = (v_reg[X] - (temp1*100)) / 10;
				temp3 = (v_reg[X]) % 10;
				memory[index_reg] = temp1;
				memory[index_reg+1] = temp2;
				memory[index_reg+2] = temp3;
				break;
			case 0x55: // 0xFX55 - Stores V0 to VX (including VX) in memory starting at address index_counter.
						// the offset from index_counter is increased by 1 for each value written. 
						// But index_count itself is not modified
				temp1 = index_reg;//temp1 == address
				if (X > 15) X = 15;
				for (int i = 0; i <= X; i++)
				{
					memory[temp1+i] = v_reg[i];
				}

				if (ambig_FX55_FX65_mode == AMBIG_FX55_FX65_INC_I)
				{
					index_reg += X;
				}

				break;
			case 0x65: // 0xFX65 - Fills V0 to VX (including) with values from memory starting at address index_count
							// offset is increase per value but index_count is left unmodified same as 0XFX55
				temp1 = index_reg;
				if (X > 15) X = 15;
				for (int i = 0; i <= X; i++)
				{
					v_reg[i] = memory[temp1+i];
				}

				if (ambig_FX55_FX65_mode == AMBIG_FX55_FX65_INC_I)
				{
					index_reg += X;
				}

				break;
			default:
				log("Unknown or unimplemented opcode 0x%X\n", opcode);
				break;
		}
		break;

	default:
		log("Unknown or unimplemented opcode 0x%X\n", opcode);
		break;
	}

	// every instruction is 2 bytes
	prog_count += 2;
}
