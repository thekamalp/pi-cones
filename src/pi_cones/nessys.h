// Project:     pi_cones (originally from ArkNESS)
// File:        nessys.h
// Author:      Kamal Pillai
// Date:        7/12/2021
// Description:	NES system structures

#ifndef __NESSYS_H
#define __NESSYS_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <stdlib.h>
#include <stdbool.h>
typedef unsigned int uint;
#else
#include "pico/stdlib.h"
#include "pico/multicore.h"
#endif

// memory map constnts
// 2KB of RAM
#define NESSYS_RAM_SIZE 0x800
#define NESSYS_RAM_MASK (NESSYS_RAM_SIZE - 1)
#define NESSYS_RAM_WIN_MIN 0x0
// 8KB ram window
#define NESSYS_RAM_WIN_SIZE 0x2000
#define NESSYS_RAM_WIN_MAX (NESSYS_RAM_WIN_MIN + NESSYS_RAM_WIN_SIZE - 1)

#define NESSYS_PPU_REG_SIZE 0x8
#define NESSYS_PPU_REG_MASK (NESSYS_PPU_REG_SIZE - 1)
#define NESSYS_PPU_WIN_MIN 0x2000
#define NESSYS_PPU_WIN_SIZE 0x2000
#define NESSYS_PPU_WIN_MAX (NESSYS_PPU_WIN_MIN + NESSYS_PPU_WIN_SIZE - 1)

#define NESSYS_APU_SIZE 0x18
#define NESSYS_APU_WIN_MIN 0x4000
#define NESSYS_APU_WIN_MAX (NESSYS_APU_WIN_MIN + NESSYS_APU_SIZE - 1)
#define NESSYS_APU_MASK 0x1F

#define NESSYS_PRG_WIN_MIN 0x4000
#define NESSYS_PRG_WIN_SIZE 0xC000
#define NESSYS_PRG_WIN_MAX (NESSYS_PRG_WIN_MIN + NESSYS_PRG_WIN_SIZE - 1)

// 2KB ram, nametable space
#define NESSYS_PPU_MEM_SIZE 0x800
#define NESSYS_PPU_NUM_SPRITES 64
#define NESSYS_PPU_SPRITE_SIZE 4
#define NESSYS_PPU_OAM_SIZE (NESSYS_PPU_NUM_SPRITES * NESSYS_PPU_SPRITE_SIZE)

// Cache the sprite pixels at 2 bpp (bottom 2 bits of palette index)
// Each entry is 16 bits, representing a row of the sprite
// There are 16 entries per sprite to handle 8x16 sprite
#define NESSYS_PPU_OAM_PIXEL_ROWS 16
#define NESSYS_PPU_OAM_PIXEL_SIZE (NESSYS_PPU_NUM_SPRITES * NESSYS_PPU_OAM_PIXEL_ROWS)

// Cache the tile pixels at 2 bpp
// Need 33 tiles, since we have 256 pixel width, divided by 8 pixels per tile, then add 1 when we are not tile aligned
#define NESSYS_PPU_TILES_PER_ROW 33
#define NESSYS_PPU_PIXEL_ROW_PER_TILE 8
#define NESSYS_PPU_TILE_PIXEL_SIZE (NESSYS_PPU_TILES_PER_ROW * NESSYS_PPU_PIXEL_ROW_PER_TILE)

// Cache attibutes
// Need 9 attrib bytes
// each byte represents a 32x32 region, so for a 256 pixel wide screen we need 8, plus 1 extra to handle unaligned case
#define NESSYS_PPU_ATTRIB_BYTES_PER_ROW 9

#define NESSYS_PPU_MAX_SPRITES_PER_SCAN_LINE 8
#define NESSYS_PPU_SCAN_LINE_OAM_SIZE (NESSYS_PPU_MAX_SPRITES_PER_SCAN_LINE * NESSYS_PPU_SPRITE_SIZE)

#define NESSYS_CHR_ROM_WIN_MIN 0x0
#define NESSYS_CHR_ROM_WIN_SIZE 0x2000
#define NESSYS_CHR_ROM_WIN_MAX (NESSYS_CHR_ROM_WIN_MIN + NESSYS_CHR_ROM_WIN_SIZE - 1)

#define NESSYS_CHR_NTB_WIN_MIN 0x2000
#define NESSYS_CHR_NTB_WIN_SIZE 0x2000
#define NESSYS_CHR_NTB_WIN_MAX (NESSYS_CHR_NTB_WIN_MIN + NESSYS_CHR_NTB_WIN_SIZE - 1)

#define NESSYS_PPU_PAL_SIZE 0x20
#define NESSYS_PPU_PAL_MASK (NESSYS_PPU_PAL_SIZE - 1)
#define NESSYS_CHR_PAL_WIN_MIN 0x3F00
#define NESSYS_CHR_PAL_WIN_SIZE 0x100
#define NESSYS_CHR_PAL_WIN_MAX (NESSYS_CHR_PAL_WIN_MIN + NESSYS_CHR_PAL_WIN_SIZE - 1)

