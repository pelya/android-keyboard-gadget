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

#ifndef _XSDL_INPUT_H_
#define _XSDL_INPUT_H_

#include <SDL/SDL.h>
#include <map>
#include <set>

enum KeyCode
{
	MAX_MODIFIERS = 8,
	MAX_KEYCODES  = 256 + MAX_MODIFIERS,
	KEY_LCTRL     = MAX_KEYCODES - 8,
	KEY_RCTRL     = MAX_KEYCODES - 7,
	KEY_LSHIFT    = MAX_KEYCODES - 6,
	KEY_RSHIFT    = MAX_KEYCODES - 5,
	KEY_LALT      = MAX_KEYCODES - 4,
	KEY_RALT      = MAX_KEYCODES - 3,
	KEY_LSUPER    = MAX_KEYCODES - 2,
	KEY_RSUPER    = MAX_KEYCODES - 1,
};

extern bool keys[MAX_KEYCODES];
extern float mouseCoords[2];
extern bool mouseButtons[SDL_BUTTON_X2+1];
extern std::map<int, int> keyMappings;
extern std::set<int> keyMappingsShift, keyMappingsCtrl, keyMappingsAlt;

void openInput();
void processKeyInput(SDLKey key, unsigned int unicode, int pressed);
void processMouseInput();

void saveKeyMappings();

#endif
