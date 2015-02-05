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

#include <string.h>
#include <SDL/SDL_image.h>

#include "gfx.h"
#include "gui.h"
#include "input.h"
#include "touchpad.h"
#include "tools.h"

std::vector<GuiElement_t> settingsGui;

static const float SIZE_X = VID_X / 14.0, SIZE_Y = SIZE_X * 1.5;

static int sCurrentScancode = 0;
static std::string sCurrentKeyname = "Unknown";
static bool sShowExtendedKeys = false;
static int sDefineUnknownKeycode = 0;

static void showScancodeDialog(int scancode);

static void closeScancodeDialog(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( pressed )
	{
		sCurrentScancode = 0;
		settingsInitGui();
	}
}

static void deleteScancode(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( pressed )
	{
		int keycode = elem->data;
		if( keycode == 0 )
			return;
		if( keyMappings.find(keycode) != keyMappings.end() )
			keyMappings.erase(keycode);
		saveKeyMappings();
		showScancodeDialog(sCurrentScancode);
	}
}

static void keyboardToggleCallback(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( GuiElement_t::toggleElement(elem, pressed) )
		SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput();
	GuiElement_t::defaultInputCallback(elem, pressed, x, y);
}

template< std::set<int> * keyset >
static void toggleModifierKey(GuiElement_t * elem, bool pressed, int x, int y)
{
	GuiElement_t::toggleElement(elem, pressed);
	if( elem->toggled && keyset->count(elem->data) == 0 )
	{
		keyset->insert(elem->data);
		saveKeyMappings();
	}
	if( !elem->toggled && keyset->count(elem->data) > 0 )
	{
		keyset->erase(elem->data);
		saveKeyMappings();
	}
}

void showScancodeDialog(int scancode)
{
	char s[512];
	sCurrentScancode = scancode;
	settingsGui.clear();
	guiWaitTouchRelease = true;
	sprintf(s, "Press a key for button %s with scancode %d", sCurrentKeyname.c_str(), scancode);
	settingsGui.push_back(GuiElement_t(s, VID_X * 0.1, VID_Y * 0.0, VID_X * 0.8, VID_Y * 0.1));
	settingsGui.push_back(GuiElement_t("Toggle keyboard", VID_X * 0.1, VID_Y * 0.1, VID_X * 0.4, VID_Y * 0.1, keyboardToggleCallback));
	settingsGui.push_back(GuiElement_t("Close", VID_X * 0.5, VID_Y * 0.1, VID_X * 0.4, VID_Y * 0.1, closeScancodeDialog));
	float y = VID_Y * 0.25;
	for( std::map<int, int>::const_iterator it = keyMappings.begin(); it != keyMappings.end(); it++ )
	{
		if( it->second != scancode )
			continue;
		int key = it->first;
		char keyname[64] = "unknown key";
		if( key < 0 && SDL_GetKeyName((SDLKey) -key) != NULL )
			strcpy(keyname, SDL_GetKeyName((SDLKey) -key));
		else
			UnicodeToUtf8(key, keyname);
		if( strcmp(keyname, "unknown key") == 0 && key < 0 )
		{
			keyname[0] = -key;
			keyname[1] = 0;
		}
		sprintf(s, "Key %s %s %d", keyname, key > 0 ? "unicode" : "keycode", abs(key));
		settingsGui.push_back(GuiElement_t(s, VID_X * 0.1, y, VID_X * 0.4, VID_Y * 0.1));
		settingsGui.back().data = key;
		settingsGui.push_back(GuiElement_t("Shift", VID_X * 0.5, y, VID_X * 0.1, VID_Y * 0.1, toggleModifierKey<&keyMappingsShift>));
		settingsGui.back().data = key;
		settingsGui.back().toggled = keyMappingsShift.count(key) > 0;
		settingsGui.push_back(GuiElement_t("Ctrl", VID_X * 0.6, y, VID_X * 0.1, VID_Y * 0.1, toggleModifierKey<&keyMappingsCtrl>));
		settingsGui.back().data = key;
		settingsGui.back().toggled = keyMappingsCtrl.count(key) > 0;
		settingsGui.push_back(GuiElement_t("Alt", VID_X * 0.7, y, VID_X * 0.1, VID_Y * 0.1, toggleModifierKey<&keyMappingsAlt>));
		settingsGui.back().data = key;
		settingsGui.back().toggled = keyMappingsAlt.count(key) > 0;
		settingsGui.push_back(GuiElement_t("Remove", VID_X * 0.8, y, VID_X * 0.1, VID_Y * 0.1, deleteScancode));
		settingsGui.back().data = key;
		y += VID_Y * 0.1;
	}
}

