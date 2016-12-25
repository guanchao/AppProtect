#include <stdio.h>
#include <stdlib.h>
#include "elf.h"

char *base = NULL;
Elf32_Ehdr *ehdr = NULL;
Elf32_Shdr *shdr = NULL;
Elf32_Phdr *phdr = NULL;
char* strtab = NULL;

void displayELFHeader(Elf32_Ehdr* ehdr){
	printf("\n=================== ELF Header ===================\n");
	printf("In memorry, the address of ELF Header is: 0x%x\n", ehdr);
	printf("e_ident=\t%s\n", ehdr->e_ident);
	printf("e_type= \t%d\n", ehdr->e_type);
	printf("e_machine=\t%d\n", ehdr->e_machine);
	printf("e_version=\t%d\n", ehdr->e_version);
	printf("e_entry=\t0x%x\n", ehdr->e_entry);
	printf("e_phoff=\t0x%x\n", ehdr->e_phoff);
	printf("e_shoff=\t0x%x\n", ehdr->e_shoff);
	printf("e_flags=\t%d\n", ehdr->e_flags);
	printf("e_ehsize=\t%d\n", ehdr->e_ehsize);
	printf("e_phentsize=\t%d\n", ehdr->e_phentsize);
	printf("e_phnum=\t%d\n", ehdr->e_phnum);
	printf("e_shentsize=\t%d\n", ehdr->e_shentsize);
	printf("e_shnum=\t%d\n", ehdr->e_shnum);
	printf("e_shstrndx=\t%d\n", ehdr->e_shstrndx);
}

void displaySectionHeaderTable(Elf32_Shdr* shdr, int shnum, char* strtab){
	printf("\n=================== Section Header Table ===================\n");
	printf("In memorry, the address of Section Header table is: 0x%x\n", shdr);
	int i;
	for(i = 0; i < shnum; ++i, ++shdr){
		printf("[%2d] sh_name= %-12s, sh_type=%-12d, sh_flags=%-4d, sh_addr=0x%-8x, sh_offset=0x%-8x, sh_size=0x%-8x, sh_link=%-4d, sh_info=%-4d, sh_addralign=%-4d, sh_entsize=%-4d\n", 
			i,
			(strtab + shdr->sh_name),
			shdr->sh_type,
			shdr->sh_flags,
			shdr->sh_addr,
			shdr->sh_offset,
			shdr->sh_size,
			shdr->sh_size,
			shdr->sh_link,
			shdr->sh_info,
			shdr->sh_addralign,
			shdr->sh_entsize);
	}
}

void displayProgramHeaderTable(Elf32_Phdr* phdr, int phnum){
	printf("\n=================== Program Header Table ===================\n");
	printf("In memorry, the address of Program Header table is: 0x%x\n", phdr);
	int i;
	for(i = 0; i < phnum; ++i, ++phdr){
		printf("p_type= %-10d, p_offset= 0x%-10x, p_vaddr= 0x%-8x, p_paddr= 0x%-8x, p_filesz= 0x%-8x, p_memsz= 0x%-10x, p_flags= %-4d, p_align= 0x%-6x\n", 
			phdr->p_type,
			phdr->p_offset,
			phdr->p_vaddr,
			phdr->p_paddr,
			phdr->p_filesz,
			phdr->p_memsz,
			phdr->p_flags,
			phdr->p_align);
	}
}

char* getStringTableAddr(char* base){
	printf("\n=================== String Table ===================\n");
	char* strtab = NULL;
	int i;

	strtab = (char*)(base + (shdr + ehdr->e_shstrndx)->sh_offset);
	return strtab;
}

Elf32_Shdr* getSectionAddr(char* section_name){
	int i;
	for(i = 0; i < ehdr->e_shnum; ++i, shdr++){
		if(strcmp(section_name, strtab + shdr->sh_name) == 0){

			return shdr;
		}
	}
	return NULL;
}

/**
 对section_name对应的整体section进行加密处理
 FILE* file: 指向so文件的指针
 char* section_name: 要处理section名称
**/
void encryptSection(FILE* file, char* section_name){
	Elf32_Shdr* target_shdr = NULL;
	char* section_addr;
	char* content = NULL;
	int i, ret;
	unsigned int length, page_size;

	target_shdr = getSectionAddr(section_name);
	printf("Target %s: 0x%x\n", section_name, target_shdr->sh_offset);

	if(target_shdr != NULL){
		length = target_shdr->sh_size;
		printf("target size=%d\n", length);
		content = (char*)malloc(sizeof(char*)*length);
		if(content == NULL){
			printf("Malloc memorry error...");
			return ;
		}
		
		memset(content, 0, length);
		fseek(file, 0, SEEK_SET);
		ret = fread(content, 1, length, file);
		if(ret != length){
			printf("Reading error...");
			return ;
		}

		//TODO 加密处理
		unsigned long text_addr = base + target_shdr->sh_offset;
		for(i = 0; i < length; i++){   
		    char *addr = (char*)(text_addr + i);
    		*addr = ~(*addr);
		}
		//由于是dynamic link library，内存中的elf header,其中e_shoff和e_entry是不用的，这里用来保存.mytext的地址
		page_size = target_shdr->sh_offset / 4096 + (target_shdr->sh_offset % 4096 == 0 ? 0 : 1);
		ehdr->e_shoff = target_shdr->sh_offset;
		ehdr->e_entry = (length << 16) + page_size;
	}
	free(content);
}

int main(){

	char name[] = "libdemo.so";
	FILE* file;
	long file_size;
	size_t ret;

	file = fopen(name, "rb");
	if(file == NULL){
		printf("Open file error...");
		return -1;
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);

	base = (char*)malloc(sizeof(char*)*file_size);
	if(base == NULL){
		printf("Malloc memorry error...");
		return -1;
	}

	memset(base, 0, file_size);
	fseek(file, 0, SEEK_SET);
	ret = fread(base, 1, file_size, file);
	if(ret != file_size){
		printf("Reading error...");
		return -1;
	}

	//Step1:解析so文件
	ehdr = (Elf32_Ehdr*)base;
	shdr = (Elf32_Shdr*)(base + ehdr->e_shoff);
	phdr = (Elf32_Phdr*)(base + ehdr->e_phoff);
	strtab = getStringTableAddr(base);

	displayELFHeader(ehdr);
	if(strtab == NULL){
		printf("Get String table rror...");
		return -1;
	}
	displaySectionHeaderTable(shdr, ehdr->e_shnum, strtab);
	displayProgramHeaderTable(phdr, ehdr->e_phnum);

	//Step2:获取目标section在内存中的地址
	Elf32_Shdr* shdr_mytext = NULL;
	shdr_mytext = getSectionAddr(".mytext");
	printf("In memorry, the address of String table is: 0x%x\n", shdr_mytext);
	
	//Step3:对目标section进行加密
	if(shdr_mytext != NULL){
		encryptSection(file, ".mytext");
	}

	//Step4:保存加固后的so到新的文件中
	FILE* fout;
	fout = fopen("libdemo.modified.so", "wb");
	if(fout == NULL){
		printf("Open file error...");
	}

	fseek(fout, 0, SEEK_SET);
	if(fwrite(base, 1, file_size, fout) != file_size){
		printf("Write error...");
	}

	free(base);
	fclose(fout);
	fclose(file);

	
	return 0;
}