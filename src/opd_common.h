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

#ifndef _OPD_COMMON_H_
#define _OPD_COMMON_H_

#define OPD_SUCCESS 0
#define OPD_ERROR -1

#define OPD_DEVICES_DIR "/dev"
#define OPD_FB_DEVICE "/dev/fb/0"
#define OPD_FB_DEVICE_FAILOVER "/dev/fb0"
//#define OPD_INPUT_DEVICE "/dev/input/event0"
#define OPD_VIDEO_DEVICE "/dev/dvb/adapter0/video0"
#define OPD_LCD_DEVICE "/dev/dbox/lcd0"
#define OPD_PROC_STB "/proc/stb"
#define OPD_LCD_XRES "/proc/stb/lcd/xres"
#define OPD_LCD_YRES "/proc/stb/lcd/yres"
#define OPD_LCD_BPP "/proc/stb/lcd/bpp"
//#define OPD_KERNEL_MTD "/dev/mtd2"
#define OPD_MAIN_DIR "/opd"
#define OPD_DATA_DIR "OPDBootI"
#define OPD_PLUGIN_DIR "/usr/lib64/enigma2/python/Plugins/Extensions/OPDBoot"
#define OPD_SCREEN_WIDTH 1920
#define OPD_SCREEN_HEIGHT 1080
#define OPD_SCREEN_BPP 32
#define OPD_APP_NAME "OPDBoot"
#define OPD_APP_VERION "1.0"
#define OPD_DISPLAY_NAME "OPDBoot"
#ifndef OPD_DEFAULT_TIMER
#define OPD_DEFAULT_TIMER 5
#endif
#define OPD_SHOWIFRAME_BIN "/usr/bin/showiframe"
#define OPD_VOLATILE_MEDIA_BIN "/etc/init.d/volatile-media.sh"
#define OPD_MDEV_BIN "/etc/init.d/mdev"
#define OPD_MODUTILS_BIN "/etc/init.d/modutils.sh"
#define OPD_INIT_BIN "/sbin/init"
#define OPD_SYSVINIT_BIN "/sbin/init.sysvinit"
#define OPD_CHROOT_BIN "/usr/sbin/chroot"
#define OPD_NANDDUMP_BIN "/usr/sbin/nanddump"
#define OPD_NANDWRITE_BIN "/usr/sbin/nandwrite"
#define OPD_FLASHERASE_BIN "/usr/sbin/flash_erase"
#define OPD_PYTHON_BIN "/usr/bin/python"
#define OPD_BRANDING_HELPER_BIN "/sbin/OPDBoot-branding-helper.py"
#define OPD_DD_BIN "/bin/dd"

#define OPD_MENU_ITEM_RADIUS 10
#define OPD_MENU_ITEM_HEIGHT 80
#define OPD_MENU_ITEM_WIDTH 900
#ifndef OPD_MENU_ITEM_COLOR
#define OPD_MENU_ITEM_COLOR 0xFA336699
#endif
#ifndef OPD_MENU_ITEM_SELECTED_COLOR
#define OPD_MENU_ITEM_SELECTED_COLOR 0xFF000000
#endif
#ifndef OPD_MENU_ITEM_TEXT_COLOR
#define OPD_MENU_ITEM_TEXT_COLOR 0xFFFFFFFF
#endif
#define OPD_MENU_ITEM_TEXT_BOTTON_MARGIN 24
#define OPD_MENU_ITEM_TEXT_FONT_SIZE 26
#define OPD_MENU_BOX_RADIUS 10
#define OPD_MENU_BOX_MARGIN 6
#ifndef OPD_MENU_BOX_COLOR
#define OPD_MENU_BOX_COLOR 0xE0336699
#endif
#define OPD_MENU_MAX_ITEMS 8
#define OPD_MENU_ARROWS_SIZE 120
#ifndef OPD_MENU_ARROWS_COLOR
#define OPD_MENU_ARROWS_COLOR 0xFA336699
#endif

#define OPD_LCD_LOGO_X 0.1 // 10% of display width
#define OPD_LCD_LOGO_Y 0.1 // like the X axis (same margin)
#define OPD_LCD_LOGO_SIZE 0.1 // 10% of display width
#define OPD_LCD_LOGO_COLOR 0xffffffff

#define OPD_LCD_TITLE_X 0.3 // 30% of display width
#define OPD_LCD_TITLE_Y 0.19 // 19% of display width (keep proportion with x axis)
#define OPD_LCD_TITLE_SIZE 0.05 // 5% of display width
#define OPD_LCD_TITLE_COLOR 0xffffffff

#define OPD_LCD_SELECTION_Y 0.75 // 75% of display height
#define OPD_LCD_SELECTION_SIZE 0.07 // 7% of display width
#define OPD_LCD_SELECTION_COLOR 0xffffffff

#define OPD_HEADER_X 20
#define OPD_HEADER_Y 40
#define OPD_HEADER_FONT_SIZE 24
#define OPD_HEADER_COLOR 0xFFFFFFFF

#define OPD_TIMER_RIGHT_MARGIN 20
#define OPD_TIMER_Y 50
#define OPD_TIMER_FONT_SIZE 34
#define OPD_TIMER_COLOR 0xFFFFFFFF

#endif // _OPD_COMMON_H_
