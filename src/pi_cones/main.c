#ifdef WIN32
#include <windows.h>
#include <timeapi.h>
#else
#include "st7789.h"
#endif
#include "font/font.h"
#include "nessys.h"
#include <stdio.h>

#define PPU_MULTI_THREAD 1

#ifdef WIN32
#define FB_FLIP_XY 0
#else
#define SYS_CLK_KHZ 250000

// Flip XY causes image to be addressed in column major order
// When sent to the display controller, we set the orientation
// to mirror in y direction
// without flip xy, the image is addressed in row major order,
// and the image is rotated by 270 degrees in the display controller
// Both modes display the same image, but Flip XY, in conjunction with
// double buffering removes the diagonal tear artifiact
// Tearng ay still occur on a scanline, but it is much less obvious
// This tearing may also be removed if the display controller exponses
// the tearing effect signal (TE).  But this logic is not yet implemented
#define FB_FLIP_XY 1
#endif

#if FB_FLIP_XY
#define FB_ADDRESS FB_ADDRESS_FLIP
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define SCREEN_ORIENT ST7789_ORIENT_MIRROR_X
#else
#define FB_ADDRESS FB_ADDRESS_NORMAL
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_ORIENT ST7789_ORIENT_ROT_270
#endif

// framebuffer width and height is the size of the image to be rendered
// if not the full screen, then the rest should be cleared before any rendering occurs
#define FB_WIDTH 256
#define FB_HEIGHT 240
#define SCREEN_WIN_WIDTH ((FB_FLIP_XY) ? FB_HEIGHT : FB_WIDTH)
#define SCREEN_WIN_HEIGHT ((FB_FLIP_XY) ? FB_WIDTH : FB_HEIGHT)
#define SCREEN_WIN_X ((SCREEN_WIDTH - SCREEN_WIN_WIDTH) >> 1)
#define SCREEN_WIN_Y ((SCREEN_HEIGHT - SCREEN_WIN_HEIGHT) >> 1)
#define FB_PIXELS (FB_WIDTH * FB_HEIGHT)
#define FB_SIZE (FB_PIXELS * sizeof(uint16_t))
#define FB_ADDRESS_NORMAL(x, y) ((y) * FB_WIDTH + (x))
#define FB_ADDRESS_FLIP(x, y) ((x) * FB_HEIGHT + (y))
// Single or double buffering
#define FB_BUFFERS 2

// additional memory used for some mappers for 4 screen, or additional cpu/ppu ram
#define NES_AUX_MEMORY_SIZE 2048

uint16_t framebuffer[FB_BUFFERS * FB_PIXELS];

uint16_t* draw_frame = framebuffer;
uint16_t* disp_frame = framebuffer + (FB_BUFFERS-1) * FB_PIXELS;

nessys_t nes;
TEXTBOX_T tbox;

uint8_t aux_mem[NES_AUX_MEMORY_SIZE];
uint8_t* aux_ptr = aux_mem;
uint32_t free_aux = NES_AUX_MEMORY_SIZE;

// Number of pixels each core attempts to render each pass
#define RENDER_PIXEL_INC_LOG2 4
#define RENDER_PIXEL_INC (1 << RENDER_PIXEL_INC_LOG2)

// number of frames until we rerender the textbox
#define TBOX_RENDER_FRAME_PERIOD 15

void* alloc_aux(uint32_t size)
{
    if (size > free_aux) {
        return NULL;
    }

    void* new_mem = aux_ptr;
    aux_ptr += size;
    free_aux -= size;
    return new_mem;
}

void dealloc_aux()
{
    aux_ptr = aux_mem;
    free_aux = NES_AUX_MEMORY_SIZE;
}

static const
#include "rom.h"

void flip_framebuffer()
{
    uint16_t* temp = draw_frame;
    draw_frame = disp_frame;
    disp_frame = temp;
}

#ifndef WIN32
void lcd_init(bool serial)
{
    st7789_cfg_t cfg;
    cfg.serial = serial;
    cfg.pin_cs = PICO_DEFAULT_SPI_CSN_PIN;
    cfg.pin_dc = 20;
    cfg.pin_rst = 21;
    cfg.pin_bl = 22;
    cfg.dma_chan = 1;
    if (serial) {
        cfg.intf.si.spi = PICO_DEFAULT_SPI_INSTANCE;
        cfg.intf.si.pin_din = PICO_DEFAULT_SPI_TX_PIN;
        cfg.intf.si.pin_clk = PICO_DEFAULT_SPI_SCK_PIN;
    } else {
        cfg.intf.pi.pio = pio0;
        cfg.intf.pi.sm = pio_claim_unused_sm(cfg.intf.pi.pio, true);
        cfg.intf.pi.num_bits = 16;
        cfg.intf.pi.pin_wr_rd_base = 18;
        cfg.intf.pi.pin_data_base = 0;
    }
    st7789_init(&cfg, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ORIENT);
}
#endif

bool ines_load_cart(const void* cart)
{
    const uint8_t* cur_pos = cart;

    const ines_header* hdr = (const ines_header*)cur_pos;
    cur_pos += sizeof(ines_header);

    if (hdr->signature != INES_SIGNATURE) {
        // not an ines file
        return false;
    }

    // continues parsing
    if (hdr->flags6 & INES_FLAGS6_TRAINER) {
        // skip over 512B trainer
        cur_pos += 512;
    }

    bool nes2 = (hdr->flags7 & INES_FLAGS7_NES2) ? true : false;

    // get mirroring mode
    nes.ppu.name_tbl_vert_mirror = hdr->flags6 & INES_FLAGS6_MIRRORING;

    // get mapper id
    nes.mapper_id = ((hdr->flags6 & INES_FLAGS6_MAPPER) >> 4) | (hdr->flags7 & INES_FLAGS7_MAPPER);

    // allocate space for 4 screen vram
    if (hdr->flags6 & INES_FLAGS6_MIRROR_CTRL_DISABLE) {
        nes.ppu.mem_4screen = alloc_aux(NESSYS_PPU_MEM_SIZE);
    }

    // allocate space for prg rom/ram and chr rom
    if (hdr->prg_rom_size) {
        nes.prg_rom_size = hdr->prg_rom_size;
        if (nes2) nes.prg_rom_size |= (hdr->flags9 & 0xf) << 8;
        nes.prg_rom_size *= 0x4000;
        nes.prg_rom_base = cur_pos;
        cur_pos += nes.prg_rom_size;
    }
    uint32_t ram_size = (nes2) ? ((hdr->flags10 & 0xf) ? 64 << (hdr->flags10 & 0xf) : 0) :
        ((hdr->prg_ram_size) ? hdr->prg_ram_size * 0x2000 : 0x2000);
    if (ram_size) {
        nes.prg_ram_size = ram_size;
        nes.prg_ram_base = alloc_aux(ram_size);
    }
    if (hdr->chr_rom_size) {
        nes.ppu.chr_rom_size = hdr->chr_rom_size;
        if (nes2) nes.ppu.chr_rom_size |= (hdr->flags9 & 0xf0) << 4;
        nes.ppu.chr_rom_size *= 0x2000;
        nes.ppu.chr_rom_base = cur_pos;
        cur_pos += nes.ppu.chr_rom_size;
        //} else if (hdr.flags11 & 0xf) {
        //	nes.ppu.chr_ram_size = 64 << (hdr.flags11 & 0xf);
        //	nes.ppu.chr_ram_base = (uint8_t*)malloc(nes.ppu.chr_ram_size);
        //	if (nes.ppu.chr_ram_base == NULL) return false;
    } else {
        nes.ppu.chr_ram_size = 0x2000;  // 8KB of ram
        nes.ppu.chr_ram_base = alloc_aux(nes.ppu.chr_ram_size);
    }

	nessys_default_memmap(nes);
	bool success = nessys_init_mapper(nes);
	if (success) {
		nessys_power_cycle(nes);
	} else {
		nessys_unload_cart(nes);
	}

	return success;
}

void ines_unload_cart()
{
	nessys_cleanup_mapper();

	nes.ppu.mem_4screen = NULL;
    nes.prg_ram_base = NULL;
    nes.prg_ram_size = 0;
    nes.ppu.chr_ram_base = NULL;
    nes.ppu.chr_ram_size = 0;
    dealloc_aux();
}

#ifdef WIN32
HCURSOR app_cursor;
HICON app_icon;
WNDCLASSEX win_class;
static const char* win_class_name = "pi_cones";
HWND hwnd;
HBITMAP hbm;
uint16_t* bmp_data;
HANDLE h_thread = NULL;

