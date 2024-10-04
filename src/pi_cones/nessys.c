// nessys.c
// system emulation of NES

#include "nessys.h"

void nessys_init()
{
	memset(&nes, 0, sizeof(nessys_t));
	nes.apu.reg = nes.apu.reg_mem;
	nes.apu.pulse[0].env.flags = NESSYS_APU_PULSE_FLAG_SWEEP_ONES_COMP;
	nes.apu.noise.shift_reg = 0x01;
	nes.apu.dmc.period = NESSYS_APU_DMC_PERIOD_TABLE[0];
	nes.ppu.draw_tile_pix = nes.ppu.tile_pix;
	nes.ppu.disp_tile_pix = nes.ppu.tile_pix + NESSYS_PPU_TILE_PIXEL_SIZE;
	nes.ppu.draw_attrib_pix = nes.ppu.attrib_pix;
	nes.ppu.disp_attrib_pix = nes.ppu.attrib_pix + NESSYS_PPU_ATTRIB_BYTES_PER_ROW;
}

void nessys_apu_reset()
{
	nes.apu.pulse[0].length = 0;
	nes.apu.pulse[1].length = 0;
	nes.apu.triangle.length = 0;
	nes.apu.noise.length = 0;
	nes.apu.noise.shift_reg = 0x01;
	nes.apu.dmc.length = 0;
	nes.apu.dmc.bytes_remaining = 0;
	nes.apu.dmc.bits_remaining = 0;
	nes.apu.dmc.flags = 0;
}

void nessys_power_cycle()
{
	nes.reg.p = 0x34;
	nes.reg.a = 0;
	nes.reg.x = 0;
	nes.reg.y = 0;
	nes.reg.s = 0xfd;
	nes.apu.reg[0x17] = 0x0;
	nes.apu.reg[0x15] = 0x0;
	memset(nes.apu.reg, 0, 14); // regs 0x0 to 0x13
	memset(nes.ppu.reg, 0, 8);  // clear all 8 regs
	memset(nes.sysmem, 0, NESSYS_RAM_SIZE);
	uint16_t bank, offset;
	nes.reg.pc = *((uint16_t*)nessys_mem(NESSYS_RST_VECTOR, &bank, &offset));
	nessys_apu_reset();
}

void nessys_reset()
{
	nes.reg.s -= 3;
	nes.reg.p |= 0x04;
	nes.apu.reg[0x15] = 0x0;
	nes.ppu.reg[0x0] = 0x0;
	nes.ppu.reg[0x1] = 0x0;
	nes.ppu.reg[0x5] = 0x0;
	nes.ppu.reg[0x6] = 0x0;
	nes.ppu.reg[0x7] = 0x0;
	uint16_t bank, offset;
	nes.reg.pc = *((uint16_t*)nessys_mem(NESSYS_RST_VECTOR, &bank, &offset));
	nessys_apu_reset();
}

bool nessys_load_cart(const void* cart)
{
	bool success;
	nessys_unload_cart();
	success = ines_load_cart(cart);
	if (success) {
		nessys_default_memmap();
		success = nessys_init_mapper();
	}
	if (success) {
		nessys_power_cycle();
	} else {
		nessys_unload_cart();
	}

	return success;
}

