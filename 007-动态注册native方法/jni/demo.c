#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>

static jstring abcdefg(JNIEnv* env){
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "call abcdefg()!!\n");
	return (*env) -> NewStringUTF(env, "native function!");
};

#define JNIREG_CLASS "com/demo/MainActivity"

static JNINativeMethod gMethods[] = 
{
	{"foo", "()Ljava/lang/String;", (void*)abcdefg}
};

static int registerNativeMethods(JNIEnv* env, const char* className, 
	JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz;
	clazz = (*env)->FindClass(env, className);
	if(clazz == NULL)
		return JNI_FALSE;

	if((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0)
		return JNI_FALSE;

	return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
	if(!registerNativeMethods(env, JNIREG_CLASS, gMethods, sizeof(gMethods)/sizeof(gMethods[0])))
		return JNI_FALSE;
	return JNI_TRUE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if( (*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_4) != JNI_OK)
		return -1;

	if(env == NULL)
		return -1;

	if(!registerNatives(env))
		return -1;

	result = JNI_VERSION_1_4;
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "pid = %d\n", getpid());
	return result;

}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved){
	JNIEnv* env = NULL;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return;
	}
	(*env) -> UnregisterNatives(env, JNIREG_CLASS);
}
