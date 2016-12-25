#include "com_demo_MainActivity.h"
#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>


#define LOG_TAG "AndroidNDK"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


jstring getString(JNIEnv*) __attribute__((section(".mytext")));
jstring getString(JNIEnv* env){
	return env->NewStringUTF("Native method return!");
}

/**
  __attribute__((constructor)表示init_security()函数放在.init section。
  .init相当于初始化，比JNI_ONLOAD早执行。
  所以在这里要对加密的section进行解密操作
**/
void init_security() __attribute__((constructor));

unsigned long getLibAddress(char *name);
void decryptSection(char* section_addr, unsigned int length);
// void printELFHeader(Elf32_Ehdr* ehdr);

void init_security(){
	LOGI("----------------init_security()---------------------");
	//TODO 解密
	char name[] = "libdemo.so";
  	Elf32_Ehdr *ehdr;
  	char *target_section_addr;
  	Elf32_Phdr *phdr;
  	unsigned long base;
  	int i = 0;
  	unsigned int length, page_size;

  	//Step1:首先获取目标so在内存中的起始地址
  	base = getLibAddress(name);
  	if(base != 0){
  		LOGI("%s address=0x%x", name, base);
  		ehdr = (Elf32_Ehdr*)base;

  		//Step2:获取加密section的内存地址、section大小和section占用的页大小
  		target_section_addr = (char*)(base + ehdr->e_shoff);
  		length = ehdr->e_entry >> 16;
  		page_size = ehdr->e_entry & 0xffff;

  		LOGI("e_entry=%x\n", ehdr->e_entry);
  		LOGI("text_addr=0x%x，0x%x\n", ehdr->e_shoff, target_section_addr);
  		LOGI("section size=%d, page size=%d\n", length, page_size);

  		//解密前的section
  // 		for(i = 0; i < length; i++){   
		//     char *addr = (char*)(target_section_addr + i);
  //   		LOGI("[#%d]%x=>%x", i, addr, *addr);
		// }

		//Step3:修改目标section所在内存区域为可写
  		if(mprotect((void*)base, 4096 * page_size, PROT_READ | PROT_EXEC | PROT_WRITE) != 0){
  			LOGI("mprotect 1 error...");
  		}

  		//Step4:对目标section解密
  		decryptSection(target_section_addr, length);
  		
  		//Step4:回复目标section所在区域为不可写
		if(mprotect((void*)base, 4096 * page_size, PROT_READ | PROT_EXEC) != 0){
  			LOGI("mprotect 2 error...");
  		}

  	}
  	LOGI("----------------init_security() Done---------------------");
}

void decryptSection(char* section_addr, unsigned int length){
	int i;
	for(i = 0; i < length; i++){   
	    char *addr = (char*)(section_addr + i);
		*addr = ~(*addr);
		// LOGI("[#%d]%x=>%x", i, addr, *addr);
	}
}

//获取so加载到内存中的起始地址
unsigned long getLibAddress(char *name){
	unsigned long ret = 0;
	char buf[4096], *temp;
	int pid;
	FILE *fp;
	pid = getpid();
	sprintf(buf, "/proc/%d/maps", pid);
	LOGI("libdemo.so pid=%d", pid);
	fp = fopen(buf, "r");
	if(fp == NULL){
		LOGI("open failed!");
		fclose(fp);
		return ret;
	}

	while(fgets(buf, sizeof(buf), fp)){
		if(strstr(buf, name)){
			temp = strtok(buf, "-");
			ret = strtoul(temp, NULL, 16);
			break;
		}
	}
	fclose(fp);
	return ret;
}

// void printELFHeader(Elf32_Ehdr* ehdr){
// 	LOGI("=================== ELF Header ===================");
// 	LOGI("In memorry, the address of ELF Header is: 0x%x", ehdr);
// 	LOGI("e_ident=\t%s", ehdr->e_ident);
// 	LOGI("e_type=\t%d", ehdr->e_type);
// 	LOGI("e_machine=\t%d", ehdr->e_machine);
// 	LOGI("e_version=\t%d", ehdr->e_version);
// 	LOGI("e_entry=\t0x%x", ehdr->e_entry);
// 	LOGI("e_phoff=\t0x%x", ehdr->e_phoff);
// 	LOGI("e_shoff=\t0x%x", ehdr->e_shoff);
// 	LOGI("e_flags=\t%d", ehdr->e_flags);
// 	LOGI("e_ehsize=\t%d", ehdr->e_ehsize);
// 	LOGI("e_phentsize=\t%d", ehdr->e_phentsize);
// 	LOGI("e_phnum=\t%d", ehdr->e_phnum);
// 	LOGI("e_shentsize=\t%d", ehdr->e_shentsize);
// 	LOGI("e_shnum=\t%d", ehdr->e_shnum);
// 	LOGI("e_shstrndx=\t%d", ehdr->e_shstrndx);
// }



JNIEXPORT jstring JNICALL Java_com_demo_MainActivity_getText
  (JNIEnv * env, jobject obj){ 	
  	return getString(env);
}