// PRG ROM/RAM address mappers are at 8KB granularity
#define NESSYS_PRG_BANK_SIZE_LOG2 13
// CHR ROM address mappers are at 1KB granularity
#define NESSYS_CHR_BANK_SIZE_LOG2 10
#define NESSYS_PRG_BANK_SIZE (1 << NESSYS_PRG_BANK_SIZE_LOG2)
#define NESSYS_CHR_BANK_SIZE (1 << NESSYS_CHR_BANK_SIZE_LOG2)
#define NESSYS_PRG_MEM_MASK (NESSYS_PRG_BANK_SIZE - 1)
#define NESSYS_CHR_MEM_MASK (NESSYS_CHR_BANK_SIZE - 1)

#define NESSYS_PRG_ADDR_SPACE 0x10000
#define NESSYS_CHR_ADDR_SPACE 0x4000
#define NESSYS_PRG_NUM_BANKS (NESSYS_PRG_ADDR_SPACE / NESSYS_PRG_BANK_SIZE)
#define NESSYS_CHR_NUM_BANKS (NESSYS_CHR_ADDR_SPACE / NESSYS_CHR_BANK_SIZE)

// typical starting address of RAM
#define NESSYS_PRG_RAM_START 0x6000
#define NESSYS_PRM_RAM_SIZE 0x2000
#define NESSYS_PRG_RAM_END (NESSYS_PRG_RAM_START + NESSYS_PRM_RAM_SIZE - 1)
// typical starting address of ROM
#define NESSYS_PRG_ROM_START 0x8000

#define NESSYS_SYS_RAM_START_BANK (NESSYS_RAM_WIN_MIN / NESSYS_PRG_BANK_SIZE)
#define NESSYS_PPU_REG_START_BANK (NESSYS_PPU_WIN_MIN / NESSYS_PRG_BANK_SIZE)
#define NESSYS_APU_REG_START_BANK (NESSYS_APU_WIN_MIN / NESSYS_PRG_BANK_SIZE)
#define NESSYS_PRG_RAM_START_BANK (NESSYS_PRG_RAM_START / NESSYS_PRG_BANK_SIZE)
#define NESSYS_PRG_ROM_START_BANK (NESSYS_PRG_ROM_START / NESSYS_PRG_BANK_SIZE)

#define NESSYS_PRM_RAM_END_BANK (NESSYS_PRG_RAM_END / NESSYS_PRG_BANK_SIZE)

#define NESSYS_CHR_ROM_START_BANK (NESSYS_CHR_ROM_WIN_MIN / NESSYS_CHR_BANK_SIZE)
#define NESSYS_CHR_NTB_START_BANK (NESSYS_CHR_NTB_WIN_MIN / NESSYS_CHR_BANK_SIZE)

#define NESSYS_CHR_ROM_END_BANK (NESSYS_CHR_ROM_WIN_MAX / NESSYS_CHR_BANK_SIZE)
#define NESSYS_CHR_NTB_END_BANK (NESSYS_CHR_NTB_WIN_MAX / NESSYS_CHR_BANK_SIZE)

// interrupt table addresses
#define NESSYS_NMI_VECTOR 0xFFFA
#define NESSYS_RST_VECTOR 0xFFFC
// also used for BRK
#define NESSYS_IRQ_VECTOR 0xFFFE

// ppu timing - ntsc
#define NESSYS_PPU_PER_CPU_CLK 3
#define NESSYS_PPU_PER_APU_CLK (2 * NESSYS_PPU_PER_CPU_CLK)
#define NESSYS_PPU_CLK_PER_SCANLINE 341
#define NESSYS_PPU_SCANLINES_RENDERED 240
#define NESSYS_PPU_SCANLINES_POST_RENDER 1
#define NESSYS_PPU_SCANLINES_VBLANK 20
#define NESSYS_PPU_SCANLINES_PRE_RENDER 1
#define NESSYS_PPU_SCANLINES_START_RENDER (NESSYS_PPU_SCANLINES_VBLANK + NESSYS_PPU_SCANLINES_PRE_RENDER)
#define NESSYS_PPU_SCANLINES_PER_FRAME (NESSYS_PPU_SCANLINES_RENDERED + NESSYS_PPU_SCANLINES_POST_RENDER + NESSYS_PPU_SCANLINES_VBLANK + NESSYS_PPU_SCANLINES_PRE_RENDER)

