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

#include "opd_common.h"
#include "opd_log.h"
#include "opd_freetype.h"
#include "opd_framebuffer.h"
#include "opd_lcd.h"
#include "opd_utils.h"
#include "opd_menu.h"

static opd_device_item *opd_device_items = NULL;
static int opd_menu_offset = 0;
static int opd_menu_selected = 0;

void opd_menu_set(opd_device_item *items)
{
	opd_device_items = items;
}

opd_device_item *opd_menu_get_last()
{
	opd_device_item *tmp = opd_device_items;
	while (tmp) {
		if (!tmp->next)
			return tmp;
		
		tmp = tmp->next;
	}
	return NULL;
}

int opd_menu_count()
{
	int count = 0;
	opd_device_item *tmp = opd_device_items;
	while (tmp) {
		count++;
		tmp = tmp->next;
	}
	return count;
}

opd_device_item *opd_menu_get(int position) {
	opd_device_item *tmp = opd_device_items;
	int count = 0;
	while (tmp) {
		if (count == position)
			return tmp;
		tmp = tmp->next;
		count++;
	}
	return NULL;
}

opd_device_item *opd_menu_get_selected()
{
	return opd_menu_get(opd_menu_selected);
}

void opd_menu_set_selected(const char *identifier)
{
	opd_device_item *tmp = opd_device_items;
	int count = 0;
	while (tmp) {
		if (strcmp(tmp->identifier, identifier) == 0) {
			opd_menu_selected = count;
			break;
		}
		tmp = tmp->next;
		count++;
	}
}

void opd_menu_next()
{
	int position = opd_menu_selected;
	position++;
	if (position >= opd_menu_count())
		position = opd_menu_count() - 1;
	opd_menu_selected = position;
	
	if (position >= opd_menu_offset + OPD_MENU_MAX_ITEMS)
		opd_menu_offset = position - OPD_MENU_MAX_ITEMS + 1;
	if (opd_menu_offset < 0)
		opd_menu_offset = 0;
}

void opd_menu_prev()
{
	int position = opd_menu_selected;
	position--;
	if (position < 0)
		position = 0;
	opd_menu_selected = position;
	
	if (position < opd_menu_offset)
		opd_menu_offset = position;
}

void opd_menu_render()
{
	int i;
	int count = opd_menu_count();
	int visible_count = count < OPD_MENU_MAX_ITEMS ? count : OPD_MENU_MAX_ITEMS;
	int screen_width = opd_get_screen_width();
	int screen_height = opd_get_screen_height();
	int box_width = OPD_MENU_ITEM_WIDTH + (OPD_MENU_BOX_MARGIN * 2);
	int box_height = (OPD_MENU_ITEM_HEIGHT * visible_count) + (OPD_MENU_BOX_MARGIN * (visible_count + 1));
	int box_x = (screen_width - box_width) / 2;
	int box_y = (screen_height - box_height) / 2;
	
	opd_draw_rounded_rect(box_x, box_y, box_width, box_height, OPD_MENU_BOX_COLOR, OPD_MENU_BOX_RADIUS);
	
	if (opd_menu_offset > 0) {
		opd_render_symbol(OPD_SYMBOL_ARROW_UP,
			box_x + OPD_MENU_BOX_MARGIN,
			box_y - 70,
			OPD_MENU_ITEM_WIDTH,
			OPD_MENU_ARROWS_COLOR,
			OPD_MENU_ARROWS_SIZE,
			OPD_TEXT_ALIGN_CENTER);
	}
		
	for (i = opd_menu_offset; i < visible_count + opd_menu_offset; i++) {
		opd_device_item *item = opd_menu_get(i);
		int color = OPD_MENU_ITEM_COLOR;
		if (i == opd_menu_selected) {
#ifdef OPD_HAVE_TEXTLCD
			opd_lcd_write_text(item->label);
#else
			int selection_y = opd_lcd_get_height() * OPD_LCD_SELECTION_Y;
			int selection_size = opd_lcd_get_width() * OPD_LCD_SELECTION_SIZE;
			
			opd_render_lcd_text(item->label,
				0,
				selection_y,
				opd_lcd_get_width(),
				OPD_LCD_SELECTION_COLOR,
				selection_size,
				OPD_TEXT_ALIGN_CENTER);
#endif
			
			color = OPD_MENU_ITEM_SELECTED_COLOR;
		}
		
		opd_draw_rounded_rect(box_x + OPD_MENU_BOX_MARGIN,
			box_y + OPD_MENU_BOX_MARGIN,
			OPD_MENU_ITEM_WIDTH,
			OPD_MENU_ITEM_HEIGHT,
			color,
			OPD_MENU_ITEM_RADIUS);
			
		opd_render_text(item->label,
			box_x + OPD_MENU_BOX_MARGIN,
			box_y + OPD_MENU_BOX_MARGIN + OPD_MENU_ITEM_HEIGHT - OPD_MENU_ITEM_TEXT_BOTTON_MARGIN,
			OPD_MENU_ITEM_WIDTH,
			OPD_MENU_ITEM_TEXT_COLOR,
			OPD_MENU_ITEM_TEXT_FONT_SIZE,
			OPD_TEXT_ALIGN_CENTER);
			
		box_y += OPD_MENU_ITEM_HEIGHT + OPD_MENU_BOX_MARGIN;
	}
	
	if (opd_menu_offset + OPD_MENU_MAX_ITEMS < count) {
		opd_render_symbol(OPD_SYMBOL_ARROW_DOWN,
			box_x + OPD_MENU_BOX_MARGIN,
			box_y + 20,
			OPD_MENU_ITEM_WIDTH,
			OPD_MENU_ARROWS_COLOR,
			OPD_MENU_ARROWS_SIZE,
			OPD_TEXT_ALIGN_CENTER);
	}
}
