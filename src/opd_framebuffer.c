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
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __sh__
#include <linux/stmfb.h>
#endif

#include "opd_common.h"
#include "opd_utils.h"
#include "opd_log.h"

#ifndef FBIO_BLIT
#define FBIO_SET_MANUAL_BLIT _IOW('F', 0x21, __u8)
#define FBIO_BLIT 0x22
#endif

#define ALPHA(x) (x >> 24) & 0xff;
#define RED(x)   (x >> 16) & 0xff;
#define GREEN(x) (x >> 8) & 0xff;
#define BLUE(x)   x & 0xff;

static int opd_fb_fd = 0;
static unsigned char* opd_fb_map = 0;
static struct fb_var_screeninfo opd_var_screen_info;
static struct fb_fix_screeninfo opd_fix_screen_info;
static int opd_screen_size;

int opd_read_screen_info()
{
	if (ioctl(opd_fb_fd, FBIOGET_FSCREENINFO, &opd_fix_screen_info) == -1) {
		opd_log(LOG_ERROR, "%-33s: cannot read fixed information", __FUNCTION__);
		return OPD_ERROR;
	}

	if (ioctl(opd_fb_fd, FBIOGET_VSCREENINFO, &opd_var_screen_info) == -1) {
		opd_log(LOG_ERROR, "%-33s: cannot read variable information", __FUNCTION__);
		return OPD_ERROR;
	}

	opd_log(LOG_DEBUG, "%-33s: current mode is %dx%d, %dbpp, stride %d", __FUNCTION__,
		opd_var_screen_info.xres, opd_var_screen_info.yres, opd_var_screen_info.bits_per_pixel, opd_fix_screen_info.line_length);
	
	opd_screen_size = opd_fix_screen_info.smem_len;//opd_var_screen_info.xres * opd_var_screen_info.yres * opd_var_screen_info.bits_per_pixel / 8;

#ifdef __sh__
	opd_screen_size -= 1920*1080*4;
#endif

	return OPD_SUCCESS;
}

int opd_set_screen_info(int width, int height, int bpp)
{
	opd_var_screen_info.xres_virtual = opd_var_screen_info.xres = width;
	opd_var_screen_info.yres_virtual = opd_var_screen_info.yres = height;
	opd_var_screen_info.bits_per_pixel = bpp;
	opd_var_screen_info.xoffset = opd_var_screen_info.yoffset = 0;
	opd_var_screen_info.height = 0;
	opd_var_screen_info.width = 0;

#ifndef __sh__
	if (ioctl(opd_fb_fd, FBIOPUT_VSCREENINFO, &opd_var_screen_info) < 0) {
		opd_log(LOG_ERROR, "%-33s: cannot set variable information", __FUNCTION__);
		return OPD_ERROR;
	}
#endif
	
	if ((opd_var_screen_info.xres != width) && (opd_var_screen_info.yres != height) && (opd_var_screen_info.bits_per_pixel != bpp)) {
		opd_log(LOG_ERROR, "%-33s: cannot set variable information: got %dx%dx%d instead of %dx%dx%d", __FUNCTION__,
			opd_var_screen_info.xres, opd_var_screen_info.yres, opd_var_screen_info.bits_per_pixel, width, height, bpp);
		return OPD_ERROR;
	}
	
	if (ioctl(opd_fb_fd, FBIOGET_FSCREENINFO, &opd_fix_screen_info) == -1) {
		opd_log(LOG_ERROR, "%-33s: cannot read fixed information", __FUNCTION__);
		return OPD_ERROR;
	}
	
	return OPD_SUCCESS;
}

int opd_map_framebuffer()
{
#ifdef __sh__
	opd_fb_map = (unsigned char *)mmap(0, opd_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, opd_fb_fd, 1920*1080*4);
#else
	opd_fb_map = (unsigned char *)mmap(0, opd_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, opd_fb_fd, 0);
#endif
	if (opd_fb_map == MAP_FAILED) {
		opd_log(LOG_ERROR, "failed to map framebuffer device to memory");
		return OPD_ERROR;
	}
	
	opd_log(LOG_DEBUG, "%-33s: the framebuffer device was mapped to memory successfully", __FUNCTION__);
	
	return OPD_SUCCESS;
}

