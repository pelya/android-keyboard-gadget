#include "beginvision.h"

#include "libmp3lame/lame.h"

#define  JNIDEFINE(fname) Java_teaonly_droideye_MainActivity_##fname
extern "C" {
    JNIEXPORT jint JNICALL JNIDEFINE(nativeOpenEncoder)(JNIEnv* env, jclass clz);
    JNIEXPORT jint JNICALL JNIDEFINE(nativeEncodingPCM)(JNIEnv* env, jclass clz, jbyteArray pcmData, jint length, jbyteArray mp3Data);
    JNIEXPORT void JNICALL JNIDEFINE(nativeCloseEncoder)(JNIEnv* env, jclass clz);
};

/***********************************************
    Define module variable
************************************************/
static lame_t lame;

JNIEXPORT jint JNICALL JNIDEFINE(nativeOpenEncoder)(JNIEnv* env, jclass clz) {
    
    lame = lame_init();
    lame_set_in_samplerate(lame, 44100);
    //lame_set_num_channels(lame, 1);
    lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);
    
    return 0;
}

JNIEXPORT void JNICALL JNIDEFINE(nativeCloseEncoder)(JNIEnv* env, jclass clz) {
    lame_close(lame);
    return;
}

JNIEXPORT jint JNICALL JNIDEFINE(nativeEncodingPCM)(JNIEnv* env, jclass clz, jbyteArray pcmData, jint pcmLength, jbyteArray mp3Data) {
    jbyte *pcm, *mp3;
    
    jboolean isCopy = false;
    pcm = env->GetByteArrayElements(pcmData, &isCopy); 
    mp3 = env->GetByteArrayElements(mp3Data, &isCopy);
    int mp3Size = env->GetArrayLength(mp3Data);
    
    //int ret = lame_encode_buffer_interleaved(lame, (short *)pcm, pcmLength/2, (unsigned char *)mp3, mp3Size);
    int ret = lame_encode_buffer(lame, (short *)pcm, (short *)pcm, pcmLength/2, (unsigned char *)mp3, mp3Size);

    env->ReleaseByteArrayElements(pcmData, pcm, JNI_ABORT);    /*Don't copy to java side*/
    env->ReleaseByteArrayElements(mp3Data, mp3, 0);             /*Copy to java side*/
    return ret;
}