template<int scancode>
static void keyDefine(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( GuiElement_t::toggleElement(elem, pressed) )
	{
		sCurrentKeyname = elem->text.size() > 0 ? elem->text[0] : "Unknown";
		if( sDefineUnknownKeycode )
		{
			keyMappings[sDefineUnknownKeycode] = scancode;
			saveKeyMappings();
			sDefineUnknownKeycode = 0;
		}
		showScancodeDialog(scancode);
	}
}

void settingsProcessKeyInput(SDLKey sdlkey, unsigned int unicode, int pressed)
{
	if( sCurrentScancode != 0 && pressed )
	{
		int code = -int(sdlkey);
		if( unicode != 0 )
			code = unicode;
		keyMappings[code] = sCurrentScancode;
		saveKeyMappings();
		showScancodeDialog(sCurrentScancode);
	}
}

void settingsDefineKeycode(SDLKey key, unsigned int unicode)
{
	sCurrentScancode = 0;
	sDefineUnknownKeycode = -int(key);
	if( unicode != 0 )
		sDefineUnknownKeycode = unicode;
	settingsInitGui();
}

static void settingsInitGuiMainKeys()
{
	const char *tilde[] = {"~", "`", NULL};
	settingsGui.push_back(GuiElement_t(tilde,   SIZE_X * 0,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x35>));
	settingsGui.push_back(GuiElement_t("1",     SIZE_X * 1,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x1e>));
	settingsGui.push_back(GuiElement_t("2",     SIZE_X * 2,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x1f>));
	settingsGui.push_back(GuiElement_t("3",     SIZE_X * 3,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x20>));
	settingsGui.push_back(GuiElement_t("4",     SIZE_X * 4,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x21>));
	settingsGui.push_back(GuiElement_t("5",     SIZE_X * 5,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x22>));
	settingsGui.push_back(GuiElement_t("6",     SIZE_X * 6,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x23>));
	settingsGui.push_back(GuiElement_t("7",     SIZE_X * 7,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x24>));
	settingsGui.push_back(GuiElement_t("8",     SIZE_X * 8,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x25>));
	settingsGui.push_back(GuiElement_t("9",     SIZE_X * 9,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x26>));
	settingsGui.push_back(GuiElement_t("0",     SIZE_X * 10, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x27>));
	const char *minus[] = {"_", "-", NULL};
	settingsGui.push_back(GuiElement_t(minus,   SIZE_X * 11, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x2d>));
	const char *plus[] = {"+", "=", NULL};
	settingsGui.push_back(GuiElement_t(plus,    SIZE_X * 12, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x2e>));
	settingsGui.push_back(GuiElement_t("<--",   SIZE_X * 13, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x2a>));

	settingsGui.push_back(GuiElement_t("Tab",   SIZE_X * 0,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x2b>));
	settingsGui.push_back(GuiElement_t("Q",     SIZE_X * 1,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x14>));
	settingsGui.push_back(GuiElement_t("W",     SIZE_X * 2,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x1a>));
	settingsGui.push_back(GuiElement_t("E",     SIZE_X * 3,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x08>));
	settingsGui.push_back(GuiElement_t("R",     SIZE_X * 4,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x15>));
	settingsGui.push_back(GuiElement_t("T",     SIZE_X * 5,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x17>));
	settingsGui.push_back(GuiElement_t("Y",     SIZE_X * 6,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x1c>));
	settingsGui.push_back(GuiElement_t("U",     SIZE_X * 7,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x18>));
	settingsGui.push_back(GuiElement_t("I",     SIZE_X * 8,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x0c>));
	settingsGui.push_back(GuiElement_t("O",     SIZE_X * 9,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x12>));
	settingsGui.push_back(GuiElement_t("P",     SIZE_X * 10, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x13>));
	const char *lbrace[] = {"{", "[", NULL};
	settingsGui.push_back(GuiElement_t(lbrace,  SIZE_X * 11, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x2f>));
	const char *rbrace[] = {"}", "]", NULL};
	settingsGui.push_back(GuiElement_t(rbrace,  SIZE_X * 12, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x30>));
	const char *backslash[] = {"|", "\\", NULL};
	settingsGui.push_back(GuiElement_t(backslash,SIZE_X * 13, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x31>));

	const char *caps[] = {"Caps", "Lock", NULL};
	settingsGui.push_back(GuiElement_t(caps,    SIZE_X * 0,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x39>));
	settingsGui.push_back(GuiElement_t("A",     SIZE_X * 1,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x04>));
	settingsGui.push_back(GuiElement_t("S",     SIZE_X * 2,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x16>));
	settingsGui.push_back(GuiElement_t("D",     SIZE_X * 3,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x07>));
	settingsGui.push_back(GuiElement_t("F",     SIZE_X * 4,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x09>));
	settingsGui.push_back(GuiElement_t("G",     SIZE_X * 5,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x0a>));
	settingsGui.push_back(GuiElement_t("H",     SIZE_X * 6,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x0b>));
	settingsGui.push_back(GuiElement_t("J",     SIZE_X * 7,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x0d>));
	settingsGui.push_back(GuiElement_t("K",     SIZE_X * 8,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x0e>));
	settingsGui.push_back(GuiElement_t("L",     SIZE_X * 9,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x0f>));
	const char *semicol[] = {":", ";", NULL};
	settingsGui.push_back(GuiElement_t(semicol, SIZE_X * 10, SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x33>));
	const char *quote[] = {"'", "\"", NULL};
	settingsGui.push_back(GuiElement_t(quote,   SIZE_X * 11, SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x34>));
	settingsGui.push_back(GuiElement_t("Enter", SIZE_X * 12, SIZE_Y * 4, SIZE_X * 2, SIZE_Y, keyDefine<0x28>));

	settingsGui.push_back(GuiElement_t("Shift", SIZE_X * 0,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<KEY_LSHIFT>));
	settingsGui.push_back(GuiElement_t("Z",     SIZE_X * 1,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x1d>));
	settingsGui.push_back(GuiElement_t("X",     SIZE_X * 2,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x1b>));
	settingsGui.push_back(GuiElement_t("C",     SIZE_X * 3,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x06>));
	settingsGui.push_back(GuiElement_t("V",     SIZE_X * 4,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x19>));
	settingsGui.push_back(GuiElement_t("B",     SIZE_X * 5,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x05>));
	settingsGui.push_back(GuiElement_t("N",     SIZE_X * 6,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x11>));
	settingsGui.push_back(GuiElement_t("M",     SIZE_X * 7,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x10>));
	const char *comma[] = {"<", ",", NULL};
	settingsGui.push_back(GuiElement_t(comma,   SIZE_X * 8,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x36>));
	const char *dot[] = {">", ".", NULL};
	settingsGui.push_back(GuiElement_t(dot,     SIZE_X * 9,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x36>));
	const char *slash[] = {"?", "/", NULL};
	settingsGui.push_back(GuiElement_t(slash,   SIZE_X * 10, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x37>));
	settingsGui.push_back(GuiElement_t("Shift", SIZE_X * 11, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<KEY_RSHIFT>));
	settingsGui.push_back(GuiElement_t("Up",    SIZE_X * 12, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x52>));
	const char *nokey[] = {"No", "key", NULL};
	settingsGui.push_back(GuiElement_t(nokey,   SIZE_X * 13, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x00>));

	settingsGui.push_back(GuiElement_t("Ctrl",  SIZE_X * 0,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<KEY_LCTRL>));
	settingsGui.push_back(GuiElement_t("Meta",  SIZE_X * 1,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<KEY_LSUPER>));
	settingsGui.push_back(GuiElement_t("Alt",   SIZE_X * 2,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<KEY_LALT>));
	settingsGui.push_back(GuiElement_t("Space", SIZE_X * 3,  SIZE_Y * 6, SIZE_X * 4, SIZE_Y, keyDefine<0x2c>));
	settingsGui.push_back(GuiElement_t("Alt",   SIZE_X * 7,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<KEY_RALT>));
	settingsGui.push_back(GuiElement_t("Meta",  SIZE_X * 8,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<KEY_RSUPER>));
	settingsGui.push_back(GuiElement_t("Menu",  SIZE_X * 9,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x76>));
	settingsGui.push_back(GuiElement_t("Ctrl",  SIZE_X * 10, SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<KEY_RCTRL>));
	settingsGui.push_back(GuiElement_t("Left",  SIZE_X * 11, SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x50>));
	settingsGui.push_back(GuiElement_t("Down",  SIZE_X * 12, SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x51>));
	settingsGui.push_back(GuiElement_t("Right", SIZE_X * 13, SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x4f>));
}

