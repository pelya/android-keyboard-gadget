#ifndef _XSDL_GFX_H_
#define _XSDL_GFX_H_

#include <SDL/SDL.h>

#ifdef ANDROID
#include <SDL/SDL_screenkeyboard.h>
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "HID keyboard", __VA_ARGS__)
#endif

enum { VID_X = 640, VID_Y = 480 };

void initSDL();
void deinitSDL();
void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf = SDL_GetVideoSurface());
void renderString(const char *c, int x, int y);
void showErrorMessage(const char *msg);
void drawImageCentered(SDL_Surface * img, int x, int y, SDL_Surface * surf = SDL_GetVideoSurface());

#endif
