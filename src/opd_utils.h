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

#ifndef _opd_UTILS_H_
#define _opd_UTILS_H_

typedef struct opd_device_item
{
	char *label;
	char *directory;
	char *identifier;
	struct opd_device_item *next;
} opd_device_item;

#define opd_SETTINGS_SELECTED "opdboot"
#define opd_SETTINGS_FORCE "force"
#define opd_SETTINGS_NEXTBOOT "nextboot"
#define opd_SETTINGS_TIMER "timer"
#define opd_SETTINGS_RCTYPE "rctype"

int opd_utils_find_and_mount();
opd_device_item *opd_utils_get_images();
void opd_utils_update_background(opd_device_item *item);
void opd_utils_free_items(opd_device_item *items);

void opd_utils_save(const char* key, const char* value);
char* opd_utils_read(const char *key);
void opd_utils_save_int(const char* key, int value);
int opd_utils_read_int(const char *key);
int opd_utils_check_lock_menu();
void opd_utils_build_platform_wrapper(opd_device_item *item);

void opd_utils_remove_nextboot();
int opd_utils_gettimer();
void opd_utils_setrctype();

void opd_utils_init_system();
void opd_utils_prepare_destination(opd_device_item *item);
void opd_utils_load_modules(opd_device_item *item);
void opd_utils_load_modules_gl(opd_device_item *item);

void opd_utils_backup_kernel(opd_device_item *item);
void opd_utils_restore_kernel(opd_device_item *item);

void opd_utils_remount_media(opd_device_item *item);

void opd_utils_reboot();
void opd_utils_sysvinit(opd_device_item *item, const char *args);

#endif // _opd_UTILS_H_
