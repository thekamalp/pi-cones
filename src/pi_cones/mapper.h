// Project:     pi_cones (originall from ArkNESS)
// File:        mapper.h
// Author:      Kamal Pillai
// Date:        8/9/2021
// Description:	NES memory mappers

#ifndef __MAPPER_H
#define __MAPPER_H

#include "nessys.h"

// return smallest mask that covers all btis of the input
inline uint8_t nes_get_mask(uint8_t n)
{
	uint8_t mask;
	for (mask = 0x80; mask != 0; mask = mask >> 1) {
		if (n & mask) {
			mask--;
			mask <<= 1;
			mask |= 1;
			break;
		}
	}
	return mask;
}

// ------------------------------------------------------------
// mapper 1 structs/constants

// mask to distinguish which register is being programmed
static const uint16_t MAPPER1_ADDR_MASK = 0xE000;

static const uint16_t MAPPER1_ADDR_CONTROL = 0x8000;
static const uint16_t MAPPER1_ADDR_CHR_BANK0 = 0xA000;
static const uint16_t MAPPER1_ADDR_CHR_BANK1 = 0xC000;
static const uint16_t MAPPER1_ADDR_PRG_BANK = 0xE000;

static const uint8_t MAPPER1_PRG_BANK_SIZE_LOG2 = 14;
static const uint8_t MAPPER1_CHR_BANK_SIZE_LOG2 = 12;
static const uint32_t MAPPER1_PRG_BANK_MASK = (1 << MAPPER1_PRG_BANK_SIZE_LOG2) - 1;
static const uint32_t MAPPER1_CHR_BANK_MASK = (1 << MAPPER1_CHR_BANK_SIZE_LOG2) - 1;

static const uint8_t MAPPER1_SHIFT_REG_RESET = 0x10;

struct mapper1_data {
	uint8_t shift_reg;
	uint8_t pad0[3];
	uint8_t control;
	uint8_t chr_bank0;
	uint8_t chr_bank1;
	uint8_t prg_bank;
	uint8_t prg_ram_bank;
};

// ------------------------------------------------------------
// mapper 2 structs/constants

static const uint8_t MAPPER2_PRG_BANK_SIZE_LOG2 = 14;
static const uint32_t MAPPER2_PRG_BANK_MASK = (1 << MAPPER2_PRG_BANK_SIZE_LOG2) - 1;

struct mapper2_data {
	uint8_t prg_bank;
};

// ------------------------------------------------------------
// mapper 3 structs/constants

static const uint8_t MAPPER3_CHR_BANK_SIZE_LOG2 = 13;
static const uint32_t MAPPER3_CHR_BANK_MASK = (1 << MAPPER3_CHR_BANK_SIZE_LOG2) - 1;

struct mapper3_data {
	uint8_t chr_bank;
};

// ------------------------------------------------------------
// mapper 4 struct/constants

// mask to distinguish which register is being programmed
static const uint16_t MAPPER4_ADDR_MASK = 0xE001;

static const uint16_t MAPPER4_ADDR_BANK_SELECT = 0x8000;
static const uint16_t MAPPER4_ADDR_BANK_DATA = 0x8001;
static const uint16_t MAPPER4_ADDR_MIRROR = 0xA000;
static const uint16_t MAPPER4_ADDR_PRG_RAM_PROTECT = 0xA001;
static const uint16_t MAPPER4_ADDR_IRQ_LATCH = 0xC000;
static const uint16_t MAPPER4_ADDR_IRQ_RELOAD = 0xC001;
static const uint16_t MAPPER4_ADDR_IRQ_DISABLE = 0xE000;
static const uint16_t MAPPER4_ADDR_IRQ_ENABLE = 0xE001;

static const uint8_t MAPPER4_PRG_BANK_SIZE_LOG2 = 13;
static const uint8_t MAPPER4_CHR_BANK_SIZE_LOG2 = 10;
static const uint32_t MAPPER4_PRG_BANK_MASK = (1 << MAPPER4_PRG_BANK_SIZE_LOG2) - 1;
static const uint32_t MAPPER4_CHR_BANK_MASK = (1 << MAPPER4_CHR_BANK_SIZE_LOG2) - 1;

static const uint8_t MAPPER4_REG_MASK[] = { 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x3f };

