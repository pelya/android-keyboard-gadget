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

	createGui();

	while( true )
	{
		SDL_Event evt;
		bool noEvents = true;
		while( SDL_PollEvent(&evt) )
		{
			noEvents = false;
			if(evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN)
			{
				if(evt.key.keysym.sym == SDLK_UNDO)
					return 0;
			}
			// PC mouse events
			if(evt.type == SDL_MOUSEBUTTONUP || evt.type == SDL_MOUSEBUTTONDOWN)
			{
				// TODO: implement PC input
				touchPointers[evt.jbutton.button].pressed = (evt.button.state == SDL_PRESSED);
			}
			if(evt.type == SDL_MOUSEMOTION)
			{
				// TODO: implement PC input
				touchPointers[evt.jbutton.button].x = evt.motion.x;
				touchPointers[evt.jbutton.button].y = evt.motion.y;
			}
			// Android-specific events - accelerometer, multitoush, and on-screen joystick
			if( evt.type == SDL_JOYAXISMOTION )
			{
				if(evt.jaxis.which == 0) // Multitouch and on-screen joysticks
				{
					if(evt.jaxis.axis >= 4)
						touchPointers[evt.jaxis.axis - 4].pressure = evt.jaxis.value;
				}
			}
			if( evt.type == SDL_JOYBUTTONDOWN || evt.type == SDL_JOYBUTTONUP )
			{
				if(evt.jbutton.which == 0) // Multitouch and on-screen joystick
					touchPointers[evt.jbutton.button].pressed = (evt.jbutton.state == SDL_PRESSED);
			}
			if( evt.type == SDL_JOYBALLMOTION )
			{
				if(evt.jball.which == 0) // Multitouch and on-screen joystick
				{
					touchPointers[evt.jball.ball].x = evt.jball.xrel;
					touchPointers[evt.jball.ball].y = evt.jball.yrel;
				}
			}
		}

		processGui();
		processInput();

		SDL_Flip(SDL_GetVideoSurface());

		if( noEvents )
			SDL_Delay(100);
	}

	deinitSDL();
	return 0;
}
