#ifndef __CAMERA_H__
#define __CAMERA_H__

// Camera always captures in RGB565 format

typedef void (*CameraCallback_t) (void);

void openCamera(int *width, int *height, int fps, unsigned char ** buffer, int *bufferLength, CameraCallback_t callback);

void closeCamera();

#endif
