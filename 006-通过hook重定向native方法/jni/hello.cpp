#include <android/log.h>
#include <stdio.h>
#include "com_demo_MainActivity.h"


#define LOG_TAG "shuwoom"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
char* getKey()
{
	return "123456";
}

}

JNIEXPORT jstring JNICALL Java_com_demo_MainActivity_test
  (JNIEnv * env, jobject obj){

	//////调用getKey方法，检测是否hook成功
	LOGI("[INFO] The key is %s\n", getKey());
  	return env->NewStringUTF(getKey());
}