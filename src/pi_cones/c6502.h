// Project:     pi_cones (originally from ArkNESS)
// File:        c6502.h
// Author:      Kamal Pillai
// Date:        7/15/2021
// Description:	6502 specific defines

#ifndef __C6502_H
#define __C6502_H

// cpu instructions and opcodes
typedef enum {
	C6502_ADDR_NONE,
	C6502_ADDR_ACCUM,
	C6502_ADDR_IMMED,
	C6502_ADDR_ZEROPAGE,
	C6502_ADDR_ABSOLUTE,
	C6502_ADDR_RELATIVE,
	C6502_ADDR_INDIRECT,
	C6502_ADDR_ZEROPAGE_X,
	C6502_ADDR_ZEROPAGE_Y,
	C6502_ADDR_ABSOLUTE_X,
	C6502_ADDR_ABSOLUTE_Y,
	C6502_ADDR_INDIRECT_X,
	C6502_ADDR_INDIRECT_Y
} c6502_addr_mode;

typedef enum {
	// undefined
	C6502_INS_NUL,
	// opcode: 0x00-0x1F
	C6502_INS_BRK, C6502_INS_ORA, C6502_INS_SLO, C6502_INS_NOP, C6502_INS_ASL,
	C6502_INS_PHP, C6502_INS_ANC, C6502_INS_BPL, C6502_INS_CLC,
	// opcode: 0x20-03F
	C6502_INS_JSR, C6502_INS_AND, C6502_INS_RLA, C6502_INS_BIT,
	C6502_INS_ROL, C6502_INS_PLP, C6502_INS_BMI, C6502_INS_SEC,
	// opcode: 0x40-5F
	C6502_INS_RTI, C6502_INS_EOR, C6502_INS_SRE, C6502_INS_LSR, C6502_INS_PHA,
	C6502_INS_ASR, C6502_INS_JMP, C6502_INS_BVC, C6502_INS_CLI,
	// opcode: 0x60-7F
	C6502_INS_RTS, C6502_INS_ADC, C6502_INS_RRA, C6502_INS_ROR,
	C6502_INS_PLA, C6502_INS_ARR, C6502_INS_BVS, C6502_INS_SEI,
	// opcode: 0x80-0x9F
	C6502_INS_STA, C6502_INS_SAX, C6502_INS_STY, C6502_INS_STX, C6502_INS_DEY,
	C6502_INS_TXA, C6502_INS_ANE, C6502_INS_BCC, C6502_INS_SHA, C6502_INS_TYA,
	C6502_INS_TXS, C6502_INS_SHS, C6502_INS_SHY, C6502_INS_SHX,
	// opcode: 0xA0-0xBF
	C6502_INS_LDY, C6502_INS_LDA, C6502_INS_LDX, C6502_INS_LAX,
	C6502_INS_TAY, C6502_INS_TAX, C6502_INS_LXA, C6502_INS_BCS,
	C6502_INS_CLV, C6502_INS_TSX, C6502_INS_LAS,
	// opcode: 0xC0-0xDF
	C6502_INS_CPY, C6502_INS_CMP, C6502_INS_DCP, C6502_INS_DEC, C6502_INS_INY,
	C6502_INS_DEX, C6502_INS_SBX, C6502_INS_BNE, C6502_INS_CLD,
	// opcode: 0xE0, 0xFF
	C6502_INS_CPX, C6502_INS_SBC, C6502_INS_ISB, C6502_INS_INC,
	C6502_INS_INX, C6502_INS_BEQ, C6502_INS_SED
} c6502_instr;

// status reg flags (P reg)
#define C6502_P_C_SHIFT 0x00
#define C6502_P_Z_SHIFT 0x01
#define C6502_P_I_SHIFT 0x02
#define C6502_P_D_SHIFT 0x03
#define C6502_P_B_SHIFT 0x04
#define C6502_P_U_SHIFT 0x05
#define C6502_P_V_SHIFT 0x06
#define C6502_P_N_SHIFT 0x07

