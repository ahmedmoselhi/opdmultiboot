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
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include "opd_common.h"
#include "opd_log.h"
#include "opd_input.h"

#define opd_input_max_fds 10

static int opd_input_num_fds = 0;
static int opd_input_fd[opd_input_max_fds];
static int opd_input_last_event_code = -1;
static int opd_input_last_event_count = 0;

int opd_input_open()
{
	while (opd_input_num_fds < opd_input_max_fds)
	{
		char filename[32];
		sprintf(filename, "/dev/input/event%d", opd_input_num_fds);
		if ((opd_input_fd[opd_input_num_fds] = open(filename, O_RDONLY | O_NONBLOCK)) == -1)
			break;
		opd_input_num_fds++;
	}

	if (opd_input_num_fds == 0)
	{
		opd_log(LOG_ERROR, "%-33s: cannot open input device", __FUNCTION__);
		return opd_ERROR;
	}

	opd_log(LOG_DEBUG, "%-33s: input device opened", __FUNCTION__);
	
	return opd_SUCCESS;
}

int opd_input_get_code()
{
	struct input_event event;
	int i = 0;
	while (i < opd_input_num_fds)
	{
		if (read(opd_input_fd[i], &event, sizeof(event)) == sizeof(event))
		{
			if (!event.code)
				continue;

			if (event.type == EV_KEY && (event.value == 0 || event.value == 2))
				return event.code;
		}
		i++;
	}

	return -1;
}

void opd_input_close()
{
	int i = 0;
	for (; i < opd_input_num_fds; i++)
		close(opd_input_fd[i]);
}
