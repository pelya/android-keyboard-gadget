/*
 * Copyright (C) 2015 Sergii Pylypenko
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _XSDL_GUI_H_
#define _XSDL_GUI_H_

#include <SDL/SDL.h>
#include <vector>
#include <string>

enum { MAX_POINTERS = 8 };

extern struct TouchPointer_t { int x; int y; int pressure; int pressed; bool delayRelease; } touchPointers[MAX_POINTERS];

struct GuiElement_t;

typedef void (*GuiElementCallback_t) (GuiElement_t * elem, bool pressed, int x, int y);

struct GuiElement_t
{
	SDL_Rect rect;
	std::vector<std::string> text;
	GuiElementCallback_t input;
	GuiElementCallback_t draw;
	bool toggled; // Default draw function will use that to change element color.
	int x, y; // Last input coordinates.
	bool locked; // For use as a toggle-button.
	int data; // Internal state to use by callbacks.

	GuiElement_t(const char * text = "", int x = 0, int y = 0, int w = 0, int h = 0, GuiElementCallback_t input = defaultInputCallback, GuiElementCallback_t draw = defaultDrawCallback)
	{
		this->rect.x = x;
		this->rect.y = y;
		this->rect.w = w;
		this->rect.h = h;
		this->text.push_back(text);
		this->input = input;
		this->draw = draw;
		this->toggled = false;
		this->x = this->y = 0;
		this->locked = false;
		this->data = 0;
	}

	GuiElement_t(const char ** text, int x = 0, int y = 0, int w = 0, int h = 0, GuiElementCallback_t input = defaultInputCallback, GuiElementCallback_t draw = defaultDrawCallback)
	{
		this->rect.x = x;
		this->rect.y = y;
		this->rect.w = w;
		this->rect.h = h;
		for( int i = 0; text[i]; i++ )
			this->text.push_back(text[i]);
		this->input = input;
		this->draw = draw;
		this->toggled = false;
		this->x = this->y = 0;
		this->locked = false;
		this->data = 0;
	}

	static void defaultInputCallback(GuiElement_t * elem, bool pressed, int x, int y);
	static void defaultDrawCallback(GuiElement_t * elem, bool pressed, int x, int y);
	static bool toggleElement(GuiElement_t * elem, bool pressed);
};

extern std::vector<GuiElement_t> settingsGui;
extern bool guiWaitTouchRelease;

float getMouseSpeed();

void createGuiMain();
void processGui();
void mainLoop(bool noHid = false);

void createDialog();
bool getDialogResult(int * result);
void addDialogText(const char *text);
void clearDialogText();
void addDialogUrlButton(const char *url);
void addDialogYesNoButtons();

void settingsInitGui();
void settingsCloseGui();
void settingsProcessKeyInput(SDLKey key, unsigned int unicode, int pressed);
void settingsDefineKeycode(SDLKey key, unsigned int unicode);

#endif
