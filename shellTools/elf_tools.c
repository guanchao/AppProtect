#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"
#include "elf_tools.h"

void encryptFunction(const char *output_filename, Elf32_Ehdr ehdr, FILE *file, long file_size, const char *name, Elf32_Sym *targetSym){
	if(getSymbolAddr(ehdr, file, name, targetSym) != 0){
		printf("Get symbol address error...\n");
		return ;
	}

	//1.申请一个同样大小的内存空间用于保存修改后的文件
	char *base;
	int i;

	base = (char*)malloc(file_size);
	if(base == NULL){
		printf("Malloc file error...\n");
		return;
	}

	fseek(file, 0, SEEK_SET);
	if(fread(base, 1, file_size, file) != file_size){
		printf("Read file content error...\n");
		return;
	}
	//加密处理
	for(i = 0; i < targetSym->st_size; i++){
		char *addr = (char*)(base + targetSym->st_value + i);
		*addr = ~(*addr);
	}

	FILE* fout;
	fout = fopen(output_filename, "wb");
	if(fout == NULL){
		printf("Open file error...");
	}

	fseek(fout, 0, SEEK_SET);
	if(fwrite(base, 1, file_size, fout) != file_size){
		printf("Write error...");
	}

	printf("Encrypt function \"%s\" successfully!\n", name);
	fclose(fout);
	free(base);
}


void encryptSection(const char *output_filename, Elf32_Shdr *shdr_tab, int shnum, char *strtab, FILE* file, const char *section_name){
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *target_shdr = NULL;
	char *section_addr;
	char *base;
	int i, shndx, file_size;
	unsigned int length, page_size;

	shndx = getSectionAddr(shdr_tab, shnum, strtab, section_name);
	if(shndx == -1){
		printf("Can not find secction: %s\n", section_name);
		return;
	}

	length = shdr_tab[shndx].sh_size;
	printf("\n[Info] Find section \"%s\": 0x%x, size=0x%d\n", section_name, shdr_tab[shndx].sh_offset, length);

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	base = (char*)malloc(file_size);
	if(base == NULL){
		printf("Malloc memorry error...");
		return ;
	}
	ehdr = (Elf32_Ehdr*)base;
	
	
	fseek(file, 0, SEEK_SET);
	if(fread(base, 1, file_size, file) != file_size){
		printf("Read file error!\n");
		return;
	}

	//TODO 加密处理
	unsigned long text_addr = (unsigned long)base + shdr_tab[shndx].sh_offset;
	for(i = 0; i < length; i++){   
	    char *addr = (char*)(text_addr + i);
		*addr = ~(*addr);
	}
	//由于是dynamic link library，内存中的elf header,其中e_shoff和e_entry是不用的，这里用来保存.mytext的地址
	page_size = shdr_tab[shndx].sh_offset / 4096 + (shdr_tab[shndx].sh_offset % 4096 == 0 ? 0 : 1);
	ehdr->e_shoff = shdr_tab[shndx].sh_offset;
	ehdr->e_entry = (length << 16) + page_size;
	
	FILE* fout;
	fout = fopen(output_filename, "wb");
	if(fout == NULL){
		printf("Open file error...");
	}

	fseek(fout, 0, SEEK_SET);
	if(fwrite(base, 1, file_size, fout) != file_size){
		printf("Write error...");
	}

	printf("Encrypt section \"%s\" successfully!\n", section_name);
	free(base);
	fclose(fout);
}