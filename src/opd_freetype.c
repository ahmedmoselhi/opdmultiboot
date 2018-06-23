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
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include "opd_common.h"
#include "opd_log.h"
#include "opd_framebuffer.h"
#include "opd_lcd.h"
#include "opd_segoe_ui_font.h"
#include "opd_lcddot_font.h"
#include "opd_icomoon_font.h"
#include "opd_freetype.h"

#define MAX_GLYPHS 255

static FT_Library opd_freetype_library;
static FT_Face opd_freetype_face;
static FT_Face opd_freetype_lcd_face;
static FT_Face opd_freetype_symbols_face;
static FT_GlyphSlot opd_freetype_slot;
static FT_GlyphSlot opd_freetype_lcd_slot;
static FT_GlyphSlot opd_freetype_symbols_slot;

int opd_init_freetype()
{
	if (FT_Init_FreeType(&opd_freetype_library) != 0) {
		opd_log(LOG_ERROR, "cannot init freetype");
		return opd_ERROR;
	}

	//opd_log(LOG_DEBUG, "opd_init_freetype boxmodel: %s",opd_vumodel);
	if (strcmp(opd_vumodel,"duo2")) {
		//opd_log(LOG_DEBUG, "opd_init_freetype opd_segoe_ui_font");
		if (FT_New_Memory_Face(opd_freetype_library, (const FT_Byte*)opd_segoe_ui_font, opd_segoe_ui_font_length, 0, &opd_freetype_face) != 0) {
			opd_log(LOG_ERROR, "%-33s: cannot open base font", __FUNCTION__);
			return OPD_ERROR;
		}
		if (FT_New_Memory_Face(opd_freetype_library, (const FT_Byte*)opd_segoe_ui_font, opd_segoe_ui_font_length, 0, &opd_freetype_lcd_face) != 0) {
			opd_log(LOG_ERROR, "%-33s: cannot open base font", __FUNCTION__);
			return OPD_ERROR;
		}
	} else {
		//opd_log(LOG_DEBUG, "opd_init_freetype opd_lcddot_font");
		if (FT_New_Memory_Face(opd_freetype_library, (const FT_Byte*)opd_segoe_ui_font, opd_segoe_ui_font_length, 0, &opd_freetype_face) != 0) {
			opd_log(LOG_ERROR, "%-33s: cannot open base font", __FUNCTION__);
			return OPD_ERROR;
		}
		if (FT_New_Memory_Face(opd_freetype_library, (const FT_Byte*)opd_lcddot_font, opd_lcddot_font_length, 0, &opd_freetype_lcd_face) != 0) {
			opd_log(LOG_ERROR, "%-33s: cannot open base font", __FUNCTION__);
			return OPD_ERROR;
		}
	}

	if (FT_New_Memory_Face(opd_freetype_library, (const FT_Byte*)opd_icomoon_font, opd_icomoon_font_length, 0, &opd_freetype_symbols_face) != 0) {
		opd_log(LOG_ERROR, "cannot open symbols font");
		return OPD_ERROR;
	}
	
	opd_freetype_slot = opd_freetype_face->glyph;
	opd_freetype_lcd_slot = opd_freetype_lcd_face->glyph;
	opd_freetype_symbols_slot = opd_freetype_symbols_face->glyph;
	
	return OPD_SUCCESS;
}

void opd_deinit_freetype()
{
	FT_Done_Face(opd_freetype_face);
	FT_Done_Face(opd_freetype_lcd_face);
	FT_Done_Face(opd_freetype_symbols_face);
	FT_Done_FreeType(opd_freetype_library);
}

int opd_render_symbol(int code, int x, int y, int width, int color, int font_size, int align)
{
	if (FT_Set_Char_Size(opd_freetype_symbols_face, font_size * 64, 0, 100, 0)) {
		opd_log(LOG_ERROR, "cannot set font size");
		return OPD_ERROR;
	}
	
	if (FT_Load_Char(opd_freetype_symbols_face, code, FT_LOAD_RENDER) != 0)
		return OPD_ERROR;
	
	int offset = 0;
	if (align == OPD_TEXT_ALIGN_CENTER)
		offset = (width - opd_freetype_symbols_slot->bitmap.width) / 2;
	else if (align == OPD_TEXT_ALIGN_RIGHT)
		offset = width - opd_freetype_symbols_slot->bitmap.width;
	
	opd_draw_character(&opd_freetype_symbols_slot->bitmap, offset + x, y, color);
	
	return OPD_SUCCESS;
}