COLORREF convert565(uint32_t color)
{
	return ((color & 0xf800) >> 8) | ((color & 0x07e0) << 5) | ((color & 0x001f) << 19);
}

void win32_fill(uint16_t color)
{
	HDC hdc = GetDC(hwnd);
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	HBRUSH brush = CreateSolidBrush(convert565(color));
	FillRect(hdc, &rect, brush);
	DeleteObject(brush);
	ReleaseDC(hwnd, hdc);
}

void win32_write(uint16_t* frame)
{
	memcpy(bmp_data, frame, FB_SIZE);
}

void win32_display(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	HDC hdc_bmp = CreateCompatibleDC(hdc);
	auto old_bmp = SelectObject(hdc_bmp, hbm);
	BitBlt(hdc, SCREEN_WIN_X, SCREEN_WIN_Y, FB_WIDTH, FB_HEIGHT, hdc_bmp, 0, 0, SRCCOPY);
	SelectObject(hdc, old_bmp);
	DeleteDC(hdc_bmp);
	EndPaint(hwnd, &ps);
}

LRESULT WINAPI win32_msg_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_PAINT:
		win32_display(hwnd);
		break;
	case WM_CLOSE:
		//if (h_thread) {
		//	CloseHandle(h_thread);
		//	h_thread = NULL;
		//}
		exit(0);
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}


void win32_init()
{
	app_cursor = LoadCursor(NULL, IDC_ARROW);
	app_icon = LoadIcon(GetModuleHandle(NULL), "IDI_MAIN");
	if (app_icon == NULL)
		app_icon = LoadIcon(NULL, IDI_APPLICATION);

	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = win32_msg_proc;
	win_class.cbClsExtra = 0L;
	win_class.cbWndExtra = 0L;
	win_class.hInstance = GetModuleHandle(NULL);
	win_class.hIcon = app_icon;
	win_class.hCursor = app_cursor;
	win_class.hbrBackground = NULL;
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = win_class_name;
	win_class.hIconSm = NULL;

	RegisterClassEx(&win_class);

	hwnd = CreateWindowEx(0, win_class_name, win_class_name,
		WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
		100, 100,
		SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL,
		win_class.hInstance, NULL);

	RECT clientR, winR;
	GetClientRect(hwnd, &clientR);
	GetWindowRect(hwnd, &winR);
	uint w, h;
	w = SCREEN_WIDTH + (winR.right - winR.left) - clientR.right;
	h = SCREEN_HEIGHT + (winR.bottom - winR.top) - clientR.bottom;

	SetWindowPos(hwnd, NULL, 100, 100, w, h, SWP_NOMOVE | SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOZORDER);

	ShowWindow(hwnd, SW_SHOWDEFAULT);

	HDC hdc = GetDC(hwnd);

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = FB_WIDTH;
	bmi.bmiHeader.biHeight = -FB_HEIGHT; // top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 16;
	bmi.bmiHeader.biCompression = BI_RGB;
	hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)(&bmp_data), NULL, NULL);

	ReleaseDC(hwnd, hdc);
}

void win32_uninit()
{
	UnregisterClass(win_class_name, GetModuleHandle(NULL));
}

uint time_us_32()
{
	return timeGetTime() * 1000;
}

void win32_ppu_entry(LPVOID d)
{
	process_ppu();
}

#endif


