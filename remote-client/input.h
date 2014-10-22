#ifndef _XSDL_INPUT_H_
#define _XSDL_INPUT_H_

#include <SDL/SDL.h>

extern bool keys[SDLK_LAST];
extern float mouseCoords[2];
extern bool mouseButtons[SDL_BUTTON_X2+1];

void openInput();
void processKeyInput(SDLKey key, int pressed);
void processMouseInput();


#endif