int opd_render_lcd_symbol(int code, int x, int y, int width, int color, int font_size, int align)
{
	if (FT_Set_Char_Size(opd_freetype_symbols_face, font_size * 64, 0, 100, 0)) {
		opd_log(LOG_ERROR, "cannot set font size");
		return OPD_ERROR;
	}
	
	if (FT_Load_Char(opd_freetype_symbols_face, code, FT_LOAD_RENDER) != 0)
		return OPD_ERROR;
	
	int offset = 0;
	if (align == OPD_TEXT_ALIGN_CENTER)
		offset = (width - opd_freetype_symbols_slot->bitmap.width) / 2;
	else if (align == OPD_TEXT_ALIGN_RIGHT)
		offset = width - opd_freetype_symbols_slot->bitmap.width;
	
	opd_lcd_draw_character(&opd_freetype_symbols_slot->bitmap, offset + x, y, color);
	
	return OPD_SUCCESS;
}

int opd_render_text(const char* text, int x, int y, int width, int color, int font_size, int align)
{
	int i, pen_x, pen_y;
	int num_chars = strlen(text);
	FT_Bitmap bitmaps[MAX_GLYPHS];
	FT_Vector pos[MAX_GLYPHS];
	
	if (num_chars > MAX_GLYPHS)
		num_chars = MAX_GLYPHS;
	
	pen_x = x;
	pen_y = y;

	if (FT_Set_Char_Size(opd_freetype_face, font_size * 64, 0, 100, 0)) {
		opd_log(LOG_ERROR, "cannot set font size");
		return OPD_ERROR;
	}

	for(i = 0; i < num_chars; i++) {
		if (FT_Load_Char(opd_freetype_face, text[i], FT_LOAD_RENDER) != 0)
			continue;
		
		FT_Bitmap_New(&bitmaps[i]);
		FT_Bitmap_Copy(opd_freetype_library, &opd_freetype_slot->bitmap, &bitmaps[i]);
		pos[i].x = pen_x + opd_freetype_slot->bitmap_left;
		pos[i].y = pen_y - opd_freetype_slot->bitmap_top;
		pen_x += opd_freetype_slot->advance.x >> 6;
	}
	
	int text_width = (pos[num_chars - 1].x + bitmaps[num_chars - 1].width) - pos[0].x;
		
	int offset = 0;
	if (align == OPD_TEXT_ALIGN_CENTER)
		offset = (width - text_width) / 2;
	else if (align == OPD_TEXT_ALIGN_RIGHT)
		offset = width - text_width;
	
	for(i = 0; i < num_chars; i++)
		opd_draw_character(&bitmaps[i], offset + pos[i].x, pos[i].y, color);

	return OPD_SUCCESS;
}

int opd_render_lcd_text(const char* text, int x, int y, int width, int color, int font_size, int align)
{
	int i, pen_x, pen_y;
	int num_chars = strlen(text);
	FT_Bitmap bitmaps[MAX_GLYPHS];
	FT_Vector pos[MAX_GLYPHS];
	
	if (num_chars > MAX_GLYPHS)
		num_chars = MAX_GLYPHS;
	
	pen_x = x;
	pen_y = y;

//	opd_log(LOG_DEBUG, "opd_render_lcd_text boxmodel: %s",opd_vumodel);
	if (strcmp(opd_vumodel,"duo2")) {
//		opd_log(LOG_DEBUG, "FT_Set_Char_Size");
		if (FT_Set_Char_Size(opd_freetype_lcd_face, font_size * 64, 0, 100, 0)) {
			opd_log(LOG_ERROR, "cannot set font size");
			return OPD_ERROR;
		}
	} else {
//		opd_log(LOG_DEBUG, "FT_Set_Pixel_Size");
		if (FT_Set_Pixel_Sizes(opd_freetype_lcd_face, 16, 16)){
			opd_log(LOG_ERROR, "cannot set font size");
			return OPD_ERROR;
		}
	}

	for(i = 0; i < num_chars; i++) {
		if (FT_Load_Char(opd_freetype_lcd_face, text[i], FT_LOAD_RENDER) != 0)
			continue;
		
		FT_Bitmap_New(&bitmaps[i]);
		FT_Bitmap_Copy(opd_freetype_library, &opd_freetype_lcd_slot->bitmap, &bitmaps[i]);
		pos[i].x = pen_x + opd_freetype_lcd_slot->bitmap_left;
		pos[i].y = pen_y - opd_freetype_lcd_slot->bitmap_top;
		pen_x += opd_freetype_lcd_slot->advance.x >> 6;

		//if (!strcmp(opd_vumodel,"duo2"))
		//	pen_x += 1;
	}
	
	int text_width = (pos[num_chars - 1].x + bitmaps[num_chars - 1].width) - pos[0].x;
		
	int offset = 0;
	if (align == OPD_TEXT_ALIGN_CENTER)
		offset = (width - text_width) / 2;
	else if (align == OPD_TEXT_ALIGN_RIGHT)
		offset = width - text_width;
	
	for(i = 0; i < num_chars; i++)
		opd_lcd_draw_character(&bitmaps[i], offset + pos[i].x, pos[i].y, color);

	return OPD_SUCCESS;
}