#define NESSYS_PPU_SCANLINES_RENDERED_CLKS (NESSYS_PPU_SCANLINES_RENDERED * NESSYS_PPU_CLK_PER_SCANLINE)
#define NESSYS_PPU_SCANLINES_POST_RENDER_CLKS (NESSYS_PPU_SCANLINES_POST_RENDER * NESSYS_PPU_CLK_PER_SCANLINE)
#define NESSYS_PPU_SCANLINES_VBLANK_CLKS (NESSYS_PPU_SCANLINES_VBLANK * NESSYS_PPU_CLK_PER_SCANLINE)
#define NESSYS_PPU_SCANLINES_PRE_RENDER_CLKS (NESSYS_PPU_SCANLINES_PRE_RENDER * NESSYS_PPU_CLK_PER_SCANLINE)
#define NESSYS_PPU_SCANLINES_PER_FRAME_CLKS (NESSYS_PPU_SCANLINES_PER_FRAME * NESSYS_PPU_CLK_PER_SCANLINE)

#define NESSYS_PPU_MAX_SPRITES_PER_SCALINE 8

// 64x3 component entry palette, in float
//static const float NESSYS_PPU_PALETTE[] = {
//	 84/255.0f,  84/255.0f,  84/255.0f,    0/255.0f,  30/255.0f, 116/255.0f,    8/255.0f,  16/255.0f, 144/255.0f,   48/255.0f,   0/255.0f, 136/255.0f,   68/255.0f,   0/255.0f, 100/255.0f,   92/255.0f,   0/255.0f,  48/255.0f,   84/255.0f,   4/255.0f,   0/255.0f,   60/255.0f,  24/255.0f,   0/255.0f,   32/255.0f,  42/255.0f,   0/255.0f,    8/255.0f,  58/255.0f,   0/255.0f,    0/255.0f,  64/255.0f,   0/255.0f,    0/255.0f,  60/255.0f,   0/255.0f,    0/255.0f,  50/255.0f,  60/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,
//	152/255.0f, 150/255.0f, 152/255.0f,    8/255.0f,  76/255.0f, 196/255.0f,   48/255.0f,  50/255.0f, 236/255.0f,   92/255.0f,  30/255.0f, 228/255.0f,  136/255.0f,  20/255.0f, 176/255.0f,  160/255.0f,  20/255.0f, 100/255.0f,  152/255.0f,  34/255.0f,  32/255.0f,  120/255.0f,  60/255.0f,   0/255.0f,   84/255.0f,  90/255.0f,   0/255.0f,   40/255.0f, 114/255.0f,   0/255.0f,    8/255.0f, 124/255.0f,   0/255.0f,    0/255.0f, 118/255.0f,  40/255.0f,    0/255.0f, 102/255.0f, 120/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,
//	236/255.0f, 238/255.0f, 236/255.0f,   76/255.0f, 154/255.0f, 236/255.0f,  120/255.0f, 124/255.0f, 236/255.0f,  176/255.0f,  98/255.0f, 236/255.0f,  228/255.0f,  84/255.0f, 236/255.0f,  236/255.0f,  88/255.0f, 180/255.0f,  236/255.0f, 106/255.0f, 100/255.0f,  212/255.0f, 136/255.0f,  32/255.0f,  160/255.0f, 170/255.0f,   0/255.0f,  116/255.0f, 196/255.0f,   0/255.0f,   76/255.0f, 208/255.0f,  32/255.0f,   56/255.0f, 204/255.0f, 108/255.0f,   56/255.0f, 180/255.0f, 204/255.0f,   60/255.0f,  60/255.0f,  60/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,
//	236/255.0f, 238/255.0f, 236/255.0f,  168/255.0f, 204/255.0f, 236/255.0f,  188/255.0f, 188/255.0f, 236/255.0f,  212/255.0f, 178/255.0f, 236/255.0f,  236/255.0f, 174/255.0f, 236/255.0f,  236/255.0f, 174/255.0f, 212/255.0f,  236/255.0f, 180/255.0f, 176/255.0f,  228/255.0f, 196/255.0f, 144/255.0f,  204/255.0f, 210/255.0f, 120/255.0f,  180/255.0f, 222/255.0f, 120/255.0f,  168/255.0f, 226/255.0f, 144/255.0f,  152/255.0f, 226/255.0f, 180/255.0f,  160/255.0f, 214/255.0f, 228/255.0f,  160/255.0f, 162/255.0f, 160/255.0f,    0/255.0f,   0/255.0f,   0/255.0f,    0/255.0f,   0/255.0f,   0/255.0f
//};

