all:
	nasm -f elf transformer.asm -g -F dwarf
	ld -m elf_i386 transformer.o -o transformer

clean:
	rm -f transformer.o transformer

run: all
	./transformer

gdb: all
	gdb ./transformer