static void settingsInitGuiExtendedKeys()
{
	settingsGui.push_back(GuiElement_t("F1",    SIZE_X * 1,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x3a>));
	settingsGui.push_back(GuiElement_t("F2",    SIZE_X * 2,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x3b>));
	settingsGui.push_back(GuiElement_t("F3",    SIZE_X * 3,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x3c>));
	settingsGui.push_back(GuiElement_t("F4",    SIZE_X * 4,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x3d>));
	settingsGui.push_back(GuiElement_t("F5",    SIZE_X * 5,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x3e>));
	settingsGui.push_back(GuiElement_t("F6",    SIZE_X * 6,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x3f>));
	settingsGui.push_back(GuiElement_t("F7",    SIZE_X * 7,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x40>));
	settingsGui.push_back(GuiElement_t("F8",    SIZE_X * 8,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x41>));

	settingsGui.push_back(GuiElement_t("F9",    SIZE_X * 1,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x42>));
	settingsGui.push_back(GuiElement_t("F10",   SIZE_X * 2,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x43>));
	settingsGui.push_back(GuiElement_t("F11",   SIZE_X * 3,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x44>));
	settingsGui.push_back(GuiElement_t("F12",   SIZE_X * 4,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x45>));

	settingsGui.push_back(GuiElement_t("F13",   SIZE_X * 1,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x68>));
	settingsGui.push_back(GuiElement_t("F14",   SIZE_X * 2,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x69>));
	settingsGui.push_back(GuiElement_t("F15",   SIZE_X * 3,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x6a>));
	settingsGui.push_back(GuiElement_t("F16",   SIZE_X * 4,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x6b>));

	settingsGui.push_back(GuiElement_t("F17",   SIZE_X * 1,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x6c>));
	settingsGui.push_back(GuiElement_t("F18",   SIZE_X * 2,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x6d>));
	settingsGui.push_back(GuiElement_t("F19",   SIZE_X * 3,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x6e>));
	settingsGui.push_back(GuiElement_t("F20",   SIZE_X * 4,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x6f>));

	settingsGui.push_back(GuiElement_t("F21",   SIZE_X * 1,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x70>));
	settingsGui.push_back(GuiElement_t("F22",   SIZE_X * 2,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x71>));
	settingsGui.push_back(GuiElement_t("F23",   SIZE_X * 3,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x72>));
	settingsGui.push_back(GuiElement_t("F24",   SIZE_X * 4,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x73>));

	const char *numlock[] = {"Num", "Lock", NULL};
	settingsGui.push_back(GuiElement_t(numlock, SIZE_X * 10, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x53>));
	settingsGui.push_back(GuiElement_t("/",     SIZE_X * 11, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x54>));
	settingsGui.push_back(GuiElement_t("*",     SIZE_X * 12, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x55>));
	settingsGui.push_back(GuiElement_t("-",     SIZE_X * 13, SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x56>));

	settingsGui.push_back(GuiElement_t("7",     SIZE_X * 10, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x5f>));
	settingsGui.push_back(GuiElement_t("8",     SIZE_X * 11, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x60>));
	settingsGui.push_back(GuiElement_t("9",     SIZE_X * 12, SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x61>));
	settingsGui.push_back(GuiElement_t("+",     SIZE_X * 13, SIZE_Y * 3, SIZE_X, SIZE_Y * 2, keyDefine<0x57>));

	settingsGui.push_back(GuiElement_t("4",     SIZE_X * 10, SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x5c>));
	settingsGui.push_back(GuiElement_t("5",     SIZE_X * 11, SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x5d>));
	settingsGui.push_back(GuiElement_t("6",     SIZE_X * 12, SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x5e>));

	settingsGui.push_back(GuiElement_t("1",     SIZE_X * 10, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x59>));
	settingsGui.push_back(GuiElement_t("2",     SIZE_X * 11, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x5a>));
	settingsGui.push_back(GuiElement_t("3",     SIZE_X * 12, SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x5b>));
	settingsGui.push_back(GuiElement_t("Enter", SIZE_X * 13, SIZE_Y * 5, SIZE_X, SIZE_Y * 2, keyDefine<0x58>));

	settingsGui.push_back(GuiElement_t("0",     SIZE_X * 10, SIZE_Y * 6, SIZE_X * 2, SIZE_Y, keyDefine<0x62>));
	settingsGui.push_back(GuiElement_t(".",     SIZE_X * 12, SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x63>));

	settingsGui.push_back(GuiElement_t("Esc",   SIZE_X * 0,  SIZE_Y * 2, SIZE_X, SIZE_Y, keyDefine<0x29>));

	const char *print[] = {"Print", "SysRq", NULL};
	settingsGui.push_back(GuiElement_t(print,   SIZE_X * 6,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x46>));
	const char *scrlock[] = {"Scroll", "Lock", NULL};
	settingsGui.push_back(GuiElement_t(scrlock, SIZE_X * 7,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x47>));
	const char *pause[] = {"Pause", "Break", NULL};
	settingsGui.push_back(GuiElement_t(pause,   SIZE_X * 8,  SIZE_Y * 3, SIZE_X, SIZE_Y, keyDefine<0x48>));

	settingsGui.push_back(GuiElement_t("Insert",SIZE_X * 6,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x49>));
	settingsGui.push_back(GuiElement_t("Home",  SIZE_X * 7,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x4a>));
	settingsGui.push_back(GuiElement_t("PgUp",  SIZE_X * 8,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x4b>));

	settingsGui.push_back(GuiElement_t("Delete",SIZE_X * 6,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x4c>));
	settingsGui.push_back(GuiElement_t("End",   SIZE_X * 7,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x4d>));
	settingsGui.push_back(GuiElement_t("PgDn",  SIZE_X * 8,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x4e>));

	settingsGui.push_back(GuiElement_t("£",     SIZE_X * 0,  SIZE_Y * 4, SIZE_X, SIZE_Y, keyDefine<0x32>));
	settingsGui.push_back(GuiElement_t("/",     SIZE_X * 0,  SIZE_Y * 5, SIZE_X, SIZE_Y, keyDefine<0x38>));
	const char *nokey[] = {"No", "key", NULL};
	settingsGui.push_back(GuiElement_t(nokey,   SIZE_X * 0,  SIZE_Y * 6, SIZE_X, SIZE_Y, keyDefine<0x00>));

	settingsGui.push_back(GuiElement_t("App",   SIZE_X * 0,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x65>));
	settingsGui.push_back(GuiElement_t("Power", SIZE_X * 1,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x66>));
	settingsGui.push_back(GuiElement_t("Exec",  SIZE_X * 2,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x74>));
	settingsGui.push_back(GuiElement_t("Help",  SIZE_X * 3,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x75>));
	settingsGui.push_back(GuiElement_t("Select",SIZE_X * 4,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x77>));
	settingsGui.push_back(GuiElement_t("Cancel",SIZE_X * 5,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x78>));
	settingsGui.push_back(GuiElement_t("Redo",  SIZE_X * 6,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x79>));
	settingsGui.push_back(GuiElement_t("Undo",  SIZE_X * 7,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x7a>));
	settingsGui.push_back(GuiElement_t("Cut",   SIZE_X * 8,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x7b>));
	settingsGui.push_back(GuiElement_t("Copy",  SIZE_X * 9,  SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x7c>));
	settingsGui.push_back(GuiElement_t("Paste", SIZE_X * 10, SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x7d>));
	settingsGui.push_back(GuiElement_t("Find",  SIZE_X * 11, SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x7e>));
	settingsGui.push_back(GuiElement_t("Mute",  SIZE_X * 12, SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x7f>));
	settingsGui.push_back(GuiElement_t("=",     SIZE_X * 13, SIZE_Y * 1, SIZE_X, SIZE_Y, keyDefine<0x67>));
}