static const uint8_t C6502_P_C = (0x01 << C6502_P_C_SHIFT);  // carry
static const uint8_t C6502_P_Z = (0x01 << C6502_P_Z_SHIFT);  // zero
static const uint8_t C6502_P_I = (0x01 << C6502_P_I_SHIFT);  // interrupt disable
static const uint8_t C6502_P_D = (0x01 << C6502_P_D_SHIFT);  // decimal
static const uint8_t C6502_P_B = (0x01 << C6502_P_B_SHIFT);  // break
static const uint8_t C6502_P_U = (0x01 << C6502_P_U_SHIFT);  // unused, set to 1 on stack
static const uint8_t C6502_P_V = (0x01 << C6502_P_V_SHIFT);  // overflow
static const uint8_t C6502_P_N = (0x01 << C6502_P_N_SHIFT);  // negative

// instruction flags
#define C6502_FL_NONE         0x00000000
#define C6502_FL_UNDOCUMENTED 0x00000001
#define C6502_FL_ILLEGAL      0x00000002

typedef struct {
	c6502_instr ins;
	const char ins_name[4];
	c6502_addr_mode addr;
	uint16_t num_bytes;
	uint16_t num_cycles;  // does not include extra cycle penalty for page crossing, nor extra cycle for branch taken
	uint16_t penalty_cycles;  // penaly cycles if page crossed or branch taken
	uint16_t flags;
} c6502_op_code_t;

#define C6502_NUM_OPCODES 256