static unsigned short red[256], green[256], blue[256], trans[256];
int opd_make_palette()
{
	int r = 8, g = 8, b = 4, i;

	struct fb_cmap colormap;
	colormap.start=0;
	colormap.len=256;
	colormap.red=red;
	colormap.green = green;
	colormap.blue = blue;
	colormap.transp=trans;

	int rs = 256 / (r - 1);
	int gs = 256 / (g - 1);
	int bs = 256 / (b - 1);

	for (i = 0; i < 256; i++) {
		colormap.red[i]   = (rs * ((i / (g * b)) % r)) * 255;
		colormap.green[i] = (gs * ((i / b) % g)) * 255;
		colormap.blue[i]  = (bs * ((i) % b)) * 255;
	}

	opd_log(LOG_DEBUG, "%-33s, set color palette disabled: FIXME !!", __FUNCTION__);
// FIXME
/*	
	if (ioctl(opd_fb_fd, FBIOPUTCMAP, &colormap) == -1) {
		opd_log(LOG_ERROR, "failed to set color palette");
		//return opd_ERROR;
		return opd_SUCCESS; // NEED TO BE FIXED FOR VU+ BOXES !!
	}
*/	
	return OPD_SUCCESS;
}

int opd_set_manual_blit()
{
	opd_log(LOG_DEBUG, "%-33s: set manual blit", __FUNCTION__);
	
#ifndef __sh__
	unsigned char tmp = 1;
	if (ioctl(opd_fb_fd, FBIO_SET_MANUAL_BLIT, &tmp)) {
		opd_log(LOG_ERROR, "failed to set manual blit");
		return OPD_ERROR;
	}
#endif
	
	return OPD_SUCCESS;
}

void opd_blit()
{
#ifdef __sh__
	STMFBIO_BLT_DATA    bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));
	bltData.operation  = BLT_OP_COPY;
	bltData.srcOffset  = 1920*1080*4;
	bltData.srcPitch   = opd_var_screen_info.xres * 4;
	bltData.dstOffset  = 0;
	bltData.dstPitch   = opd_var_screen_info.xres * 4;
	bltData.src_top    = 0;
	bltData.src_left   = 0;
	bltData.src_right  = opd_var_screen_info.xres;
	bltData.src_bottom = opd_var_screen_info.yres;
	bltData.srcFormat  = SURF_BGRA8888;
	bltData.dstFormat  = SURF_BGRA8888;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;
	bltData.dst_top    = 0;
	bltData.dst_left   = 0;
	bltData.dst_right  = opd_var_screen_info.xres;
	bltData.dst_bottom = opd_var_screen_info.yres;
	if (ioctl(opd_fb_fd, STMFBIO_BLT, &bltData ) < 0)
		opd_log(LOG_WARNING, "%-33s: cannot blit the framebuffer", __FUNCTION__);
	if (ioctl(opd_fb_fd, STMFBIO_SYNC_BLITTER) < 0)
		opd_log(LOG_WARNING, "%-33s: cannot sync blit", __FUNCTION__);
#else
	if (ioctl(opd_fb_fd, FBIO_BLIT) == -1)
		opd_log(LOG_WARNING, "%-33s: cannot blit the framebuffer, __FUNCTION__");
#endif
}

int opd_get_screen_width()
{
	return opd_var_screen_info.xres;
}

int opd_get_screen_height()
{
	return opd_var_screen_info.yres;
}

int opd_open_framebuffer()
{
	if (opd_utils_file_exists(OPD_FB_DEVICE))
	    opd_fb_fd = open(OPD_FB_DEVICE, O_RDWR);
	else
	    opd_fb_fd = open(OPD_FB_DEVICE_FAILOVER, O_RDWR);	    
	if (opd_fb_fd == -1) {
		opd_log(LOG_ERROR, "%-33s: cannot open framebuffer device", __FUNCTION__);
		return OPD_ERROR;
	}
	opd_log(LOG_DEBUG, "%-33s: the framebuffer device was opened successfully", __FUNCTION__);
	
	if (opd_read_screen_info() == OPD_ERROR)
		return OPD_ERROR;
	
	if ((opd_var_screen_info.xres != OPD_SCREEN_WIDTH)
		|| (opd_var_screen_info.yres != OPD_SCREEN_HEIGHT)
		|| (opd_var_screen_info.bits_per_pixel != OPD_SCREEN_BPP)) {
			
		if (opd_set_screen_info(OPD_SCREEN_WIDTH, OPD_SCREEN_HEIGHT, OPD_SCREEN_BPP) == OPD_ERROR)
			return OPD_ERROR;
	
		if (opd_read_screen_info() == OPD_ERROR)
			return OPD_ERROR;
	}
	
	if (opd_map_framebuffer() == OPD_ERROR)
		return OPD_ERROR;
	
	if (opd_make_palette() == OPD_ERROR)
		return OPD_ERROR;
	
	if (opd_set_manual_blit() == OPD_ERROR)
		return OPD_ERROR;
	
	return OPD_SUCCESS;
}

void opd_close_framebuffer()
{
	munmap(opd_fb_map, opd_screen_size);
	close(opd_fb_fd);
}

void opd_clear_screen()
{
	memset(opd_fb_map, '\0', opd_screen_size);
}

