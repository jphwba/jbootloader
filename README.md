# JBootloader

## Introduction

JBootloader is a custom bootloader and kernel with a shell that has basic functionality made in c and assembly x86_64. The kernel currently can load a fat-16 filesystem image in read only mode and commands such as cat and ls can be used. Everything that is required to boot is all compiled into os.bin (Releases) including the kernel, stage1/stage2.asm and the fat16 filesystem which is designed to work on QEMU. This project isnt made to be actually used (obviously) but its more of a learning journey helping me learn how a bootloader and kernel work. 

All code in this repository is mine and my own (except for early commits on the bootloader branch where I was following a tutorial to learn) but this also means that there was no external bootloader such as GRUB was used so only my code is booted into the VM. 

#### Boot architecture

First of all stage 1 is loaded which runs in real mode and all it does it load stage 2 into RAM. Stage 2 then switches into 32-bit protected mode, initialises the hardware and loads up the kernel and filesystem from os.bin.
#### If help or cat don't work, make sure you have downloaded the v1.1 binary [here](https://github.com/jphwba/jbootloader/releases/tag/v1.1)

## Getting Started

### 1. Installing Dependencies

**For Windows** (I recommend using WSL tho)

```bash
choco install binutils gcc mingw make qemu # If using Chocolatey
```

> **Caution:** There may be more dependencies required to be installed for Windows

**For macOS**

```bash
brew install nasm gcc qemu make
```

**For Linux/WSL (Debian Based)**

```bash
sudo apt update
sudo apt install nasm gcc qemu-system-x86 make libattr1 libc6-i386 binutils wget build-essential unzip gcc-multilib g++-multilib

#Set Variables
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# downloading binutils and gcc
wget https://ftp.gnu.org/gnu/binutils/binutils-2.46.1.tar.gz
wget https://gcc.gnu.org/pub/gcc/releases/gcc-16.1.0/gcc-16.1.0.tar.gz
mkdir -p $HOME/opt/cross
tar -xf binutils-2.46.1.tar.gz -C $HOME/opt/cross
tar -xf gcc-16.1.0.tar.gz -C $HOME/opt/cross

# building binutils
cd $HOME/opt/cross/binutils-2.46.1/
mkdir build && cd build
../configure
make && sudo make install

# building gcc compiler for i686-elf
cd ../../gcc-16.1.0
mkdir build && cd build
# Make sure that you are in the same terminal from the start or you will need to run the export commands again
../configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --enable-initfini-array --disable-hosted-libstdcxx
make all-gcc
make all-target-libgcc
make all-target-libstdc++-v3
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
```

> **Note:** You can install only `qemu-system-x86` if you're not building.

### 2. Building

First make sure you have cloned the project

```bash
git clone https://github.com/jphwba/jbootloader
cd jbootloader
```

Then run the build command with 

```bash
./build.sh
```

If successful then you will have a os.bin in bin/

## 3. Booting

```bash
qemu-system-i386 -drive format=raw,file=./bin/os.bin,if=ide
```

## Running the precompiled binary

Follow **step 1** from **Getting Started** then download the .bin from the releases page. Then run the command in **step 3** but changing the directory of `file` to the os.bin you downloaded.