void nessys_default_memmap()
{
	nes.prg_rom_bank[NESSYS_SYS_RAM_START_BANK] = nes.sysmem;
	nes.prg_rom_bank_mask[NESSYS_SYS_RAM_START_BANK] = NESSYS_RAM_MASK;

	nes.prg_rom_bank[NESSYS_PPU_REG_START_BANK] = nes.ppu.reg;
	nes.prg_rom_bank_mask[NESSYS_PPU_REG_START_BANK] = NESSYS_PPU_REG_MASK;

	nes.prg_rom_bank[NESSYS_APU_REG_START_BANK] = nes.apu.reg;
	nes.prg_rom_bank_mask[NESSYS_APU_REG_START_BANK] = NESSYS_APU_MASK;

	if (nes.ppu.chr_ram_base) {
		nes.prg_rom_bank[NESSYS_PRG_RAM_START_BANK] = nes.ppu.chr_ram_base;
		nes.prg_rom_bank_mask[NESSYS_PRG_RAM_START_BANK] = NESSYS_PRG_MEM_MASK;
	} else {
		// point to some junk location
		nes.prg_rom_bank[NESSYS_PRG_RAM_START_BANK] = &nes.reg.pad0;
		nes.prg_rom_bank_mask[NESSYS_PRG_RAM_START_BANK] = 0x0;
	}

	int b;
	uint32_t mem_offset = 0;
	if (nes.prg_rom_base) {
		// map the last 32KB of rom data
		// if rom is less than 32KB, then map from the beggining, and wrap the addresses
		mem_offset = (nes.prg_rom_size <= NESSYS_PRG_ADDR_SPACE - NESSYS_PRG_ROM_START) ?
			0 : (nes.prg_rom_size - (NESSYS_PRG_ADDR_SPACE - NESSYS_PRG_ROM_START));
		for (b = NESSYS_PRG_ROM_START_BANK; b < NESSYS_PRG_NUM_BANKS; b++) {
			nes.prg_rom_bank[b] = nes.prg_rom_base + mem_offset;
			nes.prg_rom_bank_mask[b] = NESSYS_PRG_MEM_MASK;
			mem_offset += NESSYS_PRG_BANK_SIZE;
			if (mem_offset >= nes.prg_rom_size) mem_offset -= nes.prg_rom_size;
		}
	} else {
		for (b = NESSYS_PRG_ROM_START_BANK; b < NESSYS_PRG_NUM_BANKS; b++) {
			nes.prg_rom_bank[b] = &nes.reg.pad0;
			nes.prg_rom_bank_mask[b] = 0x0;
		}
	}
	if (nes.prg_ram_base) {
		mem_offset = 0;
		for (b = NESSYS_PRG_RAM_START_BANK; b <= NESSYS_PRM_RAM_END_BANK; b++) {
			nes.prg_rom_bank[b] = nes.prg_ram_base + mem_offset;
			nes.prg_rom_bank_mask[b] = NESSYS_PRG_MEM_MASK;
			mem_offset += NESSYS_PRG_BANK_SIZE;
		}
	} else {
		for (b = NESSYS_PRG_RAM_START_BANK; b <= NESSYS_PRM_RAM_END_BANK; b++) {
			nes.prg_rom_bank[b] = &nes.reg.pad0;
			nes.prg_rom_bank_mask[b] = 0x0;
		}
	}
	if (nes.ppu.chr_rom_base || nes.ppu.chr_ram_base) {
		mem_offset = 0;
		const uint8_t* base = (nes.ppu.chr_ram_base) ? nes.ppu.chr_ram_base : nes.ppu.chr_rom_base;
		uint32_t size = (nes.ppu.chr_ram_base) ? nes.ppu.chr_ram_size : nes.ppu.chr_rom_size;
		for (b = 0; b <= NESSYS_CHR_ROM_END_BANK; b++) {
			nes.ppu.chr_rom_bank[b] = base + mem_offset;
			nes.ppu.chr_rom_bank_mask[b] = NESSYS_CHR_MEM_MASK;
			if (nes.ppu.chr_ram_base) {
				nes.ppu.chr_ram_bank[b] = nes.ppu.chr_ram_base + mem_offset;
				nes.ppu.chr_ram_bank_mask[b] = NESSYS_CHR_MEM_MASK;
			} else {
				nes.ppu.chr_ram_bank[b] = &nes.reg.pad0;
				nes.ppu.chr_ram_bank_mask[b] = 0x0;
			}
			mem_offset += NESSYS_CHR_BANK_SIZE;
			if (mem_offset >= size) mem_offset -= size;
		}
	} else {
		for (b = 0; b <= NESSYS_CHR_ROM_END_BANK; b++) {
			nes.ppu.chr_rom_bank[b] = &nes.reg.pad0;
			nes.ppu.chr_rom_bank_mask[b] = 0x0;
			nes.ppu.chr_ram_bank[b] = &nes.reg.pad0;
			nes.ppu.chr_ram_bank_mask[b] = 0x0;
		}
	}
	// map name table
	if (nes.ppu.mem_4screen) {
		// if we allocated space for 4 screens, then directly map address space
		nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 0] = nes.ppu.mem;
		nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 1] = nes.ppu.mem + 0x400;
		nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 2] = nes.ppu.mem_4screen;
		nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 3] = nes.ppu.mem_4screen + 0x400;
		nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 0] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 0];
		nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 1] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 1];
		nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 2] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 2];
		nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 3] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 3];
	} else {
		nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 0] = nes.ppu.mem;
		nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 3] = nes.ppu.mem + 0x400;
		nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 0] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 0];
		nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 3] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 3];
		if (nes.ppu.name_tbl_vert_mirror) {
			nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 1] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 3];
			nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 2] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 0];
			nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 1] = nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 3];
			nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 2] = nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 0];
		} else {
			nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 1] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 0];
			nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 2] = nes.ppu.chr_ram_bank[NESSYS_CHR_NTB_START_BANK + 3];
			nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 1] = nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 0];
			nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 2] = nes.ppu.chr_rom_bank[NESSYS_CHR_NTB_START_BANK + 3];
		}
	}
	for (b = NESSYS_CHR_NTB_START_BANK + 4; b <= NESSYS_CHR_NTB_END_BANK; b++) {
		nes.ppu.chr_ram_bank[b] = nes.ppu.chr_ram_bank[b - 4];
		nes.ppu.chr_rom_bank[b] = nes.ppu.chr_rom_bank[b - 4];
	}
	for (b = NESSYS_CHR_NTB_START_BANK; b <= NESSYS_CHR_NTB_END_BANK; b++) {
		nes.ppu.chr_ram_bank_mask[b] = NESSYS_CHR_MEM_MASK;
		nes.ppu.chr_rom_bank_mask[b] = NESSYS_CHR_MEM_MASK;
	}
}

