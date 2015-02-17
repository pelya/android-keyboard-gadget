#ifndef __VNC_SERVER_H__
#define __VNC_SERVER_H__

#include <string>

void vncServerStart();
void vncServerStop();
bool vncServerRunning();
std::string vncServerGetIpAddress();
void vncServerDrawVideoBuffer(int x, int y, int w, int h);

#endif
