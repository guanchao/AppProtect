#include "com_demo_MainActivity.h"
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>
#include "MiniDexFile.h"

#define LOG_TAG "AndroidNDK"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define PAGESIZE 4096            // 0X0000 1000
#define PAGEMASK (~(PAGESIZE - 1)) //0FFFF F000

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

int readUleb128(const u1* ptr, int* bytes_counts)
{
	int result = *(ptr++);
	*bytes_counts = 1;
	if(result > 0x7f)
	{
		//第2字节
		int cur = *(ptr++);
		result = (result & 0x7f) | ((cur&0x7f) << 7);
		*bytes_counts = 2;
		if(cur > 0x7f)
		{
			//第3字节
			cur = *(ptr++);
			result |= (cur & 0x7f) << 14;
			*bytes_counts = 3;
			if(cur > 0x7f)
			{
				//第4字节
				cur = *(ptr++);
				result |= (cur & 0x7f) << 21;
				*bytes_counts = 4;
				if(cur > 0x7f)
				{
					//第5字节
					cur = *(ptr++);
					result |= cur << 28;
					*bytes_counts = 5;
				}
			}
		}
	}
	return result;
}

u1* skipUleb128(int uleb_size, u1* ptr)
{
	int bytes_counts;
	u1* pStream;

	pStream = ptr;

	for(bytes_counts = 0; uleb_size; uleb_size--)
	{

		readUleb128(pStream, &bytes_counts);
		pStream = (u1*)(pStream + bytes_counts);

	}

	return pStream;
}