static const c6502_op_code_t C6502_OP_CODE[C6502_NUM_OPCODES] = {
	// opcode: 0x00-0x1F
	{C6502_INS_BRK, "BRK", C6502_ADDR_NONE,       2, 7, 0, C6502_FL_NONE         },  // opcode: 0x00
	{C6502_INS_ORA, "ORA", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x01
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x02
	{C6502_INS_SLO, "SLO", C6502_ADDR_INDIRECT_X, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x03
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x04
	{C6502_INS_ORA, "ORA", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x05
	{C6502_INS_ASL, "ASL", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_NONE         },  // opcode: 0x06
	{C6502_INS_SLO, "SLO", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x07

	{C6502_INS_PHP, "PHP", C6502_ADDR_NONE,       1, 3, 0, C6502_FL_NONE         },  // opcode: 0x08
	{C6502_INS_ORA, "ORA", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0x09
	{C6502_INS_ASL, "ASL", C6502_ADDR_ACCUM,      1, 2, 0, C6502_FL_NONE         },  // opcode: 0x0A
	{C6502_INS_ANC, "ANC", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x0B
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x0C
	{C6502_INS_ORA, "ORA", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x0D
	{C6502_INS_ASL, "ASL", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0x0E
	{C6502_INS_SLO, "SLO", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x0F

	{C6502_INS_BPL, "BPL", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0x10
	{C6502_INS_ORA, "ORA", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0x11
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x12
	{C6502_INS_SLO, "SLO", C6502_ADDR_INDIRECT_Y, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x13
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x14
	{C6502_INS_ORA, "ORA", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x15
	{C6502_INS_ASL, "ASL", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x16
	{C6502_INS_SLO, "SLO", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x17

	{C6502_INS_CLC, "CLC", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x18
	{C6502_INS_ORA, "ORA", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x19
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x1A
	{C6502_INS_SLO, "SLO", C6502_ADDR_ABSOLUTE_Y, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x1B
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x1C
	{C6502_INS_ORA, "ORA", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x1D
	{C6502_INS_ASL, "ASL", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_NONE         },  // opcode: 0x1E
	{C6502_INS_SLO, "SLO", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x1F

	// opcode: 0x20-0x3F
	{C6502_INS_JSR, "JSR", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0x20
	{C6502_INS_AND, "AND", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x21
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x22
	{C6502_INS_RLA, "RLA", C6502_ADDR_INDIRECT_X, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x23
	{C6502_INS_BIT, "BIT", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x24
	{C6502_INS_AND, "AND", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x25
	{C6502_INS_ROL, "ROL", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_NONE         },  // opcode: 0x26
	{C6502_INS_RLA, "RLA", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x27

	{C6502_INS_PLP, "PLP", C6502_ADDR_NONE,       1, 4, 0, C6502_FL_NONE         },  // opcode: 0x28
	{C6502_INS_AND, "AND", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0x29
	{C6502_INS_ROL, "ROL", C6502_ADDR_ACCUM,      1, 2, 0, C6502_FL_NONE         },  // opcode: 0x2A
	{C6502_INS_ANC, "ANC", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x2B
	{C6502_INS_BIT, "BIT", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x2C
	{C6502_INS_AND, "AND", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x2D
	{C6502_INS_ROL, "ROL", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0x2E
	{C6502_INS_RLA, "RLA", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x2F

	{C6502_INS_BMI, "BMI", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0x30
	{C6502_INS_AND, "AND", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0x31
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x32
	{C6502_INS_RLA, "RLA", C6502_ADDR_INDIRECT_Y, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x33
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x34
	{C6502_INS_AND, "AND", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x35
	{C6502_INS_ROL, "ROL", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x36
	{C6502_INS_RLA, "RLA", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x37

	{C6502_INS_SEC, "SEC", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x38
	{C6502_INS_AND, "AND", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x39
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x3A
	{C6502_INS_RLA, "RLA", C6502_ADDR_ABSOLUTE_Y, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x3B
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x3C
	{C6502_INS_AND, "AND", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x3D
	{C6502_INS_ROL, "ROL", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_NONE         },  // opcode: 0x3E
	{C6502_INS_RLA, "RLA", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x3F

	// opcode: 0x40-0x5F
	{C6502_INS_RTI, "RTI", C6502_ADDR_NONE,       1, 6, 0, C6502_FL_NONE         },  // opcode: 0x40
	{C6502_INS_EOR, "EOR", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x41
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x42
	{C6502_INS_SRE, "SRE", C6502_ADDR_INDIRECT_X, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x43
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x44
	{C6502_INS_EOR, "EOR", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x45
	{C6502_INS_LSR, "LSR", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_NONE         },  // opcode: 0x46
	{C6502_INS_SRE, "SRE", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x47

	{C6502_INS_PHA, "PHA", C6502_ADDR_NONE,       1, 3, 0, C6502_FL_NONE         },  // opcode: 0x48
	{C6502_INS_EOR, "EOR", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0x49
	{C6502_INS_LSR, "LSR", C6502_ADDR_ACCUM,      1, 2, 0, C6502_FL_NONE         },  // opcode: 0x4A
	{C6502_INS_ASR, "ASR", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x4B
	{C6502_INS_JMP, "JMP", C6502_ADDR_ABSOLUTE,   3, 3, 0, C6502_FL_NONE         },  // opcode: 0x4C
	{C6502_INS_EOR, "EOR", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x4D
	{C6502_INS_LSR, "LSR", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0x4E
	{C6502_INS_SRE, "SRE", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x4F

	{C6502_INS_BVC, "BVC", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0x50
	{C6502_INS_EOR, "EOR", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0x51
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x52
	{C6502_INS_SRE, "SRE", C6502_ADDR_INDIRECT_Y, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x53
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x54
	{C6502_INS_EOR, "EOR", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x55
	{C6502_INS_LSR, "LSR", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x56
	{C6502_INS_SRE, "SRE", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x57

	{C6502_INS_CLI, "CLI", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x58
	{C6502_INS_EOR, "EOR", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x59
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x5A
	{C6502_INS_SRE, "SRE", C6502_ADDR_ABSOLUTE_Y, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x5B
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0x5C
	{C6502_INS_EOR, "EOR", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x5D
	{C6502_INS_LSR, "LSR", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_NONE         },  // opcode: 0x5E
	{C6502_INS_SRE, "SRE", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x5F

	// opcode: 0x60-0x7F
	{C6502_INS_RTS, "RTS", C6502_ADDR_NONE,       1, 6, 0, C6502_FL_NONE         },  // opcode: 0x60
	{C6502_INS_ADC, "ADC", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x61
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x62
	{C6502_INS_RRA, "RRA", C6502_ADDR_INDIRECT_X, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x63
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x64
	{C6502_INS_ADC, "ADC", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x65
	{C6502_INS_ROR, "ROR", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_NONE         },  // opcode: 0x66
	{C6502_INS_RRA, "RRA", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x67

	{C6502_INS_PLA, "PLA", C6502_ADDR_NONE,       1, 4, 0, C6502_FL_NONE         },  // opcode: 0x68
	{C6502_INS_ADC, "ADC", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0x69
	{C6502_INS_ROR, "ROR", C6502_ADDR_ACCUM,      1, 2, 0, C6502_FL_NONE         },  // opcode: 0x6A
	{C6502_INS_ARR, "ARR", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x6B
	{C6502_INS_JMP, "JMP", C6502_ADDR_INDIRECT,   3, 5, 0, C6502_FL_NONE         },  // opcode: 0x6C
	{C6502_INS_ADC, "ADC", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x6D
	{C6502_INS_ROR, "ROR", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0x6E
	{C6502_INS_RRA, "RRA", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x6F

	{C6502_INS_BVS, "BVS", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0x70
	{C6502_INS_ADC, "ADC", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0x71
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x72
	{C6502_INS_RRA, "RRA", C6502_ADDR_INDIRECT_Y, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x73
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x74
	{C6502_INS_ADC, "ADC", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x75
	{C6502_INS_ROR, "ROR", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x76
	{C6502_INS_RRA, "RRA", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x77

	{C6502_INS_SEI, "SEI", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x78
	{C6502_INS_ADC, "ADC", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x79
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x7A
	{C6502_INS_RRA, "RRA", C6502_ADDR_ABSOLUTE_Y, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x7B
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0x7C
	{C6502_INS_ADC, "ADC", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0x7D
	{C6502_INS_ROR, "ROR", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_NONE         },  // opcode: 0x7E
	{C6502_INS_RRA, "RRA", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x7F

	// opcode: 0x80-0x9F
	{C6502_INS_NOP, "NOP", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x80
	{C6502_INS_STA, "STA", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x81
	{C6502_INS_NOP, "NOP", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x82
	{C6502_INS_SAX, "SAX", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x83
	{C6502_INS_STY, "STY", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x84
	{C6502_INS_STA, "STA", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x85
	{C6502_INS_STX, "STX", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0x86
	{C6502_INS_SAX, "SAX", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x87

	{C6502_INS_DEY, "DEY", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x88
	{C6502_INS_NOP, "NOP", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x89
	{C6502_INS_TXA, "TXA", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x8A
	{C6502_INS_ANE, "ANE", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x8B
	{C6502_INS_STY, "STY", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x8C
	{C6502_INS_STA, "STA", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x8D
	{C6502_INS_STX, "STX", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0x8E
	{C6502_INS_SAX, "SAX", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x8F

	{C6502_INS_BCC, "BCC", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0x90
	{C6502_INS_STA, "STA", C6502_ADDR_INDIRECT_Y, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0x91
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0x92
	{C6502_INS_SHA, "SHA", C6502_ADDR_INDIRECT_Y, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x93
	{C6502_INS_STY, "STY", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x94
	{C6502_INS_STA, "STA", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x95
	{C6502_INS_STX, "STX", C6502_ADDR_ZEROPAGE_Y, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0x96
	{C6502_INS_SAX, "SAX", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x97

	{C6502_INS_TYA, "TYA", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x98
	{C6502_INS_STA, "STA", C6502_ADDR_ABSOLUTE_Y, 3, 5, 0, C6502_FL_NONE         },  // opcode: 0x99
	{C6502_INS_TXS, "TXS", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0x9A
	{C6502_INS_SHS, "SHS", C6502_ADDR_ABSOLUTE_Y, 3, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x9B
	{C6502_INS_SHY, "SHY", C6502_ADDR_ABSOLUTE_X, 3, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x9C
	{C6502_INS_STA, "STA", C6502_ADDR_ABSOLUTE_X, 3, 5, 0, C6502_FL_NONE         },  // opcode: 0x9D
	{C6502_INS_SHX, "SHX", C6502_ADDR_ABSOLUTE_X, 3, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x9E
	{C6502_INS_SHA, "SHA", C6502_ADDR_ABSOLUTE_X, 3, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0x9F

	// opcode: 0xA0-0xBF
	{C6502_INS_LDY, "LDY", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xA0
	{C6502_INS_LDA, "LDA", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0xA1
	{C6502_INS_LDX, "LDX", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xA2
	{C6502_INS_LAX, "LAX", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xA3
	{C6502_INS_LDY, "LDY", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xA4
	{C6502_INS_LDA, "LDA", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xA5
	{C6502_INS_LDX, "LDX", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xA6
	{C6502_INS_LAX, "LAX", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xA7

	{C6502_INS_TAY, "TAY", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xA8
	{C6502_INS_LDA, "LDA", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xA9
	{C6502_INS_TAX, "TAX", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xAA
	{C6502_INS_LXA, "LXA", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xAB
	{C6502_INS_LDY, "LDY", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xAC
	{C6502_INS_LDA, "LDA", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xAD
	{C6502_INS_LDX, "LDX", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xAE
	{C6502_INS_LAX, "LAX", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xAF

	{C6502_INS_BCS, "BCS", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0xB0
	{C6502_INS_LDA, "LDA", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0xB1
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0xB2
	{C6502_INS_LAX, "LAX", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0xB3
	{C6502_INS_LDY, "LDY", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0xB4
	{C6502_INS_LDA, "LDA", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0xB5
	{C6502_INS_LDX, "LDX", C6502_ADDR_ZEROPAGE_Y, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0xB6
	{C6502_INS_LAX, "LAX", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xB7

	{C6502_INS_CLV, "CLV", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xB8
	{C6502_INS_LDA, "LDA", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xB9
	{C6502_INS_TSX, "TSX", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xBA
	{C6502_INS_LAS, "LAS", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0xBB
	{C6502_INS_LDY, "LDY", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xBC
	{C6502_INS_LDA, "LDA", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xBD
	{C6502_INS_LDX, "LDX", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xBE
	{C6502_INS_LAX, "LAX", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0xBF

	// opcode: 0xC0-0xDF
	{C6502_INS_CPY, "CPY", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xC0
	{C6502_INS_CMP, "CMP", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0xC1
	{C6502_INS_NOP, "NOP", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xC2
	{C6502_INS_DCP, "DCP", C6502_ADDR_INDIRECT_X, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xC3
	{C6502_INS_CPY, "CPY", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xC4
	{C6502_INS_CMP, "CMP", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xC5
	{C6502_INS_DEC, "DEC", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_NONE         },  // opcode: 0xC6
	{C6502_INS_DCP, "DCP", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xC7

	{C6502_INS_INY, "INY", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xC8
	{C6502_INS_CMP, "CMP", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xC9
	{C6502_INS_DEX, "DEX", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xCA
	{C6502_INS_SBX, "SBX", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xCB
	{C6502_INS_CPY, "CPY", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xCC
	{C6502_INS_CMP, "CMP", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xCD
	{C6502_INS_DEC, "DEC", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0xCE
	{C6502_INS_DCP, "DCP", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xCF

	{C6502_INS_BNE, "BNE", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0xD0
	{C6502_INS_CMP, "CMP", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0xD1
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0xD2
	{C6502_INS_DCP, "DCP", C6502_ADDR_INDIRECT_Y, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xD3
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xD4
	{C6502_INS_CMP, "CMP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0xD5
	{C6502_INS_DEC, "DEC", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0xD6
	{C6502_INS_DCP, "DCP", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xD7

	{C6502_INS_CLD, "CLD", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xD8
	{C6502_INS_CMP, "CMP", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xD9
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xDA
	{C6502_INS_DCP, "DCP", C6502_ADDR_ABSOLUTE_Y, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xDB
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0xDC
	{C6502_INS_CMP, "CMP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xDD
	{C6502_INS_DEC, "DEC", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_NONE         },  // opcode: 0xDE
	{C6502_INS_DCP, "DCP", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xDF

	// opcode: 0xE0-0xFF
	{C6502_INS_CPX, "CPX", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xE0
	{C6502_INS_SBC, "SBC", C6502_ADDR_INDIRECT_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0xE1
	{C6502_INS_NOP, "NOP", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xE2
	{C6502_INS_ISB, "ISB", C6502_ADDR_INDIRECT_X, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xE3
	{C6502_INS_CPX, "CPX", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xE4
	{C6502_INS_SBC, "SBC", C6502_ADDR_ZEROPAGE,   2, 3, 0, C6502_FL_NONE         },  // opcode: 0xE5
	{C6502_INS_INC, "INC", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_NONE         },  // opcode: 0xE6
	{C6502_INS_ISB, "ISB", C6502_ADDR_ZEROPAGE,   2, 5, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xE7

	{C6502_INS_INX, "INX", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xE8
	{C6502_INS_SBC, "SBC", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_NONE         },  // opcode: 0xE9
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xEA
	{C6502_INS_SBC, "SBC", C6502_ADDR_IMMED,      2, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xEB
	{C6502_INS_CPX, "CPX", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xEC
	{C6502_INS_SBC, "SBC", C6502_ADDR_ABSOLUTE,   3, 4, 0, C6502_FL_NONE         },  // opcode: 0xED
	{C6502_INS_INC, "INC", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_NONE         },  // opcode: 0xEE
	{C6502_INS_ISB, "ISB", C6502_ADDR_ABSOLUTE,   3, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xEF

	{C6502_INS_BEQ, "BEQ", C6502_ADDR_RELATIVE,   2, 2, 1, C6502_FL_NONE         },  // opcode: 0xF0
	{C6502_INS_SBC, "SBC", C6502_ADDR_INDIRECT_Y, 2, 5, 1, C6502_FL_NONE         },  // opcode: 0xF1
	{C6502_INS_NUL, "NUL", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_ILLEGAL      },  // opcode: 0xF2
	{C6502_INS_ISB, "ISB", C6502_ADDR_INDIRECT_Y, 2, 8, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xF3
	{C6502_INS_NOP, "NOP", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xF4
	{C6502_INS_SBC, "SBC", C6502_ADDR_ZEROPAGE_X, 2, 4, 0, C6502_FL_NONE         },  // opcode: 0xF5
	{C6502_INS_INC, "INC", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_NONE         },  // opcode: 0xF6
	{C6502_INS_ISB, "ISB", C6502_ADDR_ZEROPAGE_X, 2, 6, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xF7

	{C6502_INS_SED, "SED", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_NONE         },  // opcode: 0xF8
	{C6502_INS_SBC, "SBC", C6502_ADDR_ABSOLUTE_Y, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xF9
	{C6502_INS_NOP, "NOP", C6502_ADDR_NONE,       1, 2, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xFA
	{C6502_INS_ISB, "ISB", C6502_ADDR_ABSOLUTE_Y, 3, 7, 0, C6502_FL_UNDOCUMENTED },  // opcode: 0xFB
	{C6502_INS_NOP, "NOP", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_UNDOCUMENTED },  // opcode: 0xFC
	{C6502_INS_SBC, "SBC", C6502_ADDR_ABSOLUTE_X, 3, 4, 1, C6502_FL_NONE         },  // opcode: 0xFD
	{C6502_INS_INC, "INC", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_NONE         },  // opcode: 0xFE
	{C6502_INS_ISB, "ISB", C6502_ADDR_ABSOLUTE_X, 3, 7, 0, C6502_FL_UNDOCUMENTED }   // opcode: 0xFF
};

#endif