static void keyboardShowMainKeys(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( GuiElement_t::toggleElement(elem, pressed) )
	{
		sShowExtendedKeys = false;
		settingsInitGui();
	}
}

static void keyboardShowExtendedKeys(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( GuiElement_t::toggleElement(elem, pressed) )
	{
		sShowExtendedKeys = true;
		settingsInitGui();
	}
}

static void settingsClose(GuiElement_t * elem, bool pressed, int x, int y)
{
	if( GuiElement_t::toggleElement(elem, pressed) )
	{
		settingsCloseGui();
		guiWaitTouchRelease = true;
	}
}

void settingsInitGui()
{
	settingsGui.clear();
	guiWaitTouchRelease = true;

	char s[128] = "Redefine keys";
	if( sDefineUnknownKeycode )
	{
		char keyname[64] = "Unknown";
		if( sDefineUnknownKeycode < 0 && SDL_GetKeyName((SDLKey) -sDefineUnknownKeycode) != NULL )
			strcpy(keyname, SDL_GetKeyName((SDLKey) -sDefineUnknownKeycode));
		else
			UnicodeToUtf8(sDefineUnknownKeycode, keyname);
		sprintf(s, "Redefine key %s %s %d", keyname, sDefineUnknownKeycode > 0 ? "unicode" : "keycode", abs(sDefineUnknownKeycode));
	}
	settingsGui.push_back(GuiElement_t(s, 0, 0, VID_X * 0.4, SIZE_Y));
	settingsGui.push_back(GuiElement_t("Main keys", VID_X * 0.4, 0, VID_X * 0.2, SIZE_Y, keyboardShowMainKeys));
	settingsGui.back().toggled = !sShowExtendedKeys;
	settingsGui.push_back(GuiElement_t("Extended keys", VID_X * 0.6, 0, VID_X * 0.2, SIZE_Y, keyboardShowExtendedKeys));
	settingsGui.back().toggled = sShowExtendedKeys;
	settingsGui.push_back(GuiElement_t("Close", VID_X * 0.8, 0, VID_X * 0.2, SIZE_Y, settingsClose));

	if( sShowExtendedKeys )
		settingsInitGuiExtendedKeys();
	else
		settingsInitGuiMainKeys();
}