struct mapper4_data {
	uint8_t r[8];
	uint8_t bank_select;
	uint8_t prg_ram_protect;
	uint8_t irq_latch;
	uint8_t counter_write_pending;
	uint8_t irq_enable;
	uint8_t irq_counter;
	uint8_t last_scanline;
	uint8_t last_upper_ppu_addr;
};

// ------------------------------------------------------------
// mapper 5 struct/constants

// this is not a real register address, just an offset into the memory poool allocated
// for the fill data/color
static const uint32_t MAPPER5_ADDR_FILL_DATA_OFFSET = 0x0c00;

// these are register offset from the bank base (same bank as apu registers)
static const uint32_t MAPPER5_ADDR_PULSE0_CTRL_OFFSET = 0x1000;
static const uint32_t MAPPER5_ADDR_PULSE0_SWEEP_OFFSET = 0x1001;
static const uint32_t MAPPER5_ADDR_PULSE0_TIMER_OFFSET = 0x1002;
static const uint32_t MAPPER5_ADDR_PULSE0_RELOAD_OFFSET = 0x1003;
static const uint32_t MAPPER5_ADDR_PULSE1_CTRL_OFFSET = 0x1004;
static const uint32_t MAPPER5_ADDR_PULSE1_SWEEP_OFFSET = 0x1005;
static const uint32_t MAPPER5_ADDR_PULSE1_TIMER_OFFSET = 0x1006;
static const uint32_t MAPPER5_ADDR_PULSE1_RELOAD_OFFSET = 0x1007;
static const uint32_t MAPPER5_ADDR_PCM_CTRL_OFFSET = 0x1010;
static const uint32_t MAPPER5_ADDR_PCM_DATA_OFFSET = 0x1011;
static const uint32_t MAPPER5_ADDR_PRG_MODE_OFFSET = 0x1100;
static const uint32_t MAPPER5_ADDR_CHR_MODE_OFFSET = 0x1101;
static const uint32_t MAPPER5_ADDR_PRG_RAM_PROTECT1_OFFSET = 0x1102;
static const uint32_t MAPPER5_ADDR_PRG_RAM_PROTECT2_OFFSET = 0x1103;
static const uint32_t MAPPER5_ADDR_EXP_RAM_MODE_OFFSET = 0x1104;
static const uint32_t MAPPER5_ADDR_NTB_MAP_OFFSET = 0x1105;
static const uint32_t MAPPER5_ADDR_FILL_MODE_TILE_OFFSET = 0x1106;
static const uint32_t MAPPER5_ADDR_FILL_MODE_COLOR_OFFSET = 0x1107;
static const uint32_t MAPPER5_ADDR_PRG_BANK0_OFFSET = 0x1113;
static const uint32_t MAPPER5_ADDR_PRG_BANK1_OFFSET = 0x1114;
static const uint32_t MAPPER5_ADDR_PRG_BANK2_OFFSET = 0x1115;
static const uint32_t MAPPER5_ADDR_PRG_BANK3_OFFSET = 0x1116;
static const uint32_t MAPPER5_ADDR_PRG_BANK4_OFFSET = 0x1117;
static const uint32_t MAPPER5_ADDR_CHR_BANK0_OFFSET = 0x1120;
static const uint32_t MAPPER5_ADDR_CHR_BANK1_OFFSET = 0x1121;
static const uint32_t MAPPER5_ADDR_CHR_BANK2_OFFSET = 0x1122;
static const uint32_t MAPPER5_ADDR_CHR_BANK3_OFFSET = 0x1123;
static const uint32_t MAPPER5_ADDR_CHR_BANK4_OFFSET = 0x1124;
static const uint32_t MAPPER5_ADDR_CHR_BANK5_OFFSET = 0x1125;
static const uint32_t MAPPER5_ADDR_CHR_BANK6_OFFSET = 0x1126;
static const uint32_t MAPPER5_ADDR_CHR_BANK7_OFFSET = 0x1127;
static const uint32_t MAPPER5_ADDR_CHR_BANK8_OFFSET = 0x1128;
static const uint32_t MAPPER5_ADDR_CHR_BANK9_OFFSET = 0x1129;
static const uint32_t MAPPER5_ADDR_CHR_BANKA_OFFSET = 0x112A;
static const uint32_t MAPPER5_ADDR_CHR_BANKB_OFFSET = 0x112B;
static const uint32_t MAPPER5_ADDR_UPPER_CHR_BANK_OFFSET = 0x1130;
static const uint32_t MAPPER5_ADDR_VSPLIT_MODE_OFFSET = 0x1200;
static const uint32_t MAPPER5_ADDR_VSPLIT_SCROLL_OFFSET = 0x1201;
static const uint32_t MAPPER5_ADDR_VSPLIT_BANK_OFFSET = 0x1202;
static const uint32_t MAPPER5_ADDR_SCANLINE_IRQ_CMP_OFFSET = 0x1203;
static const uint32_t MAPPER5_ADDR_SCANLINE_IRQ_STATUS_OFFSET = 0x1204;
static const uint32_t MAPPER5_ADDR_MULT0_OFFSET = 0x1205;
static const uint32_t MAPPER5_ADDR_MULT1_OFFSET = 0x1206;
static const uint32_t MAPPER5_ADDR_CL3_SL3_CTRL_OFFSET = 0x1207;
static const uint32_t MAPPER5_ADDR_CL3_SL3_STATUS_OFFSET = 0x1208;
static const uint32_t MAPPER5_ADDR_TIMER_IRQ_LSB_OFFSET = 0x1209;
static const uint32_t MAPPER5_ADDR_TIMER_IRQ_MSB_OFFSET = 0x120a;
static const uint32_t MAPPER5_ADDR_EXP_RAM_START_OFFSET = 0x1c00;