void nessys_irq(uint16_t irq_vector, uint8_t clear_flag)
{
	uint8_t* ram_ptr;
	uint16_t bank, offset;
	// get the stack base
	ram_ptr = nessys_ram(0x100);
	*(ram_ptr + nes.reg.s) = nes.reg.pc >> 8;         nes.reg.s--;
	*(ram_ptr + nes.reg.s) = nes.reg.pc & 0xFF;       nes.reg.s--;
	*(ram_ptr + nes.reg.s) = nes.reg.p & ~clear_flag; nes.reg.s--;
#ifdef _DEBUG
	// TODO: fix the stack tracing code
	nes.stack_trace[nes.stack_trace_entry].scanline = nes.scanline;
	nes.stack_trace[nes.stack_trace_entry].scanline_cycle = nes.scanline_cycle;
	nes.stack_trace[nes.stack_trace_entry].frame = nes.frame;
	nes.stack_trace[nes.stack_trace_entry].return_addr = nes.reg.pc;
#endif
	nes.reg.pc = *((uint16_t*)nessys_mem(irq_vector, &bank, &offset));
	if (irq_vector == NESSYS_NMI_VECTOR) {
		nes.reg.p |= C6502_P_I;
		nes.in_nmi++;
		//nes.ppu.old_status = (nes.ppu.status & 0xe0);
		//nes.ppu.old_status |= (nes.ppu.reg[2] & 0x1f);
	}
#ifdef _DEBUG
			// TODO: fix irq tracing code
	nes.stack_trace[nes.stack_trace_entry].jump_addr = nes.reg.pc;
	nes.irq_trace[nes.irq_trace_entry] = nes.stack_trace[nes.stack_trace_entry];
	nes.stack_trace_entry++;
	if (nes.stack_trace_entry >= NESSYS_STACK_TRACE_ENTRIES) nes.stack_trace_entry = 0;
	nes.irq_trace_entry++;
	if (nes.irq_trace_entry >= NESSYS_STACK_TRACE_ENTRIES) nes.irq_trace_entry = 0;
#endif

}