#ifdef WIN32
void process_pixels(uint min_x, uint max_x, uint y, render_state_t* rstate)
#else
void __no_inline_not_in_flash_func(process_pixels)(uint min_x, uint max_x, uint y, render_state_t* rstate)
#endif
{
	uint16_t text_color = 0xf800;

	bool sprite_hit;
	uint16_t sprite_color;
	uint16_t background_color;

	uint tile_x, tile_y;
	uint sprite_x, sprite_y;
	uint tile_addr;
	uint attr_addr;
	uint attr_offset;
	uint pat_addr;
	uint planes;
	uint8_t pal_index;
	uint8_t pal;
	uint x;

	bool enable_background, enable_sprite;
	//uint tile_base_x = (nes.ppu.reg[0] << 8) & 0x100;
	uint tile_base_y = (nes.ppu.reg[0] << 7) & 0x100;

	//tile_base_x += nes.ppu.scroll[0];
	tile_base_y += nes.ppu.scroll_y;

	for (x = min_x; x < max_x; x++) {
		bool in_text = x >= tbox.start_x && x < tbox.end_x && y >= tbox.start_y && y < tbox.end_y;
		//in_text = in_text && (nes.rendered_frames == 0);
		if (in_text) {
			//if(rstate == &nes.c1_rstate) text_color = 0x07e0;
			if (!tbox.line_done) {
				// We're in the text bounding box
				// Now check if we hit the glyph
				in_text = (tbox.cur_glyph << tbox.offset_x) & 0x80;
			} else {
				in_text = false;
			}
			if (x == tbox.end_x - 1) {
				// end of row
				tbox.offset_x = 0;
				tbox.char_index = 0;
				tbox.offset_y++;
				tbox.line_done = false;
				if (tbox.offset_y >= tbox.font->height) {
					// end of a line
					tbox.offset_y = 0;
					tbox.line++;
				}
				textbox_set_cur_glyph(&tbox);
			} else {
				tbox.offset_x++;
				if (tbox.offset_x >= tbox.font->width) {
					// end of a character
					tbox.offset_x = 0;
					tbox.char_index++;
					textbox_set_cur_glyph(&tbox);
				}
			}
		}

		// PPU processing
		// determine if we need to process sprites or thte background
		enable_background = (nes.ppu.reg[1] & 0x8) && ((x >= 8) || (nes.ppu.reg[1] & 0x2));
		enable_sprite = (nes.ppu.reg[1] & 0x10) && ((x >= 8) || (nes.ppu.reg[1] & 0x4)) &&
			(x >= nes.ppu.scan_line_min_sprite_x) && (x <= nes.ppu.scan_line_max_sprite_x);

		// First check for sprite hit
		sprite_hit = false;
		if (enable_sprite && (nes.ppu.scan_line_sprite[x] & 0x3)) {
			if (rstate->sprite_index != (nes.ppu.scan_line_sprite[x] & 0xfc)) {
				rstate->sprite_index = (nes.ppu.scan_line_sprite[x] & 0xfc);
				//sprite_y = y - nes.ppu.oam[rstate->sprite_index];
				//// Flip y if vertical flip bit is set
				//sprite_y = (nes.ppu.oam[rstate->sprite_index + 2] & 0x80) ? (7 + ((nes.ppu.reg[0] >> 1) & 0x8)) - sprite_y : sprite_y;
				//pat_addr = nes.ppu.oam[rstate->sprite_index + 1];
				//// for 8x16 sprites, lsb is the bank select
				//// otherwise, bank select is bit 3 ppu reg0
				//pat_addr |= 0x100 & (((pat_addr << 8) & (((uint16_t)nes.ppu.reg[0]) << 3)) |
				//	((((uint16_t)nes.ppu.reg[0]) << 5) & ~(((uint16_t)nes.ppu.reg[0]) << 3)));
				//// clear out lsb for 8x16, and fill it in based on whether sprite y is 8 or above
				//pat_addr &= 0xfffe | ~(((uint16_t)nes.ppu.reg[0]) >> 5);
				//pat_addr |= (((uint16_t)nes.ppu.reg[0]) >> 5) & (sprite_y >= 8);
				//// shift up the address and fill in the y offset
				//pat_addr <<= 4;
				//pat_addr |= sprite_y & 0x7;

				// get the pattern plane bits
				//rstate->sp_pat_planes = *(nessys_ppu_mem(pat_addr));
				//rstate->sp_pat_planes |= (*(nessys_ppu_mem(pat_addr | 0x8)) << 8);
				//rstate->sp_pat_planes = nes.ppu.oam_pix[(NESSYS_PPU_OAM_PIXEL_ROWS / 4) * rstate->sprite_index + sprite_y];
				rstate->sp_pal_base = ((nes.ppu.oam[rstate->sprite_index + 2]) & 0x3) << 2;
				rstate->sp_pal_base |= 0x10;
				rstate->sprite_background = (nes.ppu.oam[rstate->sprite_index + 2] & 0x20) != 0;

				//rstate->sprite_x = x - nes.ppu.oam[rstate->sprite_index + 3];
				// flip x if horizontal flip bit is set
				//rstate->sprite_x = (nes.ppu.oam[rstate->sprite_index + 2] & 0x40) ? 7 - rstate->sprite_x : rstate->sprite_x;
				//rstate->sprite_x_inc = (nes.ppu.oam[rstate->sprite_index + 2] & 0x40) ? -1 : 1;
				//sprite_x = x - nes.ppu.oam[rstate->sprite_index + 3];
				//rstate->sprite_h_flip = (nes.ppu.oam[rstate->sprite_index + 2] & 0x40);
				//rstate->sprite_plane_shift = (rstate->sprite_h_flip) ? sprite_x : (7 - sprite_x);
			}

			//pal_index = ((rstate->sp_pat_planes >> (7 - rstate->sprite_x)) & 0x1) | ((rstate->sp_pat_planes >> (14 - rstate->sprite_x)) & 0x2);
			//pal_index = ((rstate->sp_pat_planes >> (rstate->sprite_plane_shift)) & 0x1) | ((rstate->sp_pat_planes >> (7 + rstate->sprite_plane_shift)) & 0x2);
			pal_index = nes.ppu.scan_line_sprite[x] & 0x3;
			sprite_hit = (pal_index != 0);
			if (sprite_hit) {
				pal_index |= rstate->sp_pal_base;
				pal = nes.ppu.pal[pal_index] & 0x3f;
				sprite_color = NESSYS_PPU_PALETTE[pal];
			}
			nes.ppu.scan_line_sprite[x] = 0x0;
			//rstate->sprite_x += rstate->sprite_x_inc;
			rstate->sp_pat_planes = (rstate->sprite_h_flip) ? (rstate->sp_pat_planes >> 1) : (rstate->sp_pat_planes << 1);
		}
		//for (sp = 0; sp < nes.ppu.num_scan_line_oam; sp++) {
		//	sp_x = *(nes.ppu.scan_line_oam[sp] + 3);
		//	if (x >= sp_x && x < sp_x + 8) {
		//		// TODO: implement sprite evaluation
		//		sprite_color = 0xf81f;
		//		sprite_hit = true;
		//		sp = nes.ppu.num_scan_line_oam;  // to break out of for loop
		//	}
		//}
		// Evaluate tile
		enable_background = enable_background && (!sprite_hit || rstate->sprite_background);
		if ((rstate->tile_x & 0x7) == 0) {
			//tile_x = x;
			tile_y = y;
			//tile_x += tile_base_x;
			tile_y += tile_base_y;
			if (nes.ppu.scroll_y < NESSYS_PPU_SCANLINES_RENDERED && tile_y > NESSYS_PPU_SCANLINES_RENDERED) {
				// skip over attribute section of table
				tile_y += 16;
			}
			//tile_x &= 0x1ff;
			tile_y &= 0x1ff;
			//// this takes the all but the top bit and bottom 3 bits of x and y, moves them into place
			//// to form a linear address within each page
			//tile_addr = ((tile_y & 0xf8) << 2) | ((tile_x & 0xf8) >> 3);
			//// Select the page - if the top x bit is set, add 0x400; if top y bit is set, add 0x800
			//tile_addr += ((tile_x << 2) & 0x400);
			//tile_addr += ((tile_y << 3) & 0x800);
			//// Add base of name table
			//tile_addr += NESSYS_CHR_NTB_WIN_MIN;

			tile_x = (x + (nes.ppu.scroll[0] & 0x1f));
			if ((rstate->tile_x & 0xf) == 0x0) {
				//// the attr addr is similar, except each byte corresponds to a 32x32 region, instea of 8x8
				//attr_addr = 0x3c0 + (((tile_y & 0xe0) >> 2) | ((tile_x & 0xe0) >> 5));
				//
				//// Do same page select for attr addr
				//attr_addr += ((tile_x << 2) & 0x400);
				//attr_addr += ((tile_y << 3) & 0x800);
				//// Add base of name table
				//attr_addr += NESSYS_CHR_NTB_WIN_MIN;
				//attr_addr = *(nessys_ppu_mem(attr_addr));
				//rstate->attr_bits = (attr_addr << 2);
				rstate->attr_bits = (nes.ppu.disp_attrib_pix[tile_x >> 5] << 2);

				// attribute offset is which 16x16 within the 32x32 we are in
				// each 16x16 uses 2 bits
				attr_offset = ((tile_x & 0x10) >> 3) | ((tile_y & 0x10) >> 2);
				rstate->pal_base = (rstate->attr_bits >> attr_offset) & 0xc;
			}

			// Keep the tile offsets
			rstate->tile_x = tile_x;
			
			// attribute offset is which 16x16 within the 32x32 we are in
			// each 16x16 uses 2 bits
			//attr_offset = ((rstate->tile_x & 0x10) >> 3) | ((rstate->tile_y & 0x10) >> 2);

			//pat_addr = (*(nessys_ppu_mem(tile_addr)) << 4) | (tile_y & 0x7) | ((nes.ppu.reg[0] & 0x10) << 8);
			//planes = *(nessys_ppu_mem(pat_addr));
			//planes |= (*(nessys_ppu_mem(pat_addr | 0x8)) << 8);

			// interleave the 2 bytes
			//planes = (planes & 0xf00f) | ((planes & 0x0f00) >> 4) | ((planes & 0x00f0) << 4);
			//planes = (planes & 0xc3c3) | ((planes & 0x3030) >> 2) | ((planes & 0x0c0c) << 2);
			//planes = (planes & 0x9999) | ((planes & 0x4444) >> 1) | ((planes & 0x2222) << 1);

			// interleave and reverse the the 2 bytes
			//planes = (planes & 0x0ff0) | ((planes & 0xf000) >> 12) | ((planes & 0x000f) << 12);
			//planes = (planes & 0x3c3c) | ((planes & 0xc0c0) >> 6) | ((planes & 0x0303) << 6);
			//planes = (planes & 0x6666) | ((planes & 0x8888) >> 3) | ((planes & 0x1111) << 3);

			//rstate->pat_planes = (planes << (tile_x & 0x7));
			tile_x = (x + (nes.ppu.scroll[0] & 0x7));
			rstate->pat_planes = nes.ppu.disp_tile_pix[(tile_x & 0x1f8) + (y + nes.ppu.scroll_y & 0x7)];
			if (x == min_x) {
				rstate->pat_planes >>= (2 * (tile_x & 0x7));
			}

			//rstate->pal_base = (*(nessys_ppu_mem(attr_addr) >> attr_offset) & 0x3) << 2;
		}
		if (enable_background) {
			pal_index = rstate->pat_planes & 0x3;// ((rstate->pat_planes >> 7) & 0x1) | ((rstate->pat_planes >> 14) & 0x2);
			//pal_index = ((rstate->pat_planes >> 14) & 0x3);
			//pal_index = (rstate->pat_planes & 0x3);
			// we continue to hit the sprite if it's not in the background or background color is clear
			//sprite_hit = sprite_hit && (!sprite_background || (pal_index == 0));
			if (rstate->sprite_background && pal_index != 0) {
				sprite_hit = false;
			}
			pal_index |= (pal_index) ? rstate->pal_base : 0;
		} else {
			pal_index = 0;
		}
		if (!sprite_hit) {
			pal = nes.ppu.pal[pal_index] & 0x3f;
			background_color = NESSYS_PPU_PALETTE[pal];
		}
		draw_frame[FB_ADDRESS(x, y)] = (in_text) ? text_color : ((sprite_hit) ? sprite_color : background_color);
		rstate->tile_x++;
		//rstate->tile_x &= 0x7;
		//rstate->pat_planes <<= 1;
		//rstate->pat_planes <<= 2;
		rstate->pat_planes >>= 2;
	}
}

#ifdef WIN32
void process_ppu()
#else
void __no_inline_not_in_flash_func(process_ppu)()
#endif
{
	uint y, min_x, max_x;
#ifdef PPU_MULTI_THREAD
	while (1)
#endif
	{

		// Check if we are in a renderable portion of the frame
		if (nes.scan_line >= NESSYS_PPU_SCANLINES_START_RENDER && (nes.frame_delta_time <= 0)) {
			//if (nes.ppu.scroll_y != 0) {
			//	printf("pause\n");
			//}
			y = nes.scan_line - NESSYS_PPU_SCANLINES_START_RENDER;
			nes.c1_render_done = (nes.rendered_scan_clk >= nes.scan_clk) || (nes.rendered_scan_clk >= FB_WIDTH) || (y >= FB_HEIGHT);
			if (!nes.c1_render_done) {
				min_x = nes.rendered_scan_clk;
				nes.rendered_scan_clk += RENDER_PIXEL_INC;
				max_x = nes.scan_clk;
				nes.rendered_scan_clk = (nes.rendered_scan_clk > FB_WIDTH) ? FB_WIDTH : nes.rendered_scan_clk;
				nes.rendered_scan_clk = (nes.rendered_scan_clk > max_x) ? max_x : nes.rendered_scan_clk;

				//max_x = nes.scan_clk;
				//max_x = (max_x > FB_WIDTH) ? FB_WIDTH : max_x;
				//max_x = (max_x > nes.rendered_scan_clk + RENDER_PIXEL_INC) ? nes.rendered_scan_clk + RENDER_PIXEL_INC : max_x;
				//nes.rendered_scan_clk = max_x;

				process_pixels(min_x, nes.rendered_scan_clk, y, &nes.c1_rstate);

			}
			//nes.rendered_scan_clk = nes.scan_clk;
		}
	}
}

