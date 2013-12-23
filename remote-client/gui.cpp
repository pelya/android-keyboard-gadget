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
	toggleElement(elem, pressed);

	keys[key] = elem->toggled || SDL_GetKeyState(NULL)[key];
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

void createGui()
{
	gui.push_back(GuiElement_t("Left", VID_X * 0.7, 0, VID_X * 0.1, VID_Y * 0.3, mouseInputCallback<SDL_BUTTON_LEFT>));
	gui.push_back(GuiElement_t("Right", VID_X * 0.9, 0, VID_X * 0.1, VID_Y * 0.3, mouseInputCallback<SDL_BUTTON_RIGHT>));
	gui.push_back(GuiElement_t("Wheel", VID_X * 0.8, VID_Y * 0.1, VID_X * 0.1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_MIDDLE>));
	gui.push_back(GuiElement_t("Up", VID_X * 0.8, 0, VID_X * 0.1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_WHEELUP>));
	gui.push_back(GuiElement_t("Down", VID_X * 0.8, VID_Y * 0.1 * 2, VID_X * 0.1, VID_Y * 0.1, mouseInputCallback<SDL_BUTTON_WHEELDOWN>));
	gui.push_back(GuiElement_t("Mouse", VID_X * 0.7, VID_Y * 0.3, VID_X * 0.3, VID_Y * 0.7, mouseMovementCallback));

	gui.push_back(GuiElement_t("Keyboard", VID_X * 0.5, 0, VID_X * 0.1, VID_Y * 0.1, keyboardToggleCallback));

	//SDL_ShowScreenKeyboard(NULL);
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
