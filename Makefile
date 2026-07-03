FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -Wextra -O0 -Isrc

STAGE2_SECTORS = 8
KERNEL_SECTORS = 64
KERNEL_MAX_BYTES = $(shell echo $$(($(KERNEL_SECTORS) * 512)))

C_SOURCES = src/kernel.c \
			src/kernel/vga.c \
			src/kernel/idt.c \
			src/kernel/isr.c \
			src/kernel/irq.c \
			src/kernel/pic.c \
			src/kernel/pit.c \
			src/kernel/keyboard.c

ASM_OBJ_SOURCES = src/kernel.asm \
				  src/kernel/isr.asm \
				  src/kernel/irq.asm
C_OBJECTS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst src/%.asm, build/%.asm.o, $(ASM_OBJ_SOURCES))

all: dirs
	nasm -f bin -I src/ ./src/stage1.asm -o ./bin/stage1.bin
	nasm -f bin -I src/ ./src/stage2.asm -o ./bin/stage2.bin
	$(MAKE) $(ASM_OBJECTS) $(C_OBJECTS)
	i686-elf-ld -g -relocatable $(ASM_OBJECTS) $(C_OBJECTS) -o ./build/completeKernel.o
	i686-elf-gcc $(FLAGS) -T ./src/linkerscript.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/completeKernel.o

	@ksize = $$(stat -c%s ./bin/kernel.bin); \
	if [ $$ksize -gt $(KERNEL_MAX_BYTES) ]; then \
		echo "kernel.bin ($$ksize bytes) exceeds KERNEL_SECTORS budget ($(KERNEL_MAX_BYTES) bytes) - raise KERNEL_SECTORS in the Makefile and config.inc"; \
		exit 1; \
	fi

	rm -f ./bin/os.bin
	cat ./bin/stage1.bin ./bin/stage2.bin ./bin/kernel.bin > ./bin/os.bin
	truncate -s $$(( (1 + $(STAGE2_SECTORS) + $(KERNEL_SECTORS)) * 512 )) ./bin/os.bin

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	i686-elf-gcc -std=gnu99 $(FLAGS) -c $< -o $@

build/%.asm.o: src/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf -g $< -o $@

dirs:
	mkdir -p ./bin ./build ./build/kernel

clean:
	rm -rf ./bin ./build

run: all
	qemu-system-i386 -drive format=raw,file=./bin/os.bin

.PHONY: all dirs clean run