static const uint32_t MAPPER5_ADDR_PULSE0_CTRL = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE0_CTRL_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE0_SWEEP = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE0_SWEEP_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE0_TIMER = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE0_TIMER_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE0_RELOAD = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE0_RELOAD_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE1_CTRL = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE1_CTRL_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE1_SWEEP = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE1_SWEEP_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE1_TIMER = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE1_TIMER_OFFSET;
static const uint32_t MAPPER5_ADDR_PULSE1_RELOAD = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PULSE1_RELOAD_OFFSET;
static const uint32_t MAPPER5_ADDR_PCM_CTRL = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PCM_CTRL_OFFSET;
static const uint32_t MAPPER5_ADDR_PCM_DATA = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PCM_DATA_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_MODE = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_MODE_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_MODE = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_MODE_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_RAM_PROTECT1 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_RAM_PROTECT1_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_RAM_PROTECT2 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_RAM_PROTECT2_OFFSET;
static const uint32_t MAPPER5_ADDR_EXP_RAM_MODE = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_EXP_RAM_MODE_OFFSET;
static const uint32_t MAPPER5_ADDR_NTB_MAP = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_NTB_MAP_OFFSET;
static const uint32_t MAPPER5_ADDR_FILL_MODE_TILE = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_FILL_MODE_TILE_OFFSET;
static const uint32_t MAPPER5_ADDR_FILL_MODE_COLOR = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_FILL_MODE_COLOR_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_BANK0 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_BANK0_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_BANK1 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_BANK1_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_BANK2 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_BANK2_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_BANK3 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_BANK3_OFFSET;
static const uint32_t MAPPER5_ADDR_PRG_BANK4 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_PRG_BANK4_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK0 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK0_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK1 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK1_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK2 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK2_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK3 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK3_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK4 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK4_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK5 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK5_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK6 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK6_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK7 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK7_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK8 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK8_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANK9 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANK9_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANKA = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANKA_OFFSET;
static const uint32_t MAPPER5_ADDR_CHR_BANKB = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CHR_BANKB_OFFSET;
static const uint32_t MAPPER5_ADDR_UPPER_CHR_BANK = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_UPPER_CHR_BANK_OFFSET;
static const uint32_t MAPPER5_ADDR_VSPLIT_MODE = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_VSPLIT_MODE_OFFSET;
static const uint32_t MAPPER5_ADDR_VSPLIT_SCROLL = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_VSPLIT_SCROLL_OFFSET;
static const uint32_t MAPPER5_ADDR_VSPLIT_BANK = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_VSPLIT_BANK_OFFSET;
static const uint32_t MAPPER5_ADDR_SCANLINE_IRQ_CMP = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_SCANLINE_IRQ_CMP_OFFSET;
static const uint32_t MAPPER5_ADDR_SCANLINE_IRQ_STATUS = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_SCANLINE_IRQ_STATUS_OFFSET;
static const uint32_t MAPPER5_ADDR_MULT0 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_MULT0_OFFSET;
static const uint32_t MAPPER5_ADDR_MULT1 = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_MULT1_OFFSET;
static const uint32_t MAPPER5_ADDR_CL3_SL3_CTRL = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CL3_SL3_CTRL_OFFSET;
static const uint32_t MAPPER5_ADDR_CL3_SL3_STATUS = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_CL3_SL3_STATUS_OFFSET;
static const uint32_t MAPPER5_ADDR_TIMER_IRQ_LSB = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_TIMER_IRQ_LSB_OFFSET;
static const uint32_t MAPPER5_ADDR_TIMER_IRQ_MSB = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_TIMER_IRQ_MSB_OFFSET;
static const uint32_t MAPPER5_ADDR_EXP_RAM_START = NESSYS_APU_WIN_MIN + MAPPER5_ADDR_EXP_RAM_START_OFFSET;

