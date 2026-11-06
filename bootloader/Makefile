CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
MACH    = cortex-m3

# Bootloader source files and shared core dependencies
SRC_DIRS = . ../src/core
BUILD    = build

# Explicitly select bootloader main, startup, and crc modules
SRC = main.c ../src/core/startup.c ../src/core/crc.c
OBJS = $(BUILD)/main.o $(BUILD)/startup.o $(BUILD)/crc.o
DEPS = $(OBJS:.o=.d)

INCLUDES = -I. -I../src/core
CFLAGS   = -mcpu=$(MACH) -mthumb -std=gnu11 -Os -Wall -g -MMD -MP $(INCLUDES)
LDFLAGS  = -mcpu=$(MACH) -mthumb -T link.ld -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,-Map=bootloader.map

.PHONY: all clean

all: bootloader.elf bootloader.bin

$(BUILD)/main.o: main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/startup.o: ../src/core/startup.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/crc.o: ../src/core/crc.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bootloader.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

bootloader.bin: bootloader.elf
	$(OBJCOPY) -O binary $< $@

-include $(DEPS)

clean:
	rm -rf $(BUILD) *.elf *.map *.bin
