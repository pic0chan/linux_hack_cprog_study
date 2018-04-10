/* copyright Linux Kernel Hack */
/* gcc -g -O2 -o tunpong tunpong.c */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int tun_open(void)
{
		struct ifreq ifr;
		int fd;
		char dev[IFNAMSIZ];
		char buf[512];

		/* TUN/TAPデバイスを操作するためのファイルをオープン*/
		fd = open("/dev/net/tun", O_RDWR);

		/* TUNデバイス(tun0)を生成*/
		memset(&ifr, 0, sizeof(ifr));
		ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
		strncpy(ifr.ifr_name, "tun%d", IFNAMSIZ);
		ioctl(fd, TUNSETIFF, &ifr);

		strncpy(dev, ifr.ifr_name, IFNAMSIZ);

		/* ifconfigコマンドでIPアドレスを追加*/
		sprintf(buf, "ifconfig %s 192.168.20.1 pointopoint 192.168.20.2", dev);
		system(buf);

		return fd;
}

void dump_pkt(unsigned char *pkt, int len)
{
		int i;
		for (i = 0; i < len; i++)
				printf("%02x ", pkt[i]);
		printf("\n");
}

void pingpong(int fd)
{
		fd_set fds;
		int len;
		unsigned char pkt[512];

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		select(fd + 1, &fds, NULL, NULL, NULL);
		if (FD_ISSET(fd, &fds)) {
				len = read(fd, pkt, 512);
				dump_pkt(pkt, len);
				/*ICMPヘッダ部分が0x08(echo request)*/
				if (pkt[20] != 0x08)
						return;
				/*IPヘッダのsrcを192.168.20.2, dstを192.168.20.1に*/
				pkt[15] = 0x02;
				pkt[19] = 0x01;
				pkt[20] = 0x00; /*echo reply*/
				write(fd, pkt, len);
		}
}

int main(int argc, char **argv)
{
		int fd;

		fd = tun_open();
		for(;;)
				pingpong(fd);
		return 0;
}

