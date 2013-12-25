/*
 * This software is released under the terms of Apache license.
 * http://www.apache.org/licenses/
 */

#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include "gfx.h"
#include "gui.h"
#include "input.h"

int main(int argc, char* argv[])
{
	initSDL();
	SDL_EnableUNICODE(1);
	SDL_WM_SetCaption("Remote PC Keyboard", "Remote PC Keyboard");

	openInput();

	createGuiMain();

	while( true )
	{
		mainLoop();
	}

	deinitSDL();
	return 0;
}
