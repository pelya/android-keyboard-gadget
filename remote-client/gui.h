#ifndef _XSDL_GUI_H_
#define _XSDL_GUI_H_

#include <SDL/SDL.h>
#include <vector>
#include <string>

enum { MAX_POINTERS = 8 };

extern struct TouchPointer_t { int x; int y; int pressure; int pressed; } touchPointers[MAX_POINTERS];

struct GuiElement_t;

typedef void (*GuiElementCallback_t) (GuiElement_t * elem, bool pressed, int x, int y);

struct GuiElement_t
{
	SDL_Rect rect;
	std::string text;
	GuiElementCallback_t input;
	GuiElementCallback_t draw;
	bool toggled; // Default draw function will use that to change element color.
	int x, y; // Internal state to use by callbacks.
	bool locked; // Internal state to use by callbacks.

	GuiElement_t(const char * text = "", int x = 0, int y = 0, int w = 0, int h = 0, GuiElementCallback_t input = defaultInputCallback, GuiElementCallback_t draw = defaultDrawCallback)
	{
		this->rect.x = x;
		this->rect.y = y;
		this->rect.w = w;
		this->rect.h = h;
		this->text = text;
		this->input = input;
		this->draw = draw;
		this->toggled = false;
		this->x = this->y = 0;
		this->locked = false;
	}

	static void defaultInputCallback(GuiElement_t * elem, bool pressed, int x, int y);
	static void defaultDrawCallback(GuiElement_t * elem, bool pressed, int x, int y);
};

extern std::vector<GuiElement_t> gui;

void createGuiMain();
void processGui();
void mainLoop();

void createDialog();
bool getDialogResult(int * result);
void addDialogText(const char *text);
void clearDialogText();
void addDialogUrlButton(const char *url);
void addDialogYesNoButtons();

#endif
