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

#ifndef _XSDL_SCANCODES_H_
#define _XSDL_SCANCODES_H_

#include <SDL/SDL.h>

enum { SDL_NUM_SCANCODES = 512 };

extern const int scancodes_table[SDL_NUM_SCANCODES];

#endif