//#ifdef WIN32
void main_loop()
//#else
//void __no_inline_not_in_flash_func(main_loop)()
//#endif
{
	uint16_t clear_color = 0x07e0;
	char text_str[64];
	memset(&tbox, 0, sizeof(TEXTBOX_T));
	textbox_set_font(&tbox, font[FONT_ID_AIXOID9_F16]);
	textbox_set_position(&tbox, 0, 0);

	uint in_text_line = 0;
	uint32_t cur_time, last_time = 0;
	float fps;

	const uint8_t* pc_ptr = NULL;
	const uint8_t* pc_ptr_next = NULL;
	const c6502_op_code_t* op = NULL;
	const c6502_op_code_t* op_next = NULL;

	uint16_t addr = 0x0, indirect_addr = 0x0;
	uint16_t penalty_cycles;
	const uint8_t* operand = NULL;
	uint8_t* ram_ptr = NULL;
	uint16_t result;
	uint8_t overflow;
	bool apu_write, ppu_write, rom_write;
	uint8_t data_change;  // when writing to addressable memory, indicates which bits changed

	uint16_t bank, offset;
	uint16_t mem_addr_mask;

	uint sp_x;
	uint8_t sp;
	uint loop_count = 0;

	uint y, min_x, max_x;
	uint32_t next_line_scan_clk;
	uint next_scan_line;

#ifdef WIN32
	MSG msg;

	win32_fill(clear_color);
#else
	st7789_fill(clear_color);
	st7789_wait_for_write();
	st7789_set_window(SCREEN_WIN_X, SCREEN_WIN_Y, SCREEN_WIN_X + SCREEN_WIN_WIDTH - 1, SCREEN_WIN_Y + SCREEN_WIN_HEIGHT - 1);
#endif
	nessys_init();
	bool rom_ok = ines_load_cart(rom_nes);
	nes.scan_clk = 0;
	nes.rendered_scan_clk = 0;
	next_line_scan_clk = 0;
	next_scan_line = 0;
	pc_ptr_next = nessys_mem(nes.reg.pc, &bank, &offset);
	op_next = C6502_OP_CODE + *pc_ptr_next;
	uint skipped_frames = 0;
	uint total_skipped_frames = 0;
	nes.frame_delta_time = 0;
	last_time = time_us_32();

#ifdef PPU_MULTI_THREAD
#ifdef WIN32
	CreateThread(NULL, 0, win32_ppu_entry, NULL, 0, &h_thread);
#else
	multicore_launch_core1(process_ppu);
#endif
#endif

	while (1) {
		cur_time = time_us_32();

		nes.frame_delta_time += 0;// cur_time - last_time - 16667;
		if (skipped_frames > 1) {
			nes.frame_delta_time = 0;
		}
		//nes.frame_delta_time = ((nes.frame & 0x1) == 0) ? 1 : 0;

		// Wait for 16.667 ms, to render at roughly 60fps
		// May still cause tearing artifiact, since this is not synchronized
		// to display controller
		while (cur_time - last_time < 16667 && skipped_frames == 0) {
			cur_time = time_us_32();
		}

		//if (nes.frame_delta_time <= 0 && skipped_frames != 0) {
		//	for (y = 0; y < 256; y++) {
		//		nes.ppu.scan_line_sprite[y] = 0;
		//	}
		//}

		//if(skip_render == 0) skip_render += (cur_time - last_time) / 16666;

		nes.rendered_time += cur_time - last_time;
		last_time = cur_time;

		if (nes.rendered_frames >= TBOX_RENDER_FRAME_PERIOD) {
			// Calculate and report out the frames per second
			fps = nes.rendered_time;
			fps = (nes.rendered_frames * 1000000) / fps;
			sprintf(text_str, "%0.2f %d", fps, total_skipped_frames);
			textbox_set_text(&tbox, text_str, 0);
			nes.rendered_frames = 0;
			nes.rendered_time = 0;
			loop_count = 0;
			total_skipped_frames = 0;
		}

		textbox_reset(&tbox);

		nes.scan_line = 0;
		// set vblank irq status
		nes.ppu.reg[2] |= 0x80;
		if (nes.ppu.reg[0] & 0x80) {
			// if NMI is enabled, take the IRQ
			nessys_irq(NESSYS_NMI_VECTOR, C6502_P_B);
			pc_ptr_next = nessys_mem(nes.reg.pc, &bank, &offset);
			op_next = C6502_OP_CODE + *pc_ptr_next;
			next_line_scan_clk += NESSYS_PPU_PER_CPU_CLK * 7;
		}
		nes.sprite0_hit_scan_clk = ~0;  // don't enable sprite 0 hit now
		while (nes.scan_line < NESSYS_PPU_SCANLINES_PER_FRAME) {
			// reset tile_x to ensure that a new tile address is computed
			nes.c0_rstate.tile_x = 0x100;
			nes.c1_rstate.tile_x = 0x100;
			// reset sprite index to NULL value
			nes.c0_rstate.sprite_index = 0xff;
			//nes.c0_rstate.sprite_x_inc = 1;
			nes.c1_rstate.sprite_index = 0xff;
			//nes.c1_rstate.sprite_x_inc = 1;
			// restart this line's clk
			nes.scan_clk = 0;
			nes.rendered_scan_clk = 0; // reset to 0 to render the full scan line
			while (nes.scan_clk < NESSYS_PPU_CLK_PER_SCANLINE) {
				pc_ptr = pc_ptr_next;
				op = op_next;
				ram_ptr = NULL;
				ppu_write = false;
				penalty_cycles = 0;
				bank = 0;
				switch (op->addr) {
				case C6502_ADDR_NONE:
					break;
				case C6502_ADDR_ACCUM:
					operand = &nes.reg.a;
					ram_ptr = &nes.reg.a;
					break;
				case C6502_ADDR_IMMED:
					operand = nessys_mem(nes.reg.pc + 1, &bank, &offset);
					break;
				case C6502_ADDR_ZEROPAGE:
					addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_ABSOLUTE:
					addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					addr |= ((uint16_t)*nessys_mem(nes.reg.pc + 2, &bank, &offset)) << 8;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_RELATIVE:
					indirect_addr = nes.reg.pc + op->num_bytes;
					addr = indirect_addr + ((int8_t)*nessys_mem(nes.reg.pc + 1, &bank, &offset));
					// branch penalty of 2 cycles if page changes, otherwise just 1 cycle
					penalty_cycles += 1;
					penalty_cycles += ((addr & 0xFF00) != (indirect_addr & 0xFF00));
					break;
				case C6502_ADDR_INDIRECT:
					indirect_addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					indirect_addr |= ((uint16_t)*nessys_mem(nes.reg.pc + 2, &bank, &offset)) << 8;
					addr = *nessys_mem(indirect_addr, &bank, &offset);
					indirect_addr++;
					if ((indirect_addr & 0xff) == 0) indirect_addr -= 0x100;
					addr |= ((uint16_t)*nessys_mem(indirect_addr, &bank, &offset)) << 8;
					break;
				case C6502_ADDR_ZEROPAGE_X:
					addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					addr += nes.reg.x;
					addr &= 0xFF;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_ZEROPAGE_Y:
					addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					addr += nes.reg.y;
					addr &= 0xFF;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_ABSOLUTE_X:
					addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					penalty_cycles += (addr + nes.reg.x >= 0x100) * op->penalty_cycles;
					addr |= ((uint16_t)*nessys_mem(nes.reg.pc + 2, &bank, &offset)) << 8;
					addr += nes.reg.x;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_ABSOLUTE_Y:
					addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					penalty_cycles += (addr + nes.reg.y >= 0x100) * op->penalty_cycles;
					addr |= ((uint16_t)*nessys_mem(nes.reg.pc + 2, &bank, &offset)) << 8;
					addr += nes.reg.y;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_INDIRECT_X:
					indirect_addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					indirect_addr += nes.reg.x;
					indirect_addr &= 0xFF;
					addr = *nessys_mem(indirect_addr, &bank, &offset);
					indirect_addr++;
					indirect_addr &= 0xFF;
					addr |= ((uint16_t)*nessys_mem(indirect_addr, &bank, &offset)) << 8;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				case C6502_ADDR_INDIRECT_Y:
					indirect_addr = *nessys_mem(nes.reg.pc + 1, &bank, &offset);
					addr = *nessys_mem(indirect_addr, &bank, &offset);
					penalty_cycles += (addr + nes.reg.y >= 0x100) * op->penalty_cycles;
					indirect_addr++;
					indirect_addr &= 0xFF;
					addr |= ((uint16_t)*nessys_mem(indirect_addr, &bank, &offset)) << 8;
					addr += nes.reg.y;
					operand = nessys_mem(addr, &bank, &offset);
					break;
				}

				// if operand is writeable, get its address
				if (ram_ptr == NULL) ram_ptr = (addr < 0x4018) ? (uint8_t*)operand : NULL;

				// increment pc
				nes.reg.pc += op->num_bytes;

				switch (bank) {
				case NESSYS_PPU_REG_START_BANK:
					switch (offset) {
					case 2:
						nes.ppu.status &= ~0x80;
						break;
					case 4:
						nes.ppu.reg[4] = nes.ppu.oam[nes.ppu.reg[3]];
						break;
					case 7:
						// immediately update data if reading from palette data; otherwise defer until after this instruction
						if ((nes.ppu.mem_addr & 0x3f00) == 0x3f00) nes.ppu.reg[7] = *nessys_ppu_mem(nes.ppu.mem_addr);
						break;
					}
					break;
				//case NESSYS_APU_REG_START_BANK:
				//	if (offset >= NESSYS_APU_JOYPAD0_OFFSET && offset <= NESSYS_APU_JOYPAD1_OFFSET) {
				//		uint8_t j = offset - NESSYS_APU_JOYPAD0_OFFSET;
				//		nes->apu.reg[offset] = (nes->apu.latched_joypad[j] & 0x1) | (0x40);
				//	} else if (offset == NESSYS_APU_STATUS_OFFSET) {
				//		nessys_gen_sound(nes);
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] &= 0xc0;
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] |= (nes->apu.pulse[0].length) ? 0x1 : 0x0;
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] |= (nes->apu.pulse[1].length) ? 0x2 : 0x0;
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] |= (nes->apu.triangle.length) ? 0x4 : 0x0;
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] |= (nes->apu.noise.length) ? 0x8 : 0x0;
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] |= (nes->dmc_bits_to_play >= 8) ? 0x10 : 0x0;
				//		nes->apu.reg[NESSYS_APU_STATUS_OFFSET] |= (nes->frame_irq) ? 0x40 : 0x0;
				//		clear_frame_irq = true;
				//	} else if (offset >= NESSYS_APU_SIZE) {
				//		uint8_t* op = nes->mapper_read(nes, addr);
				//		if (op) operand = op;
				//	}
				//	break;
				}

				// execute instruction
				switch (op->ins) {
				case C6502_INS_ADC:
				case C6502_INS_SBC:
					result = *operand;
					if (op->ins == C6502_INS_SBC) result = ~result;
					// overflow possible if bit 7 of two operands are the same
					overflow = (result & 0x80) == (nes.reg.a & 0x80);
					result += nes.reg.a + ((nes.reg.p & C6502_P_C) >> C6502_P_C_SHIFT);
					overflow &= (result & 0x80) != (nes.reg.a & 0x80);
					nes.reg.a = (uint8_t)result;
					// clear N/V/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_V | C6502_P_Z | C6502_P_C);
					nes.reg.p |= (nes.reg.a == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= overflow << C6502_P_V_SHIFT;
					overflow = (result & 0x100) >> 8;  // carry bit
					if (op->ins == C6502_INS_SBC) overflow = !overflow;
					nes.reg.p |= overflow << C6502_P_C_SHIFT;
					nes.reg.p |= (nes.reg.a & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_AND:
					nes.reg.a &= *operand;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= (nes.reg.a == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (nes.reg.a & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_ASL:
					overflow = ((*operand & 0x80) != 0x00);
					result = *operand << 1;
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= overflow << C6502_P_C_SHIFT;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = (uint8_t)result;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = (uint8_t)result;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = (uint8_t)result;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ (uint8_t)result);
							*ram_ptr = (uint8_t)result;
						}
					} else {
						rom_write = true;
						if (nes.mapper_flags & NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE) result = *operand;
					}
					break;
				case C6502_INS_BCC:
					if ((nes.reg.p & C6502_P_C) == 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BCS:
					if ((nes.reg.p & C6502_P_C) != 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BEQ:
					if ((nes.reg.p & C6502_P_Z) != 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BIT:
					// clear N/V/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_V | C6502_P_Z);
					result = *operand;
					nes.reg.p |= (result & 0xC0);  // bit 7 & 6 go into the N/V bits respectively
					result &= nes.reg.a;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					break;
				case C6502_INS_BMI:
					if ((nes.reg.p & C6502_P_N) != 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BNE:
					if ((nes.reg.p & C6502_P_Z) == 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BPL:
					if ((nes.reg.p & C6502_P_N) == 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BRK:
					nessys_irq(NESSYS_IRQ_VECTOR, 0);
					break;
				case C6502_INS_BVC:
					if ((nes.reg.p & C6502_P_V) == 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_BVS:
					if ((nes.reg.p & C6502_P_V) != 0x00) {
						// take the branch
						nes.reg.pc = addr;
					} else {
						// if we don't take the branch, there is no penalty
						penalty_cycles = 0;
					}
					break;
				case C6502_INS_CLC:
					nes.reg.p &= ~C6502_P_C;
					break;
				case C6502_INS_CLD:
					nes.reg.p &= ~C6502_P_D;
					break;
				case C6502_INS_CLI:
					nes.iflag_delay = nes.reg.p;
					nes.reg.p &= ~C6502_P_I;
					nes.iflag_delay ^= nes.reg.p;
					nes.iflag_delay &= C6502_P_I;
					break;
				case C6502_INS_CLV:
					nes.reg.p &= ~C6502_P_V;
					break;
				case C6502_INS_CMP:
					overflow = (nes.reg.a >= *operand);
					result = nes.reg.a - *operand;
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= overflow << C6502_P_C_SHIFT;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_CPX:
					overflow = (nes.reg.x >= *operand);
					result = nes.reg.x - *operand;
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= overflow << C6502_P_C_SHIFT;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_CPY:
					overflow = (nes.reg.y >= *operand);
					result = nes.reg.y - *operand;
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= overflow << C6502_P_C_SHIFT;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_DEC:
					result = *operand - 1;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = (uint8_t)result;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = (uint8_t)result;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = (uint8_t)result;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ (uint8_t)result);
							*ram_ptr = (uint8_t)result;
						}
					} else {
						rom_write = true;
						if (nes.mapper_flags & NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE) result = *operand;
					}
					break;
				case C6502_INS_DEX:
					nes.reg.x--;
					result = nes.reg.x;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_DEY:
					nes.reg.y--;
					result = nes.reg.y;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_EOR:
					nes.reg.a ^= *operand;
					result = nes.reg.a;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_INC:
					result = *operand + 1;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = (uint8_t)result;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = (uint8_t)result;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = (uint8_t)result;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ (uint8_t)result);
							*ram_ptr = (uint8_t)result;
						}
					} else {
						rom_write = true;
						if (nes.mapper_flags & NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE) result = *operand;
					}
					break;
				case C6502_INS_INX:
					nes.reg.x++;
					result = nes.reg.x;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_INY:
					nes.reg.y++;
					result = nes.reg.y;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_JMP:
					nes.reg.pc = addr;
					break;
				case C6502_INS_JSR:
					result = nes.reg.pc - 1;
					// get the stack base
					ram_ptr = nessys_ram(0x100);
					*(ram_ptr + nes.reg.s) = result >> 8;   nes.reg.s--;
					*(ram_ptr + nes.reg.s) = result & 0xFF; nes.reg.s--;
#ifdef _DEBUG
					nes.stack_trace[nes.stack_trace_entry].scanline = nes.scanline;
					nes.stack_trace[nes.stack_trace_entry].scanline_cycle = nes.scanline_cycle;
					nes.stack_trace[nes.stack_trace_entry].frame = nes.frame;
					nes.stack_trace[nes.stack_trace_entry].return_addr = nes.reg.pc;
					nes.stack_trace[nes.stack_trace_entry].jump_addr = addr;
					nes.stack_trace_entry++;
					if (nes.stack_trace_entry >= NESSYS_STACK_TRACE_ENTRIES) nes.stack_trace_entry = 0;
#endif
					nes.reg.pc = addr;
					break;
				case C6502_INS_LDA:
					nes.reg.a = *operand;
					result = nes.reg.a;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_LDX:
					nes.reg.x = *operand;
					result = nes.reg.x;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_LDY:
					nes.reg.y = *operand;
					result = nes.reg.y;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_LSR:
					overflow = *operand & 0x01;
					result = *operand >> 1;
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= overflow << C6502_P_C_SHIFT;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = (uint8_t)result;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = (uint8_t)result;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = (uint8_t)result;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ (uint8_t)result);
							*ram_ptr = (uint8_t)result;
						}
					} else {
						rom_write = true;
						if (nes.mapper_flags & NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE) result = *operand;
					}
					break;
				case C6502_INS_ORA:
					nes.reg.a |= *operand;
					result = nes.reg.a;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_PHA:
					// get the stack base
					ram_ptr = nessys_ram(0x100);
					*(ram_ptr + nes.reg.s) = nes.reg.a;   nes.reg.s--;
					break;
				case C6502_INS_PHP:
					// get the stack base
					ram_ptr = nessys_ram(0x100);
					*(ram_ptr + nes.reg.s) = nes.reg.p;   nes.reg.s--;
					break;
				case C6502_INS_PLA:
					// get the stack base
					operand = nessys_mem(0x100, &bank, &offset);
					nes.reg.s++; nes.reg.a = *(operand + nes.reg.s);
					result = nes.reg.a;
					// clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_PLP:
					// get the stack base
					operand = nessys_mem(0x100, &bank, &offset);
					nes.iflag_delay = nes.reg.p;
					nes.reg.s++; nes.reg.p = *(operand + nes.reg.s) | C6502_P_U | C6502_P_B;
					nes.iflag_delay ^= nes.reg.p;
					nes.iflag_delay &= C6502_P_I;
					break;
				case C6502_INS_ROL:
					result = (*operand << 1) | ((nes.reg.p & C6502_P_C) >> C6502_P_C_SHIFT);
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= (result & 0x100) >> (8 - C6502_P_C_SHIFT);
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = (uint8_t)result;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = (uint8_t)result;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = (uint8_t)result;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ (uint8_t)result);
							*ram_ptr = (uint8_t)result;
						}
					} else {
						rom_write = true;
						if (nes.mapper_flags & NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE) result = *operand;
					}
					break;
				case C6502_INS_ROR:
					result = (*operand >> 1) | ((nes.reg.p & C6502_P_C) << (7 - C6502_P_C_SHIFT));
					// clear N/Z/C
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z | C6502_P_C);
					nes.reg.p |= (*operand & 0x1) << C6502_P_C_SHIFT;
					nes.reg.p |= ((result & 0xFF) == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (result & 0x80) >> (7 - C6502_P_N_SHIFT);
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = (uint8_t)result;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = (uint8_t)result;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = (uint8_t)result;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ (uint8_t)result);
							*ram_ptr = (uint8_t)result;
						}
					} else {
						rom_write = true;
						if (nes.mapper_flags & NESSYS_MAPPER_FLAG_DATA_FROM_SOURCE) result = *operand;
					}
					break;
				case C6502_INS_RTI:
					// get the stack base
					ram_ptr = nessys_ram(0x100);
					nes.reg.s++; nes.reg.p = *(ram_ptr + nes.reg.s) | C6502_P_U | C6502_P_B;
					nes.reg.s++; nes.reg.pc = *(ram_ptr + nes.reg.s);
					nes.reg.s++; nes.reg.pc |= ((uint16_t) * (ram_ptr + nes.reg.s)) << 8;
					if (nes.in_nmi) {
						nes.in_nmi--;
						//nes.ppu.reg[2] = nes.ppu.old_status;
						//reset_ppu_status_after_nmi = 3;
					}