//获取so加载到内存中的起始地址
unsigned int getLibAddress(const char *name)
{
	unsigned int ret = 0;
	char buf[4096], *temp;
	int pid;
	FILE *fp;
	pid = getpid();
	sprintf(buf, "/proc/%d/maps", pid);
	LOGI("Pid=%d", pid);
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

bool findmagic(const void* dex_addr)
{	
	
	char dest[8];
	memcpy(dest, "dex\n035", 8);
	int ret = 0;

	ret = memcmp(dex_addr, dest, 8);
	if(ret == 0)
		return true;
	else
		return false;
}

int getStringIdx(MiniDexFile* odexFile, const char* target_string, int size)
{
	int idx;
	int string_ids_off = -1;
	int string_ids_size = -1;

	u4 string_data_addr;

	string_ids_off = odexFile->pHeader->stringIdsOff;
	string_ids_size = odexFile->pHeader->stringIdsSize;


	LOGI("string_ids_size=%d, string_ids_off=%d, string_ids_addr=%0x,", string_ids_size, string_ids_off, odexFile->pStringIds);

	if(string_ids_size > 0)
	{
		int bytes_counts = 0;
		int str_uleb_size = 0;
		for(idx=0; idx < string_ids_size; idx++)
		{
			string_data_addr = (u4)odexFile->baseAddr + odexFile->pStringIds[idx].stringDataOff;
			str_uleb_size = readUleb128((u1*)string_data_addr, &bytes_counts);
			LOGI("[%d]string data off: %x, uleb byte counts:%d, string size:%d, string ids:%s", 
				idx,
				odexFile->pStringIds[idx].stringDataOff, 
				bytes_counts, 
				str_uleb_size, 
				(char*)string_data_addr + bytes_counts);
			if((size == str_uleb_size) && !strncmp((char*)string_data_addr + bytes_counts, target_string, size))
			{
				//找到目标字符串的索引
				return idx;
			}
		}
	}
	return -1;
}

int getTypeIdx(MiniDexFile* odexFile, int string_idx)
{
	int idx;

	int type_ids_off = -1;
	int type_ids_size = -1;

	u4 descriptor_idx;	

	type_ids_off = odexFile->pHeader->typeIdsSize;
	type_ids_size = odexFile->pHeader->typeIdsOff;

	LOGI("type_ids_size=%d, type_ids_off=%d, type_ids_addr=%0x,", type_ids_size, type_ids_off, odexFile->pTypeIds);

	if(type_ids_size > 0)
	{
		for(idx = 0; idx < type_ids_size; idx++)
		{
			descriptor_idx = odexFile->pTypeIds[idx].descriptorIdx;
			LOGI("[%d] descriptorIdx:%d", idx, descriptor_idx);
			if(string_idx == descriptor_idx)
			{
				return idx;
			}
		}
	}
	return -1;
}

char* getClassDefItem(MiniDexFile* odexFile, int type_idx)
{
	int idx;
	int class_def_off = -1;
	int class_def_size = -1;
	char* class_def_addr;

	class_def_off = odexFile->pHeader->classDefsOff;
	class_def_size = odexFile->pHeader->classDefsSize;

	if(class_def_size > 0)
	{	
		
		for(idx = 0; idx < class_def_size; idx++)
		{
			if(odexFile->pClassDefs[idx].classIdx == type_idx)
				return (char*)&odexFile->pClassDefs[idx];
		}
	}

	return NULL;
}

int getMethodIdx(MiniDexFile* odexFile, int string_idx_methodname, int type_idx_classname)
{
	int idx;
	int method_ids_off = -1;
	int method_ids_size = -1;

	method_ids_off = odexFile->pHeader->methodIdsOff;
	method_ids_size = odexFile->pHeader->methodIdsSize;

	if(method_ids_size > 0)
	{
		for(idx = 0; idx < method_ids_size; idx++)
		{
			if((odexFile->pMethodIds[idx].nameIdx == string_idx_methodname) 
				&& (odexFile->pMethodIds[idx].classIdx == type_idx_classname))
			{
				return idx;
			}
		}
	}
	return -1;
}

//返回目标方法的DexCode在内存中的地址
int getCodeItem(MiniDexFile* odexFile, char* class_def_addr, int method_idx)
{
	//获取方法所在DexClassData的地址
	int class_data_off;
	char* class_data_addr;

	class_data_off = ((DexClassDef*)class_def_addr)->classDataOff;
	class_data_addr = (char*)(odexFile->baseAddr + class_data_off);

	char* class_data_addr_new_start = class_data_addr;
	int bytes_counts;
	u4 static_fields_size;
	u4 instance_fields_size;
	u4 direct_methods_size;
	u4 virtual_methods_size;

	u1* static_fields_addr;
	u1* instance_fields_addr;
	u1* direct_methods_addr;
	u1* virtual_methods_addr;


	static_fields_size = readUleb128( (u1*)class_data_addr_new_start, &bytes_counts);

	class_data_addr_new_start = (char*)(class_data_addr_new_start + bytes_counts);
	instance_fields_size = readUleb128( (u1*)class_data_addr_new_start, &bytes_counts);

	class_data_addr_new_start = (char*)(class_data_addr_new_start + bytes_counts);
	direct_methods_size = readUleb128( (u1*)class_data_addr_new_start, &bytes_counts);

	class_data_addr_new_start = (char*)(class_data_addr_new_start + bytes_counts);
	virtual_methods_size = readUleb128( (u1*)class_data_addr_new_start, &bytes_counts);

	LOGI("static_fields_size:%x, instance_fields_size:%x, direct_methods_size:%x, virtual_methods_size:%x", 
		static_fields_size,
		instance_fields_size,
		direct_methods_size,
		virtual_methods_size
		);

	static_fields_addr = (u1*)(class_data_addr_new_start + bytes_counts);
	//跳过static DexField
	instance_fields_addr = skipUleb128(2*static_fields_size, (u1*)static_fields_addr);//乘以而是因为DexField有两个uleb成员
	//跳过instance DexField
	direct_methods_addr = skipUleb128(2*instance_fields_size, (u1*)instance_fields_addr);
	//跳过direct Methods
	virtual_methods_addr = skipUleb128(3*direct_methods_size, (u1*)direct_methods_addr);

	LOGI("In dex offset, direct_methods_addr:%d, virtual_methods_addr:%d", 
		direct_methods_addr - odexFile->baseAddr,
		virtual_methods_addr - odexFile->baseAddr);

	//检测direct methods列表
	int i;
	int DexMethod_method_idx = 0;
	u1* DexMethod_accessFlags_start;
	u1* DexMethod_codeOff_start = 0;

	for(i=0; i < direct_methods_size; i++)
	{
		DexMethod_method_idx = readUleb128(direct_methods_addr, &bytes_counts);
		DexMethod_accessFlags_start = (u1*)(direct_methods_addr + bytes_counts);

		if(DexMethod_method_idx == method_idx)
		{
			readUleb128(direct_methods_addr, &bytes_counts);
			DexMethod_codeOff_start = (u1*)(direct_methods_addr + bytes_counts);
			LOGI("Direct DexMethod_codeOff_start:%x", (char*)DexMethod_codeOff_start - (char*)odexFile->baseAddr);
			return readUleb128(DexMethod_codeOff_start, &bytes_counts);
		}
		direct_methods_addr = skipUleb128(2, DexMethod_accessFlags_start);
	}

	//监测virual methods列表
	for(i=0; i < virtual_methods_size; i++)
	{
		DexMethod_method_idx = readUleb128(virtual_methods_addr, &bytes_counts);

		DexMethod_accessFlags_start = (u1*)(virtual_methods_addr + bytes_counts);


		if(DexMethod_method_idx == method_idx)
		{
			readUleb128(DexMethod_accessFlags_start, &bytes_counts);
			DexMethod_codeOff_start = (u1*)(DexMethod_accessFlags_start + bytes_counts);

			LOGI("Virtual DexMethod_methodIdx_start:0x%x, DexMethod_accessFlags_start:0x%x,DexMethod_codeOff_start:0x%x", 
				(char*)virtual_methods_addr - (char*)odexFile->baseAddr,
				(char*)DexMethod_accessFlags_start - (char*)odexFile->baseAddr,
				(char*)DexMethod_codeOff_start - (char*)odexFile->baseAddr);

			return readUleb128(DexMethod_codeOff_start, &bytes_counts) + (int)odexFile->baseAddr;
		}
		virtual_methods_addr = skipUleb128(2, DexMethod_accessFlags_start);
	}


}

JNIEXPORT void JNICALL Java_com_demo_MainActivity_test
  (JNIEnv * env, jobject obj)
 {

  	//1.获取odex文件的内存地址
  	const char* packagename = "/data/dalvik-cache/data@app@com.demo";
  	unsigned int odex_addr = getLibAddress(packagename);

  	//2.获取dex文件的内存地址
  	unsigned int dex_addr = -1;

  	dex_addr = odex_addr + 0x28;
  	while(!findmagic( (void*)(dex_addr) ))
  	{
  		odex_addr += PAGESIZE;
  		dex_addr = odex_addr + 0x28; //odex文件中dex文件的位移
  	}

  	char* dex_base = (char*)dex_addr;
  	MiniDexFile* odexFile = (MiniDexFile*)malloc(sizeof(MiniDexFile));

  	//初始化MiniDexFile结构体
  	odexFile->pOptHeader = (const DexOptHeader*)odex_addr;
  	odexFile->baseAddr = (const u1*)dex_base;
  	odexFile->pHeader = (const DexHeader*)dex_base;
  	odexFile->pStringIds = (const DexStringId*)(dex_addr + odexFile->pHeader->stringIdsOff);
  	odexFile->pTypeIds = (const DexTypeId*)(dex_addr + odexFile->pHeader->typeIdsOff);
  	odexFile->pFieldIds = (const DexFieldId*)(dex_addr + odexFile->pHeader->fieldIdsOff);
  	odexFile->pMethodIds = (const DexMethodId*)(dex_addr + odexFile->pHeader->methodIdsOff);
  	odexFile->pProtoIds = (const DexProtoId*)(dex_addr + odexFile->pHeader->protoIdsOff);
  	odexFile->pClassDefs = (const DexClassDef*)(dex_addr + odexFile->pHeader->classDefsOff);

  	LOGI("Odex addr:%x, Dex addr: %x", odexFile->pOptHeader, odexFile->baseAddr);

  	LOGI("dex magic:%s", odexFile->pHeader->magic);

  	int string_idx_classname = -1;
  	int string_idx_methodname = -1;
  	string_idx_classname = getStringIdx(odexFile, "Lcom/demo/Test;", 15);
  	string_idx_methodname = getStringIdx(odexFile, "add", 3);

  	LOGI("string_idx_classname = %d, string_idx_methodname = %d", string_idx_classname, string_idx_methodname);

  	int type_idx_classname = -1;
  	type_idx_classname = getTypeIdx(odexFile, string_idx_classname);
  	LOGI("classname type idx:%d", type_idx_classname);

  	char* class_def_item_addr;
  	class_def_item_addr = getClassDefItem(odexFile, type_idx_classname);
  	LOGI("class def item addr:%x, in dex offset: 0x%x", class_def_item_addr, class_def_item_addr - dex_base);

  	int method_idx;
  	method_idx = getMethodIdx(odexFile, string_idx_methodname, type_idx_classname);
  	LOGI("method idx: %d", method_idx);

  	//获取目标方法的DexCode在内存中的位置
  	int dexCode_addr;
  	dexCode_addr = getCodeItem(odexFile, class_def_item_addr, method_idx);
  	LOGI("DexCode addr:%x", dexCode_addr - dex_addr);

  	//修复字节码
  	unsigned int insns_addr;
  	insns_addr = dexCode_addr + 16;//DexCode结构体中，截至insns刚好是16bytes
  	//页对其参考：https://github.com/gttiankai/Blog/wiki/c%E8%AF%AD%E8%A8%80%E4%B8%AD%E7%9A%84%E9%A1%B5%E5%AF%B9%E9%BD%90%E7%9A%84%E6%80%BB%E7%BB%93
  	
  	unsigned int aligened_insns_addr = insns_addr & PAGEMASK;
  	unsigned int length = 6 + (aligened_insns_addr & (~PAGEMASK)); 

  	mprotect((void*)aligened_insns_addr, length, 3);

  	LOGI("insns addr:%0x", (char*)insns_addr - (char*)odexFile->baseAddr);

  	char inject[] = {0x90,0x00,0x02,0x03,0x0f,0x00};
  	memcpy((void*)insns_addr, &inject, 6);

  	LOGI("Hello World...\n");
}