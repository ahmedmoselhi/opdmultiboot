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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "opd_common.h"
#include "opd_log.h"
#include "opd_lcd.h"

#ifndef LCD_IOCTL_ASC_MODE
#define LCDSET					0x1000
#define LCD_IOCTL_ASC_MODE		(21|LCDSET)
#define	LCD_MODE_ASC			0
#define	LCD_MODE_BIN			1
#endif

#define RED(x)   (x >> 16) & 0xff;
#define GREEN(x) (x >> 8) & 0xff;
#define BLUE(x)   x & 0xff;

static int opd_lcd_fd = -1;
static int opd_lcd_width = 0;
static int opd_lcd_height = 0;
static int opd_lcd_stride = 0;
static int opd_lcd_bpp = 0;
static unsigned char *opd_lcd_buffer = NULL;


int opd_lcd_read_value(const char *filename)
{
	int value = 0;
	FILE *fd = fopen(filename, "r");
	if (fd) {
		int tmp;
		if (fscanf(fd, "%x", &tmp) == 1)
			value = tmp;
		fclose(fd);
	}
	return value;
}

int opd_lcd_open()
{
	opd_lcd_fd = open("/dev/dbox/lcd0", O_RDWR);
	if (opd_lcd_fd == -1)
		opd_lcd_fd = open("/dev/lcd0", O_RDWR);
	if (opd_lcd_fd == -1)
		opd_lcd_fd = open("/dev/dbox/oled0", O_RDWR);
	if (opd_lcd_fd == -1)
		opd_lcd_fd = open("/dev/oled0", O_RDWR);
	if (opd_lcd_fd == -1) {
		opd_log(LOG_ERROR, "%-33s: cannot open lcd device", __FUNCTION__);
		return OPD_ERROR;
	}

#ifdef OPD_HAVE_TEXTLCD
	return OPD_SUCCESS;
#endif

	int tmp = LCD_MODE_BIN;
	if (ioctl(opd_lcd_fd, LCD_IOCTL_ASC_MODE, &tmp)) {
		opd_log(LOG_ERROR, "%-33s: failed to set lcd bin mode", __FUNCTION__);
#ifndef OPD_DREAMBOX
		return OPD_ERROR;
#endif
	}
	
	opd_lcd_width = opd_lcd_read_value(OPD_LCD_XRES);
	if (opd_lcd_width == 0) {
		opd_log(LOG_ERROR, "%-33s: cannot read lcd x resolution", __FUNCTION__);
		return OPD_ERROR;
	}
	
	opd_lcd_height = opd_lcd_read_value(OPD_LCD_YRES);
	if (opd_lcd_height == 0) {
		opd_log(LOG_ERROR, "%-33s: cannot read lcd y resolution", __FUNCTION__);
		return OPD_ERROR;
	}
	opd_lcd_bpp = opd_lcd_read_value(OPD_LCD_BPP);
	if (opd_lcd_bpp == 0) {
		opd_log(LOG_ERROR, "%-33s: cannot read lcd bpp", __FUNCTION__);
		return OPD_ERROR;
	}
	
	opd_lcd_stride = opd_lcd_width * (opd_lcd_bpp / 8);
	opd_lcd_buffer = malloc(opd_lcd_height * opd_lcd_stride);
	
	opd_log(LOG_DEBUG, "%-33s: current lcd is %dx%d, %dbpp, stride %d", __FUNCTION__, opd_lcd_width, opd_lcd_height, opd_lcd_bpp, opd_lcd_stride);


	//vusolo4k and vuultimo4k need a brightness to enable lcd
	int fb =  open("/proc/stb/fp/oled_brightness", O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (fb) {
	    write(fb,"127",3);
	    close(fb);
	}

	return OPD_SUCCESS;
}

int opd_lcd_get_width()
{
	return opd_lcd_width;
}

int opd_lcd_get_height()
{
	return opd_lcd_height;
}

void opd_lcd_clear()
{
	if (!opd_lcd_buffer)
		return;
	
	memset(opd_lcd_buffer, '\0', opd_lcd_height * opd_lcd_stride);
}

void opd_lcd_update()
{
	if (!opd_lcd_buffer)
		return;
	
	write(opd_lcd_fd, opd_lcd_buffer, opd_lcd_height * opd_lcd_stride);
}

void opd_lcd_close()
{
	if (opd_lcd_fd >= 0)
		close(opd_lcd_fd);
	
	if (opd_lcd_buffer)
		free(opd_lcd_buffer);
}

void opd_lcd_draw_character(FT_Bitmap* bitmap, FT_Int x, FT_Int y, int color)
{
	if (!opd_lcd_buffer)
		return;
		
	int i, j, z = 0;
	long int location = 0;
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	red = (red >> 3) & 0x1f;
	green = (green >> 3) & 0x1f;
	blue = (blue >> 3) & 0x1f;
	
	for (i = y; i < y + bitmap->rows; i++) {
		for (j = x; j < x + bitmap->width; j++) {
			if (i < 0 || j < 0 || i > opd_lcd_height || j > opd_lcd_width) {
				z++;
				continue;
			}
			
			if (bitmap->buffer[z] != 0x00) {
				location = (j * (opd_lcd_bpp / 8)) +
					(i * opd_lcd_stride);
			
				if ( opd_lcd_bpp == 32) {
					opd_lcd_buffer[location] = RED(color);
					opd_lcd_buffer[location + 1] = GREEN(color);
					opd_lcd_buffer[location + 2] = BLUE(color) ;
					opd_lcd_buffer[location + 3] = 0xff;
				} else {
					opd_lcd_buffer[location] = red << 3 | green >> 2;
					opd_lcd_buffer[location + 1] = green << 6 | blue << 1;
				}
			}
			// vusolo4k needs alpha channel
			if ( opd_lcd_bpp == 32)
				opd_lcd_buffer[location + 3] = 0xff;
			z++;
		}
	}
}

void opd_lcd_write_text(const char* text)
{
	if(opd_lcd_fd < 0)
		return;

	write(opd_lcd_fd, text, strlen(text));
}