#ifdef WIN32
// Apparently, windows native 16bpp format uses A1R5G5B5 format, instead of the more typical R5G6B5 format
static const uint16_t NESSYS_PPU_PALETTE[] = {
	0x294A, 0x006E, 0x0452, 0x1811, 0x200C, 0x2C06, 0x2800, 0x1C60, 
	0x10A0, 0x04E0, 0x0100, 0x00E0, 0x00C7, 0x0000, 0x0000, 0x0000, 
	0x4E53, 0x4E53, 0x0538, 0x18DD, 0x2C7C, 0x4456, 0x504C, 0x4C84, 
	0x3CE0, 0x2960, 0x15C0, 0x05E0, 0x01C5, 0x018F, 0x0000, 0x0000, 
	0x77BD, 0x267D, 0x3DFD, 0x599D, 0x715D, 0x7576, 0x75AC, 0x6A24, 
	0x52A0, 0x3B00, 0x2744, 0x1F2D, 0x1ED9, 0x1CE7, 0x0000, 0x0000, 
	0x77BD, 0x573D, 0x5EFD, 0x6ADD, 0x76BD, 0x76BA, 0x76D6, 0x7312, 
	0x674F, 0x5B6F, 0x5792, 0x4F96, 0x535C, 0x5294, 0x0000, 0x0000
};
#else
static const uint16_t NESSYS_PPU_PALETTE[] = {
	0x52AA, 0x00AE, 0x0030, 0x282F, 0x5809, 0x7002, 0x6800, 0x4840,
	0x10C0, 0x0140, 0x0180, 0x0161, 0x0128, 0x0000, 0x0000, 0x0000,
	0xA534, 0x02B8, 0x21FC, 0x695B, 0xA8D4, 0xD0AB, 0xD100, 0xA1A0,
	0x6280, 0x1B20, 0x0380, 0x0386, 0x0350, 0x0000, 0x0000, 0x0000,
	0xFFFF, 0x2D5F, 0x5C1F, 0x9B9F, 0xF39F, 0xFBB7, 0xFBEE, 0xFC45,
	0xCD00, 0x85C0, 0x3E46, 0x166F, 0x0E3A, 0x39E7, 0x0000, 0x0000,
	0xFFFF, 0xA6FF, 0xB65F, 0xCDFF, 0xF61F, 0xFE3D, 0xFE39, 0xFE75,
	0xEEB2, 0xD712, 0xB734, 0x9F58, 0xA10C, 0xAD75, 0x0000, 0x0000
};
#endif

//static const uint16_t NESSYS_PPU_PALETTE[] = {
//	0x52AA, 0x00EE, 0x0892, 0x3011, 0x400C, 0x5806, 0x5020, 0x38C0,
//	0x2140, 0x09C0, 0x0200, 0x01E0, 0x0187, 0x0000, 0x0000, 0x0000,
//	0x9CB3, 0x9CB3, 0x0A78, 0x319D, 0x58FC, 0x88B6, 0xA0AC, 0x9904,
//	0x79E0, 0x52C0, 0x2B80, 0x0BE0, 0x03A5, 0x032F, 0x0000, 0x0000,
//	0xEF7D, 0x4CDD, 0x7BFD, 0xB31D, 0xE2BD, 0xEAD6, 0xEB4C, 0xD444,
//	0xA540, 0x7620, 0x4E84, 0x3E6D, 0x3DB9, 0x39E7, 0x0000, 0x0000,
//	0xEF7D, 0xAE7D, 0xBDFD, 0xD59D, 0xED7D, 0xED7A, 0xEDB6, 0xE632,
//	0xCE8F, 0xB6EF, 0xAF12, 0x9F16, 0xA6BC, 0xA514, 0x0000, 0x0000
//};

#define NESSYS_APU_STATUS_OFFSET 0x15
#define NESSYS_APU_JOYPAD0_OFFSET 0x16
#define NESSYS_APU_JOYPAD1_OFFSET 0x17
#define NESSYS_APU_FRAME_COUNTER_OFFSET 0x17

#define NESSYS_STD_CONTROLLER_BUTTON_A      0x0
#define NESSYS_STD_CONTROLLER_BUTTON_B      0x1
#define NESSYS_STD_CONTROLLER_BUTTON_SELECT 0x2
#define NESSYS_STD_CONTROLLER_BUTTON_START  0x3
#define NESSYS_STD_CONTROLLER_BUTTON_UP     0x4
#define NESSYS_STD_CONTROLLER_BUTTON_DOWN   0x5
#define NESSYS_STD_CONTROLLER_BUTTON_LEFT   0x6
#define NESSYS_STD_CONTROLLER_BUTTON_RIGHT  0x7

#define NESSYS_STD_CONTROLLER_BUTTON_A_MASK      (1 << NESSYS_STD_CONTROLLER_BUTTON_A)
#define NESSYS_STD_CONTROLLER_BUTTON_B_MASK      (1 << NESSYS_STD_CONTROLLER_BUTTON_B)
#define NESSYS_STD_CONTROLLER_BUTTON_SELECT_MASK (1 << NESSYS_STD_CONTROLLER_BUTTON_SELECT)
#define NESSYS_STD_CONTROLLER_BUTTON_START_MASK  (1 << NESSYS_STD_CONTROLLER_BUTTON_START)
#define NESSYS_STD_CONTROLLER_BUTTON_UP_MASK     (1 << NESSYS_STD_CONTROLLER_BUTTON_UP)
#define NESSYS_STD_CONTROLLER_BUTTON_DOWN_MASK   (1 << NESSYS_STD_CONTROLLER_BUTTON_DOWN)
#define NESSYS_STD_CONTROLLER_BUTTON_LEFT_MASK   (1 << NESSYS_STD_CONTROLLER_BUTTON_LEFT)
#define NESSYS_STD_CONTROLLER_BUTTON_RIGHT_MASK  (1 << NESSYS_STD_CONTROLLER_BUTTON_RIGHT)