void nessys_gen_oam_pix(uint8_t sprite_index)
{
	uint y, sprite_y;
	uint max_y = (nes.ppu.reg[0] & 0x10) ? 16 : 8;
	uint pat_addr;
	uint32_t pat_planes;
	pat_addr = nes.ppu.oam[4 * sprite_index + 1];
	// for 8x16 sprites, lsb is the bank select
	// otherwise, bank select is bit 3 ppu reg0
	pat_addr |= 0x100 & (((pat_addr << 8) & (((uint16_t)nes.ppu.reg[0]) << 3)) |
		((((uint16_t)nes.ppu.reg[0]) << 5) & ~(((uint16_t)nes.ppu.reg[0]) << 3)));
	// clear out lsb for 8x16, and fill it in based on whether sprite y is 8 or above
	pat_addr &= 0xfffe | ~(((uint16_t)nes.ppu.reg[0]) >> 5);
	//pat_addr |= (((uint16_t)nes.ppu.reg[0]) >> 5) & (sprite_y >= 8);
	// shift up the address and fill in the y offset
	pat_addr <<= 4;
	bool v_flip = ((nes.ppu.oam[4 * sprite_index + 2] & 0x80) != 0);
	bool h_flip = ((nes.ppu.oam[4 * sprite_index + 2] & 0x40) != 0);
	sprite_y = (v_flip) ? max_y - 2 : 0;
	uint32_t* oam_pix = (uint32_t*)(nes.ppu.oam_pix + NESSYS_PPU_OAM_PIXEL_ROWS * sprite_index + sprite_y);
	for (y = 0; y < max_y; y+=2) {
		// for y >= 8, set bit 4 of pat_addr; only applies to 8x16 sprites
		pat_addr |= (y << 1) & 0x10;

		// get the pattern plane bits
		if (v_flip) {
			pat_planes = (*(nessys_ppu_mem(pat_addr)) << 16);
			pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x8)) << 24);
			pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x1)));
			pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x9)) << 8);
		} else {
			pat_planes = *(nessys_ppu_mem(pat_addr));
			pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x8)) << 8);
			pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x1)) << 16);
			pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x9)) << 24);
		}

		// interleave and reverse the the 2 bytes
		if (h_flip) {
			pat_planes = (pat_planes & 0xf00ff00f) | ((pat_planes & 0x0f000f00) >> 4) | ((pat_planes & 0x00f000f0) << 4);
			pat_planes = (pat_planes & 0xc3c3c3c3) | ((pat_planes & 0x30303030) >> 2) | ((pat_planes & 0x0c0c0c0c) << 2);
			pat_planes = (pat_planes & 0x99999999) | ((pat_planes & 0x44444444) >> 1) | ((pat_planes & 0x22222222) << 1);
		} else {
			pat_planes = (pat_planes & 0x0ff00ff0) | ((pat_planes & 0xf000f000) >> 12) | ((pat_planes & 0x000f000f) << 12);
			pat_planes = (pat_planes & 0x3c3c3c3c) | ((pat_planes & 0xc0c0c0c0) >> 6) | ((pat_planes & 0x03030303) << 6);
			pat_planes = (pat_planes & 0x66666666) | ((pat_planes & 0x88888888) >> 3) | ((pat_planes & 0x11111111) << 3);
			pat_planes = ((pat_planes & 0xaaaaaaaa) >> 1) | ((pat_planes & 0x55555555) << 1);
		}

		*oam_pix = pat_planes;
		if (v_flip) oam_pix--; else oam_pix++;
		//nes.ppu.oam_pix[NESSYS_PPU_OAM_PIXEL_ROWS * sprite_index + y] = pat_planes;

		pat_addr+=2;
		pat_addr &= 0xfff7;
		//if (v_flip) sprite_y--; else sprite_y++;
	}
}

