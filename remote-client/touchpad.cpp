#include <stdlib.h>
#include <SDL/SDL.h>

#include "input.h"
#include "gfx.h"
#include "gui.h"
#include "touchpad.h"

struct TouchPointer_t oldTouchPointers[MAX_POINTERS];
bool stopLeftClick = 0;
bool stopRightClick = 0;
int pressedX = 0, pressedY = 0;
Uint32 pressedTime = 0;
enum { LEFT_CLICK_TIMEOUT = 700, LEFT_CLICK_THRESHOLD = VID_Y / 30, WHEEL_THRESHOLD = VID_Y / 20 };

void processTouchpad(int bounds_x0, int bounds_y0, int bounds_x1, int bounds_y1)
{
	if (touchPointers[0].x < bounds_x0 || touchPointers[0].x > bounds_x1 ||
		touchPointers[0].y < bounds_y0 || touchPointers[0].y > bounds_y1)
	{
		return;
	}

	if (!touchPointers[0].pressed && oldTouchPointers[0].pressed)
	{
		if (!stopLeftClick && !mouseButtons[SDL_BUTTON_LEFT])
		{
			//printf("Left button tap");
			mouseButtons[SDL_BUTTON_LEFT] = 1;
			processMouseInput();
		}
		//printf("All buttons release");
		mouseButtons[SDL_BUTTON_LEFT] = 0;
		mouseButtons[SDL_BUTTON_RIGHT] = 0;
		mouseButtons[SDL_BUTTON_MIDDLE] = 0;
	}

	if (!touchPointers[0].pressed)
	{
		stopLeftClick = 0;
		stopRightClick = 0;
		memcpy(&oldTouchPointers, touchPointers, sizeof(touchPointers));
		return;
	}

	Uint32 curTime = SDL_GetTicks();

	if (!oldTouchPointers[0].pressed)
	{
		pressedX = touchPointers[0].x;
		pressedY = touchPointers[0].y;
		pressedTime = curTime;
		oldTouchPointers[0].x = pressedX;
		oldTouchPointers[0].y = pressedY;
	}

	mouseCoords[0] += touchPointers[0].x - oldTouchPointers[0].x;
	mouseCoords[1] += touchPointers[0].y - oldTouchPointers[0].y;

	if (!mouseButtons[SDL_BUTTON_LEFT] && curTime >= pressedTime + LEFT_CLICK_TIMEOUT && !stopLeftClick)
	{
		mouseButtons[SDL_BUTTON_LEFT] = 1;
		stopLeftClick = 1;
		//printf("Left button timeout click: mouseButtons[SDL_BUTTON_LEFT] %d", mouseButtons[SDL_BUTTON_LEFT]);
	}

	if (!stopLeftClick && (abs(pressedX - touchPointers[0].x) > LEFT_CLICK_THRESHOLD || abs(pressedY - touchPointers[0].y) > LEFT_CLICK_THRESHOLD))
	{
		//printf("Left button stop threshold");
		stopLeftClick = 1;
	}

	if (!touchPointers[1].pressed && oldTouchPointers[1].pressed && !stopRightClick)
	{
		//printf("Right click");
		stopLeftClick = 1;
		pressedX = touchPointers[0].x;
		pressedY = touchPointers[0].y;
		mouseButtons[SDL_BUTTON_LEFT] = 0;
		mouseButtons[SDL_BUTTON_RIGHT] = 1;
		processMouseInput();
		mouseButtons[SDL_BUTTON_RIGHT] = 0;
	}

	if (touchPointers[2].pressed && !mouseButtons[SDL_BUTTON_MIDDLE])
	{
		//printf("Middle click");
		stopLeftClick = 1;
		stopRightClick = 1;
		mouseButtons[SDL_BUTTON_LEFT] = 0;
		mouseButtons[SDL_BUTTON_RIGHT] = 0;
		mouseButtons[SDL_BUTTON_MIDDLE] = 1;
	}

	if (!touchPointers[2].pressed && mouseButtons[SDL_BUTTON_MIDDLE])
	{
		//printf("Middle release");
		mouseButtons[SDL_BUTTON_MIDDLE] = 0;
	}

	if (touchPointers[1].pressed && (abs(pressedX - touchPointers[0].x) > WHEEL_THRESHOLD || abs(pressedY - touchPointers[0].y) > WHEEL_THRESHOLD))
	{
		stopLeftClick = 1;
		stopRightClick = 1;
		if (touchPointers[0].x > pressedX + WHEEL_THRESHOLD)
		{
			pressedX += WHEEL_THRESHOLD;
			mouseButtons[SDL_BUTTON_X1] = 1;
			processMouseInput();
			mouseButtons[SDL_BUTTON_X1] = 0;
		}
		if (touchPointers[0].x < pressedX - WHEEL_THRESHOLD)
		{
			pressedX -= WHEEL_THRESHOLD;
			mouseButtons[SDL_BUTTON_X2] = 1;
			processMouseInput();
			mouseButtons[SDL_BUTTON_X2] = 0;
		}
		if (touchPointers[0].y > pressedY + WHEEL_THRESHOLD)
		{
			pressedY += WHEEL_THRESHOLD;
			mouseButtons[SDL_BUTTON_WHEELUP] = 1;
			processMouseInput();
			mouseButtons[SDL_BUTTON_WHEELUP] = 0;
		}
		if (touchPointers[0].y < pressedY - WHEEL_THRESHOLD)
		{
			pressedY -= WHEEL_THRESHOLD;
			mouseButtons[SDL_BUTTON_WHEELDOWN] = 1;
			processMouseInput();
			mouseButtons[SDL_BUTTON_WHEELDOWN] = 0;
		}
	}

	memcpy(&oldTouchPointers, touchPointers, sizeof(touchPointers));
}
