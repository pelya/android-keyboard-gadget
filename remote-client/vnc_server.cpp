#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <rfb/rfb.h>
#include <rfb/keysym.h>

#include "gfx.h"
#include "vnc_server.h"
#include "camera.h"
#include "input.h"

static bool serverRunning = false;

void vncServerStart()
{
	serverRunning = true;
	//openCamera(int *width, int *height, unsigned char ** buffer, int *bufferLength, CameraCallback_t callback);
}
void vncServerStop()
{
	serverRunning = false;
	//closeCamera();
}
bool vncServerRunning()
{
	return serverRunning;
}

std::string vncServerGetIpAddress()
{
	std::string ret;
	int sd, addr, ifc_num, i;
	struct ifconf ifc;
	struct ifreq ifr[20];
	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd > 0)
	{
		ifc.ifc_len = sizeof(ifr);
		ifc.ifc_ifcu.ifcu_buf = (caddr_t)ifr;

		if (ioctl(sd, SIOCGIFCONF, &ifc) == 0)
		{
			ifc_num = ifc.ifc_len / sizeof(struct ifreq);

			for (i = 0; i < ifc_num; ++i)
			{
				int addr = 0;
				char saddr[32];
				if (ifr[i].ifr_addr.sa_family != AF_INET)
					continue;

				if (ioctl(sd, SIOCGIFADDR, &ifr[i]) == 0)
					addr = ((struct sockaddr_in *)(&ifr[i].ifr_addr))->sin_addr.s_addr;
				if (addr == 0)
					continue;
				sprintf (saddr, "%d.%d.%d.%d", (addr & 0xFF), (addr >> 8 & 0xFF), (addr >> 16 & 0xFF), (addr >> 24 & 0xFF));
				if (strcmp(saddr, "127.0.0.1") == 0)
					continue;
				ret = saddr;
			}
		}

		close(sd);
	}
	return ret;
}
