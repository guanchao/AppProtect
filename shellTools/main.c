#include "elf_parser.h"

void displayProgramHeaderTable(Elf32_Phdr *phdr_tab, int phnum){
	printf("\n=================== Program Header Table ===================\n");
	int i;
	for(i = 0; i < phnum; ++i){
		printf("p_type= %-10d p_offset= 0x%-10x p_vaddr= 0x%-8x p_paddr= 0x%-8x p_filesz= 0x%-8x p_memsz= 0x%-10x p_flags= %-4d p_align= 0x%-6x\n", 
			phdr_tab[i].p_type,
			phdr_tab[i].p_offset,
			phdr_tab[i].p_vaddr,
			phdr_tab[i].p_paddr,
			phdr_tab[i].p_filesz,
			phdr_tab[i].p_memsz,
			phdr_tab[i].p_flags,
			phdr_tab[i].p_align);
	}
}

void displaySectionHeaderTable(Elf32_Shdr *shdr_tab, int shnum, char *strtab){
	printf("\n=================== Section Header Table ===================\n");
	int i;
	for(i = 0; i < shnum; i++){
		printf("[%2d] sh_name= %-24s sh_type=%-12d sh_flags=%-4d sh_addr=0x%-8x sh_offset=0x%-8x sh_size=0x%-8x sh_link=%-4d sh_info=%-4d sh_addralign=%-4d sh_entsize=%-4d\n", 
			i,
			(strtab + shdr_tab[i].sh_name),
			shdr_tab[i].sh_type,
			shdr_tab[i].sh_flags,
			shdr_tab[i].sh_addr,
			shdr_tab[i].sh_offset,
			shdr_tab[i].sh_size,
			shdr_tab[i].sh_size,
			shdr_tab[i].sh_link,
			shdr_tab[i].sh_info,
			shdr_tab[i].sh_addralign,
			shdr_tab[i].sh_entsize);
	}
}



int main(){
	char name[] = "libdemo.so";
	// char function_name[] = "_Z9getStringP7_JNIEnv";
	char function_name[] = "Java_com_demo_MainActivity_getText";
	char target_section[] = ".mytext";
	long file_size;
	int i;
	size_t ret;
	Elf32_Ehdr ehdr;
	Elf32_Phdr *phdr_tab;
	Elf32_Shdr *shdr_tab;
	char *strtab;

	FILE *file;
	Elf32_Sym targetSym;

	file = fopen(name, "rb");
	if(file == NULL){
		printf("Open file error...");
		return -1;
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);

	if(fread(&ehdr, 1, sizeof(Elf32_Ehdr), file) != sizeof(Elf32_Ehdr)){
		printf("Read elf header error...\n");
		return -1;
	}

	//////////////////////初始化Program Header Table//////////////////////
	phdr_tab = (Elf32_Phdr*)malloc(ehdr.e_phnum * sizeof(Elf32_Phdr));
	if(phdr_tab == NULL){
		printf("Malloc program header table error!!!\n");
		return -1;
	}

	fseek(file, ehdr.e_phoff, SEEK_SET);
	for(i = 0; i < ehdr.e_phnum; i++){
		if(fread(phdr_tab + i, 1, sizeof(Elf32_Phdr), file) != sizeof(Elf32_Phdr)){
			printf("Read program header error!\n");
			return -1;
		}
	}

	//////////////////////初始化Section Header Table//////////////////////
	shdr_tab = (Elf32_Shdr*)malloc(ehdr.e_shnum * sizeof(Elf32_Shdr));
	if(shdr_tab == NULL){
		printf("Malloc section header table error!!!\n");
		return -1;
	}

	fseek(file, ehdr.e_shoff, SEEK_SET);
	for(i = 0; i < ehdr.e_shnum; i++){
		if(fread(shdr_tab + i, 1, sizeof(Elf32_Shdr), file) != sizeof(Elf32_Shdr)){
			printf("Read section header error!\n");
			return -1;
		}

	}

	//////////////////////初始化String Table//////////////////////
	strtab = (char*)malloc(shdr_tab[ehdr.e_shstrndx].sh_size);
	if(strtab == NULL){
		printf("Malloc string table error!!!\n");
		return -1;
	}

	fseek(file, shdr_tab[ehdr.e_shstrndx].sh_offset, SEEK_SET);
	if(fread(strtab, 1, shdr_tab[ehdr.e_shstrndx].sh_size, file) != shdr_tab[ehdr.e_shstrndx].sh_size){
		printf("Read string table error!\n");
		return -1;
	}

	displayProgramHeaderTable(phdr_tab, ehdr.e_phnum);
	displaySectionHeaderTable(shdr_tab, ehdr.e_shnum, strtab);

	
	//////////////////////查找symbol name//////////////////////
	if(getSymbolAddr(ehdr, file, function_name, &targetSym) != 0){
		printf("Can not find: %s\n", function_name);
	}

	//////////////////////查找section name//////////////////////
	int shndx;
	shndx = getSectionAddr(shdr_tab, ehdr.e_shnum, strtab, target_section);
	if(shndx != -1){
		printf("Section \"%s\": idx:%d offset:0x%x, size:0x%x\n", target_section, shndx, shdr_tab[shndx].sh_offset, shdr_tab[shndx].sh_size);
	}
	
	
	//////////////////////加密函数//////////////////////
	//encryptFunction("libdemo.hacked.function.so", ehdr, file, file_size, function_name, &targetSym);
	

	//////////////////////加密section//////////////////////
	//encryptSection("libdemo.hacked.section.so", shdr_tab, ehdr.e_shnum, strtab, file, target_section);
	

	free(strtab);
	free(shdr_tab);
	free(phdr_tab);
	fclose(file);
	return 0;
}