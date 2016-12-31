#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"

static unsigned elfhash(const char *_name);
int getSymbolAddr(Elf32_Ehdr ehdr, FILE *file, const char *name, Elf32_Sym *targetSym);
int getSectionAddr(Elf32_Shdr *shdr_tab, int shnum, char *strtab, const char *section_name);

// Elf32_Shdr* getSectionAddr(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, char* strtab, char* section_name);

// Elf32_Shdr* getSectionAddr(Elf32_Ehdr ehdr, , char* strtab, char* section_name){
// 	int i;
// 	Elf32_Shdr *shdr;
// 	shdr = (Elf32_Shdr*)()
// 	for(i = 0; i < ehdr.e_shnum; ++i, shdr++){
// 		if(strcmp(section_name, strtab + shdr->sh_name) == 0){

// 			return shdr;
// 		}
// 	}
// 	return NULL;
// }


// char* getStringTableAddr(char* base){
// 	printf("\n=================== String Table ===================\n");
// 	char* strtab = NULL;
// 	int i;

// 	strtab = (char*)(base + (shdr + ehdr->e_shstrndx)->sh_offset);
// 	return strtab;
// }