#ifndef __VNC_KEYSYMS_H__
#define __VNC_KEYSYMS_H__

#include <stdint.h>
#include <map>

#include <SDL_keysym.h>
#include <rfb/keysym.h>

extern std::map<uint32_t, SDLKey> vncKeysymToSDLKey;
extern std::map<uint32_t, uint32_t> vncKeysymToUnicode;

#endif
