#include "elf_parser.h"

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

int getSymbolAddr(Elf32_Ehdr ehdr, FILE *file, const char *name, Elf32_Sym *targetSym){
	//1.读取program header中PT_DYNAMIC属性的segment
	Elf32_Phdr phdr;
	Elf32_Dyn dyn;
	Elf32_Dyn dyn_strsz, dyn_hash, dyn_symtab, dyn_strtab;
	char *dynstr;
	int i, flag = -1;

	fseek(file, ehdr.e_phoff, SEEK_SET);
	for(i = 0; i < ehdr.e_phnum; i++){
		if(fread(&phdr, 1, sizeof(Elf32_Phdr), file) != sizeof(Elf32_Phdr)){
			printf("Read program header table error...\n");
			return -1;
		}

		if(phdr.p_type == PT_DYNAMIC){
			++flag;
		}
	}

	//2:找出DT_STRSZ、DT_HASH、DT_STRTAB、DT_SYMTAB
	for(i = 0; i < phdr.p_filesz/sizeof(Elf32_Dyn); i++){
		fseek(file, phdr.p_offset + sizeof(Elf32_Dyn)*i, SEEK_SET);
		if(fread(&dyn, 1, sizeof(Elf32_Dyn), file) != sizeof(Elf32_Dyn)){
			printf("Read symbol error...\n");
			return -1;
		}

		if(dyn.d_tag == DT_STRSZ){
			++flag;
			dyn_strsz = dyn;
		}

		if(dyn.d_tag == DT_HASH){
			++flag;
			dyn_hash = dyn;
		}

		if(dyn.d_tag == DT_SYMTAB){
			++flag;
			dyn_symtab = dyn;
		}

		if(dyn.d_tag == DT_STRTAB){
			++flag;
			dyn_strtab = dyn;
		}
	}

	if(flag != 4){
		printf("Error...\n");
	}

	dynstr = (char*)malloc(dyn_strsz.d_un.d_val);
	if(dynstr == NULL){
		printf("Malloc dynstr error...\n");
		return -1;
	}

	fseek(file, dyn_strtab.d_un.d_ptr, SEEK_SET);
	if(fread(dynstr, 1, dyn_strsz.d_un.d_val, file) != dyn_strsz.d_un.d_val){
		printf("Read dynstr error...\n");
		return -1;
	}


	//3.跳转到哈希表，获取nbucket、nchain
	unsigned nbucket, nchain;
	unsigned funHash, funIndex;

	fseek(file, dyn_hash.d_un.d_ptr, SEEK_SET);
	if(fread(&nbucket, 1, 4, file) != 4){
		printf("Read nbucket error...\n");
		return -1;
	}

	fseek(file, dyn_hash.d_un.d_ptr + 4, SEEK_SET);
	if(fread(&nchain, 1, 4, file) != 4){
		printf("Read nchain error...\n");
		return -1;
	}

	funHash = elfhash(name);
	funHash = funHash % nbucket;

	//4.获取function的索引
	fseek(file, funHash*4, SEEK_CUR);
	if (fread(&funIndex, 1, 4, file) != 4){
		printf("Read function index error...\n");
		return -1;
	}

	//5.获取该function索引对应的字符串名称
	Elf32_Sym funSym;

	fseek(file, dyn_symtab.d_un.d_ptr + funIndex*sizeof(Elf32_Sym), SEEK_SET);
	if(fread(&funSym, 1, sizeof(Elf32_Sym), file) != sizeof(Elf32_Sym)){
		printf("Read function symbol error...\n");
		return -1;
	}

	//6.如果index=bucket[X % nbucket]对应的symbolTable[index]不是要查找的，
	//则chain[index]的值是具有相同哈希值的下一个符号的索引，沿着chain查找，直到chain[index] = 0
	if(strcmp(dynstr + funSym.st_name, name) != 0){
		while(1){
			//6.1获取index = chain[index]
			fseek(file, dyn_hash.d_un.d_ptr + 8 + (nbucket + funIndex)*4, SEEK_SET);
			if(fread(&funIndex, 1, 4, file) != 4){
				printf("Read funIndex in chain error...\n");
				return -1;
			}

			if(funIndex == 0){
				printf("Can not find function...\n");
				return -1;
			}

			//6.2根据新的方法index到symtab中查找
			fseek(file, dyn_symtab.d_un.d_ptr + funIndex * sizeof(Elf32_Sym), SEEK_SET);
			if(fread(&funSym, 1, sizeof(Elf32_Sym), file) != sizeof(Elf32_Sym)){
				printf("Read function symbol error...\n");
				return -1;
			}

			//6.3对比符号名是否相等
			if(strcmp(dynstr + funSym.st_name, name) == 0){
				break;
			}
		}
	}

	targetSym->st_value = funSym.st_value;
	targetSym->st_size = funSym.st_size;
	targetSym->st_info = funSym.st_info;
	targetSym->st_other = funSym.st_other;
	targetSym->st_shndx = funSym.st_shndx;

	printf("\"%s\": offset=>0x%x, size=>%d\n", name, targetSym->st_value, targetSym->st_size);

	free(dynstr);
	return 0;
}


int getSectionAddr(Elf32_Shdr *shdr_tab, int shnum, char *strtab, const char *section_name){
	int shndx;
	for(shndx = 0; shndx < shnum; shndx++){
		// printf("-----%s\n",strtab + shdr_tab[shndx].sh_name);
		if(strcmp(section_name, strtab + shdr_tab[shndx].sh_name) == 0){

			return shndx;
		}
	}
	return -1;
}