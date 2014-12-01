#include <string.h>
#include <SDL/SDL_image.h>

#include "gfx.h"
#include "gui.h"
#include "input.h"
#include "touchpad.h"

struct TouchPointer_t touchPointers[MAX_POINTERS];
static SDL_Surface* sClipboardImage1 = NULL;
static SDL_Surface* sClipboardImage2 = NULL;
static SDL_Surface* sKeyboardImage = NULL;
static float mouseSpeed = 1.0f;
static const char * mouseSpeedSaveFile = "mouse-speed.cfg";

std::vector<GuiElement_t> gui;

enum { TOUCHPAD_X0 = 0, TOUCHPAD_Y0 = 0, TOUCHPAD_X1 = int(VID_X * 0.6), TOUCHPAD_Y1 = VID_Y };

float getMouseSpeed()
{
	return mouseSpeed;
}

void GuiElement_t::defaultInputCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	elem->toggled = pressed;
	elem->x = x;
	elem->y = y;
}

void GuiElement_t::defaultDrawCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	SDL_Rect r;
	Uint32 color = SDL_MapRGB(SDL_GetVideoSurface()->format, elem->toggled ? 192 : 128, elem->toggled ? 192 : 128, elem->toggled ? 192 : 128);
	SDL_FillRect(SDL_GetVideoSurface(), &elem->rect, color);
	color = SDL_MapRGB(SDL_GetVideoSurface()->format, 255, 255, 255);
	r = elem->rect;
	r.w = 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	r.x = elem->rect.x + elem->rect.w - 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	r = elem->rect;
	r.h = 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	r.y = elem->rect.y + elem->rect.h - 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	for( int i = 0; i < elem->text.size(); i++ )
	{
		renderStringColor(	elem->text[i].c_str(), elem->rect.x + elem->rect.w / 2,
							elem->rect.y + elem->rect.h / 2 + i * TEXT_H * 1.2f - (elem->text.size() - 1) * TEXT_H * 1.2f / 2,
							elem->toggled ? 0 : 255, elem->toggled ? 0 : 255, elem->toggled ? 0 : 255);
	}
}

static bool toggleElement(GuiElement_t * elem, bool pressed)
{
	if( !pressed )
		elem->locked = false;
	if( pressed && !elem->locked )
	{
		elem->toggled = !elem->toggled;
		elem->locked = true;
		return true;
	}
	return false;
}

template<SDLKey key>
static void keyInputCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( toggleElement(elem, pressed) )
		processKeyInput(key, elem->toggled || SDL_GetKeyState(NULL)[key]);
}

template<int button>
static void mouseInputCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	static bool toggled = 0;
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
	if( toggled != elem->toggled )
	{
		mouseButtons[button] = elem->toggled; // || (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
		toggled = elem->toggled;
	}
}

static void mouseMovementCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( elem->toggled && pressed )
	{
		mouseCoords[0] += (x - elem->x) * mouseSpeed;
		mouseCoords[1] += (y - elem->y) * mouseSpeed;
	}
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
}

static void mouseSpeedCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	static bool oldPressed = 0;
	static int guiMouseSpeedX = 0;
	char s[128];

	if( pressed )
	{
		if( !oldPressed )
			guiMouseSpeedX = x;
		mouseSpeed += (x - guiMouseSpeedX) / 300.0f;
		guiMouseSpeedX = x;
		sprintf(s, "Mouse speed %4.2f - swipe", mouseSpeed);
		elem->text[0] = s;
		oldPressed = true;
	}
	else
	{
		if( oldPressed )
		{
			FILE * ff = fopen(mouseSpeedSaveFile, "w");
			if( ff )
			{
				fprintf(ff, "%f", mouseSpeed);
				fclose(ff);
				sprintf(s, "Mouse speed %4.2f", mouseSpeed);
				elem->text[0] = s;
			}
		}
		oldPressed = false;
	}

	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
}

static void touchpadCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	// Just highlight it
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
}

static void keyboardToggleCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( toggleElement(elem, pressed) )
	{
		//printf( "Show screen keyboard: %d", elem->toggled);
		SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput();
	}
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
}

static void DrawKeyboardImageCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	GuiElement_t::defaultDrawCallback(elem, pressed, x, y);
	drawImageCentered(sKeyboardImage, elem->rect.x + elem->rect.w / 2, elem->rect.y + elem->rect.h / 2);
}

static void DrawClipboardImageCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	GuiElement_t::defaultDrawCallback(elem, pressed, x, y);
	drawImageCentered(elem->toggled ? sClipboardImage2 : sClipboardImage1, elem->rect.x + elem->rect.w / 2, elem->rect.y + elem->rect.h / 2);
}