#ifdef _DEBUG
					if (nes.stack_trace_entry == 0) nes.stack_trace_entry = NESSYS_STACK_TRACE_ENTRIES;
					nes.stack_trace_entry--;
					if (nes.irq_trace_entry == 0) nes.irq_trace_entry = NESSYS_STACK_TRACE_ENTRIES;
					nes.irq_trace_entry--;
#endif
					break;
				case C6502_INS_RTS:
					// get the stack base
					ram_ptr = nessys_ram(0x100);
					nes.reg.s++; result = *(ram_ptr + nes.reg.s);
					nes.reg.s++; result |= ((uint16_t) * (ram_ptr + nes.reg.s)) << 8;
					nes.reg.pc = result + 1;
#ifdef _DEBUG
					if (nes.stack_trace_entry == 0) nes.stack_trace_entry = NESSYS_STACK_TRACE_ENTRIES;
					nes.stack_trace_entry--;
#endif
					break;
				case C6502_INS_SEC:
					nes.reg.p |= C6502_P_C;
					break;
				case C6502_INS_SED:
					nes.reg.p |= C6502_P_D;
					break;
				case C6502_INS_SEI:
					nes.iflag_delay = nes.reg.p;
					nes.reg.p |= C6502_P_I;
					nes.iflag_delay ^= nes.reg.p;
					nes.iflag_delay &= C6502_P_I;
					break;
				case C6502_INS_STA:
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = nes.reg.a;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = nes.reg.a;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = nes.reg.a;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ nes.reg.a);
							*ram_ptr = nes.reg.a;
						}
					} else {
						rom_write = true;
						result = nes.reg.a;
					}
					break;
				case C6502_INS_STX:
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_STATUS_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = nes.reg.x;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = nes.reg.x;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = nes.reg.x;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ nes.reg.x);
							*ram_ptr = nes.reg.x;
						}
					} else {
						rom_write = true;
						result = nes.reg.x;
					}
					break;
				case C6502_INS_STY:
					if (bank < NESSYS_PRG_ROM_START_BANK && !(bank == NESSYS_APU_REG_START_BANK && offset >= NESSYS_APU_SIZE)) {
						ppu_write = (bank == NESSYS_PPU_REG_START_BANK) || (bank == NESSYS_APU_REG_START_BANK && offset == 0x14);
						apu_write = (bank == NESSYS_APU_REG_START_BANK);
						if (apu_write && offset >= NESSYS_APU_JOYPAD0_OFFSET) {
							switch (offset) {
							case NESSYS_APU_STATUS_OFFSET:
								nes.apu.status = nes.reg.y;
								break;
							case NESSYS_APU_JOYPAD0_OFFSET:
								nes.apu.joy_control = nes.reg.y;
								break;
							case NESSYS_APU_FRAME_COUNTER_OFFSET:
								nes.apu.frame_counter = nes.reg.y;
								break;
							}
						} else if (ram_ptr) {
							data_change = (*operand ^ nes.reg.y);
							*ram_ptr = nes.reg.y;
						}
					} else {
						rom_write = true;
						result = nes.reg.y;
					}
					break;
				case C6502_INS_TAX:
					nes.reg.x = nes.reg.a;
					//clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= (nes.reg.x == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (nes.reg.x & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_TAY:
					nes.reg.y = nes.reg.a;
					//clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= (nes.reg.y == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (nes.reg.y & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_TSX:
					nes.reg.x = nes.reg.s;
					//clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= (nes.reg.x == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (nes.reg.x & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_TXA:
					nes.reg.a = nes.reg.x;
					//clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= (nes.reg.a == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (nes.reg.a & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				case C6502_INS_TXS:
					nes.reg.s = nes.reg.x;
					break;
				case C6502_INS_TYA:
					nes.reg.a = nes.reg.y;
					//clear N/Z
					nes.reg.p &= ~(C6502_P_N | C6502_P_Z);
					nes.reg.p |= (nes.reg.a == 0x00) << C6502_P_Z_SHIFT;
					nes.reg.p |= (nes.reg.a & 0x80) >> (7 - C6502_P_N_SHIFT);
					break;
				default:
					// everything not decoded is a NOP
					// TODO: undocumented instructions
					// TODO: handle writing to memory mappers (writing to rom space)
					break;
				}

				if (bank == NESSYS_PPU_REG_START_BANK) {
					switch (offset) {
					case 2:
						// if we read or write ppu status, update it's value from the master status reg
						//nes.ppu.reg[2] &= 0x1F;
						//nes.ppu.reg[2] |= nes->ppu.status;
						nes.ppu.addr_toggle = 0;
						break;
					case 7:
						// latch read data after the instruction
						if (!ppu_write) {
							nes.ppu.reg[7] = *nessys_ppu_mem(nes.ppu.mem_addr);
							// if bit 2 is 0, increment address by 1 (one step horizontal), otherwise, increment by 32 (one step vertical)
							nes.ppu.mem_addr += !((nes.ppu.reg[0] & 0x4) >> 1) + ((nes.ppu.reg[0] & 0x4) << 3);
							nes.ppu.mem_addr &= NESSYS_PPU_WIN_MAX;
						}
						break;
					}
				}

				// process PPU writes
				if (ppu_write) {
					if (bank == NESSYS_PPU_REG_START_BANK) {
						nes.ppu.reg[2] = (nes.ppu.status & 0xE0) | (nes.ppu.reg[offset & 0x7] & 0x1F);
					}
					switch (offset) {
					case 0x0:
						// if the app toggles vblank enable from 0 to 1 while in vblank, retrigger a vblank
						if ((nes.ppu.reg[2] & 0x80) && (data_change & 0x80) && (nes.ppu.reg[0] & 0x80)) {
							nessys_irq(NESSYS_NMI_VECTOR, C6502_P_B);
							next_line_scan_clk += NESSYS_PPU_PER_CPU_CLK * 7;
						}
						break;
					case 0x4:
						nes.ppu.oam[nes.ppu.reg[3]] = nes.ppu.reg[4];
						if (nes.scan_line >= NESSYS_PPU_SCANLINES_START_RENDER &&
							(((nes.ppu.reg[3] & 0x3) == 0x1) || ((nes.ppu.reg[3] & 0x3) == 0x2))) {
							// Changing sprite in the middle of a frame should be very rare, but in case, regenerate that sprite
							nessys_gen_oam_pix(nes.ppu.reg[3] >> 2);
						}
						nes.ppu.reg[3]++;
						break;
					case 0x5:
						// update the vram address as well
						if (nes.ppu.addr_toggle) {
							nes.ppu.t_mem_addr &= 0x0c1f;
							nes.ppu.t_mem_addr |= ((uint16_t)nes.ppu.reg[5] << 2) & 0x03e0;
							nes.ppu.t_mem_addr |= ((uint16_t)nes.ppu.reg[5] << 12) & 0x7000;
						} else {
							nes.ppu.t_mem_addr &= 0xffe0;
							nes.ppu.t_mem_addr |= (nes.ppu.reg[5] >> 3) & 0x1f;
						}
						nes.ppu.scroll[nes.ppu.addr_toggle] = nes.ppu.reg[5];
						nes.ppu.addr_toggle = !nes.ppu.addr_toggle;
						break;
					case 0x6:
						mem_addr_mask = 0xFF00 >> 8 * (nes.ppu.addr_toggle);
						nes.ppu.t_mem_addr &= ~mem_addr_mask;
						nes.ppu.t_mem_addr |= (((uint16_t)nes.ppu.reg[6] << 8) | nes.ppu.reg[6]) & mem_addr_mask;
						// update scroll and nametable select signals
						if (nes.ppu.addr_toggle) {
							nes.ppu.scroll[0] &= 0x07;
							nes.ppu.scroll[0] |= (nes.ppu.reg[6] << 3) & 0xf8;
							nes.ppu.scroll[1] &= 0xc7;
							nes.ppu.scroll[1] |= (nes.ppu.reg[6] >> 2) & 0x38;
						} else {
							nes.ppu.scroll[1] &= 0x38;
							nes.ppu.scroll[1] |= (nes.ppu.reg[6] << 6) & 0xc0;
							nes.ppu.scroll[1] |= (nes.ppu.reg[6] >> 4) & 0x03;
							nes.ppu.reg[0] &= 0xfc;
							nes.ppu.reg[0] |= (nes.ppu.reg[6] >> 2) & 0x3;
						}
						if (nes.ppu.addr_toggle) {
							nes.ppu.mem_addr = nes.ppu.t_mem_addr;
							nes.ppu.mem_addr &= NESSYS_PPU_WIN_MAX;
							nes.ppu.scroll_y = ((nes.ppu.reg[0] & 0x2) << 7) | nes.ppu.scroll[1];
							//nes.ppu.max_y = 240 + (nes.ppu.scroll_y & 0x100);
							nes.ppu.scroll_y_changed = true;
						}
						nes.ppu.addr_toggle = !nes.ppu.addr_toggle;
						break;
					case 0x7:
						*nessys_ppu_ram(nes.ppu.mem_addr) = nes.ppu.reg[7];
						if (nes.ppu.mem_addr >= NESSYS_CHR_PAL_WIN_MIN) {
							// alias 3f10, 3f14, 3f18 and 3f1c to corresponding 3f0x
							// and vice versa
							if ((nes.ppu.mem_addr & 0x3) == 0x0) {
								nes.ppu.pal[(nes.ppu.mem_addr & NESSYS_PPU_PAL_MASK) ^ 0x10] = nes.ppu.reg[7];
							}
						}
						// if bit 2 is 0, increment address by 1 (one step horizontal), otherwise, increment by 32 (one step vertical)
						nes.ppu.mem_addr += !((nes.ppu.reg[0] & 0x4) >> 1) + ((nes.ppu.reg[0] & 0x4) << 3);
						nes.ppu.mem_addr &= NESSYS_PPU_WIN_MAX;
						break;
					case 0x14:
						addr = nes.apu.reg[0x14] << 8;
						operand = nessys_mem(addr, &bank, &offset);
						memcpy(nes.ppu.oam, operand, NESSYS_PPU_OAM_SIZE);
						if (nes.scan_line >= NESSYS_PPU_SCANLINES_START_RENDER) {
							// Changing sprite in the middle of a frame should be very rare, but in case, regenerate sprites
							for (sp = 0; sp < NESSYS_PPU_NUM_SPRITES; sp++) {
								nessys_gen_oam_pix(sp);
							}
						}
						penalty_cycles += 514;
						break;
					}
				}

				// increment pc
				pc_ptr_next = nessys_mem(nes.reg.pc, &bank, &offset);
				op_next = C6502_OP_CODE + *pc_ptr_next;
				nes.scan_clk += NESSYS_PPU_PER_CPU_CLK * (op->num_cycles + penalty_cycles) + next_line_scan_clk;  // add the extra cycles from the prior line, or IRQ
				next_line_scan_clk = 0;
				if (nes.scan_clk >= nes.sprite0_hit_scan_clk) {
					nes.ppu.reg[2] |= 0x40;
					nes.sprite0_hit_scan_clk = ~0;
				}

				// if we are not in the renderable part of the frame, copy the vertical scroll value
				if (nes.scan_line < NESSYS_PPU_SCANLINES_START_RENDER) {
					nes.ppu.scroll_y = ((nes.ppu.reg[0] & 0x2) << 7) | nes.ppu.scroll[1];
					//nes.ppu.max_y = 240 + (nes.ppu.scroll_y & 0x100);
					nes.ppu.scroll_y_changed = true;
				}
