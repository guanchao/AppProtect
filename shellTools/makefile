run : main.o elf_parser.o elf_tools.o
		cc -o run main.o elf_parser.o elf_tools.o
main.o : main.c elf_parser.h
		cc -c main.c
elf_parser.o : elf_parser.c elf_parser.h elf.h
		cc -c elf_parser.c
elf_tools.o : elf_tools.c elf_tools.h elf.h
		cc -c elf_tools.c
clean : 
		rm run main.o elf_parser.o elf_tools.o