static int checkShiftRequired( int *sym )
{
	switch( *sym )
	{
		case '!': *sym = '1'; return 1;
		case '@': *sym = '2'; return 1;
		case '#': *sym = '3'; return 1;
		case '$': *sym = '4'; return 1;
		case '%': *sym = '5'; return 1;
		case '^': *sym = '6'; return 1;
		case '&': *sym = '7'; return 1;
		case '*': *sym = '8'; return 1;
		case '(': *sym = '9'; return 1;
		case ')': *sym = '0'; return 1;
		case '_': *sym = '-'; return 1;
		case '+': *sym = '='; return 1;
		case '|': *sym = '\\';return 1;
		case '<': *sym = ','; return 1;
		case '>': *sym = '.'; return 1;
		case '?': *sym = '/'; return 1;
		case ':': *sym = ';'; return 1;
		case '"': *sym = '\'';return 1;
		case '{': *sym = '['; return 1;
		case '}': *sym = ']'; return 1;
		case '~': *sym = '`'; return 1;
		default: if( *sym >= 'A' && *sym <= 'Z' ) { *sym += 'a' - 'A'; return 1; };
	}
	return 0;
}

static void ProcessClipboardImageCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( toggleElement(elem, pressed) )
	{
		char buf[1024];
		SDL_ANDROID_GetClipboardText(buf, sizeof(buf));
		for( int i = 0; buf[i]; i++ )
		{
			int key = buf[i];
			int shiftRequired = checkShiftRequired(&key);

			if( shiftRequired )
				processKeyInput(SDLK_LSHIFT, 1);

			processKeyInput((SDLKey)key, 1);
			processKeyInput((SDLKey)key, 0);

			if( shiftRequired )
				processKeyInput(SDLK_LSHIFT, 0);
		}
	}
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
}

void createGuiMain()
{
	if( !sClipboardImage1 )
		sClipboardImage1 = IMG_Load("ic_menu_paste_holo_dark.png");
	if( !sClipboardImage2 )
		sClipboardImage2 = IMG_Load("ic_menu_paste_holo_light.png");
	if( !sKeyboardImage )
		sKeyboardImage = IMG_Load("keyboard.png");

	gui.clear();
	gui.push_back(GuiElement_t("Left", VID_X * 0.8,                                 0, VID_X * 0.2 / 3, VID_Y * 0.3, mouseInputCallback<SDL_BUTTON_LEFT>));
	gui.push_back(GuiElement_t("Up",   VID_X * 0.8 + VID_X * 0.2 / 3,               0, VID_X * 0.2 / 3 + 1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_WHEELUP>));
	gui.push_back(GuiElement_t("Mid",  VID_X * 0.8 + VID_X * 0.2 / 3,     VID_Y * 0.1, VID_X * 0.2 / 3 + 1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_MIDDLE>));
	gui.push_back(GuiElement_t("Down", VID_X * 0.8 + VID_X * 0.2 / 3, VID_Y * 0.1 * 2, VID_X * 0.2 / 3 + 1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_WHEELDOWN>));
	gui.push_back(GuiElement_t("Right",VID_X * 0.8 + VID_X * 0.4 / 3,               0, VID_X * 0.2 / 3 + 1, VID_Y * 0.3, mouseInputCallback<SDL_BUTTON_RIGHT>));
	gui.push_back(GuiElement_t("Mouse - swipe to move", VID_X * 0.6, VID_Y * 0.3, VID_X * 0.5, VID_Y * 0.6, mouseMovementCallback));
	gui.push_back(GuiElement_t("Swipe to change mouse speed", VID_X * 0.6, VID_Y * 0.9, VID_X * 0.5, VID_Y * 0.1, mouseSpeedCallback));
	gui.push_back(GuiElement_t( (const char *[])
		{
			"Touchpad",
			"",
			"Swipe to move mouse cursor",
			"",
			"Tap or hold for left mouse click",
			"to send left mouse click",
			"",
			"Touch with two fingers",
			"to send right mouse click",
			"",
			"Touch with three fingers",
			"to send middle mouse click",
			"",
			"Swipe with two fingers",
			"to scroll mouse wheel",
			NULL
		},
		TOUCHPAD_X0, TOUCHPAD_Y0, TOUCHPAD_X1, TOUCHPAD_Y1, touchpadCallback));

	gui.push_back(GuiElement_t("",      VID_X * 0.6, 0,           VID_X * 0.1, VID_Y * 0.1, keyboardToggleCallback, DrawKeyboardImageCallback));
	gui.push_back(GuiElement_t("",      VID_X * 0.7, 0,           VID_X * 0.1, VID_Y * 0.1, ProcessClipboardImageCallback, DrawClipboardImageCallback));
	gui.push_back(GuiElement_t("Ctrl",  VID_X * 0.6, VID_Y * 0.1, VID_X * 0.1, VID_Y * 0.1, keyInputCallback<SDLK_LCTRL>));
	gui.push_back(GuiElement_t("Alt",   VID_X * 0.7, VID_Y * 0.1, VID_X * 0.1, VID_Y * 0.1, keyInputCallback<SDLK_LALT>));
	gui.push_back(GuiElement_t("Shift", VID_X * 0.6, VID_Y * 0.2, VID_X * 0.1, VID_Y * 0.1, keyInputCallback<SDLK_LSHIFT>));
	gui.push_back(GuiElement_t("Meta",  VID_X * 0.7, VID_Y * 0.2, VID_X * 0.1, VID_Y * 0.1, keyInputCallback<SDLK_LSUPER>));

	FILE * ff = fopen(mouseSpeedSaveFile, "r");
	if( ff )
	{
		char s[128] = "1.0";
		fgets(s, sizeof(s) - 1, ff);
		fclose(ff);
		mouseSpeed = atof(s);
	}
	//SDL_ShowScreenKeyboard(NULL);
}

