#include <jni.h>
#include <SDL_android.h>

#include "camera.h"
#include "input.h"
#include "gfx.h"

static JNIEnv* javaEnv = NULL;
static jclass cameraClass = NULL;
static jmethodID initCamera = NULL;
static jmethodID deinitCamera = NULL;
static int width;
static int height;
static uint8_t * cameraBuffer = NULL;
static CameraCallback_t cameraCallback = NULL;

static void toRGB565(uint8_t* yuvs, unsigned int width, unsigned int height, uint8_t* rgbs);

extern "C" void remote_hid_keyboard_client_CameraFeed_actualCameraSize(JNIEnv* env, jobject thiz, jint w, jint h)
{
	width = w;
	height = h;
}

extern "C" void remote_hid_keyboard_client_CameraFeed_pushImage(JNIEnv* env, jobject thiz, jbyteArray frame)
{
	jboolean isCopy = JNI_TRUE;
	jbyte *buffer = (jbyte *) env->GetPrimitiveArrayCritical(frame, &isCopy);
	toRGB565((uint8_t *)buffer, width, height, cameraBuffer);
	if( cameraCallback )
		cameraCallback();
	env->ReleasePrimitiveArrayCritical(frame, buffer, 0);
}

void openCamera(int *w, int *h, unsigned char ** buffer, int *bufferLength, CameraCallback_t callback)
{
	if( !javaEnv )
	{
		SDL_ANDROID_JavaVM()->AttachCurrentThread(&javaEnv, NULL);
		cameraClass = javaEnv->FindClass("remote/hid/keyboard/client/CameraFeed");
		initCamera = javaEnv->GetMethodID(cameraClass, "initCamera", "(II)V");
		deinitCamera = javaEnv->GetMethodID(cameraClass, "deinitCamera", "()V");
	}
	javaEnv->CallStaticVoidMethod(cameraClass, initCamera, (jint)*w, (jint)*h);
	*w = width;
	*h = height;
	cameraBuffer = (uint8_t *)malloc(width * height * 2);
	*buffer = cameraBuffer;
	if( bufferLength )
		*bufferLength = width * height * 2;
	cameraCallback = callback;
}

void closeCamera()
{
	if( !javaEnv )
		return;
	javaEnv->CallStaticVoidMethod(cameraClass, deinitCamera);
}

/**
 * Converts semi-planar YUV420 as generated for camera preview into RGB565
 * format for use as an OpenGL ES texture. It assumes that both the input
 * and output data are contiguous and start at zero.
 * 
 * @param yuvs the array of YUV420 semi-planar data
 * @param rgbs an array into which the RGB565 data will be written
 * @param width the number of pixels horizontally
 * @param height the number of pixels vertically
 */
//we tackle the conversion two pixels at a time for greater speed
void toRGB565(uint8_t* yuvs, unsigned int width, unsigned int height, uint8_t* rgbs) {
	//the end of the luminance data
	unsigned int lumEnd = width * height;
	//points to the next luminance value pair
	unsigned int lumPtr = 0;
	//points to the next chromiance value pair
	unsigned int chrPtr = lumEnd;
	//points to the next byte output pair of RGB565 value
	unsigned int outPtr = 0;
	//the end of the current luminance scanline
	unsigned int lineEnd = width;

	while (true) {

		//skip back to the start of the chromiance values when necessary
		if (lumPtr == lineEnd) {
			if (lumPtr == lumEnd)
				break; //we've reached the end
			//division here is a bit expensive, but's only done once per scanline
			chrPtr = lumEnd + ((lumPtr >> 1) / width) * width;
			lineEnd += width;
		}

		//read the luminance and chromiance values
		uint8_t Y1 = yuvs[lumPtr++] & 0xff;
		uint8_t Y2 = yuvs[lumPtr++] & 0xff;
		int8_t Cr = (yuvs[chrPtr++] & 0xff) - 128;
		int8_t Cb = (yuvs[chrPtr++] & 0xff) - 128;
		uint8_t R, G, B;

		//generate first RGB components
		B = Y1 + ((454 * Cb) >> 8);
		if (B < 0)
			B = 0;
		else if (B > 255)
			B = 255;
		G = Y1 - ((88 * Cb + 183 * Cr) >> 8);
		if (G < 0)
			G = 0;
		else if (G > 255)
			G = 255;
		R = Y1 + ((359 * Cr) >> 8);
		if (R < 0)
			R = 0;
		else if (R > 255)
			R = 255;
		//NOTE: this assume little-endian encoding
		rgbs[outPtr++] = (uint8_t) (((G & 0x3c) << 3) | (B >> 3));
		rgbs[outPtr++] = (uint8_t) ((R & 0xf8) | (G >> 5));

		//generate second RGB components
		B = Y2 + ((454 * Cb) >> 8);
		if (B < 0)
			B = 0;
		else if (B > 255)
			B = 255;
		G = Y2 - ((88 * Cb + 183 * Cr) >> 8);
		if (G < 0)
			G = 0;
		else if (G > 255)
			G = 255;
		R = Y2 + ((359 * Cr) >> 8);
		if (R < 0)
			R = 0;
		else if (R > 255)
			R = 255;
		//NOTE: this assume little-endian encoding
		rgbs[outPtr++] = (uint8_t) (((G & 0x3c) << 3) | (B >> 3));
		rgbs[outPtr++] = (uint8_t) ((R & 0xf8) | (G >> 5));
	}
}