#ifndef PPU_MULTI_THREAD
				process_ppu();
#endif

			}

			next_line_scan_clk = nes.scan_clk - NESSYS_PPU_CLK_PER_SCANLINE;
			next_scan_line = nes.scan_line + 1;
			y = next_scan_line - NESSYS_PPU_SCANLINES_START_RENDER;

			// if background is enabled and we're going to the first rendered line, or we're at the beginning of a new row of tiles,
			// regenerate the tile pixels
			bool gen_tile_pix = (nes.ppu.reg[1] & 0x8) &&
				((next_scan_line == NESSYS_PPU_SCANLINES_START_RENDER) || (((y + nes.ppu.scroll_y) & 0x7) == 0x0));

			if (next_scan_line >= NESSYS_PPU_SCANLINES_START_RENDER) {
				if (gen_tile_pix) {
					nessys_gen_tile_pix(y);
				}
			}

#ifdef PPU_MULTI_THREAD
			// check if we rendered a scanline
			if (nes.scan_line >= NESSYS_PPU_SCANLINES_START_RENDER && (nes.frame_delta_time <= 0) &&
				nes.scan_line < NESSYS_PPU_SCANLINES_START_RENDER + FB_HEIGHT) {

				//bool c0_render_done = false;
				//nes.scan_clk = FB_WIDTH;
				//
				//y = nes.scan_line - NESSYS_PPU_SCANLINES_START_RENDER;
				//while (!c0_render_done) {
				//	nes.scan_clk -= RENDER_PIXEL_INC;
				//	if (nes.scan_clk > nes.rendered_scan_clk) {
				//		min_x = nes.scan_clk;
				//		max_x = nes.scan_clk + RENDER_PIXEL_INC;
				//		max_x = (max_x > FB_WIDTH) ? FB_WIDTH : max_x;
				//
				//		process_pixels(min_x, max_x, y, &nes.c0_rstate);
				//	} else {
				//		nes.scan_clk += RENDER_PIXEL_INC;
				//		c0_render_done = true;
				//	}
				//}

				// This first assign is to prevent c1 from getting into the right half of the screen while the mid point is being computed
				nes.scan_clk = FB_WIDTH / 2;  
				nes.scan_clk = ((FB_WIDTH / 2 + RENDER_PIXEL_INC - 1) + (nes.rendered_scan_clk >> 1)) & ~(RENDER_PIXEL_INC - 1);
				uint y = nes.scan_line - NESSYS_PPU_SCANLINES_START_RENDER;
				nes.c0_rstate.tile_x = 0;  // force tile re-evaluation
				process_pixels(nes.scan_clk, FB_WIDTH, y, &nes.c0_rstate);

				loop_count = nes.scan_clk;
				while (!nes.c1_render_done)
					;

			}