static int dialogResult = 0;
static bool dialogAnswered = false;
enum { DIALOG_MESSAGE_LINES = 7 };
static char dialogMessage[DIALOG_MESSAGE_LINES][256];

bool getDialogResult(int * result)
{
	*result = dialogResult;
	return dialogAnswered;
}

void addDialogText(const char *text)
{
	for(int i = 0; i < DIALOG_MESSAGE_LINES - 1; i++)
		strcpy(dialogMessage[i], dialogMessage[i+1]);
	strncpy(dialogMessage[DIALOG_MESSAGE_LINES - 1], text, sizeof(dialogMessage[DIALOG_MESSAGE_LINES - 1]));
	dialogMessage[DIALOG_MESSAGE_LINES - 1][sizeof(dialogMessage[DIALOG_MESSAGE_LINES - 1]) - 1] = 0;
}

void clearDialogText()
{
	for(int i = 0; i < DIALOG_MESSAGE_LINES; i++)
		addDialogText("");
}

static void dialogDrawTextCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	SDL_Rect r;
	Uint32 color = SDL_MapRGB(SDL_GetVideoSurface()->format, 255, 255, 255);
	r = elem->rect;
	r.w = 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	r.x = elem->rect.x + elem->rect.w - 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	r = elem->rect;
	r.h = 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	r.y = elem->rect.y + elem->rect.h - 1;
	SDL_FillRect(SDL_GetVideoSurface(), &r, color);
	for( int i = 0; i < DIALOG_MESSAGE_LINES; i++ )
	{
		renderStringColor(dialogMessage[i], elem->rect.x + elem->rect.w / 2, elem->rect.y + i * VID_Y * 0.1 + VID_Y * 0.04, 255, 255, 255);
	}
}

template<int result>
static void dialogInputCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
	if( elem->toggled )
	{
		dialogResult = result;
		dialogAnswered = true;
	}
}

void createDialog()
{
	gui.clear();

	clearDialogText();
	gui.push_back(GuiElement_t("Dialog Text", VID_X * 0.01, VID_Y * 0.01, VID_X * 0.98, VID_Y * 0.98, GuiElement_t::defaultInputCallback, dialogDrawTextCallback));
}

static const char * dialogUrlButtonUrl = "";
static void dialogUrlButton(GuiElement_t * elem, bool pressed, int x, int y)
{
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
	if( elem->toggled )
	{
		char cmd[1024] = "am start --user 0 -a android.intent.action.VIEW -d ";
		strcat(cmd, dialogUrlButtonUrl);
		system(cmd);
		exit(0);
	}
}

void addDialogUrlButton(const char *url)
{
	dialogUrlButtonUrl = url;
	int i, pos = 0;
	for(int i = 0; i < DIALOG_MESSAGE_LINES - 1; i++)
		if( strcmp(dialogMessage[i], "") != 0 )
			pos = i + 1;
	gui.push_back(GuiElement_t(url, VID_X * 0.05, pos * VID_Y * 0.1, VID_X * 0.90, VID_Y * 0.1, dialogUrlButton));
}

