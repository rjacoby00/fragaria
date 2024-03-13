SHELL = /bin/bash

arch = x86_64-elf
kernel := build/kernel.bin
img := build/fragaria.img
iso := build/fragaria.iso

assembly_source_files := $(wildcard src/*.asm)
assembly_object_files := $(patsubst src/%.asm, build/%.o, \
			 $(assembly_source_files))

c_source_files := $(wildcard src/*.c)
c_object_files := $(patsubst src/%.c, build/%.o, $(c_source_files))

cc := $(arch)-gcc
ld := $(arch)-gcc
asm = nasm

cflags = -c -g -Werror -Wall -ffreestanding -mno-red-zone
ldflags = -n -nostdlib -lgcc

.PHONY: fragaria run runiso debugiso img iso clean

fragaria: $(kernel)
	$(MAKE) -j8 $(kernel)

run: $(img)
	qemu-system-x86_64 -s -drive format=raw,file=$(img) -serial stdio

runiso: $(iso)
	qemu-system-x86_64 -s -cdrom $(iso) -serial stdio

debugiso: $(iso)
	qemu-system-x86_64 -s -cdrom $(iso)&
	gdb -x ./.gdbinit

img: $(img)

iso: $(iso)

.ONESHELL:
$(img): $(kernel) src/grub.cfg
	mkdir -p build/imgfiles/boot/grub
	cp $(kernel) build/imgfiles/boot/kernel.bin
	cp src/grub.cfg build/imgfiles/boot/grub/grub.cfg
	dd if=/dev/zero of=$@ bs=512 count=32768
	parted $@ mklabel msdos
	parted $@ mkpart primary fat32 2048s 30720s
	parted $@ set 1 boot on
	loopdir="$$(sudo losetup --show -Pf build/os.img)"
	echo $$loopdir
	sudo mkdosfs -F32 -f 2 $${loopdir}p1
	mkdir -p build/fatgrub
	sudo mount $${loopdir}p1 build/fatgrub
	sudo ./grub/grub-install -d grub/grub-core/ \
		--root-directory=build/fatgrub --no-floppy \
	       	--modules="normal part_msdos fat ext2 multiboot multiboot2" \
		--target=i386-pc $${loopdir}
	sudo cp -r build/imgfiles/* build/fatgrub
	sudo umount build/fatgrub
	sudo losetup -d $$loopdir
	
$(iso): $(kernel) src/grub.cfg
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp src/grub.cfg build/isofiles/boot/grub/grub.cfg
	grub2-mkrescue -o $@ build/isofiles

$(kernel): $(assembly_object_files) $(c_object_files) src/linker.ld
	$(ld) $(ldflags) -T src/linker.ld -o $@ $(assembly_object_files) \
		$(c_object_files)

build/%.o: src/%.asm
	mkdir -p $(@D)
	$(asm) -felf64 $< -o $@

build/%.o: src/%.c
	mkdir -p $(@D)
	$(cc) $(cflags) $< -o $@

clean:
	rm -rf build/