#define NESSYS_SND_SAMPLES_PER_SECOND 44100
#define NESSYS_SND_BITS_PER_SAMPLE 16
#define NESSYS_SND_BUFFERS 16
#define NESSYS_SND_SAMPLES ((NESSYS_SND_BUFFERS * NESSYS_SND_SAMPLES_PER_SECOND) / 60)
#define NESSYS_SND_BYTES ((NESSYS_SND_BITS_PER_SAMPLE * NESSYS_SND_SAMPLES) / 8)
#define NESSYS_SND_BYTES_SKEW 20
#define NESSYS_SND_SAMPLES_PER_BUFFER (NESSYS_SND_SAMPLES / NESSYS_SND_BUFFERS)
#define NESSYS_SND_BYTES_PER_BUFFER ((NESSYS_SND_BITS_PER_SAMPLE * NESSYS_SND_SAMPLES_PER_BUFFER) / 8)
#define NESSYS_SND_START_POSITION ((NESSYS_SND_BUFFERS / 2) * NESSYS_SND_BYTES_PER_BUFFER)

// samples are counted in fixed point (12.20)
#define NESSYS_SND_SAMPLES_FRAC_LOG2 20
#define NESSYS_SND_SAMPLES_FRAC (1 << NESSYS_SND_SAMPLES_FRAC_LOG2)
#define NESSYS_SND_SAMPLES_FRAC_MASK (NESSYS_SND_SAMPLES_FRAC - 1)
#define NESSYS_SND_SAMPLES_FRAC_PER_CYCLE ((NESSYS_SND_SAMPLES_PER_BUFFER << NESSYS_SND_SAMPLES_FRAC_LOG2) / NESSYS_PPU_SCANLINES_PER_FRAME_CLKS)

#define NESSYS_SND_APU_FRAC_LOG2 10
#define NESSYS_SND_APU_FRAC (1 << NESSYS_SND_APU_FRAC_LOG2)
#define NESSYS_SND_APU_FRAC_MASK (NESSYS_SND_APU_FRAC - 1)
#define NESSYS_SND_APU_CLKS_PER_FRAME (NESSYS_PPU_SCANLINES_PER_FRAME_CLKS / NESSYS_PPU_PER_APU_CLK)
#define NESSYS_SND_APU_FRAC_PER_FRAME (NESSYS_SND_APU_CLKS_PER_FRAME << NESSYS_SND_APU_FRAC_LOG2)
#define NESSYS_SND_APU_FRAC_PER_SAMPLE (NESSYS_SND_APU_FRAC_PER_FRAME / NESSYS_SND_SAMPLES_PER_BUFFER)
#define NESSYS_SND_CPU_CLKS_PER_FRAME (NESSYS_PPU_SCANLINES_PER_FRAME_CLKS / NESSYS_PPU_PER_CPU_CLK)
#define NESSYS_SND_CPU_FRAC_PER_FRAME (NESSYS_SND_CPU_CLKS_PER_FRAME << NESSYS_SND_APU_FRAC_LOG2)
#define NESSYS_SND_CPU_FRAC_PER_SAMPLE (NESSYS_SND_CPU_FRAC_PER_FRAME / NESSYS_SND_SAMPLES_PER_BUFFER)

#define NESSYS_SND_FRAME_FRAC_LOG2 20
#define NESSYS_SND_FRAME_FRAC (1 << NESSYS_SND_FRAME_FRAC_LOG2)
#define NESSYS_SND_FRAME_FRAC_MASK (NESSYS_SND_FRAME_FRAC - 1)
#define NESSYS_SND_FRAME_FRAC_PER_SAMPLE ((4 << NESSYS_SND_FRAME_FRAC_LOG2) / NESSYS_SND_SAMPLES_PER_BUFFER)

// system structs
typedef struct {
	uint16_t pc;
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint8_t s;
	uint8_t p;
	uint8_t pad0;
} nessys_cpu_regs_t;

#define NESSYS_APU_PULSE_FLAG_SWEEP_ONES_COMP_BIT 0
#define NESSYS_APU_PULSE_FLAG_ENV_START_BIT 1
#define NESSYS_APU_PULSE_FLAG_SWEEP_NEGATE_BIT 3
#define NESSYS_APU_PULSE_FLAG_CONST_VOLUME_BIT 4
#define NESSYS_APU_PULSE_FLAG_HALT_LENGTH_BIT 5
#define NESSYS_APU_PULSE_FLAG_SWEEP_RELOAD_BIT 6
#define NESSYS_APU_PULSE_FLAG_SWEEP_EN_BIT 7

