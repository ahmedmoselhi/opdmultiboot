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
#include <dirent.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <mntent.h>
#include <unistd.h>

#include "opd_common.h"
#include "opd_log.h"
#include "opd_utils.h"
#include "opd_branding.h"

#define opd_FS_MAX 3
static const char *opd_utils_fs_types[opd_FS_MAX] = { "ext4", "ext3" };

int opd_utils_dir_exists(const char* folder)
{
	DIR *fd = opendir(folder);
	if (fd) {
		closedir(fd);
		return 1;
	}
	return 0;
}

int opd_utils_file_exists(const char* filename)
{
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}

void opd_utils_create_dir_tree()
{
	char tmp[255];
	if (!opd_utils_dir_exists(opd_MAIN_DIR))
		mkdir(opd_MAIN_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	
	sprintf(tmp, "%s/.kernels", opd_MAIN_DIR);
	if (!opd_utils_dir_exists(tmp))
		mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int opd_utils_mount(const char* device, const char* mountpoint)
{
	int i;
	for (i = 0; i < opd_FS_MAX; i++)
		if (mount(device, mountpoint, opd_utils_fs_types[i], 0, NULL) == 0)
			return opd_SUCCESS;
	
	return opd_ERROR;
}

int opd_utils_is_mounted(const char *mountpoint)
{
	FILE* mtab = NULL;
	struct mntent* part = NULL;
	int is_mounted = 0;
	
	if ((mtab = setmntent("/etc/mtab", "r")) != NULL) {
		while ((part = getmntent(mtab)) != NULL) {
			if (part->mnt_dir != NULL
				&& strcmp(part->mnt_dir, mountpoint) == 0) {
					
				is_mounted = 1;
			}
		}
		endmntent(mtab);
	}
	
	return is_mounted;
}

int opd_utils_umount(const char* mountpoint)
{
	return umount(mountpoint) == 0 ? opd_SUCCESS : opd_ERROR;
}

void opd_utils_remount_media(opd_device_item *item)
{
	FILE* mtab = NULL;
	struct mntent* part = NULL;
	char media[255];
	char base[255];
	char vol[255];
	sprintf(media, "%s/%s/%s/media", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
	sprintf(base, "%s/%s/%s", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
	sprintf(vol, "%s/%s/%s/etc/init.d/volatile-media.sh", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
	
	if (opd_utils_file_exists(vol)) {
		opd_log(LOG_DEBUG, "%-33s:remount /media into %s", __FUNCTION__, media);
		if (!opd_utils_is_mounted(media))
			if (mount("tmpfs", media, "tmpfs", 0, "size=64k") != 0)
				opd_log(LOG_ERROR, "%-33s: cannot mount %s", __FUNCTION__, media);
	}		
	if ((mtab = setmntent("/etc/mtab", "r")) != NULL) {
		while ((part = getmntent(mtab)) != NULL) {
			if (part->mnt_dir != NULL
				&& strlen(part->mnt_dir) > 6
				&& memcmp(part->mnt_dir, "/media", 6) == 0) {
					char tmp[255];
					sprintf(tmp, "%s/%s", base, part->mnt_dir);
					
					if (opd_utils_umount(part->mnt_dir) == opd_ERROR)
						opd_log(LOG_WARNING, "%-33s: cannot umount %s", __FUNCTION__, part->mnt_dir);
					
					if (!opd_utils_dir_exists(tmp))
						mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

					if (opd_utils_mount(part->mnt_fsname, tmp) == opd_ERROR)
						opd_log(LOG_WARNING, "%-33s: cannot mount %s", __FUNCTION__, tmp);
				}
		}
		endmntent(mtab);
	}

	if (opd_utils_umount("/media") == opd_ERROR)
		opd_log(LOG_WARNING, "%-33s: cannot umount /media", __FUNCTION__);
}

int opd_utils_find_and_mount()
{
	struct dirent *dir;
	DIR *fd = opendir(opd_DEVICES_DIR);
	if (fd) {
		opd_utils_create_dir_tree();
		
		while ((dir = readdir(fd)) != NULL) {
			if (strlen(dir->d_name) == 4 && memcmp(dir->d_name, "sd", 2) == 0) {
				char device[255];
				sprintf(device, "%s/%s", opd_DEVICES_DIR, dir->d_name);
				opd_log(LOG_DEBUG, "%-33s: check device %s", __FUNCTION__, device);
				
				opd_utils_umount(opd_MAIN_DIR); // just force umount without check
				if (opd_utils_mount(device, opd_MAIN_DIR) == opd_SUCCESS) {
					char datadir[255];
					sprintf(datadir, "%s/%s", opd_MAIN_DIR, opd_DATA_DIR);
					if (opd_utils_dir_exists(datadir)) {
						opd_log(LOG_DEBUG, "%-33s: found data on device %s", __FUNCTION__, device);
						closedir(fd);
						return opd_SUCCESS;
					}
					
					if (opd_utils_umount(opd_MAIN_DIR) == opd_ERROR)
						opd_log(LOG_ERROR, "%-33s: cannot umount %s", __FUNCTION__, opd_MAIN_DIR);
				}
			}
		}	
		closedir(fd);
	}
	return opd_ERROR;
}

opd_device_item *opd_utils_get_images()
{
	struct dirent *dir;
	char datadir[255];
	DIR *fd;
	
	opd_device_item *first = NULL;
	opd_device_item *last = NULL;
	
	opd_log(LOG_DEBUG, "%-33s: discover images", __FUNCTION__);
	
	opd_device_item *item = opd_branding_read_info("", "flash");
	if (item != NULL) {
		if (first == NULL)
			first = item;
		if (last != NULL)
			last->next = item;
		last = item;
	}

	sprintf(datadir, "%s/%s", opd_MAIN_DIR, opd_DATA_DIR);
	fd = opendir(datadir);
	if (fd) {
		while ((dir = readdir(fd)) != NULL) {
			if (strlen(dir->d_name) > 0 && dir->d_name[0] != '.') {
				char base_dir[255];
				sprintf(base_dir, "%s/%s", datadir, dir->d_name);

				if (!opd_branding_is_compatible(base_dir)) {
					opd_log(LOG_DEBUG ,"%-33s: skipping image %s", __FUNCTION__, base_dir);
					continue;
				}

				opd_device_item *item = opd_branding_read_info(base_dir, dir->d_name);
				if (item != NULL) {
					if (first == NULL)
						first = item;
					if (last != NULL)
						last->next = item;
					last = item;
				}
			}
		}
		closedir(fd);
	}
	return first;
}

void opd_utils_free_items(opd_device_item *items)
{
	opd_device_item *tmp = items;
	while (tmp) {
		opd_device_item *tmp2 = tmp;
		tmp = tmp->next;
			
		free(tmp2->label);
		free(tmp2->directory);
		free(tmp2->identifier);
		free(tmp2);
	}
}

void opd_utils_update_background(opd_device_item *item)
{
	char tmp[255];
	sprintf(tmp, "%s %s/usr/share/bootlogo.mvi", opd_SHOWIFRAME_BIN, item->directory);
	system(tmp);
}

void opd_utils_remove_nextboot()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", opd_MAIN_DIR, opd_DATA_DIR, opd_SETTINGS_NEXTBOOT);
	if(opd_utils_file_exists(tmp)) {
		char cmd[255];
		sprintf(cmd, "rm -rf %s", tmp);
		system(cmd);
	}
}

int opd_utils_gettimer()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", opd_MAIN_DIR, opd_DATA_DIR, opd_SETTINGS_TIMER);
	if(opd_utils_file_exists(tmp)) {
		char *tmp = opd_utils_read(opd_SETTINGS_TIMER);
		if (tmp) {
			int ret = atoi(tmp);
			free(tmp);
			return ret;
		}
	}
	return opd_DEFAULT_TIMER;
}

void opd_utils_setrctype()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", opd_MAIN_DIR, opd_DATA_DIR, opd_SETTINGS_RCTYPE);
	if(opd_utils_file_exists(tmp)) {
		char *tmp = opd_utils_read(opd_SETTINGS_RCTYPE);
		if (tmp) {
			int ret = atoi(tmp);
			free(tmp);
			if (ret) {
				char cmd[255];
				sprintf(cmd, "echo %d > /proc/stb/ir/rc/type", ret);
				system(cmd);
			}
		}
	}
}

void opd_utils_save(const char* key, const char* value)
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", opd_MAIN_DIR, opd_DATA_DIR, key);
	FILE *fd = fopen(tmp, "w");
	if (fd) {
		fwrite(value, 1, strlen(value), fd);
		fclose(fd);
		sync();
	}
}

int opd_utils_check_lock_menu()
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.bootmenu.lock", opd_MAIN_DIR, opd_DATA_DIR);
	if (opd_utils_file_exists(tmp)) {
		opd_log(LOG_DEBUG ,"%-33s: bootmenu disabled!", __FUNCTION__);
		return 1;
	}
	
	return 0;
	
}