static const uint32_t MAPPER5_EXP_RAM_SIZE = 0x400;

static const uint32_t MAPPER5_MEM_SIZE_LOG2 = 13;  // 8 KB
static const uint32_t MAPPER5_MEM_SIZE = 1 << MAPPER5_MEM_SIZE_LOG2;

static const uint32_t MAPPER5_PRG_BANK_BASE_SIZE_LOG2 = 13;  // 8 KB
static const uint32_t MAPPER5_CHR_BANK_BASE_SIZE_LOG2 = 10;  // 1 KB

static const uint32_t MAPPER5_PRG_BANK_BASE_SIZE = 1 << MAPPER5_PRG_BANK_BASE_SIZE_LOG2;
static const uint32_t MAPPER5_CHR_BANK_BASE_SIZE = 1 << MAPPER5_CHR_BANK_BASE_SIZE_LOG2;

static const uint32_t MAPPER5_PRG_BANK_BASE_MASK = MAPPER5_PRG_BANK_BASE_SIZE - 1;
static const uint32_t MAPPER5_CHR_BANK_BASE_MASK = MAPPER5_CHR_BANK_BASE_SIZE - 1;

struct mapper5_data {
	uint8_t mem[MAPPER5_MEM_SIZE];
	uint8_t prg_mode;
	uint8_t chr_mode;
	uint8_t prg_ram_protect1;
	uint8_t prg_ram_protect2;
	uint8_t exp_ram_mode;
	uint8_t ntb_map;
	uint8_t msb_chr_bank;
	uint8_t prg_bank[5];
	uint16_t chr_bank[12];
	uint8_t upper_reg_touched;
	uint8_t vsplit_mode;
	uint8_t vsplit_scroll;
	uint8_t vsplit_bank;
	uint8_t scanline_irq_cmp;
	uint8_t scanline_irq_status;
	uint8_t scanline_irq_pend_clear;
	uint8_t mult[2];
	uint8_t cl3_sl3_ctrl;
	uint8_t cl3_sl3_status;
	uint16_t timer_irq;
	int32_t last_scanline;
	uint8_t scroll_save[2];
	uint16_t scroll_save_y;
	nessys_apu_pulse_t pulse[2];
	bool exp_surf_dirty;
};

// ------------------------------------------------------------
// mapper 7 struct/constants

static const uint8_t MAPPER7_PRG_BANK_SIZE_LOG2 = 15;
static const uint32_t MAPPER7_PRG_BANK_MASK = (1 << MAPPER7_PRG_BANK_SIZE_LOG2) - 1;

static const uint8_t MAPPER7_PRG_BANK_BITS = 0x7;
static const uint8_t MAPPER7_NTB_SELECT = 0x10;

struct mapper7_data {
	uint8_t bank;
};

// ------------------------------------------------------------
// mapper 9 struct/constants

static const uint16_t MAPPER9_ADDR_MASK = 0xF000;

static const uint16_t MAPPER9_ADDR_PRG_ROM_BANK  = 0xA000;
static const uint16_t MAPPER9_ADDR_CHR_ROM_BANK0 = 0xB000;
static const uint16_t MAPPER9_ADDR_CHR_ROM_BANK1 = 0xC000;
static const uint16_t MAPPER9_ADDR_CHR_ROM_BANK2 = 0xD000;
static const uint16_t MAPPER9_ADDR_CHR_ROM_BANK3 = 0xE000;
static const uint16_t MAPPER9_ADDR_MIRROR        = 0xF000;

