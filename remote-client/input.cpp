#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "gfx.h"
#include "gui.h"
#include "input.h"
#include "scancodes.h"
#include "flash-kernel.h"

bool keys[SDLK_LAST];
//bool oldkeys[SDLK_LAST];
int mouseCoords[2];
bool mouseButtons[SDL_BUTTON_WHEELDOWN+1];
bool oldmouseButtons[SDL_BUTTON_WHEELDOWN+1];

int keyboardFd = -1;
int mouseFd = -1;

static int keycode_to_scancode[SDLK_LAST];


static int openDevices()
{
	keyboardFd = open("/dev/hidg0", O_RDWR, 0666);
	mouseFd = open("/dev/hidg1", O_RDWR, 0666);
}

void openInput()
{
	openDevices();
	if( keyboardFd == -1 || mouseFd == -1 )
	{
		if( !flashCustomKernel() )
			showErrorMessage("Cannot initialize keyboard/mouse device\n"
							 "To use this app, you have to install custom kernel\n"
							 "with HID keyboard/mouse gadget driver.\n"
							 "To install it, visit webpage\n"
							 "https://github.com/pelya/android-keyboard-gadget");
	}
	for( int k = SDLK_FIRST; k < SDLK_LAST; k++ )
	{
		for( int s = 0; s < SDL_NUM_SCANCODES; s++ )
		{
			if( scancodes_table[s] == k )
			{
				keycode_to_scancode[k] = s;
				break;
			}
		}
	}
}

static void outputSendKeys()
{
	uint8_t event[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	if( keyboardFd == -1 || mouseFd == -1 )
		openDevices();
	if( keyboardFd == -1 || mouseFd == -1 )
		return;

	event[0] |= keys[SDLK_LCTRL] ? 0x1 : 0;
	event[0] |= keys[SDLK_RCTRL] ? 0x10 : 0;
	event[0] |= keys[SDLK_LSHIFT] ? 0x2 : 0;
	event[0] |= keys[SDLK_RSHIFT] ? 0x20 : 0;
	event[0] |= keys[SDLK_LALT] ? 0x4 : 0;
	event[0] |= keys[SDLK_RALT] ? 0x40 : 0;
	event[0] |= keys[SDLK_LSUPER] ? 0x8 : 0;
	event[0] |= keys[SDLK_RSUPER] ? 0x80 : 0;
	
	int pos = 2;
	for(int i = 1; i < SDLK_LAST; i++)
	{
		if( keys[i] && keycode_to_scancode[i] < 255 )
		{
			event[pos] = keycode_to_scancode[i];
			pos++;
			if( pos >= sizeof(event) )
				break;
		}
	}
	//printf("Send key event: %d %d %d %d %d %d %d %d", event[0], event[1], event[2], event[3], event[4], event[5], event[6], event[7]);
	if( write(keyboardFd, event, sizeof(event)) != sizeof(event))
	{
		close(keyboardFd);
		close(mouseFd);
		keyboardFd = -1;
		mouseFd = -1;
	}
}

static void outputSendMouse(int x, int y, int b1, int b2, int b3, int wheel)
{
	uint8_t event[4] = {0, 0, 0, 0};

	if( keyboardFd == -1 || mouseFd == -1 )
		openDevices();
	if( keyboardFd == -1 || mouseFd == -1 )
		return;

	event[0] |= b1 ? 1 : 0;
	event[0] |= b2 ? 2 : 0;
	event[0] |= b3 ? 4 : 0;
	event[1] = x;
	event[2] = y;
	event[3] = wheel >= 0 ? wheel : 256 + wheel;
	if( write(mouseFd, event, sizeof(event)) != sizeof(event))
	{
		close(keyboardFd);
		close(mouseFd);
		keyboardFd = -1;
		mouseFd = -1;
	}
}

void processKeyInput(SDLKey key, int pressed)
{
	if( keys[key] == pressed )
	{
		//printf("processKeyInput: duplicate event %d %d", key, pressed);
		return;
	}
	//printf("processKeyInput: %d %d", key, pressed);
	keys[key] = pressed;
	outputSendKeys();
	//oldkeys[key] = keys[key];
}

void processMouseInput()
{
	if( mouseCoords[0] != 0 || mouseCoords[1] != 0 ||
		memcmp(oldmouseButtons, mouseButtons, sizeof(mouseButtons)) )
	{
		outputSendMouse(mouseCoords[0], mouseCoords[1],
						mouseButtons[SDL_BUTTON_LEFT], mouseButtons[SDL_BUTTON_RIGHT], mouseButtons[SDL_BUTTON_MIDDLE],
						(mouseButtons[SDL_BUTTON_WHEELUP] != oldmouseButtons[SDL_BUTTON_WHEELUP] && mouseButtons[SDL_BUTTON_WHEELUP]) ? 1 :
						(mouseButtons[SDL_BUTTON_WHEELDOWN] != oldmouseButtons[SDL_BUTTON_WHEELDOWN] && mouseButtons[SDL_BUTTON_WHEELDOWN]) ? -1 : 0);
	}
	mouseCoords[0] = 0;
	mouseCoords[1] = 0;
	memcpy(oldmouseButtons, mouseButtons, sizeof(mouseButtons));
}
