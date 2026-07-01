FILES = ./build/kernel.asm.o ./build/kernel.o
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O8 -Iinc

STAGE2_SECTORS = 8
KERNEL_SECTORS = 64
KERNEL_MAX_BYTES = $(shell echo $$(($(KERNEL_SECTORS) * 512)))

all: dirs
	nasm -f bin ./src/stage1.asm -o ./bin/stage1.bin
	nasm -f bin ./src/stage2.asm -o ./bin/stage2.bin
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o
	i686-elf-ld -g -relocatable $(FILES) -o ./build/completeKernel.o
	i686-elf-gcc $(FLAGS) -T ./src/linkerscript.ld -o ./bin/kernel.bin -ffreestanding -O8 -nostdlib ./build/completeKernel.o

	@ksize=$$(stat -c%s ./bin/kernel.bin); \
	if [ $$ksize -gt $(KERNEL_MAX_BYTES) ]; then \
		echo "kernel.bin ($$ksize bytes) exceeds KERNEL_SECTORS budget ($(KERNEL_MAX_BYTES) bytes) - raise KERNEL_SECTORS in the Makefile and config.inc"; \
		exit 1; \
	fi

	rm -f ./bin/os.bin
	cat ./bin/stage1.bin ./bin/stage2.bin ./bin/kernel.bin > ./bin/os.bin
	truncate -s $$(( (1 + $(STAGE2_SECTORS) + $(KERNEL_SECTORS)) * 512 )) ./bin/os.bin

dirs:
	mkdir -p ./bin ./build

clean:
	rm -f ./bin/stage1.bin
	rm -f ./bin/stage2.bin
	rm -f ./bin/kernel.bin
	rm -f ./bin/os.bin
	rm -f ./build/kernel.asm.os
	rm -f ./build/kernel.os
	rm -f ./build/completeKernel.o

run: all
	qemu-system-x86_64 -drive format=raw,file=./bin/os.bin