static const uint8_t MAPPER9_PRG_BANK_SIZE_LOG2 = 13;  // 8KB granularity
static const uint32_t MAPPER9_PRG_BANK_MASK = (1 << MAPPER9_PRG_BANK_SIZE_LOG2) - 1;
static const uint8_t MAPPER9_CHR_BANK_SIZE_LOG2 = 12;  // 4KB granularity
static const uint32_t MAPPER9_CHR_BANK_MASK = (1 << MAPPER9_CHR_BANK_SIZE_LOG2) - 1;

static const uint8_t MAPPER9_MIRROR_MODE_VERTICAL = 0x0;
static const uint8_t MAPPER9_MIRROR_MODE_HORIZONTAL = 0x1;

static const uint8_t MAPPER9_PRG_BANK_BITS = 0x0F;
static const uint8_t MAPPER9_CHR_BANK_BITS = 0x1F;
static const uint8_t MAPPER9_MIRROR_BITS = 0x1;

struct mapper9_data {
	uint8_t prg_bank;
	uint8_t chr_bank[4];
	uint8_t mirror;
};

// ------------------------------------------------------------
// mapper 69 struct/constants

static const uint16_t MAPPER69_ADDR_MASK = 0xE000;

static const uint16_t MAPPER69_ADDR_COMMAND = 0x8000;
static const uint16_t MAPPER69_ADDR_PARAMETER = 0xA000;

static const uint8_t MAPPER69_PRG_BANK_SIZE_LOG2 = 13;
static const uint8_t MAPPER69_CHR_BANK_SIZE_LOG2 = 10;
static const uint32_t MAPPER69_PRG_BANK_MASK = (1 << MAPPER69_PRG_BANK_SIZE_LOG2) - 1;
static const uint32_t MAPPER69_CHR_BANK_MASK = (1 << MAPPER69_CHR_BANK_SIZE_LOG2) - 1;

static const uint8_t MAPPER69_FLAGS_IRQ_ENABLE = 0x01;
static const uint8_t MAPPER69_FLAGS_IRQ_COUNTER_ENABLE = 0x80;
static const uint8_t MAPPER69_FLAGS_MIRROR_MODE = 0x30;
static const uint8_t MAPPER69_MIRROR_MODE_SHIFT = 4;

static const uint8_t MAPPER69_MIRROR_MODE_VERTICAL = 0x00;
static const uint8_t MAPPER69_MIRROR_MODE_HORIZONTAL = 0x10;
static const uint8_t MAPPER69_MIRROR_MODE_ONE_SCREEN_LOWER = 0x20;
static const uint8_t MAPPER69_MIRROR_MODE_ONE_SCREEN_UPPER = 0x30;

static const uint8_t MAPPER69_PRG_BANK0_RAM_SELECT = 0x40;

struct mapper69_data {
	uint8_t command;
	uint8_t flags;
	uint16_t irq_counter;
	uint8_t prg_bank[4];
	uint8_t chr_bank[8];
};

// ------------------------------------------------------------
// mapper 71 structs/constants

static const uint16_t MAPPER71_ADDR_MASK = 0xF000;

static const uint16_t MAPPER71_ADDR_MIRROR = 0x9000;
static const uint16_t MAPPER71_ADDR_BANK_SEL0 = 0xC000;
static const uint16_t MAPPER71_ADDR_BANK_SEL1 = 0xD000;
static const uint16_t MAPPER71_ADDR_BANK_SEL2 = 0xE000;
static const uint16_t MAPPER71_ADDR_BANK_SEL3 = 0xF000;

static const uint8_t MAPPER71_PRG_BANK_SIZE_LOG2 = 14;
static const uint32_t MAPPER71_PRG_BANK_MASK = (1 << MAPPER71_PRG_BANK_SIZE_LOG2) - 1;

struct mapper71_data {
	uint8_t mirror;
	uint8_t prg_bank;
};

// ------------------------------------------------------------
// mapper 180 structs/constants

static const uint8_t MAPPER180_PRG_BANK_SIZE_LOG2 = 14;
static const uint32_t MAPPER180_PRG_BANK_MASK = (1 << MAPPER2_PRG_BANK_SIZE_LOG2) - 1;

struct mapper180_data {
	uint8_t prg_bank;
};

#endif
