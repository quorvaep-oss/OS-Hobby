CC      := clang
LD      := ld.lld
TARGET  := --target=x86_64-unknown-elf

CFLAGS  := $(TARGET) -std=c11 -O2 -g -Wall -Wextra \
           -ffreestanding -fno-stack-protector -fno-stack-check \
           -fno-pic -fno-pie -fno-lto -m64 -march=x86-64 \
           -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-80387 \
           -mcmodel=kernel -Ikernel/include -Ikernel/src

LDFLAGS := -nostdlib -static -z max-page-size=0x1000 -T kernel/linker.ld

BUILD   := build
SRCS    := $(wildcard kernel/src/*.c)
ASMS	:= $(wildcard kernel/src/*.S)
OBJS    := $(patsubst kernel/src/%.c,$(BUILD)/%.o,$(SRCS)) \
	   $(patsubst kernel/src/%.S,$(BUILD)/%.o,$(ASMS))
KERNEL  := $(BUILD)/kernel.elf
ISO     := $(BUILD)/quorv-os.iso

LIMINE_DIR := limine
LIMINE_TAG := v8.x-binary

.PHONY: all iso run run-headless clean distclean

all: $(KERNEL)

$(BUILD)/%.o: kernel/src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: kernel/src/%.S
	@mkdir -p $(BUILD)
	$(CC) $(TARGET) -c $< -o $@

$(KERNEL): $(OBJS) kernel/linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(LIMINE_DIR):
	git clone --depth=1 --branch=$(LIMINE_TAG) https://github.com/limine-bootloader/limine.git $(LIMINE_DIR)
	$(MAKE) -C $(LIMINE_DIR)

iso: $(ISO)

$(ISO): $(KERNEL) limine.conf | $(LIMINE_DIR)
	rm -rf $(BUILD)/iso_root
	mkdir -p $(BUILD)/iso_root/boot/limine $(BUILD)/iso_root/EFI/BOOT
	cp $(KERNEL) $(BUILD)/iso_root/boot/kernel.elf
	cp limine.conf $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin \
	   $(LIMINE_DIR)/limine-uefi-cd.bin $(BUILD)/iso_root/boot/limine/
	cp $(LIMINE_DIR)/BOOTX64.EFI $(BUILD)/iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
	   -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	   -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
	   -efi-boot-part --efi-boot-image --protective-msdos-label \
	   $(BUILD)/iso_root -o $@ 2>/dev/null
	./$(LIMINE_DIR)/limine bios-install $@

run: $(ISO)
	qemu-system-x86_64 -M q35 -m 256M -cdrom $(ISO) -serial stdio

run-headless: $(ISO)
	qemu-system-x86_64 -M q35 -m 256M -cdrom $(ISO) -serial stdio -display none -no-reboot

clean:
	rm -rf $(BUILD)

distclean: clean
	rm -rf $(LIMINE_DIR)
