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

#ifndef _opd_COMMON_H_
#define _opd_COMMON_H_

#define opd_SUCCESS 0
#define opd_ERROR -1

#define opd_DEVICES_DIR "/dev"
#define opd_FB_DEVICE "/dev/fb/0"
#define opd_FB_DEVICE_FAILOVER "/dev/fb0"
//#define opd_INPUT_DEVICE "/dev/input/event0"
#define opd_VIDEO_DEVICE "/dev/dvb/adapter0/video0"
#define opd_LCD_DEVICE "/dev/dbox/lcd0"
#define opd_PROC_STB "/proc/stb"
#define opd_LCD_XRES "/proc/stb/lcd/xres"
#define opd_LCD_YRES "/proc/stb/lcd/yres"
#define opd_LCD_BPP "/proc/stb/lcd/bpp"
//#define opd_KERNEL_MTD "/dev/mtd2"
#define opd_MAIN_DIR "/opd"
#define opd_DATA_DIR "OPDBootI"
#define opd_PLUGIN_DIR "/usr/lib/enigma2/python/Plugins/Extensions/OPDBoot"
#define opd_SCREEN_WIDTH 1920
#define opd_SCREEN_HEIGHT 1080
#define opd_SCREEN_BPP 32
#define opd_APP_NAME "OPDBoot"
#define opd_APP_VERION "1.0"
#define opd_DISPLAY_NAME "OPDBoot"
#ifndef opd_DEFAULT_TIMER
#define opd_DEFAULT_TIMER 10
#endif
#define opd_SHOWIFRAME_BIN "/usr/bin/showiframe"
#define opd_VOLATILE_MEDIA_BIN "/etc/init.d/volatile-media.sh"
#define opd_MDEV_BIN "/etc/init.d/mdev"
#define opd_MODUTILS_BIN "/etc/init.d/modutils.sh"
#define opd_INIT_BIN "/sbin/init"
#define opd_SYSVINIT_BIN "/sbin/init.sysvinit"
#define opd_CHROOT_BIN "/usr/sbin/chroot"
#define opd_NANDDUMP_BIN "/usr/sbin/nanddump"
#define opd_NANDWRITE_BIN "/usr/sbin/nandwrite"
#define opd_FLASHERASE_BIN "/usr/sbin/flash_erase"
#define opd_PYTHON_BIN "/usr/bin/python"
#define opd_BRANDING_HELPER_BIN "/sbin/OPDBoot-branding-helper.py"
#define opd_DD_BIN "/bin/dd"

#define opd_MENU_ITEM_RADIUS 10
#define opd_MENU_ITEM_HEIGHT 80
#define opd_MENU_ITEM_WIDTH 900
#ifndef opd_MENU_ITEM_COLOR
#define opd_MENU_ITEM_COLOR 0xFA336699
#endif
#ifndef opd_MENU_ITEM_SELECTED_COLOR
#define opd_MENU_ITEM_SELECTED_COLOR 0xFF000000
#endif
#ifndef opd_MENU_ITEM_TEXT_COLOR
#define opd_MENU_ITEM_TEXT_COLOR 0xFFFFFFFF
#endif
#define opd_MENU_ITEM_TEXT_BOTTON_MARGIN 24
#define opd_MENU_ITEM_TEXT_FONT_SIZE 26
#define opd_MENU_BOX_RADIUS 10
#define opd_MENU_BOX_MARGIN 6
#ifndef opd_MENU_BOX_COLOR
#define opd_MENU_BOX_COLOR 0xE0336699
#endif
#define opd_MENU_MAX_ITEMS 8
#define opd_MENU_ARROWS_SIZE 120
#ifndef opd_MENU_ARROWS_COLOR
#define opd_MENU_ARROWS_COLOR 0xFA336699
#endif

#define opd_LCD_LOGO_X 0.1 // 10% of display width
#define opd_LCD_LOGO_Y 0.1 // like the X axis (same margin)
#define opd_LCD_LOGO_SIZE 0.1 // 10% of display width
#define opd_LCD_LOGO_COLOR 0xffffffff

#define opd_LCD_TITLE_X 0.3 // 30% of display width
#define opd_LCD_TITLE_Y 0.19 // 19% of display width (keep proportion with x axis)
#define opd_LCD_TITLE_SIZE 0.05 // 5% of display width
#define opd_LCD_TITLE_COLOR 0xffffffff

#define opd_LCD_SELECTION_Y 0.75 // 75% of display height
#define opd_LCD_SELECTION_SIZE 0.07 // 7% of display width
#define opd_LCD_SELECTION_COLOR 0xffffffff

#define opd_HEADER_X 20
#define opd_HEADER_Y 40
#define opd_HEADER_FONT_SIZE 24
#define opd_HEADER_COLOR 0xFFFFFFFF

#define opd_TIMER_RIGHT_MARGIN 20
#define opd_TIMER_Y 50
#define opd_TIMER_FONT_SIZE 34
#define opd_TIMER_COLOR 0xFFFFFFFF

#endif // _opd_COMMON_H_
