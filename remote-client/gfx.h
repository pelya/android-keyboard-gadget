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

#ifndef _XSDL_GFX_H_
#define _XSDL_GFX_H_

#include <SDL/SDL.h>

#ifdef ANDROID
#include <SDL/SDL_screenkeyboard.h>
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "HID keyboard", __VA_ARGS__)
#endif

enum { VID_X = 640, VID_Y = 480, TEXT_H = 14 };

void initSDL();
void deinitSDL();
void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf = SDL_GetVideoSurface());
void renderString(const char *c, int x, int y);
void showErrorMessage(const char *msg);
void drawImageCentered(SDL_Surface * img, int x, int y, SDL_Surface * surf = SDL_GetVideoSurface());

#endif
