#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "gfx.h"
#include "gui.h"
#include "input.h"

bool keys[SDLK_LAST];
bool oldkeys[SDLK_LAST];
int mouseCoords[2];
bool mouseButtons[SDL_BUTTON_WHEELDOWN+1];
bool oldmouseButtons[SDL_BUTTON_WHEELDOWN+1];

int keyboardFd = -1;
int mouseFd = -1;

void openInput()
{
	keyboardFd = open("/dev/hidg0", O_RDWR, 0666);
	mouseFd = open("/dev/hidg1", O_RDWR, 0666);
	if( keyboardFd == -1 || mouseFd == -1 )
	{
		showErrorMessage("Cannot initialize keyboard/mouse device\n"
						 "To use this app, you have to install custom kernel\n"
						 "with HID keyboard/mouse gadget driver.\n"
						 "To install it, visit webpage\n"
						 "https://github.com/pelya/android-keyboard-gadget");
	}
}

static void outputSendKey(int key, bool pressed)
{
	uint8_t event[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	//write(keyboardFd, event, sizeof(event));
}

static void outputSendMouse(int x, int y, int b1, int b2, int b3, int wheel)
{
	uint8_t event[4] = {0, 0, 0, 0};
	event[0] |= b1 ? 1 : 0;
	event[0] |= b2 ? 2 : 0;
	event[0] |= b3 ? 4 : 0;
	event[1] = x;
	event[2] = y;
	event[3] = wheel;
	write(mouseFd, event, sizeof(event));
}

void processInput()
{
	for(int i = 0; i < SDLK_LAST; i++)
	{
		if( oldkeys[i] != keys[i] )
		{
			outputSendKey(i, keys[i]);
		}
	}
	if( mouseCoords[0] != 0 || mouseCoords[1] != 0 ||
		memcmp(oldmouseButtons, mouseButtons, sizeof(mouseButtons)) )
	{
		outputSendMouse(mouseCoords[0], mouseCoords[1],
						mouseButtons[SDL_BUTTON_LEFT], mouseButtons[SDL_BUTTON_RIGHT], mouseButtons[SDL_BUTTON_MIDDLE],
						mouseButtons[SDL_BUTTON_WHEELUP] != oldmouseButtons[SDL_BUTTON_WHEELUP] ? -1 :
						mouseButtons[SDL_BUTTON_WHEELDOWN] != mouseButtons[SDL_BUTTON_WHEELDOWN] ? 1 : 0);
	}
	memcpy(oldkeys, keys, sizeof(keys));
	mouseCoords[0] = 0;
	mouseCoords[1] = 0;
	memcpy(oldmouseButtons, mouseButtons, sizeof(mouseButtons));
}