void addDialogYesNoButtons()
{
	dialogAnswered = false;
	dialogResult = 0;
	gui.push_back(GuiElement_t("Yes", VID_X * 0.2, VID_Y * 0.8, VID_X * 0.2, VID_Y * 0.1, dialogInputCallback<1>));
	gui.push_back(GuiElement_t("No", VID_X * 0.6, VID_Y * 0.8, VID_X * 0.2, VID_Y * 0.1, dialogInputCallback<0>));
}

void processGui()
{
	for( int ii = 0; ii < gui.size(); ii++ )
	{
		bool processed = false;
		for( int i = 0; i < MAX_POINTERS; i++ )
		{
			if( touchPointers[i].pressed &&
				touchPointers[i].x >= gui[ii].rect.x &&
				touchPointers[i].x <= gui[ii].rect.x + gui[ii].rect.w &&
				touchPointers[i].y >= gui[ii].rect.y &&
				touchPointers[i].y <= gui[ii].rect.y + gui[ii].rect.h )
			{
				processed = true;
				gui[ii].input(&gui[ii], true, touchPointers[i].x - gui[ii].rect.x, touchPointers[i].y - gui[ii].rect.y);
				gui[ii].draw(&gui[ii], true, touchPointers[i].x - gui[ii].rect.x, touchPointers[i].y - gui[ii].rect.y);
				break;
			}
		}
		if( !processed )
		{
			gui[ii].input(&gui[ii], false, touchPointers[0].x - gui[ii].rect.x, touchPointers[0].y - gui[ii].rect.y);
			gui[ii].draw(&gui[ii], false, touchPointers[0].x - gui[ii].rect.x, touchPointers[0].y - gui[ii].rect.y);
		}
	}
}

void mainLoop(bool noHid)
{
	static Uint32 lastEvent = 0;
	SDL_Event evt;
	while( SDL_PollEvent(&evt) )
	{
		lastEvent = SDL_GetTicks();
		if( evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN )
		{
			if( evt.key.keysym.sym == SDLK_UNDO )
				exit(0);
			processKeyInput(evt.key.keysym.sym, evt.key.state == SDL_PRESSED);
			//printf("Got key %d %d", evt.key.keysym.sym, evt.key.state == SDL_PRESSED);
		}
		// PC mouse events
		if( evt.type == SDL_MOUSEBUTTONUP || evt.type == SDL_MOUSEBUTTONDOWN )
		{
			// TODO: implement PC input
			touchPointers[evt.jbutton.button].pressed = (evt.button.state == SDL_PRESSED);
			/*
			if( evt.button.state == SDL_PRESSED )
				touchPointers[evt.jbutton.button].pressedTime = lastEvent;
			else
				touchPointers[evt.jbutton.button].releasedTime = lastEvent;
			*/
		}
		if( evt.type == SDL_MOUSEMOTION )
		{
			// TODO: implement PC input
			touchPointers[0].x = evt.motion.x;
			touchPointers[0].y = evt.motion.y;
		}
		// Android-specific events - accelerometer, multitoush, and on-screen joystick
		if( evt.type == SDL_JOYAXISMOTION )
		{
			if(evt.jaxis.which == 0) // Multitouch and on-screen joysticks
			{
				if(evt.jaxis.axis >= 4)
					touchPointers[evt.jaxis.axis - 4].pressure = evt.jaxis.value;
			}
		}
		if( evt.type == SDL_JOYBUTTONDOWN || evt.type == SDL_JOYBUTTONUP )
		{
			if(evt.jbutton.which == 0) // Multitouch and on-screen joystick
				touchPointers[evt.jbutton.button].pressed = (evt.jbutton.state == SDL_PRESSED);
			//printf("Touch press %d: %d", evt.jbutton.button, evt.jbutton.state);
		}
		if( evt.type == SDL_JOYBALLMOTION )
		{
			if(evt.jball.which == 0) // Multitouch and on-screen joystick
			{
				touchPointers[evt.jball.ball].x = evt.jball.xrel;
				touchPointers[evt.jball.ball].y = evt.jball.yrel;
				//printf("Touch %d: %d %d", evt.jball.ball, evt.jball.xrel, evt.jball.yrel);
			}
		}
	}

	processGui();
	if( !noHid )
	{
		processTouchpad(TOUCHPAD_X0, TOUCHPAD_Y0, TOUCHPAD_X1, TOUCHPAD_Y1);
		processMouseInput();
	}

	SDL_Flip(SDL_GetVideoSurface());
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);

	if( lastEvent + 1000 < SDL_GetTicks() )
		SDL_Delay(150);
}
