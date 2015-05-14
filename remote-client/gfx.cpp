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

#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "gfx.h"

static TTF_Font* sFont;

void showErrorMessage(const char *msg)
{
	SDL_Event event;
	const char * s;
	int y = VID_Y/4;
	printf("Error: %s", msg);
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
	for( s = msg; s && s[0]; s = strchr(s, '\n'), s += (s ? 1 : 0), y += 35 )
	{
		const char * s1 = strchr(s, '\n');
		int len = s1 ? s1 - s : strlen(s);
		char buf[512];
		strncpy(buf, s, len);
		buf[len] = 0;
		if( len > 0 )
			renderString(buf, VID_X/2, y);
	}
	SDL_Flip(SDL_GetVideoSurface());
	while (1)
	{
		while (SDL_WaitEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UNDO || event.key.keysym.sym == SDLK_ESCAPE)
					exit(1);
				//break;
			}
		}
	}
}

void initSDL()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

#ifdef __ANDROID__
	if( SDL_ListModes(NULL, 0)[0]->w > SDL_ListModes(NULL, 0)[0]->h )
		SDL_SetVideoMode(VID_X, VID_Y, 16, SDL_SWSURFACE);
	else
		SDL_SetVideoMode(VID_Y, VID_X, 16, SDL_SWSURFACE);
#else
	SDL_SetVideoMode(VID_X, VID_Y, 16, SDL_SWSURFACE);
#endif
	SDL_EnableUNICODE(1);
	TTF_Init();
	sFont = TTF_OpenFont("Roboto-Regular.ttf", TEXT_H);
	if (!sFont)
	{
		printf("Error: cannot open font file, please reinstall the app");
		exit(1);
	}
	SDL_JoystickOpen(0);

	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
	renderString("Opening /dev/hidg0 and /dev/hidg1", VID_X / 2, VID_Y / 2);
	SDL_Flip(SDL_GetVideoSurface());
}

void deinitSDL()
{
	TTF_CloseFont(sFont);
	sFont = NULL;
	TTF_Quit();
	SDL_Quit();
}

void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf)
{
	if( strcmp(c, "") == 0 )
		return;
	SDL_Color fColor = {r, g, b};
	SDL_Rect fontRect = {0, 0, 0, 0};
	SDL_Surface* fontSurface = TTF_RenderUTF8_Blended(sFont, c, fColor);
	fontRect.w = fontSurface->w;
	fontRect.h = fontSurface->h;
	fontRect.x = x - fontRect.w / 2;
	fontRect.y = y - fontRect.h / 2;
	SDL_BlitSurface(fontSurface, NULL, surf, &fontRect);
	SDL_FreeSurface(fontSurface);
}

void renderString(const char *c, int x, int y)
{
	renderStringColor(c, x, y, 255, 255, 255, SDL_GetVideoSurface());
}

void drawImageCentered(SDL_Surface * img, int x, int y, SDL_Surface * surf)
{
	SDL_Rect r;
	r.x = x - img->w / 2;
	r.y = y - img->h / 2;
	r.w = img->w;
	r.h = img->h;
	SDL_BlitSurface(img, NULL, surf, &r);
}