char* opd_utils_read(const char *key)
{
	char tmp[255];
	sprintf(tmp, "%s/%s/.%s", opd_MAIN_DIR, opd_DATA_DIR, key);
	FILE *fd = fopen(tmp, "r");
	if (fd) {
		char line[1024];
		if (fgets(line, 1024, fd)) {
			strtok(line, "\n");
			char *ret = malloc(strlen(line) + 1);
			strcpy(ret, line);
			fclose(fd);
			return ret;
		}
		fclose(fd);
	}
	return NULL;
}

void opd_utils_save_int(const char* key, int value)
{
	char tmp[255];
	sprintf(tmp, "%d", value);
	opd_utils_save(key, tmp);
}

int opd_utils_read_int(const char *key)
{
	int ret = 0;
	char *tmp = opd_utils_read(key);
	if (tmp) {
		ret = atoi(tmp);
		free(tmp);
	}
	opd_log(LOG_DEBUG, "%-33s: selected %d", __FUNCTION__, ret);
	return ret;
}

void opd_utils_build_platform_wrapper(opd_device_item *item)
{
	FILE *fp;
	char tmp[255];
	char cmd[512];
	
	sprintf(tmp, "%s/%s/%s/usr/bin/platform-util-wrapper.sh", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
 	fp = fopen(tmp,"w");
 	fprintf(fp,"%s","#!/bin/sh\n\n");
 	fprintf(fp,"%s","export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin\n");
 	fprintf(fp,"%s","/etc/init.d/vuplus-platform-util start\n");
	fprintf(fp,"%s","/etc/init.d/platform-util start\n");
	fprintf(fp,"%s","/etc/init.d/gigablue-platform-util start\n");
 	fclose(fp);
 	
 	sprintf(cmd, "chmod 0755 %s/%s/%s/usr/bin/platform-util-wrapper.sh", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
 	system(cmd);
}

void opd_utils_init_system()
{
	opd_log(LOG_DEBUG, "%-33s: mount /proc", __FUNCTION__);
	if (!opd_utils_is_mounted("/proc"))
		if (mount("proc", "/proc", "proc", 0, NULL) != 0)
			opd_log(LOG_ERROR, "%-33s: cannot mount /proc", __FUNCTION__);
	
	opd_log(LOG_DEBUG, "%-33s: mount /sys", __FUNCTION__);
	if (!opd_utils_is_mounted("/sys"))
		if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0)
			opd_log(LOG_ERROR, "%-33s: cannot mount /sys", __FUNCTION__);
	
	opd_log(LOG_DEBUG, "%-33s: mount /media", __FUNCTION__);
	if (!opd_utils_is_mounted("/media"))
		if (mount("tmpfs", "/media", "tmpfs", 0, "size=64k") != 0)
			opd_log(LOG_ERROR, "%-33s: cannot mount /media", __FUNCTION__);

	opd_log(LOG_DEBUG, "%-33s: run volatile media", __FUNCTION__);
	system(opd_VOLATILE_MEDIA_BIN);

	opd_log(LOG_DEBUG, "%-33s: run mdev", __FUNCTION__);
	system(opd_MDEV_BIN);
	
	// we really need this sleep?? :( - (wait for mdev to finalize)
	sleep(5);
}

/*
 **
 * by Meo.
 * We don't need to load drivers when we have not to show the bootmenu (force = 0).
 * So we split the original load_modules function to load drivers
 * only when needed.
 **
 */

void opd_utils_prepare_destination(opd_device_item *item)
{	
	opd_log(LOG_DEBUG, "%-33s: prepare destination", __FUNCTION__);

	if (item != NULL && strcmp(item->identifier, "flash") != 0)
	{
		char dev[255];
		char proc[255];
		char sys[255];
		char opd[255];
		char opd_plugin[255];
		sprintf(dev, "%s/%s/%s/dev", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		sprintf(proc, "%s/%s/%s/proc", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		sprintf(sys, "%s/%s/%s/sys", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		sprintf(opd, "%s/%s/%s/%s", opd_MAIN_DIR, opd_DATA_DIR, item->identifier, opd_MAIN_DIR);
		sprintf(opd_plugin, "%s/%s/%s/%s", opd_MAIN_DIR, opd_DATA_DIR, item->identifier, opd_PLUGIN_DIR);
		
		if (!opd_utils_is_mounted(dev))
			if (mount("/dev", dev, NULL, MS_BIND, NULL) != 0)
				opd_log(LOG_ERROR, "%-33s: cannot bind /dev to %s", __FUNCTION__, dev);
		
		if (!opd_utils_is_mounted(proc))
			if (mount("/proc", proc, NULL, MS_BIND, NULL) != 0)
				opd_log(LOG_ERROR, "%-33s: cannot bind /proc to %s", __FUNCTION__, proc);
		
		if (!opd_utils_is_mounted(sys))
			if (mount("/sys", sys, NULL, MS_BIND, NULL) != 0)
				opd_log(LOG_ERROR, "%-33s: cannot bind /sys to %s", __FUNCTION__, sys);

		if (!opd_utils_dir_exists(opd))
			mkdir(opd, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		if (!opd_utils_is_mounted(opd))
			if (mount(opd_MAIN_DIR, opd, NULL, MS_BIND, NULL) != 0)
				opd_log(LOG_ERROR, "%-33s: cannot bind %s to %s", __FUNCTION__, opd_MAIN_DIR, opd);
				
		if (!opd_utils_dir_exists(opd_plugin))
			mkdir(opd_plugin, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		if (!opd_utils_is_mounted(opd_plugin))
			if (mount(opd_PLUGIN_DIR, opd_plugin, NULL, MS_BIND, NULL) != 0)
				opd_log(LOG_ERROR, "%-33s: cannot bind %s to %s", __FUNCTION__, opd_PLUGIN_DIR, opd_plugin);
	}
	
}

void opd_utils_load_modules(opd_device_item *item)
{
	int i;
	
	opd_log(LOG_DEBUG, "%-33s: load modules", __FUNCTION__);

	if (item == NULL || strcmp(item->identifier, "flash") == 0) {
		system(opd_MODUTILS_BIN);
	}
	else {
		
		char cmd[512];
		
		sprintf(cmd, "%s %s/%s/%s %s", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier, opd_MODUTILS_BIN);
		system(cmd);
	}
	
	for (i = 0; i < 200; i++) {
		if (opd_utils_file_exists(opd_VIDEO_DEVICE))
			break;
		
		usleep(10000);
	}

#ifdef __sh__
	opd_log(LOG_DEBUG, "%-33s: load lirc", __FUNCTION__);
	if (item == NULL || strcmp(item->identifier, "flash") == 0) {
		system("/etc/init.d/populate-volatile.sh start");
		system("/etc/init.d/lircd start");
	}
	else {
		char cmd[255];
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/populate-volatile.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/lircd start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);
	}
#endif

}

/*
 **
 * by Meo.
 * OpenGles modules are loaded at the end of rcS
 * So we need additional stuffs and a different procedure.
 **
 */
void opd_utils_load_modules_gl(opd_device_item *item)
{	
	opd_log(LOG_DEBUG, "%-33s: load platform-util", __FUNCTION__);
	
	int i;

	if (item == NULL || strcmp(item->identifier, "flash") == 0) 
	{
		system("/etc/init.d/mountall.sh start");
		system("/etc/init.d/modload.sh start");
		system("/etc/init.d/modutils.sh start");
		system("/etc/init.d/populate-volatile.sh start");
		system("/etc/init.d/bootmisc.sh start");
		system("/etc/init.d/vuplus-platform-util start");
		system("/etc/init.d/platform-util start");
		system("/etc/init.d/gigablue-platform-util start");		
	}

	else 
	{
		
		char tmp[255];
		char cmd[512];
			

		sprintf(tmp, "%s/%s/%s/etc/init.d/mountrun.sh", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		if(opd_utils_file_exists(tmp)) {
			sprintf(cmd, "%s %s/%s/%s /etc/init.d/mountrun.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
			system(cmd);
		}

		sprintf(cmd, "%s %s/%s/%s /etc/init.d/mountall.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);
		
		sprintf(tmp, "%s/%s/%s/etc/init.d/modload.sh", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		if(opd_utils_file_exists(tmp)) {
			sprintf(cmd, "%s %s/%s/%s /etc/init.d/modload.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
			system(cmd);
		}
		
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/modutils.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);
		
		sprintf(cmd, "%s %s/%s/%s /etc/init.d/populate-volatile.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);

		sprintf(cmd, "%s %s/%s/%s /etc/init.d/bootmisc.sh start", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);

// prevent missing path in chroot
		opd_utils_build_platform_wrapper(item);

		sprintf(cmd, "%s %s/%s/%s /usr/bin/platform-util-wrapper.sh", opd_CHROOT_BIN, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		system(cmd);
		
	}

	for (i = 0; i < 500; i++) {
		if (opd_utils_file_exists(opd_VIDEO_DEVICE))
			break;
		
		usleep(10000);
	}
}

void opd_utils_backup_kernel(opd_device_item *item)
{
	char cmd[512];

	if (!item)
		return;
	
	opd_log(LOG_DEBUG, "%-33s: backup kernel for image '%s'", __FUNCTION__, item->identifier);
#ifdef opd_DREAMBOX
	sprintf(cmd, "%s %s -nof %s/%s/.kernels/%s.bin", opd_NANDDUMP_BIN, opd_KERNEL_MTD, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
#elif defined(opd_MMCBLK)
	if (opd_utils_file_exists(opd_PROC_STB))
		sprintf(cmd, "%s if=%s of=%s/%s/.kernels/%s.bin", opd_DD_BIN, opd_KERNEL_MTD, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
#else
	sprintf(cmd, "%s %s -f %s/%s/.kernels/%s.bin", opd_NANDDUMP_BIN, opd_KERNEL_MTD, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
#endif
	system(cmd);
//	opd_log(LOG_DEBUG, "opd_utils_backup_kernel(): cmd: %s");
}

void opd_utils_restore_kernel(opd_device_item *item)
{
	char cmd[512];
	char filename[255];

	if (!item)
		return;
	
	sprintf(filename, "%s/%s/.kernels/%s.bin", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
	if (opd_utils_file_exists(filename)) {
#ifndef opd_MMCBLK
		opd_log(LOG_DEBUG, "%-33s: erasing MTD", __FUNCTION__);
		sprintf(cmd, "%s %s 0 0", opd_FLASHERASE_BIN, opd_KERNEL_MTD);
		system(cmd);
#endif
	
		opd_log(LOG_DEBUG, "%-33s: restore kernel for image '%s'", __FUNCTION__, item->identifier);
#ifdef opd_DREAMBOX
		sprintf(cmd, "%s -mno %s %s/%s/.kernels/%s.bin", opd_NANDWRITE_BIN, opd_KERNEL_MTD, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
#elif defined(opd_MMCBLK)
		if (opd_utils_file_exists(opd_PROC_STB))
			sprintf(cmd, "%s of=%s if=%s/%s/.kernels/%s.bin", opd_DD_BIN, opd_KERNEL_MTD, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
#else
		sprintf(cmd, "%s -pm %s %s/%s/.kernels/%s.bin", opd_NANDWRITE_BIN, opd_KERNEL_MTD, opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
#endif
		system(cmd);
	}
}

void opd_utils_reboot()
{
	opd_utils_sysvinit(NULL, "6");
}

void opd_utils_sysvinit(opd_device_item *item, const char *args)
{
	if (item == NULL || strcmp(item->identifier, "flash") == 0) {
		execl(opd_SYSVINIT_BIN, opd_SYSVINIT_BIN, args, NULL);
	}
	else {
		char path[255];
		char udev[255];
		sprintf(path, "%s/%s/%s", opd_MAIN_DIR, opd_DATA_DIR, item->identifier);
		sprintf(udev, "%s/%s/%s/%s", opd_MAIN_DIR, opd_DATA_DIR, item->identifier, "/etc/init.d/udev");
		if (opd_utils_file_exists(udev))
				system("/etc/init.d/mdev stop");
				
		execl(opd_CHROOT_BIN, opd_CHROOT_BIN, path, opd_INIT_BIN, args, NULL);
	}
}