#endif

			nes.scan_line++;

			// if we're going into the renderable part of the frame, reload the scan_line oam at the end of each scan line
			if (nes.scan_line >= NESSYS_PPU_SCANLINES_START_RENDER) {
				uint i, sp_height = (nes.ppu.reg[0] & 0x20) ? 16 : 8;
				uint sp_y;

				// if background is enabled and we're going to the first rendered line, or we're at the beginning of a new row of tiles,
				// regenerate the tile pixels
				if (gen_tile_pix) {
					//nessys_gen_tile_pix(y);
					uint16_t* temp = nes.ppu.draw_tile_pix;
					nes.ppu.draw_tile_pix = nes.ppu.disp_tile_pix;
					nes.ppu.disp_tile_pix = temp;
					if (((y + nes.ppu.scroll_y) & 0x1f) == 0x0) {
						uint8_t* temp = nes.ppu.draw_attrib_pix;
						nes.ppu.draw_attrib_pix = nes.ppu.disp_attrib_pix;
						nes.ppu.disp_attrib_pix = temp;
					}
				}

				// needed for sprite0 hit evaluation
				uint sprite_x, sprite_y, pat_addr, sp_planes, pal_index;
				uint max_sprites = (nes.frame_delta_time <= 0) ? NESSYS_PPU_NUM_SPRITES : 1;
				//bool h_flip;

				// initialize to crossed range to indicate no sprites
				nes.ppu.scan_line_min_sprite_x = 0xff;
				nes.ppu.scan_line_max_sprite_x = 0;
				for (i = 0, sp = 0; i < max_sprites && sp < NESSYS_PPU_MAX_SPRITES_PER_SCAN_LINE; i++) {
					sp_y = nes.ppu.oam[4 * i];
					// Get y coordinate in sprite space
					sprite_y = y - sp_y;
					if (y >= sp_y && sprite_y < sp_height) {
						//if (i == 0) {
						//	// determine pattern planes for sprite 0
						//	sprite_y = y - sp_y;
						//	// Flip y if vetinal bit is set
						//	sprite_y = (nes.ppu.oam[2] & 0x80) ? (7 + ((nes.ppu.reg[0] >> 1) & 0x8)) - sprite_y : sprite_y;
						//	pat_addr = nes.ppu.oam[1];
						//	// for 8x16 sprites, lsb is the bank select
						//	// otherwise, bank select is bit 3 ppu reg0
						//	pat_addr |= 0x100 & (((pat_addr << 8) & (((uint16_t)nes.ppu.reg[0]) << 3)) |
						//		((((uint16_t)nes.ppu.reg[0]) << 5) & ~(((uint16_t)nes.ppu.reg[0]) << 3)));
						//	// clear out lsb for 8x16, and fill it in based on whether sprite y is 8 or above
						//	pat_addr &= 0xfffe | ~(((uint16_t)nes.ppu.reg[0]) >> 5);
						//	pat_addr |= (((uint16_t)nes.ppu.reg[0]) >> 5) & (sprite_y >= 8);
						//	// shift up the address and fill in the y offset
						//	pat_addr <<= 4;
						//	pat_addr |= sprite_y & 0x7;
						//	sp_planes = *(nessys_ppu_mem(pat_addr));
						//	sp_planes |= (*(nessys_ppu_mem(pat_addr | 0x8)) << 8);
						//	h_flip = ((nes.ppu.oam[2] & 0x40) != 0);
						//	sp_plane_shift = (h_flip) ? 0 : 7;
						//}

						// Flip y if vertical bit is set
						//sprite_y = (nes.ppu.oam[4 * i + 2] & 0x80) ? (7 + ((nes.ppu.reg[0] >> 1) & 0x8)) - sprite_y : sprite_y;
						sp_planes = nes.ppu.oam_pix[NESSYS_PPU_OAM_PIXEL_ROWS * i | sprite_y];
						//h_flip = ((nes.ppu.oam[4 * i + 2] & 0x40) != 0);
						//sp_plane_shift = 0;// (h_flip) ? 14 : 0;

						sp_x = *(nes.ppu.oam + 4 * i + 3);
						offset = sp_x + 8;
						offset = (offset >= 256) ? 256 : offset;
						nes.ppu.scan_line_min_sprite_x = (sp_x < nes.ppu.scan_line_min_sprite_x) ? sp_x : nes.ppu.scan_line_min_sprite_x;
						nes.ppu.scan_line_max_sprite_x = (offset > nes.ppu.scan_line_max_sprite_x) ? offset-1 : nes.ppu.scan_line_max_sprite_x;
						for (; sp_x < offset; sp_x++) {
							if ((nes.ppu.scan_line_sprite[sp_x] & 0x3) == 0) {
								pal_index = sp_planes & 0x3;// (sp_planes >> sp_plane_shift) & 0x3;
								if(nes.frame_delta_time <= 0) nes.ppu.scan_line_sprite[sp_x] = (i << 2) | pal_index;
								// TODO: need to check if sprite 0 actually overlaps background
								if (i == 0 && nes.sprite0_hit_scan_clk == ~0 && pal_index != 0) {
									nes.sprite0_hit_scan_clk = sp_x;
								}
								sp_planes = (sp_planes >> 2);// (h_flip) ? (sp_planes << 2) : (sp_planes >> 2);
							}
						}
						//nes.ppu.scan_line_oam[sp] = nes.ppu.oam + 4 * i;
						sp++;
					}
				}
				//nes.ppu.num_scan_line_oam = sp;
			}

			if (nes.scan_line == NESSYS_PPU_SCANLINES_START_RENDER) {
				// clear ppu status flag as we begin rendering
				nes.ppu.reg[2] &= ~0xE0;
				// regenerate the sprites
				for (sp = 0; sp < NESSYS_PPU_NUM_SPRITES; sp++) {
					nessys_gen_oam_pix(sp);
				}
			}
		}

		if (nes.frame_delta_time <= 0) {
			skipped_frames = 0;
			flip_framebuffer();
		} else {
			skipped_frames++;
			total_skipped_frames++;
		}

#ifdef WIN32
		ZeroMemory(&msg, sizeof(MSG));

		bool got_msg = (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) != 0);
		while (got_msg) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			got_msg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
		}
		win32_write(disp_frame);
		InvalidateRect(hwnd, NULL, false);
		//win32_display(hwnd);
#else
		// Wait for prior DMA before issuing the next frame's
		//if ((frame & 0x3f) == 0) {
		st7789_wait_for_write();
		st7789_write(disp_frame, FB_SIZE);
		//}
#endif
		nes.frame++;
		nes.rendered_frames++;
	}
}

void main()
{
#ifdef WIN32
	win32_init();
#else
    //uint vco_freq, postdiv1, postdiv2;
    //check_sys_clock_khz(125000, &vco_freq, &postdiv1, &postdiv2);
    set_sys_clock_khz(SYS_CLK_KHZ, false);
    clock_configure(clk_peri,
        0, // Only AUX mux on ADC
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        SYS_CLK_KHZ * 1000,
        SYS_CLK_KHZ * 1000);
    stdio_init_all();
    lcd_init(true);
#endif


	main_loop();
}
