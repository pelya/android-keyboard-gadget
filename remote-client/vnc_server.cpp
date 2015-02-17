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
static int width, height, videoBufferLength;
static unsigned char *videoBuffer;
static rfbScreenInfoPtr server;

static void mouseEvent(int buttonMask, int x, int y, rfbClientPtr cl);
static void keyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl);

static void onCameraFrame()
{
	rfbMarkRectAsModified(server, 0, 0, width, height);
}

void vncServerStart()
{
	if (serverRunning)
		return;
	printf("Starting VNC server");
	serverRunning = true;
	width = 1280;
	height = 720;
	openCamera(&width, &height, 5, &videoBuffer, &videoBufferLength, onCameraFrame);
	server = rfbGetScreen(NULL, NULL, width, height, 5, 3, 2);
	// RGB565
	server->serverFormat.redMax = 31;
	server->serverFormat.greenMax = 63;
	server->serverFormat.blueMax = 31;
	server->serverFormat.redShift = 11;
	server->serverFormat.greenShift = 5;
	server->serverFormat.blueShift = 0;
	char serialno[256] = "Unknown";
	FILE *ff = popen("getprop ro.serialno", "r");
	if (ff)
	{
		fgets(serialno, sizeof(serialno), ff);
		if (strstr(serialno, "\n") != NULL)
			strstr(serialno, "\n")[0] = 0;
		pclose(ff);
	}
	server->desktopName = strdup(serialno);
	server->frameBuffer = (char *)videoBuffer;
	server->alwaysShared = TRUE;
	server->ptrAddEvent = mouseEvent;
	server->kbdAddEvent = keyEvent;
	rfbInitServer(server);
	rfbRunEventLoop(server, -1, TRUE);
	printf("VNC server started");
}

void vncServerStop()
{
	printf("Stopping VNC server");
	rfbShutdownServer(server, TRUE);
	//rfbScreenCleanup(server); // TODO: crash here
	closeCamera();
	serverRunning = false;
	videoBuffer = NULL;
	printf("VNC server stopped");
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

void vncServerDrawVideoBuffer(int x, int y, int w, int h)
{
	if (!serverRunning)
		return;

	Uint16 *pixels = (Uint16 *)SDL_GetVideoSurface()->pixels;
	Uint32 pitch = SDL_GetVideoSurface()->w;
	Uint32 srcWidth = server->width;
	Uint32 srcHeight = server->height;
	pixels += x + y * pitch;

	for(Uint32 hh = 0; hh < h; hh++, pixels += pitch)
	{
		Uint16 *src = (Uint16 *)videoBuffer;
		src += (srcHeight * hh / h) * srcWidth;
		for (Uint32 ww = 0; ww < w; ww++)
		{
			pixels[ww] = src[srcWidth * ww / w];
		}
	}
}

void mouseEvent(int buttonMask, int x, int y, rfbClientPtr cl)
{
	int dx = x - server->width / 2;
	int dy = y - server->height / 2;
	mouseCoords[0] += dx;
	mouseCoords[1] += dy;
	mouseButtons[SDL_BUTTON_LEFT] = (buttonMask & 0x1) != 0;
	mouseButtons[SDL_BUTTON_MIDDLE] = (buttonMask & 0x2) != 0;
	mouseButtons[SDL_BUTTON_RIGHT] = (buttonMask & 0x4) != 0;
	mouseButtons[SDL_BUTTON_WHEELUP] = (buttonMask & 0x8) != 0;
	mouseButtons[SDL_BUTTON_WHEELDOWN] = (buttonMask & 0x10) != 0;
	mouseButtons[SDL_BUTTON_X1] = (buttonMask & 0x20) != 0;
	mouseButtons[SDL_BUTTON_X2] = (buttonMask & 0x40) != 0;
	processMouseInput();
	rfbDefaultPtrAddEvent(buttonMask, server->width / 2, server->height / 2, cl);
	cl->cursorWasMoved = TRUE;
}

void keyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
}
