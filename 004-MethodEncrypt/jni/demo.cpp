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

unsigned long getLibAddress(const char *name);
void decryptFunction(unsigned long base, Elf32_Sym *funSym);
// void printELFHeader(Elf32_Ehdr* ehdr);
static unsigned elfhash(const char *_name);

// void displayProgramHeaderTable(Elf32_Phdr* phdr, int phnum){
//   printf("\n=================== Program Header Table ===================\n");
//   printf("In memorry, the address of Program Header table is: 0x%x\n", phdr);
//   int i;
//   for(i = 0; i < phnum; ++i, ++phdr){
//     LOGI("p_type= %d, p_offset= 0x%x, p_vaddr= 0x%x, p_paddr= 0x%x, p_filesz= 0x%x, p_memsz= 0x%x, p_flags= %d, p_align= 0x%x\n", 
//       phdr->p_type,
//       phdr->p_offset,
//       phdr->p_vaddr,
//       phdr->p_paddr,
//       phdr->p_filesz,
//       phdr->p_memsz,
//       phdr->p_flags,
//       phdr->p_align);
//   }
// }


static unsigned elfhash(const char *_name){
    const unsigned char *name = (const unsigned char *) _name;
    unsigned h = 0, g;

    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

void init_security(){
	LOGI("----------------init_security()---------------------");
  unsigned long base;
  Elf32_Ehdr *ehdr;
  Elf32_Phdr *phdr;
  int i, flag = -1;


	//1.获取so文件内存地址
  base = getLibAddress("libdemo.so");
  if(base == 0){
    LOGI("Get so address error...");
    return;
  }
  // LOGI("so addr:0x%x\n", base);

  //2.解析elf header和program header
  ehdr = (Elf32_Ehdr*)base;
  phdr = (Elf32_Phdr*)(base + ehdr->e_phoff);

  //3.解析program header table，找到PT_DYNAMIC属性的segment
  for(i = 0; i < ehdr->e_phnum; ++i, ++phdr){
    if(phdr->p_type == PT_DYNAMIC){
      ++flag;
      break;
    }
  }

  if(flag != 0){
    LOGI("Find PT_DYNAMIC segment error...\n");
    return;
  }
  LOGI("Dynamic link table addr:0x%x\n", phdr->p_offset);

  //4.遍历Dynamic link table获取DT_STRTAB、DT_SYMTAB、DT_STRSZ、DT_HASH四个属性的Elf32_Dyn对象
  Elf32_Dyn *dyn_strtab, *dyn_symtab, *dyn_strsz, *dyn_hash;
  Elf32_Dyn *dyn;
  for(i = 0; i < phdr->p_filesz/sizeof(Elf32_Dyn); i++){
    dyn = (Elf32_Dyn*)(base + phdr->p_offset + i*sizeof(Elf32_Dyn));

    if(dyn->d_tag == DT_STRTAB){
      ++flag;
      dyn_strtab = dyn;
      LOGI("Find DT_STRTAB!");
    }

    if(dyn->d_tag == DT_SYMTAB){
      ++flag;
      dyn_symtab = dyn;
      LOGI("Find DT_SYMTAB!");
    }

    if(dyn->d_tag == DT_STRSZ){
      ++flag;
      dyn_strsz = dyn;
      LOGI("Find DT_STRSZ!");
    }

    if(dyn->d_tag == DT_HASH){
      ++flag;
      dyn_hash = dyn;
      LOGI("Find DT_HASH!");
    }

  }

  if(flag != 4){
    LOGI("Error...");
  }

  //5.初始化dynstr符号列表(从STRTAB读取)
  char *dynstr;
  dynstr = (char*)(base + dyn_strtab->d_un.d_ptr);

  //6.初始化哈希表,nbucket, nchain, bucket, chain
  unsigned nbucket, nchain;
  unsigned *bucket, *chain;
  unsigned funHash, funIndex;
  Elf32_Sym *funSym;
  char funName[] = "Java_com_demo_MainActivity_getText";
  int isFind = -1;

  
  nbucket = *(unsigned*)(base + dyn_hash->d_un.d_ptr);
  nchain = *(unsigned*)(base + dyn_hash->d_un.d_ptr + 4);
  bucket = (unsigned*)(base + dyn_hash->d_un.d_ptr + 8);
  chain = (unsigned*)(base + dyn_hash->d_un.d_ptr + 8 + nbucket*4);
  funHash = elfhash(funName);
  funHash = funHash % nbucket;

  //6.1首先根据index = bucket[X % nbucket],去symtab查找对应的Elf32_Sym对象
  funIndex = *(bucket + funHash);
  funSym = (Elf32_Sym*)(base + dyn_symtab->d_un.d_ptr + funIndex * sizeof(Elf32_Sym));
  LOGI("nbucket=%d, nchain=%d, funHash=%d, funIndex=%d", nbucket, nchain,funHash, funIndex);
  LOGI("%s", (char*)(dynstr + funSym->st_name));

  if(strcmp(dynstr + funSym->st_name, funName) != 0){
    //6.2如果不相等，则获取index = chain[index]，再进行查找
    while(1){
      funIndex = *(chain + funHash);
      funSym = (Elf32_Sym*)(base + dyn_symtab->d_un.d_ptr + funIndex * sizeof(Elf32_Sym));
      LOGI("funIndex=%d, funSym=%s",funIndex, dynstr + funSym->st_name);

      if(funIndex == 0){
        LOGI("Can not find function: %s", funName);
      }

      if(strcmp(dynstr + funSym->st_name, funName) == 0){
        isFind = 0;
        break;
      }
    }

  }else{
    isFind = 0;
  }
  
  if(isFind == 0){
    //找到目标函数，开始解密
    unsigned int page_size;

    page_size = funSym->st_size / PAGE_SIZE + ((funSym->st_size % PAGE_SIZE == 0) ? 0: 1);

    LOGI("Function is found! Addr:0x%x, Size:%d", funSym->st_value, funSym->st_size);

    if(mprotect( (void*)((base + funSym->st_value)/PAGE_SIZE * PAGE_SIZE), page_size * PAGE_SIZE, PROT_READ | PROT_EXEC | PROT_WRITE) != 0){
      LOGI("mprotect 1 error...\n");
      return ;
    }

    decryptFunction(base, funSym);

    if(mprotect( (void*)((base + funSym->st_value)/PAGE_SIZE * PAGE_SIZE), page_size * PAGE_SIZE, PROT_READ | PROT_EXEC) != 0){
      LOGI("mprotect 2 error...\n");
      return ;
    }

    
  }

  LOGI("----------------init_security() Done---------------------");
}

void decryptFunction(unsigned long base, Elf32_Sym *funSym){
	int i;
  unsigned int length;

  length = funSym->st_size;

	for(i = 0; i < length; i++){   
	    char *addr = (char*)(base + funSym->st_value + i);
		  *addr = ~(*addr);
	}
  LOGI("%s", "Decrypt function successfully!");
}

//获取so加载到内存中的起始地址
unsigned long getLibAddress(const char *name){
	unsigned long ret = 0;
	char buf[PAGE_SIZE], *temp;
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
  	// return getString(env);
    LOGI("Call Java_com_demo_MainActivity_getText!");
    return env->NewStringUTF("Native method return!"); 
}