void opd_draw_rect(int x, int y, int width, int height, int color)
{
	int i, j;
	long int location = 0;
	unsigned char alpha = ALPHA(color);
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	for (i = y; i < y + height; i++) {
		for (j = x; j < x + width; j++) {
			
			if (i < 0 || j < 0 || i > opd_var_screen_info.yres || j > opd_var_screen_info.xres)
				continue;

			location = ((j + opd_var_screen_info.xoffset) * (opd_var_screen_info.bits_per_pixel / 8)) +
				((i + opd_var_screen_info.yoffset) * opd_fix_screen_info.line_length);

			*(opd_fb_map + location) = blue;
			*(opd_fb_map + location + 1) = green;
			*(opd_fb_map + location + 2) = red;
			*(opd_fb_map + location + 3) = alpha;
		}
	}
}

static inline int opd_is_point_inside_circle(int x, int y, int radius)
{
	if (((x - radius) * (x - radius)) + ((y - radius) * (y - radius)) < radius * radius)
		return 1;
	return 0;
}

void opd_draw_rounded_rect(int x, int y, int width, int height, int color, int radius)
{
	int i, j;
	long int location = 0;
	unsigned char alpha = ALPHA(color);
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	for (i = y; i < y + height; i++) {
		for (j = x; j < x + width; j++) {
			if (i < 0 || j < 0 || i > opd_var_screen_info.yres || j > opd_var_screen_info.xres)
				continue;
			
			int relative_x = j - x;
			int relative_y = i - y;
			
			// top left corner
			if (relative_y < radius && relative_x < radius) {
				if (!opd_is_point_inside_circle(relative_x, relative_y, radius)) {
					continue;
				}
			}
			
			// top right corner
			else if (relative_y < radius && width - relative_x < radius) {
				if (!opd_is_point_inside_circle(width - relative_x, relative_y, radius)) {
					continue;
				}
			}
			
			// bottom left corner
			else if (height - relative_y < radius && relative_x < radius) {
				if (!opd_is_point_inside_circle(relative_x, height - relative_y, radius)) {
					continue;
				}
			}

			// bottom right corner
			else if (height - relative_y < radius && width - relative_x < radius) {
				if (!opd_is_point_inside_circle(width - relative_x, height - relative_y, radius)) {
					continue;
				}
			}

			location = ((j + opd_var_screen_info.xoffset) * (opd_var_screen_info.bits_per_pixel / 8)) +
				((i + opd_var_screen_info.yoffset) * opd_fix_screen_info.line_length);

			*(opd_fb_map + location) = blue;
			*(opd_fb_map + location + 1) = green;
			*(opd_fb_map + location + 2) = red;
			*(opd_fb_map + location + 3) = alpha;
		}
	}
}

static inline unsigned char opd_blend_pixel(unsigned char background, unsigned char foreground, unsigned char foreground_alpha)
{
	return (foreground * (foreground_alpha / 255.0)) + (background * (1.0 - (foreground_alpha / 255.0)));
}

void opd_draw_character(FT_Bitmap* bitmap, FT_Int x, FT_Int y, int color)
{
	int i, j, z = 0;
	long int location = 0;
	unsigned char red = RED(color);
	unsigned char green = GREEN(color);
	unsigned char blue = BLUE(color);
	
	for (i = y; i < y + bitmap->rows; i++) {
		for (j = x; j < x + bitmap->width; j++) {
			if (i < 0 || j < 0 || i > opd_var_screen_info.yres || j > opd_var_screen_info.xres) {
				z++;
				continue;
			}
			
			if (bitmap->buffer[z] != 0x00) {
				location = ((j + opd_var_screen_info.xoffset) * (opd_var_screen_info.bits_per_pixel / 8)) +
					((i + opd_var_screen_info.yoffset) * opd_fix_screen_info.line_length);
			
				if (*(opd_fb_map + location + 3) == 0x00) {
					*(opd_fb_map + location) = blue;
					*(opd_fb_map + location + 1) = green;
					*(opd_fb_map + location + 2) = red;
					*(opd_fb_map + location + 3) = bitmap->buffer[z];
				}
				else {
					*(opd_fb_map + location) = opd_blend_pixel(*(opd_fb_map + location), blue, bitmap->buffer[z]);
					*(opd_fb_map + location + 1) = opd_blend_pixel(*(opd_fb_map + location + 1), green, bitmap->buffer[z]);
					*(opd_fb_map + location + 2) = opd_blend_pixel(*(opd_fb_map + location + 2), red, bitmap->buffer[z]);
					if (bitmap->buffer[z] == 0xff)
						*(opd_fb_map + location + 3) = bitmap->buffer[z];
				}
			}
			z++;
		}
	}
}
