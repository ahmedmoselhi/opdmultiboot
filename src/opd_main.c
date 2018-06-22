/*
 *
 * Copyright (C) 2014 Impex-Sat Gmbh & Co.KG
 * Written by Sandro Cavazzoni <sandro@skanetwork.com>
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>

#include "opd_common.h"
#include "opd_log.h"
#include "opd_freetype.h"
#include "opd_framebuffer.h"
#include "opd_lcd.h"
#include "opd_input.h"
#include "opd_utils.h"
#include "opd_menu.h"

static int opd_timer_enabled;
static int opd_current_timer;
static int opd_timer;
char opd_vumodel[63];

void opd_draw_header()
{
	char tmp[255];
	sprintf(tmp, "%s %s", opd_DISPLAY_NAME, opd_APP_VERION);
	opd_render_text(tmp,
		opd_HEADER_X + 45,
		opd_HEADER_Y,
		400,
		opd_HEADER_COLOR,
		opd_HEADER_FONT_SIZE,
		opd_TEXT_ALIGN_LEFT);
}

void opd_draw_lcd()
{
	char tmp[255];
	sprintf(tmp, "%s %s", opd_DISPLAY_NAME, opd_APP_VERION);
	
	int logo_x = opd_lcd_get_width() * opd_LCD_LOGO_X;
	int logo_y = opd_lcd_get_height() * opd_LCD_LOGO_Y;
	int logo_size = opd_lcd_get_width() * opd_LCD_LOGO_SIZE;
	
	int title_x = opd_lcd_get_width() * opd_LCD_TITLE_X;
	int title_y = opd_lcd_get_height() * opd_LCD_TITLE_Y;
	int title_size = opd_lcd_get_width() * opd_LCD_TITLE_SIZE;

	if (! strcmp(opd_vumodel,""))
	opd_render_lcd_symbol(opd_SYMBOL_LOGO,
		logo_x,
		logo_y,
		0,
		opd_LCD_LOGO_COLOR,
		logo_size,
		opd_TEXT_ALIGN_LEFT);
	else {
		if (! strcmp(opd_vumodel,"duo2"))
			title_y += 2;

		sprintf(tmp, "VU+ %s %s", opd_DISPLAY_NAME, opd_APP_VERION);
		title_x = logo_x;
	}
	opd_render_lcd_text(tmp,
		title_x,
		title_y,
		0,
		opd_LCD_TITLE_COLOR,
		title_size,
		opd_TEXT_ALIGN_LEFT);
}

void opd_draw_timer()
{
	if (opd_timer_enabled) {
		char tmp[255];
		sprintf(tmp, "%d", opd_current_timer);
		opd_render_text(tmp,
			opd_get_screen_width() - (400 + opd_TIMER_RIGHT_MARGIN),
			opd_TIMER_Y,
			400,
			opd_TIMER_COLOR,
			opd_TIMER_FONT_SIZE,
			opd_TEXT_ALIGN_RIGHT);
	}
}

void opd_refresh_gui()
{
	opd_clear_screen();
	opd_lcd_clear();
	
	opd_draw_lcd();
	opd_draw_header();
	opd_draw_timer();
	opd_menu_render();
	
	opd_blit();
	opd_lcd_update();
}

int opd_show_menu()
{
	struct timeval start, end;
	
	if (opd_open_framebuffer() == opd_ERROR)
		return opd_ERROR;
	
	if (opd_init_freetype() == opd_ERROR)
		return opd_ERROR;
	
	if (opd_input_open() == opd_ERROR)
		return opd_ERROR;
	
	opd_lcd_open();
	
	opd_timer_enabled = 1;
	opd_timer = opd_utils_gettimer();
	opd_current_timer = opd_timer;
	gettimeofday(&start, NULL);
	
	opd_refresh_gui();
	
	for(;;) {
		usleep(20000);
		int need_refresh_gui = 0;
		int code = opd_input_get_code();
		if (code == KEY_OK)
			break;
		else if (code == KEY_UP) {
			opd_menu_prev();
			opd_utils_update_background(opd_menu_get_selected());
			need_refresh_gui = 1;
			opd_timer_enabled = 0;
		}
		else if (code == KEY_DOWN) {
			opd_menu_next();
			opd_utils_update_background(opd_menu_get_selected());
			need_refresh_gui = 1;
			opd_timer_enabled = 0;
		}
		
		if (opd_timer_enabled) {
			long mtime, seconds, useconds;
			gettimeofday(&end, NULL);
			
			seconds  = end.tv_sec  - start.tv_sec;
			useconds = end.tv_usec - start.tv_usec;

			mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
			int last_value = opd_current_timer;
			opd_current_timer = opd_timer - (mtime / 1000);
			
			if (opd_current_timer != last_value)
				need_refresh_gui = 1;
		}
		
		if (need_refresh_gui)
			opd_refresh_gui();
		
		
		if (opd_current_timer == 0)
			break;
	}

	opd_clear_screen();
	opd_blit();
	
	opd_lcd_clear();
	opd_lcd_update();
	
	opd_lcd_close();
	opd_input_close();
	opd_deinit_freetype();
	opd_close_framebuffer();
	
	return opd_SUCCESS;
}

int main(int argc, char *argv[]) 
{
	int is_rebooting = 0;

	if (argc > 1 && getppid() > 1) {
		opd_utils_sysvinit(NULL, argv[1]);
	}
	else {
		opd_vumodel[0] = '\0';

		opd_utils_init_system();
		opd_device_item *item = NULL;
		opd_device_item *items = NULL;
		char *selected = NULL;
		char *nextboot = NULL;
		if (opd_utils_find_and_mount() == opd_SUCCESS) {
			items = opd_utils_get_images();
			opd_menu_set(items);
			selected = opd_utils_read(opd_SETTINGS_SELECTED);
			if (!selected) {
				selected = malloc(6);
				strcpy(selected, "flash");
			}
			opd_menu_set_selected(selected);
			item = opd_menu_get_selected();
		}
/*
 * by Meo. load_modules moved !
 */
		opd_utils_prepare_destination(item);

		int lock_menu = opd_utils_check_lock_menu();
		int force = opd_utils_read_int(opd_SETTINGS_FORCE);
		if (!force && items) 
		{
			opd_log(LOG_DEBUG, "%-33s: preparing environment...", __FUNCTION__);
			if (!lock_menu) {
				opd_log(LOG_DEBUG, "%-33s: loading modules...", __FUNCTION__);
				opd_utils_load_modules(item);
				if (!opd_utils_file_exists(opd_VIDEO_DEVICE)) {
					opd_utils_load_modules_gl(item);
				}
				opd_utils_setrctype();
			}
			opd_utils_update_background(item);
			opd_utils_backup_kernel(item);

			nextboot = opd_utils_read(opd_SETTINGS_NEXTBOOT);
			if (nextboot) {
				opd_menu_set_selected(nextboot);
				opd_utils_remove_nextboot();
				item = opd_menu_get_selected();
				opd_utils_update_background(item);
				free(nextboot);
			}
			
			if (!lock_menu) {
				opd_log(LOG_DEBUG, "%-33s: menu enabled", __FUNCTION__);
				FILE *fvu = fopen("/proc/stb/info/vumodel", "r");
				if (fvu) {
					char tmp[63];
					if (fscanf(fvu, "%s", &tmp) == 1) {
						strcpy(opd_vumodel, tmp);
					}
					fclose(fvu);
				}
				opd_log(LOG_DEBUG, "%-33s: boxmodel: %s", __FUNCTION__, opd_vumodel);
				opd_show_menu();
			} else {
				opd_log(LOG_DEBUG, "%-33s: menu disabled", __FUNCTION__);
			}
		}
		else {
			opd_log(LOG_DEBUG, "%-33s: opd_utils_save_int(opd_SETTINGS_FORCE, 0)", __FUNCTION__);
			opd_utils_save_int(opd_SETTINGS_FORCE, 0);
		}

		item = opd_menu_get_selected();
		if ((item && selected && strcmp(selected, item->identifier)) != 0 || (item && strstr(item->identifier, "vti") && !force)) {
			opd_utils_restore_kernel(item);
			opd_utils_save(opd_SETTINGS_SELECTED, item->identifier);
			opd_utils_save_int(opd_SETTINGS_FORCE, 1);
			opd_utils_umount(opd_MAIN_DIR);
			opd_utils_reboot();
			is_rebooting = 1;
		}
		
		if (!is_rebooting) {
			if (item != NULL && strcmp(item->identifier, "flash") != 0)
				opd_utils_remount_media(item);
			opd_utils_umount(opd_MAIN_DIR);
			opd_utils_sysvinit(item, NULL);
		}

		if (items) opd_utils_free_items(items);
		if (selected) free(selected);
	}

	return opd_SUCCESS;
}

