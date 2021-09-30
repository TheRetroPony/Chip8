#include <stdint.h>

#define CHIP8_GRAPHICS_WIDTH 64
#define CHIP8_GRAPHICS_HEIGHT 32
#define CHIP8_GRAPHICSMEM_TOTAL (CHIP8_GRAPHICS_WIDTH * CHIP8_GRAPHICS_HEIGHT)

#define CHIP8_TOTAL_MEMSIZE 4096

#define CHIP8_FONTSET_MEM_START 0x050 
#define CHIP8_WORK_MEM_START 0x200

#define CHIP8_TOTAL_V_REGS 16
#define CHIP8_V_REG_CARRYFLAG 15

#define CHIP8_INPUT_KEYS 16
#define CHIP8_STACK_SIZE 16

#define CHIP8_FONTSET_SIZE 80

#define check_bit(pos, var) ((var)&(1 << pos))

#define get_nibble_0(var) ((var >> 12))
#define get_nibble_1(var) ((var >> 8) & ((1 << 4)-1))
#define get_nibble_2(var) ((var >> 4) & ((1 << 4)-1))
#define get_nibble_3(var) ((var) & ((1 << 4)-1))
#define get_nibbles123(var) ((var) & ((1 << 12)-1))
#define get_nibbles23(var) ((var) & ((1 << 8)-1))

typedef void(*chip8_log_func)(const char*, ...);

class Chip8
{
private:

	// fontset
	const uint8_t chip8_fontset[CHIP8_FONTSET_SIZE] =
	{
	  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	  0x20, 0x60, 0x20, 0x20, 0x70, // 1
	  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// opcodes are 2-bytes, short is 2 bytes
	unsigned short last_opcode;
	unsigned short opcode;

	// 4k bytes of memory
	uint8_t memory[CHIP8_TOTAL_MEMSIZE];
	uint8_t romcopy[CHIP8_TOTAL_MEMSIZE];

	// 15 8-bit general purpose CPU registers
	// V0 - VE = registers
	//		VF = carry flag
	uint8_t v_reg[CHIP8_TOTAL_V_REGS];

	// index register I / also called address reg, stores memory addresses for some opcodes, etc
	// 12 bits wide!
	unsigned short index_reg;

	// program counter: values from 0x000 to 0xFFF
	// we'll use a short and just discard the last 4 bits
	unsigned short prog_count;

	/*
		Memory Map:
		0x000 - 0x1FF : Chip 8 interpreter (contains font set when emulating)
		0x050 - 0x0A0 : Used for the built-in 4x5 pixel font set (0-F)
		0x200 - 0xFFF : Program ROM and work RAM (in one chunk!)
	*/

	// interupt registers
	uint8_t delay_timer;
	uint8_t sound_timer; // buzzer sounds when non-zero

	// Stack : Chip 8 instruction set does not mention a stack, but jump
	//  instructions are possible! So we need to implement one in interpreter
	// original RCA 1802 version alloted 12 levels, we're gonna have 16
	// would have been stored in the top end of work ram
	unsigned short stack[CHIP8_STACK_SIZE];
	unsigned short stack_pointer;

	// Keycode registers
	// Chip 8 has a HEX based keypad 0x0 - 0xF
	uint8_t keys[CHIP8_INPUT_KEYS];

	// Graphics memory
	// Chip 8 has one [1] sprite draw instruction that is done in XOR mode
	//  if a pixel is turned OFF as a result of drawing, the VF register is set

	// Chip 8 has 2k pixels that are black or white only.
	// a whole uint8_t is kinda overkill here but, who cares, easier
	uint8_t screen_buf[CHIP8_GRAPHICSMEM_TOTAL];

	//ambiguous function toggles
#define AMBIG_8XY6_SHIFTMODE_SET_VX_TO_VY 0
#define AMBIG_8XY6_SHIFTMODE_DONOTMODIFY_VX 1
#define AMBIG_8XY6_DEFAULT 0
	bool ambig_8XY6_8XYE_VY_mode;

#define AMBIG_BNNN_ADD_V0 0
#define AMBIG_BNNN_ADD_VX 1
#define AMBIG_BNNN_DEFAULT 0
	bool ambig_BNNN_mode;

#define AMBIG_FX55_FX65_INC_I 0
#define AMBIG_FX55_FX65_DONOT_INC_I 1
#define AMBIG_FX55_FX65_DEFAULT 1
	bool ambig_FX55_FX65_mode;

	// external value used to generate a rand by using remainder of div by 256
	long long system_ticks;

	// optional log function
	chip8_log_func log;

	// true if waiting on user input for opcode 0xFX0A (a blocking operation)
	// set false at the end of every tick, and true if a user input is detected
	bool user_keypressed;
	uint8_t last_keypressed;

private:
	void reset();
	void clear_memory();
	void clear_screen();
	void clear_keys();

	void fetch_opcode();
	void execute_opcode();

public:

	const uint8_t GRAPHICS_WIDTH = CHIP8_GRAPHICS_WIDTH;
	const uint8_t GRAPHICS_HEIGHT = CHIP8_GRAPHICS_HEIGHT;

	Chip8();
	~Chip8();

	void Init();
	void Tick(long long sys_ticks);
	uint8_t * GetScreenBuf();

	void Reset();

	// todo: load rom file
	bool LoadROM(uint8_t * rom, long size);
	bool LoadROMFromFile(const char * filename);

	void SetLogFunc(chip8_log_func func);

	void SetKey(uint8_t key, bool pressed);

	long GetMemorySize();
	uint8_t* GetMemory();
	uint8_t* GetVRegs();
	uint8_t* GetKeys();
	uint8_t GetSoundTimer();
	uint8_t GetDelayTimer();

	short GetIndexRef();
	short GetProgCount();

	bool GetConfig_8XY6_8XYE_VY_mode();
	void SetConfig_8XY6_8XYE_VY_mode(bool mode);

	bool GetConfig_BNNN_ADD_mode();
	void SetConfig_BNNN_ADD_mode(bool mode);

	bool GetConfig_FX55_FX65_VY_mode();
	void SetConfig_FX55_FX65_VY_mode(bool mode);
};