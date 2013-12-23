#ifndef _XSDL_INPUT_H_
#define _XSDL_INPUT_H_

extern bool keys[SDLK_LAST];
extern int mouseCoords[2];
extern bool mouseButtons[SDL_BUTTON_WHEELDOWN+1];

void openInput();
void processInput();

#endif
