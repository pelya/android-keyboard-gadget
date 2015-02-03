#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include "gfx.h"
#include "gui.h"
#include "input.h"
#include "scancodes.h"
#include "flash-kernel.h"

bool keys[SDLK_LAST];
//bool oldkeys[SDLK_LAST];
float mouseCoords[2];
bool mouseButtons[SDL_BUTTON_X2+1];
bool oldmouseButtons[SDL_BUTTON_X2+1];

int keyboardFd = -1;
int mouseFd = -1;

static int keycode_to_scancode[SDLK_LAST];

static const char *DEV_KEYBOARD = "/dev/hidg0";
static const char *DEV_MOUSE = "/dev/hidg1";
static const char *DEV_CHECK_REPLUGGED = "/sys/devices/virtual/hidg/hidg0";

static int runSu()
{
	int pipe[2];
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, pipe) != 0)
	{
		printf("%s: socketpair() failed: %s", __func__, strerror(errno));
		return -1;
	}
	// Child will auto-free process table when terminated
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	pid_t pid = fork();
	if (pid == 0)
	{
		close(pipe[0]);
		// Redirect both stdin and stdout
		dup2(pipe[1], 0);
		dup2(pipe[1], 1);
		// We don't care about stderr
		// Terminate when stdin closes
		signal(SIGPIPE, SIG_DFL);
		execlp("su", "su", NULL);
		exit(1);
	}
	close(pipe[1]);
	return pipe[0];
}

static void openDevices()
{
	//printf("openDevices() %s %s", DEV_KEYBOARD, DEV_MOUSE);

	if (keyboardFd != -1)
		close(keyboardFd);
	if (mouseFd != -1)
		close(mouseFd);

	keyboardFd = open(DEV_KEYBOARD, O_RDWR, 0666);
	mouseFd = open(DEV_MOUSE, O_RDWR, 0666);
}

static void openDevicesSuperuser()
{
	char cmd[256];
	int count;
	//printf("openDevicesSuperuser() %s %s", DEV_KEYBOARD, DEV_MOUSE);

	if (keyboardFd != -1)
		close(keyboardFd);
	if (mouseFd != -1)
		close(mouseFd);

	keyboardFd = -1;
	mouseFd = -1;

	keyboardFd = runSu();
	if (keyboardFd == -1)
		return;

	sprintf(cmd, "ls %s && { cat %s & cat > %s ; } || echo Cannot open device ...... \n", DEV_KEYBOARD, DEV_KEYBOARD, DEV_KEYBOARD);
	write(keyboardFd, cmd, strlen(cmd));
	count = read(keyboardFd, cmd, 11);
	cmd[11] = 0;
	printf("openDevicesSuperuser(): su returned: %s", cmd);
	if (count < 0 || strstr(cmd, DEV_KEYBOARD) == NULL)
		goto errorKb;

	mouseFd = runSu();
	if (mouseFd == -1)
		goto errorKb;

	sprintf(cmd, "ls %s && { cat %s & cat > %s ; } || echo Cannot open device ...... \n", DEV_MOUSE, DEV_MOUSE, DEV_MOUSE);
	write(mouseFd, cmd, strlen(cmd));
	count = read(mouseFd, cmd, 11);
	cmd[11] = 0;
	if (count < 0 || strstr(cmd, DEV_MOUSE) == NULL)
		goto errorMouse;

	return;

	errorMouse:
	close(mouseFd);
	mouseFd = -1;

	errorKb:
	close(keyboardFd);
	keyboardFd = -1;
}

static void changeDevicePermissions()
{
	char cmd[256];
	printf("%s: $USER=%d", __func__, getuid());
	sprintf(cmd, "echo chown %d %s %s | su", getuid(), DEV_KEYBOARD, DEV_MOUSE);
	printf("%s: %s", __func__, cmd);
	system(cmd);
	sprintf(cmd, "echo chmod 600 %s %s | su", DEV_KEYBOARD, DEV_MOUSE);
	printf("%s: %s", __func__, cmd);
	system(cmd);
}

static int deviceExist(const char *path)
{
	struct stat st;

	/*
	** On Lollipop, we don't have permission to stat a device; but we know
	** it exists because errno is set to EACCES, and not ENOENT.
	*/
	return 0 == stat(path, &st) || EACCES == errno;
}

void openInput()
{
	if( !deviceExist(DEV_KEYBOARD) || !deviceExist(DEV_MOUSE) )
	{
		flashCustomKernel();
		return;
	}
	openDevices();
	if( keyboardFd == -1 || mouseFd == -1 )
	{
		changeDevicePermissions();
		openDevices();
	}
	if( keyboardFd == -1 || mouseFd == -1 )
	{
		openDevicesSuperuser();
	}
	if( (keyboardFd == -1 || mouseFd == -1) )
	{
		char cmd[256];
		createDialog();
		addDialogText("Your kernel is supported by this app");
		addDialogText("But your system is not rooted - cannot open device files");
		addDialogText("Please execute following command from the root shell, and restart this app:");
		addDialogText("");
		sprintf(cmd, "chmod 666 %s %s", DEV_KEYBOARD, DEV_MOUSE);
		addDialogText(cmd);
		addDialogText("");
		addDialogText("");
		addDialogText("Press Back to exit");
		while( true )
			mainLoop();
		exit(0);
	}
	if( keyboardFd == -1 || mouseFd == -1 )
		flashCustomKernel();
	for( int k = SDLK_FIRST; k < SDLK_LAST; k++ )
	{
		for( int s = 0; s < SDL_NUM_SCANCODES; s++ )
		{
			if( scancodes_table[s] == k )
			{
				keycode_to_scancode[k] = s;
				break;
			}
		}
	}
}