#define NESSYS_APU_PULSE_FLAG_SWEEP_ONES_COMP (1 << NESSYS_APU_PULSE_FLAG_SWEEP_ONES_COMP_BIT)
#define NESSYS_APU_PULSE_FLAG_ENV_START (1 << NESSYS_APU_PULSE_FLAG_ENV_START_BIT)
#define NESSYS_APU_PULSE_FLAG_SWEEP_NEGATE (1 << NESSYS_APU_PULSE_FLAG_SWEEP_NEGATE_BIT)
#define NESSYS_APU_PULSE_FLAG_CONST_VOLUME (1 << NESSYS_APU_PULSE_FLAG_CONST_VOLUME_BIT)
#define NESSYS_APU_PULSE_FLAG_HALT_LENGTH (1 << NESSYS_APU_PULSE_FLAG_HALT_LENGTH_BIT)
#define NESSYS_APU_PULSE_FLAG_SWEEP_RELOAD (1 << NESSYS_APU_PULSE_FLAG_SWEEP_RELOAD_BIT)
#define NESSYS_APU_PULSE_FLAG_SWEEP_EN (1 << NESSYS_APU_PULSE_FLAG_SWEEP_EN_BIT)

#define NESSYS_APU_TRIANGLE_FLAG_RELOAD_BIT 0
#define NESSYS_APU_TRIANGLE_FLAG_CONTROL_BIT 7

#define NESSYS_APU_TRIANGLE_FLAG_RELOAD (1 << NESSYS_APU_TRIANGLE_FLAG_RELOAD_BIT)
#define NESSYS_APU_TRIANGLE_FLAG_CONTROL (1 << NESSYS_APU_TRIANGLE_FLAG_CONTROL_BIT)

#define NESSYS_APU_NOISE_FLAG_MODE_BIT 7
#define NESSYS_APU_NOISE_FLAG_MODE (1 << NESSYS_APU_NOISE_FLAG_MODE_BIT)

#define NESSYS_APU_DMC_FLAG_IRQ_ENABLE_BIT 7
#define NESSYS_APU_DMC_FLAG_LOOP_BIT 6
#define NESSYS_APU_DMC_FLAG_DMA_ENABLE_BIT 4

#define NESSYS_APU_DMC_FLAG_IRQ_ENABLE (1 << NESSYS_APU_DMC_FLAG_IRQ_ENABLE_BIT)
#define NESSYS_APU_DMC_FLAG_LOOP (1 << NESSYS_APU_DMC_FLAG_LOOP_BIT)
#define NESSYS_APU_DMC_FLAG_DMA_ENABLE (1 << NESSYS_APU_DMC_FLAG_DMA_ENABLE_BIT)

static const uint8_t NESSYS_APU_PULSE_DUTY_TABLE[4] = { 0x02, 0x06, 0x1e, 0xf9 };

static const uint8_t NESSYS_APU_PULSE_LENGTH_TABLE[32] = { 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
													12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };

static const uint16_t NESSYS_APU_NOISE_PERIOD_TABLE[16] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };

static const uint16_t NESSYS_APU_DMC_PERIOD_TABLE[16] = { 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54 };

#define NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE 0x01

#define NESSYS_MAPPER_SETUP_DEFAULT 0x00
#define NESSYS_MAPPER_SETUP_DRAW_INCOMPLETE 0x01
#define NESSYS_MAPPER_SETUP_CUSTOM 0x2

#define NESSYS_MAX_MID_SCAN_NTB_BANK_CHANGES 4

typedef struct {
	uint8_t volume;
	uint8_t divider;
	uint8_t decay;
	uint8_t flags;
} nessys_apu_envelope_t;

typedef struct {
	uint32_t cur_time_frac;
	uint16_t period;
	uint8_t duty;
	uint8_t duty_phase;
	nessys_apu_envelope_t env;
	uint8_t length;
	uint8_t sweep_period;
	uint8_t sweep_shift;
	uint8_t sweep_divider;
	uint16_t sweep_orig_period;
	uint16_t pad;
} nessys_apu_pulse_t;

typedef struct {
	uint32_t cur_time_frac;
	uint16_t period;
	uint8_t length;
	uint8_t linear;
	uint8_t reload;
	uint8_t sequence;
	uint8_t flags;
	uint8_t pad;
} nessys_apu_triangle_t;

typedef struct {
	uint32_t cur_time_frac;
	uint16_t period;
	uint16_t shift_reg;
	nessys_apu_envelope_t env;
	uint8_t length;
	uint8_t pad[3];
} nessys_apu_noise_t;

typedef struct {
	uint32_t cur_time_frac;
	uint16_t start_addr;
	uint16_t length;
	uint16_t cur_addr;
	uint16_t bytes_remaining;
	uint16_t period;
	uint16_t pad;
	uint8_t delta_buffer;
	uint8_t output;
	uint8_t flags;
	uint8_t bits_remaining;
} nessys_apu_dmc_t;

