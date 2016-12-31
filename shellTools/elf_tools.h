#include "elf_parser.h"

void encryptFunction(const char *output_filename, Elf32_Ehdr ehdr, FILE *file, long file_size, const char *name, Elf32_Sym *targetSym);
void encryptSection(const char *output_filename, Elf32_Shdr *shdr_tab, int shnum, char *strtab, FILE* file, const char *section_name);

