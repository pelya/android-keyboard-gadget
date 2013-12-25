#include <string.h>
#include "gfx.h"
#include "gui.h"
#include "input.h"

struct TouchPointer_t touchPointers[MAX_POINTERS];

std::vector<GuiElement_t> gui;

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
	renderStringColor(elem->text.c_str(), elem->rect.x + elem->rect.w / 2, elem->rect.y + elem->rect.h / 2, elem->toggled ? 0 : 255, elem->toggled ? 0 : 255, elem->toggled ? 0 : 255);
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
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
	mouseButtons[button] = elem->toggled || (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
}

static void mouseMovementCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( elem->toggled && pressed )
	{
		mouseCoords[0] = x - elem->x;
		mouseCoords[1] = y - elem->y;
	}
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

void createGuiMain()
{
	gui.clear();
	gui.push_back(GuiElement_t("Left", VID_X * 0.7, 0, VID_X * 0.1, VID_Y * 0.3, mouseInputCallback<SDL_BUTTON_LEFT>));
	gui.push_back(GuiElement_t("Right", VID_X * 0.9, 0, VID_X * 0.1, VID_Y * 0.3, mouseInputCallback<SDL_BUTTON_RIGHT>));
	gui.push_back(GuiElement_t("Wheel", VID_X * 0.8, VID_Y * 0.1, VID_X * 0.1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_MIDDLE>));
	gui.push_back(GuiElement_t("Up", VID_X * 0.8, 0, VID_X * 0.1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_WHEELUP>));
	gui.push_back(GuiElement_t("Down", VID_X * 0.8, VID_Y * 0.1 * 2, VID_X * 0.1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_WHEELDOWN>));
	gui.push_back(GuiElement_t("Mouse", VID_X * 0.5, VID_Y * 0.3, VID_X * 0.5, VID_Y * 0.7, mouseMovementCallback));

	gui.push_back(GuiElement_t("Keyboard", VID_X * 0.5, 0, VID_X * 0.15, VID_Y * 0.1, keyboardToggleCallback));
	gui.push_back(GuiElement_t("Ctrl", VID_X * 0.500, VID_Y * 0.1, VID_X * 0.075, VID_Y * 0.1, keyInputCallback<SDLK_LCTRL>));
	gui.push_back(GuiElement_t("Alt", VID_X * 0.575, VID_Y * 0.1, VID_X * 0.075, VID_Y * 0.1, keyInputCallback<SDLK_LALT>));
	gui.push_back(GuiElement_t("Shift", VID_X * 0.500, VID_Y * 0.2, VID_X * 0.075, VID_Y * 0.1, keyInputCallback<SDLK_LSHIFT>));
	gui.push_back(GuiElement_t("Meta", VID_X * 0.575, VID_Y * 0.2, VID_X * 0.075, VID_Y * 0.1, keyInputCallback<SDLK_LSUPER>));

	//SDL_ShowScreenKeyboard(NULL);
}

static int dialogResult = 0;
static bool dialogAnswered = false;
enum { DIALOG_MESSAGE_LINES = 5 };
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
		renderStringColor(dialogMessage[i], elem->rect.x + elem->rect.w / 2, elem->rect.y + i * VID_Y * 0.1 + VID_Y * 0.05, 255, 255, 255);
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
	gui.push_back(GuiElement_t("Dialog Text", VID_X * 0.1, VID_Y * 0.1, VID_X * 0.8, VID_Y * 0.8, GuiElement_t::defaultInputCallback, dialogDrawTextCallback));
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
	gui.push_back(GuiElement_t(url, VID_X * 0.11, VID_Y * 0.5, VID_X * 0.78, VID_Y * 0.1, dialogUrlButton));
}

void addDialogYesNoButtons()
{
	dialogAnswered = false;
	dialogResult = 0;
	gui.push_back(GuiElement_t("Yes", VID_X * 0.2, VID_Y * 0.7, VID_X * 0.2, VID_Y * 0.1, dialogInputCallback<1>));
	gui.push_back(GuiElement_t("No", VID_X * 0.6, VID_Y * 0.7, VID_X * 0.2, VID_Y * 0.1, dialogInputCallback<0>));
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

void mainLoop()
{
	static Uint32 lastEvent = 0;
	SDL_Event evt;
	while( SDL_PollEvent(&evt) )
	{
		lastEvent = SDL_GetTicks();
		if(evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN)
		{
			if(evt.key.keysym.sym == SDLK_UNDO)
				exit(0);
			processKeyInput(evt.key.keysym.sym, evt.key.state == SDL_PRESSED);
			//printf("Got key %d %d", evt.key.keysym.sym, evt.key.state == SDL_PRESSED);
		}
		// PC mouse events
		if(evt.type == SDL_MOUSEBUTTONUP || evt.type == SDL_MOUSEBUTTONDOWN)
		{
			// TODO: implement PC input
			touchPointers[evt.jbutton.button].pressed = (evt.button.state == SDL_PRESSED);
		}
		if(evt.type == SDL_MOUSEMOTION)
		{
			// TODO: implement PC input
			touchPointers[evt.jbutton.button].x = evt.motion.x;
			touchPointers[evt.jbutton.button].y = evt.motion.y;
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
	processMouseInput();

	SDL_Flip(SDL_GetVideoSurface());
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);

	if( lastEvent + 1000 < SDL_GetTicks() )
		SDL_Delay(150);
}
