/*
 * uevent_listen.c - dump netlink uevents from the kernel to stdout
 *
 *	Only kernels from version 2.6.10* on provide the uevent netlink socket.
 *	Until the libc-kernel-headers are updated, you need to compile with:
 *
 *	  gcc -I /lib/modules/`uname -r`/build/include -o uevent_listen uevent_listen.c
 *
 * Copyright (C) 2004 Kay Sievers <kay.sievers@vrfy.org>
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <asm/types.h>
#include <linux/netlink.h>

/* environment buffer, the kernel's size in lib/kobject_uevent.c should fit in */
#define HOTPLUG_BUFFER_SIZE		1024
#define HOTPLUG_NUM_ENVP		32
#define OBJECT_SIZE			512

void foo(void)
{
	int sock;
	struct sockaddr_nl snl;
	int retval;

	if (getuid() != 0) {
		printf("need to be root, exit\n");
		exit(1);
	}

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock == -1) {
		printf("error getting socket, exit\n");
		exit(1);
	}

	retval = bind(sock, (struct sockaddr *) &snl,
		      sizeof(struct sockaddr_nl));
	if (retval < 0) {
		printf("bind failed, exit %d, %s\n", __LINE__, strerror(errno));
	}

	close(sock);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_nl snl;
	int retval;

	if (getuid() != 0) {
		printf("need to be root, exit\n");
		exit(1);
	}

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock == -1) {
		printf("error getting socket, exit\n");
		exit(1);
	}

	retval = bind(sock, (struct sockaddr *) &snl,
		      sizeof(struct sockaddr_nl));
	if (retval < 0) {
		printf("bind failed, exit\n");
		goto exit;
	}

	retval = bind(sock, (struct sockaddr *) &snl,
		      sizeof(struct sockaddr_nl));
	if (retval < 0) {
		printf("bind failed, exit %d, %s\n", __LINE__, strerror(errno));
		exit(1);
	} else {
		printf("double bind success\n");
	}

	//foo();

	while (1) {
		static char buffer[HOTPLUG_BUFFER_SIZE + OBJECT_SIZE];
		static char object[OBJECT_SIZE];
		const char *devpath;
		const char *action;
		const char *envp[HOTPLUG_NUM_ENVP];
		int i;
		char *pos;
		size_t bufpos;
		ssize_t buflen;

		buflen = recv(sock, &buffer, sizeof(buffer), 0);
		if (buflen <  0) {
			printf("error receiving message\n");
			continue;
		}

		if ((size_t)buflen > sizeof(buffer)-1)
			buflen = sizeof(buffer)-1;

		buffer[buflen] = '\0';

		/* save start of payload */
		bufpos = strlen(buffer) + 1;

		/* action string */
		action = buffer;
		pos = strchr(buffer, '@');
		if (!pos)
			continue;
		pos[0] = '\0';

		/* sysfs path */
		devpath = &pos[1];

		/* hotplug events have the environment attached - reconstruct envp[] */
		for (i = 0; (bufpos < (size_t)buflen) && (i < HOTPLUG_NUM_ENVP-1); i++) {
			int keylen;
			char *key;

			key = &buffer[bufpos];
			keylen = strlen(key);
			envp[i] = key;
			bufpos += keylen + 1;
		}
		envp[i] = NULL;

		printf("[%i] received '%s' from '%s'\n", time(NULL), action, devpath);

		/* print payload environment */
		for (i = 0; envp[i] != NULL; i++)
			printf("%s\n", envp[i]);

		printf("\n");
	}

exit:
	close(sock);
	exit(1);
}
