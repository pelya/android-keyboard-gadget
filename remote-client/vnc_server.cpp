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
#include "gui.h"
#include "vnc_keysyms.h"

static bool serverRunning = false;
static int width, height, videoBufferLength;
static float mouseSpeedX = 0, mouseSpeedY = 0;
static unsigned char *videoBuffer;
static rfbScreenInfoPtr server;
enum { MOUSE_SPEED_NEAR_EDGES = 10 };

static void mouseEvent(int buttonMask, int x, int y, rfbClientPtr cl);
static void keyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl);
static void processMouseBorders();

static void onCameraFrame()
{
	if( settingsGuiShown() )
	{
		SDL_SemWaitTimeout(screenRedrawSemaphore, 200);
		// Copy settings GUI to VNC framebuffer
		Uint32 pitch = SDL_GetVideoSurface()->w;
		Uint32 dstPitch = server->width;
		Uint16 *pixels = (Uint16 *)SDL_GetVideoSurface()->pixels;
		Uint16 *dst = (Uint16 *)videoBuffer;
		dst += (server->width - SDL_GetVideoSurface()->w) / 2;
		dst += dstPitch * (server->height - SDL_GetVideoSurface()->h) / 2;

		for(Uint32 y = SDL_GetVideoSurface()->h; y > 0; y--, pixels += pitch, dst += dstPitch)
		{
			for (Uint32 x = 0; x < pitch; x++)
			{
				dst[x] = pixels[x];
			}
		}
	}
	rfbMarkRectAsModified(server, 0, 0, width, height);
	processMouseBorders();
}

void vncServerStart()
{
	if (serverRunning)
		return;
	printf("Starting VNC server");
	serverRunning = true;
	width = 1280;
	height = 720;
	mouseSpeedX = 0;
	mouseSpeedY = 0;
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
	mouseSpeedX = 0;
	mouseSpeedY = 0;
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
	static int oldX = 0, oldY = 0;
	if( settingsGuiShown() )
	{
		rfbDefaultPtrAddEvent(buttonMask, x, y, cl);
		int dx = (server->width - SDL_GetVideoSurface()->w) / 2;
		int dy = (server->height - SDL_GetVideoSurface()->h) / 2;
		x -= dx;
		y -= dy;
		if (x >= 0 && y >= 0 && x < SDL_GetVideoSurface()->w && y < SDL_GetVideoSurface()->h)
		{
			touchPointers[0].x = x;
			touchPointers[0].y = y;
			if( (buttonMask & 0x1) != 0 )
				touchPointers[0].pressed = true;
			else
				touchPointers[0].delayRelease = true;
		}
		else
		{
			touchPointers[0].pressed = false;
		}
	}
	else
	{
		int dx = x - oldX;
		int dy = y - oldY;
		//printf("Mouse moved %04d %04d delta %05d %05d", x, y, dx, dy);

		// Stupid clients ignore server command to set pointer coordinates
#if 0
		if( abs(x - server->width / 2) + abs(y - server->height / 2) > 300 && abs(dx) + abs(dy) < 50 )
		{
			//printf("Moving mouse to %04d %04d", server->width / 2, server->height / 2);
			x = server->width / 2;
			y = server->height / 2;
			rfbDefaultPtrAddEvent(buttonMask, server->width / 2, server->height / 2, cl);
			cl->cursorWasMoved = TRUE;
			return;
		}
#endif

		oldX = x;
		oldY = y;
		rfbDefaultPtrAddEvent(buttonMask, x, y, cl);

		if( abs(dx) + abs(dy) > 100 )
		{
			//printf("Mouse warped - ignoring event");
			return;
		}

		mouseCoords[0] += dx * getMouseSpeed();
		mouseCoords[1] += dy * getMouseSpeed();
		mouseButtons[SDL_BUTTON_LEFT] = (buttonMask & 0x1) != 0;
		mouseButtons[SDL_BUTTON_MIDDLE] = (buttonMask & 0x2) != 0;
		mouseButtons[SDL_BUTTON_RIGHT] = (buttonMask & 0x4) != 0;
		mouseButtons[SDL_BUTTON_WHEELUP] = (buttonMask & 0x8) != 0;
		mouseButtons[SDL_BUTTON_WHEELDOWN] = (buttonMask & 0x10) != 0;
		mouseButtons[SDL_BUTTON_X1] = (buttonMask & 0x20) != 0;
		mouseButtons[SDL_BUTTON_X2] = (buttonMask & 0x40) != 0;
		processMouseInput();

		mouseSpeedX = 0;
		mouseSpeedY = 0;

		if (x < width / 20)
			mouseSpeedX = -MOUSE_SPEED_NEAR_EDGES * getMouseSpeed();
		if (x > width - (width / 20))
			mouseSpeedX = MOUSE_SPEED_NEAR_EDGES * getMouseSpeed();
		if (y < height / 20)
			mouseSpeedY = -MOUSE_SPEED_NEAR_EDGES * getMouseSpeed();
		if (y > height - (height / 20))
			mouseSpeedY = MOUSE_SPEED_NEAR_EDGES * getMouseSpeed();
	}
}

void keyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
	bool keyProcessed = false;

	if( key == XK_Scroll_Lock )
	{
		if( !down )
			return;
		if( !settingsGuiShown() )
		{
			settingsShowGui();
			return;
		}
		else
		{
			settingsCloseGui();
			processKeyInput(SDLK_SCROLLOCK, 0, true);
			processKeyInput(SDLK_SCROLLOCK, 0, false);
		}
		return;
	}

	SDLKey keysym = SDLK_UNKNOWN;
	uint32_t unicode = 0;

	if( vncKeysymToSDLKey.count(key) > 0 )
		keysym = vncKeysymToSDLKey[key];
	else if( vncKeysymToUnicode.count(key) > 0 )
		unicode = vncKeysymToUnicode[key];

	if(keysym == SDLK_UNKNOWN && unicode == 0)
		return;

	if( settingsGuiShown() )
	{
		settingsProcessKeyInput(keysym, unicode, down != FALSE);
		return;
	}

	if( keysym != SDLK_UNKNOWN )
		keyProcessed = processKeyInput(keysym, 0, down != FALSE);
	else if( unicode > 0 )
		keyProcessed = processKeyInput(SDLK_UNKNOWN, unicode, down != FALSE);

	if( !keyProcessed )
	{
		settingsShowGui();
		settingsDefineKeycode(keysym, unicode);
	}
}

void processMouseBorders()
{
	if( mouseSpeedX != 0 || mouseSpeedY != 0 )
	{
		mouseCoords[0] += mouseSpeedX;
		mouseCoords[1] += mouseSpeedY;
		processMouseInput();
	}
}