typedef struct {
	uint8_t reg_mem[NESSYS_APU_SIZE];
	uint8_t joy_control;
	uint8_t frame_counter;
	uint8_t status;
	uint8_t pad1;
	uint8_t joypad[2];
	uint8_t latched_joypad[2];
	uint32_t sample_frac_generated;
	uint32_t frame_frac_counter;
	nessys_apu_pulse_t pulse[2];
	nessys_apu_triangle_t triangle;
	nessys_apu_noise_t noise;
	nessys_apu_dmc_t dmc;
	uint8_t* reg;
} nessys_apu_regs_t;

typedef struct {
	uint8_t reg[NESSYS_PPU_REG_SIZE];
	uint8_t status;
	uint8_t scroll[2];
	uint8_t mem[NESSYS_PPU_MEM_SIZE];
	uint8_t oam[NESSYS_PPU_OAM_SIZE];
	uint8_t pal[NESSYS_PPU_PAL_SIZE];
	uint16_t oam_pix[NESSYS_PPU_OAM_PIXEL_SIZE];
	uint16_t tile_pix[2 * NESSYS_PPU_TILE_PIXEL_SIZE];
	uint8_t attrib_pix[2 * NESSYS_PPU_ATTRIB_BYTES_PER_ROW];
	uint16_t* draw_tile_pix;
	uint16_t* disp_tile_pix;
	uint8_t* draw_attrib_pix;
	uint8_t* disp_attrib_pix;
	//const uint8_t* scan_line_oam[NESSYS_PPU_SCAN_LINE_OAM_SIZE];
	uint8_t scan_line_sprite[256];
	uint8_t scan_line_min_sprite_x;  // minimum x position of any sprite in this scanline
	uint8_t scan_line_max_sprite_x;  // maximum x position of any sprite in this scanline
	uint16_t scroll_y;
	uint16_t mem_addr;
	uint16_t t_mem_addr;
	uint8_t addr_toggle;
	//uint8_t num_scan_line_oam;
	bool scroll_y_changed;
	bool name_tbl_vert_mirror;
	uint32_t chr_rom_size;
	uint32_t chr_ram_size;
	const uint8_t* chr_rom_base;
	uint8_t* chr_ram_base;
	uint16_t chr_rom_bank_mask[NESSYS_CHR_NUM_BANKS];
	const uint8_t* chr_rom_bank[NESSYS_CHR_NUM_BANKS];
	uint16_t chr_ram_bank_mask[NESSYS_CHR_NUM_BANKS];
	uint8_t* chr_ram_bank[NESSYS_CHR_NUM_BANKS];
	uint8_t* mem_4screen;
} nessys_ppu_t;

#define NESSYS_NUM_CPU_BACKTRACE_ENTRIES 16
typedef struct {
	int32_t scanline;
	int32_t scanline_cycle;
	nessys_cpu_regs_t reg;
	uint32_t sprite0_hit_cycles;
} nessys_cpu_backtrace_t;

#define NESSYS_STACK_TRACE_ENTRIES 4
typedef struct {
	int32_t scanline;
	int32_t scanline_cycle;
	uint32_t frame;
	uint16_t jump_addr;
	uint16_t return_addr;
} nessys_stack_trace_entry_t;

typedef struct {
	uint tile_x;
	uint8_t pal_base, sp_pal_base;
	uint16_t attr_bits;
	uint16_t pat_planes, sp_pat_planes;
	//uint sprite_x, sprite_y;
	uint sprite_index;
	//int sprite_x_inc;
	uint8_t sprite_plane_shift;
	uint8_t sprite_h_flip;
	bool sprite_background;
} render_state_t;

