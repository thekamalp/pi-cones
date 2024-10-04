// Project:     pi_cones (originally from ArkNESS)
// File:        nessys.h
// Author:      Kamal Pillai
// Date:        7/12/2021
// Description:	NES menu functions

#ifndef __NESMENU_H
#define __NESMENU_H

struct nessys_t;

typedef enum {
	NONE,  // emulation is running
	MAIN,
	OPEN,
	OPTIONS,
} nesmenu_pane_t;

static const uint32_t NESMENU_ITEM_FLAG_NONE = 0x0;
static const uint32_t NESMENU_ITEM_FLAG_DIRECTORY = 0x1;
static const uint32_t NESMENU_ITEM_FLAG_UP_DIR = 0x2;
static const uint32_t NESMENU_ITEM_FLAG_FILE = 0x4;

typedef struct {
	const char* item;
	uint32_t flag;
} nesmenu_list_item;

typedef struct {
	nesmenu_pane_t pane;
	uint32_t select;
	uint32_t list_start;
	nesmenu_list_item* cur_list;
	uint32_t cur_list_size;
	uint32_t cur_list_alloc_size;
	const char* message_box;
	uint8_t last_num_joy;
	uint8_t last_joypad_state[2];
	uint8_t sprite_line_limit;
} nesmenu_data;

#define NESMENU_MAIN_ITEMS 3
static const uint8_t nesmenu_main_item_open = 0;
static const uint8_t nesmenu_main_item_options = 1;
static const uint8_t nesmenu_main_item_exit = 2;
static const char nesmenu_main[NESMENU_MAIN_ITEMS][32] = { "Open", "Options", "Exit" };

#define NESMENU_OPTIONS_ITEMS 1
static const uint8_t nesmenu_options_item_sprite_line_limit = 0;
static const char nesmenu_options[NESMENU_OPTIONS_ITEMS][32] = { "Sprite line limit: " };

void nesmenu_init();
void nesmenu_update_list();
void nesmenu_display();
void nesmenu_load_options();
void nesmenu_save_options();
void nesmenu_cleanup();

#endif