static void checkDeviceReplugged()
{
	static unsigned long last_mtime;
	static unsigned long last_mtime_nsec;
	struct stat st;
	if (stat(DEV_CHECK_REPLUGGED, &st) != 0)
		return;
	//printf("DEV_CHECK_REPLUGGED: %s %ld", DEV_CHECK_REPLUGGED, st.st_mtime);
	if (st.st_mtime != last_mtime || st.st_mtime_nsec != last_mtime_nsec)
	{
		openInput();
	}
	last_mtime = st.st_mtime;
	last_mtime_nsec = st.st_mtime_nsec;
}

static void outputSendKeys()
{
	uint8_t event[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	checkDeviceReplugged();
	if( keyboardFd == -1 || mouseFd == -1 )
		openDevices();
	if( keyboardFd == -1 || mouseFd == -1 )
		return;

	event[0] |= keys[SDLK_LCTRL] ? 0x1 : 0;
	event[0] |= keys[SDLK_RCTRL] ? 0x10 : 0;
	event[0] |= keys[SDLK_LSHIFT] ? 0x2 : 0;
	event[0] |= keys[SDLK_RSHIFT] ? 0x20 : 0;
	event[0] |= keys[SDLK_LALT] ? 0x4 : 0;
	event[0] |= keys[SDLK_RALT] ? 0x40 : 0;
	event[0] |= keys[SDLK_LSUPER] ? 0x8 : 0;
	event[0] |= keys[SDLK_RSUPER] ? 0x80 : 0;
	
	int pos = 2;
	for(int i = 1; i < SDLK_LAST; i++)
	{
		if( keys[i] && keycode_to_scancode[i] < 255 )
		{
			event[pos] = keycode_to_scancode[i];
			pos++;
			if( pos >= sizeof(event) )
				break;
		}
	}
	//printf("Send key event: %d %d %d %d %d %d %d %d", event[0], event[1], event[2], event[3], event[4], event[5], event[6], event[7]);
	if( write(keyboardFd, event, sizeof(event)) != sizeof(event))
	{
		close(keyboardFd);
		close(mouseFd);
		keyboardFd = -1;
		mouseFd = -1;
	}
}

static void outputSendMouse(int x, int y, int b1, int b2, int b3, int wheel, int b6, int b7)
{
	uint8_t event[4] = {0, 0, 0, 0};

	checkDeviceReplugged();
	if( keyboardFd == -1 || mouseFd == -1 )
		openDevices();
	if( keyboardFd == -1 || mouseFd == -1 )
		return;

	//printf("outputSendMouse: %d %d b %d %d %d %d", x, y, b1, b2, b3, wheel);

	event[0] |= b1 ? 1 : 0;
	event[0] |= b2 ? 2 : 0;
	event[0] |= b3 ? 4 : 0;
	event[0] |= b6 ? 32 : 0;
	event[0] |= b7 ? 64 : 0;
	event[1] = (x > SCHAR_MAX) ? SCHAR_MAX : (x < SCHAR_MIN + 1) ? SCHAR_MIN + 1 : x;
	event[2] = (y > SCHAR_MAX) ? SCHAR_MAX : (y < SCHAR_MIN + 1) ? SCHAR_MIN + 1 : y;
	event[3] = (wheel >= 0) ? wheel : UCHAR_MAX + 1 + wheel;
	if( write(mouseFd, event, sizeof(event)) != sizeof(event))
	{
		close(keyboardFd);
		close(mouseFd);
		keyboardFd = -1;
		mouseFd = -1;
	}
}

void processKeyInput(SDLKey key, int pressed)
{
	if( keys[key] == pressed )
	{
		//printf("processKeyInput: duplicate event %d %d", key, pressed);
		return;
	}
	//printf("processKeyInput: %d %d", key, pressed);
	keys[key] = pressed;
	outputSendKeys();
	//oldkeys[key] = keys[key];
}

void processMouseInput()
{
	int coords[2] = { (int)mouseCoords[0], (int)mouseCoords[1] };
	if( coords[0] != 0 || coords[1] != 0 ||
		memcmp(oldmouseButtons, mouseButtons, sizeof(mouseButtons)) )
	{
		outputSendMouse(coords[0], coords[1],
						mouseButtons[SDL_BUTTON_LEFT], mouseButtons[SDL_BUTTON_RIGHT], mouseButtons[SDL_BUTTON_MIDDLE],
						(mouseButtons[SDL_BUTTON_WHEELUP] != oldmouseButtons[SDL_BUTTON_WHEELUP] && mouseButtons[SDL_BUTTON_WHEELUP]) ? 1 :
						(mouseButtons[SDL_BUTTON_WHEELDOWN] != oldmouseButtons[SDL_BUTTON_WHEELDOWN] && mouseButtons[SDL_BUTTON_WHEELDOWN]) ? -1 : 0,
						mouseButtons[SDL_BUTTON_X1], mouseButtons[SDL_BUTTON_X2]);
		mouseCoords[0] -= coords[0];
		mouseCoords[1] -= coords[1];
	}
	memcpy(oldmouseButtons, mouseButtons, sizeof(mouseButtons));
}
