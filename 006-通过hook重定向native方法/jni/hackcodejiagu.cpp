#include <android/log.h>
#include <stdio.h>
#include "substrate.h"

#define LOG_TAG "shuwoom"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void* lookup_symbol(char* libraryname,char* symbolname)  
{  
    //获取so库的句柄  
    void *handle = dlopen(libraryname, RTLD_GLOBAL | RTLD_NOW);  
    if (handle != NULL){  
        LOGI("[INFO] open %s success!", libraryname);
        void* symbol = dlsym(handle, symbolname);  
        if (symbol != NULL){  
        	LOGI("[INFO] Find symbol %s:0x%x success!", symbolname, symbol);
            return symbol;  
        }else{  
            LOGI("[INFO] Find symbol %s failed!", symbolname); 
            return NULL;  
        }  
    }else{  
    	LOGI("[INFO] open %s failed!", libraryname);
        return NULL;  
    }  
}  

//原函数
char* (*getKey_old)();

//实际hook后调用的函数
char* getKey_new()
{
    LOGI("[INFO] The original function return:%s\n", getKey_old());
    LOGI("[INFO] Hooked by libhackcodejiagu.so! Redirected to getKey_new()");

    return "654321";
}

/**
//检测系统函数是否可以hook
int(*gettimeofday_f)(struct timeval*tv, struct timezone *tz);

int gettimeofday_hook(struct timeval*tv, struct timezone *tz)
{
    LOGI("hook libc function:gettimeofday success!");
    return gettimeofday_f(tv,tz);
}

void test_hook_system_function1()
{
    
    void* sym = lookup_symbol("/system/lib/libc.so","gettimeofday");  
    if(sym != NULL)
    {
        MSHookFunction((void *)sym, (void*)gettimeofday_hook, (void**)&gettimeofday_f);
    }

}
*/


/*
//检测系统函数是否可以hook
//void* mmap(void *addr, size_t size, int prot, int flags, int fd, long offset)
void* (*mmap_f)(void *addr, size_t size, int prot, int flags, int fd, long offset);

void* mmap_hook(void *addr, size_t size, int prot, int flags, int fd, long offset)
{
    LOGI("hook libc function:mmap success!");
    return mmap_f(addr, size, prot, flags, fd, offset);
}

void test_hook_system_function2()
{
    
    void* sym = lookup_symbol("/system/lib/libc.so","mmap");  
    if(sym != NULL)
    {
        MSHookFunction((void *)sym, (void*)mmap_hook, (void**)&mmap_f);
    }

}
*/


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGI("In JNI_OnLoad()...");
	///////hook getKey方法，并重定向到getKey_new方法
	void* sym = lookup_symbol("/data/data/com.demo/lib/libhello.so","getKey");  
	if(sym != NULL)
	{
		MSHookFunction((void*)sym, (void*)getKey_new, (void**)&getKey_old);
	}

    //检测系统函数是否可以hook(测试正常)
    //test_hook_system_function1();
    //test_hook_system_function2();

	return JNI_VERSION_1_4;
}