typedef struct {
	uint32_t mapper_id;
	uint32_t (*mapper_bg_setup)(uint32_t phase);
	uint32_t (*mapper_sprite_setup)(uint32_t phase);
	void (*mapper_cpu_setup)();
	void (*mapper_audio_tick)();
	int16_t(*mapper_gen_sound)();
	uint8_t* (*mapper_read)(uint16_t addr);
	bool (*mapper_write)(uint16_t addr, uint8_t data);
	bool (*mapper_update)();
	void* mapper_data;
	uint32_t cycle;
	uint32_t frame;
	uint32_t rendered_frames;
	uint32_t rendered_time;
	volatile int frame_delta_time;
	volatile uint32_t scan_line;
	volatile uint32_t scan_clk;
	volatile uint32_t rendered_scan_clk;
	render_state_t c0_rstate;
	render_state_t c1_rstate;
	volatile bool c1_render_done;
	uint sprite0_hit_scan_clk;
	bool vblank_irq;
	bool mapper_irq;
	bool frame_irq;
	bool dmc_irq;
	uint32_t dmc_bits_to_play;
	uint32_t dmc_bit_timer;
	uint32_t dmc_buffer_full;
	int32_t scanline;  // scanline number
	int32_t scanline_cycle;  // cycles after the start of current scanline
	uint32_t cpu_cycle_inc;
	uint8_t in_nmi;
	uint8_t mapper_flags;
	uint8_t iflag_delay;  // if set, the polarity of iflag is reversed for 1 instruction
	uint8_t pad0[1];
	nessys_cpu_regs_t reg;
	nessys_apu_regs_t apu;
	nessys_ppu_t ppu;
	uint32_t prg_rom_size;
	uint32_t prg_ram_size;
	uint8_t sysmem[NESSYS_RAM_SIZE];
	const uint8_t* prg_rom_base;
	uint8_t* prg_ram_base;
	uint16_t prg_rom_bank_mask[NESSYS_PRG_NUM_BANKS];
	const uint8_t* prg_rom_bank[NESSYS_PRG_NUM_BANKS];
#ifdef _DEBUG
	uint32_t backtrace_entry;
	uint32_t stack_trace_entry;
	uint32_t irq_trace_entry;
	nessys_cpu_backtrace_t backtrace[NESSYS_NUM_CPU_BACKTRACE_ENTRIES];
	nessys_stack_trace_entry_t stack_trace[NESSYS_STACK_TRACE_ENTRIES];
	nessys_stack_trace_entry_t irq_trace[NESSYS_STACK_TRACE_ENTRIES];
#endif
	//nesmenu_data menu;
	//uint32_t num_joy;
	//nesjoy_data joy_data[2];
} nessys_t;

#include "c6502.h"
#include "ines.h"
#include "nesmenu.h"

extern nessys_t nes;

void nessys_apu_env_tick(nessys_apu_envelope_t* envelope);
void nessys_apu_tri_linear_tick(nessys_apu_triangle_t* triangle);
void nessys_apu_tri_length_tick(nessys_apu_triangle_t* triangle);
void nessys_apu_noise_length_tick(nessys_apu_noise_t* noise);
void nessys_apu_sweep_tick(nessys_apu_pulse_t* pulse);
void nessys_gen_sound(nessys_t* nes);

uint8_t nessys_apu_gen_pulse(nessys_apu_pulse_t* pulse);
uint8_t nessys_apu_gen_triangle(nessys_apu_triangle_t* triangle);
uint8_t nessys_apu_gen_noise(nessys_apu_noise_t* noise);
uint8_t nessys_apu_gen_dmc(nessys_t* nes);

void nessys_init();
void nessys_power_cycle();
void nessys_reset();
bool nessys_load_cart(const void* cart);
bool nessys_init_mapper();
void nessys_default_memmap();
void nessys_irq(uint16_t irq_vector, uint8_t clear_flag);
void nessys_gen_oam_pix(uint8_t sprite_index);
void nessys_gen_tile_pix(uint y);
void nessys_cleanup_mapper();
void nessys_unload_cart();
void nessys_cleanup();

void process_ppu();

//uint8_t* nessys_ram(uint16_t addr);
//const uint8_t* nessys_mem(uint16_t addr, uint16_t* bank, uint16_t* offset);
//const uint8_t* nessys_ppu_mem(uint16_t addr);
//uint8_t* nessys_ppu_ram(uint16_t addr);

static inline uint8_t* nessys_ram(uint16_t addr)
{
	return nes.sysmem + (addr & NESSYS_RAM_MASK);
}

static inline const uint8_t* nessys_mem(uint16_t addr, uint16_t* bank, uint16_t* offset)
{
	uint16_t b = addr >> NESSYS_PRG_BANK_SIZE_LOG2;
	uint16_t o = addr & nes.prg_rom_bank_mask[b];
	*bank = b;
	*offset = o;
	return nes.prg_rom_bank[b] + o;
}

static inline const uint8_t* nessys_ppu_mem(uint16_t addr)
{
	if (addr >= NESSYS_CHR_PAL_WIN_MIN) return nes.ppu.pal + (addr & NESSYS_PPU_PAL_MASK);
	uint16_t b = addr >> NESSYS_CHR_BANK_SIZE_LOG2;
	return nes.ppu.chr_rom_bank[b] + (addr & nes.ppu.chr_rom_bank_mask[b]);
}

static inline uint8_t* nessys_ppu_ram(uint16_t addr)
{
	if (addr >= NESSYS_CHR_PAL_WIN_MIN) return nes.ppu.pal + (addr & NESSYS_PPU_PAL_MASK);
	uint16_t b = addr >> NESSYS_CHR_BANK_SIZE_LOG2;
	return nes.ppu.chr_ram_bank[b] + (addr & nes.ppu.chr_ram_bank_mask[b]);
}

static inline uint8_t nessys_get_scan_position()
{
	uint32_t position = nes.scanline_cycle + 28;
	position &= ~0x7;
	position -= nes.ppu.scroll[0];
	if (position >= 256 || nes.scanline < 0) position = 0;
	return (uint8_t)position;
}

#endif