//#ifdef WIN32
void nessys_gen_tile_pix(uint y)
//#else
//void __no_inline_not_in_flash_func(nessys_gen_tile_pix)(uint y)
//#endif
{
	uint tile_base_addr, tile_addr;
	uint tile_x, tile_y;
	uint x;
	uint pat_addr, last_pat_addr;
	uint32_t pat_planes;

	tile_x = nes.ppu.scroll[0];
	tile_y = nes.ppu.scroll_y;

	tile_y += y;

	if (nes.ppu.scroll_y < NESSYS_PPU_SCANLINES_RENDERED && tile_y > NESSYS_PPU_SCANLINES_RENDERED) {
		// skip over attribute section of table
		tile_y += 16;
	}

	tile_x |= (nes.ppu.reg[0] << 8) & 0x100;
	tile_y |= (nes.ppu.reg[0] << 7) & 0x100;

	tile_x &= 0x1f8;
	tile_y &= 0x1f8;

	tile_base_addr = NESSYS_CHR_NTB_WIN_MIN;
	// if top y bit is set, add 0x800
	tile_base_addr |= ((tile_y << 3) & 0x800);
	// Add in tile-y bits to base address
	tile_base_addr |= ((tile_y & 0xf8) << 2);

	// regenerate attribute bits if we're on the first scanline, or first row of a 32x32 block
	bool gen_attr = (y == 0) || ((tile_y & 0x1f) == 0);
	uint attr_base_addr, attr_addr, attr_x;
	uint32_t* tile_pix = (uint32_t*)nes.ppu.draw_tile_pix;
	//uint8_t* attr_pix = nes.ppu.attrib_pix;
	if (gen_attr) {
		// compute attr base address, using the y coordinate
		attr_base_addr = 0x3c0 | ((tile_y & 0xe0) >> 2);
		attr_base_addr |= ((tile_y << 3) & 0x800);
		attr_base_addr |= NESSYS_CHR_NTB_WIN_MIN;
		attr_x = 0;
	}

	last_pat_addr = ~0;
	for (x = 0; x < NESSYS_PPU_TILES_PER_ROW; x++) {
		// this takes the all but the top bit and bottom 3 bits of x and y, moves them into place
		// to form a linear address within each page
		tile_addr = tile_base_addr | ((tile_x & 0xf8) >> 3);
		// Select the page - if the top x bit is set, add 0x400
		tile_addr += ((tile_x << 2) & 0x400);

		pat_addr = (*(nessys_ppu_mem(tile_addr)) << 4) | ((nes.ppu.reg[0] & 0x10) << 8);
		if (pat_addr == last_pat_addr) {
			for (y = 0; y < 8; y += 2) {
				*tile_pix = *(tile_pix - 4);
				tile_pix++;
			}
		} else {
			for (y = 0; y < 8; y += 2) {
				pat_planes = *(nessys_ppu_mem(pat_addr | y));
				pat_planes |= (*(nessys_ppu_mem(pat_addr | y | 0x8)) << 8);
				pat_planes |= (*(nessys_ppu_mem(pat_addr | y | 0x1)) << 16);
				pat_planes |= (*(nessys_ppu_mem(pat_addr | y | 0x9)) << 24);

				// interleave and reverse the the 2 bytes
				pat_planes = (pat_planes & 0x0ff00ff0) | ((pat_planes & 0xf000f000) >> 12) | ((pat_planes & 0x000f000f) << 12);
				pat_planes = (pat_planes & 0x3c3c3c3c) | ((pat_planes & 0xc0c0c0c0) >> 6) | ((pat_planes & 0x03030303) << 6);
				pat_planes = (pat_planes & 0x66666666) | ((pat_planes & 0x88888888) >> 3) | ((pat_planes & 0x11111111) << 3);
				pat_planes = ((pat_planes & 0xaaaaaaaa) >> 1) | ((pat_planes & 0x55555555) << 1);

				//nes.ppu.tile_pix[NESSYS_PPU_PIXEL_ROW_PER_TILE * x + y] = pat_planes;
				*tile_pix = pat_planes;
				tile_pix++;
			}
			last_pat_addr = pat_addr;
		}

		// compute attribute bits if on the first pixel of scan line or on the beginning of every 32x32 tile
		if (gen_attr && ((x == 0) || ((tile_x & 0x1f) == 0))){
			attr_addr = attr_base_addr | ((tile_x & 0xe0) >> 5);
			attr_addr |= ((tile_x << 2) & 0x400);
			pat_planes = *(nessys_ppu_mem(attr_addr));
			nes.ppu.draw_attrib_pix[attr_x] = pat_planes;
			attr_x++;
			//*attr_pix = pat_planes;
			//attr_pix++;
		}
		tile_x += 8;
	}

}

void nessys_unload_cart()
{
	nessys_cleanup_mapper();
	ines_unload_cart();
	nes.prg_rom_base = NULL;
	nes.prg_rom_size = 0;
	nes.ppu.chr_rom_base = NULL;
	nes.ppu.chr_rom_size = 0;
}

void nessys_cleanup()
{

}

bool nessys_init_mapper()
{
	return true;
}

void nessys_cleanup_mapper()
{

}

//uint8_t* nessys_ram(uint16_t addr)
//{
//	return nes.sysmem + (addr & NESSYS_RAM_MASK);
//}
//
//const uint8_t* nessys_mem(uint16_t addr, uint16_t* bank, uint16_t* offset)
//{
//	uint16_t b = addr >> NESSYS_PRG_BANK_SIZE_LOG2;
//	uint16_t o = addr & nes.prg_rom_bank_mask[b];
//	*bank = b;
//	*offset = o;
//	return nes.prg_rom_bank[b] + o;
//}
//
//const uint8_t* nessys_ppu_mem(uint16_t addr)
//{
//	if (addr >= NESSYS_CHR_PAL_WIN_MIN) return nes.ppu.pal + (addr & NESSYS_PPU_PAL_MASK);
//	uint16_t b = addr >> NESSYS_CHR_BANK_SIZE_LOG2;
//	return nes.ppu.chr_rom_bank[b] + (addr & nes.ppu.chr_rom_bank_mask[b]);
//}
//
//uint8_t* nessys_ppu_ram(uint16_t addr)
//{
//	if (addr >= NESSYS_CHR_PAL_WIN_MIN) return nes.ppu.pal + (addr & NESSYS_PPU_PAL_MASK);
//	uint16_t b = addr >> NESSYS_CHR_BANK_SIZE_LOG2;
//	return nes.ppu.chr_ram_bank[b] + (addr & nes.ppu.chr_ram_bank_mask[b]);
//}
