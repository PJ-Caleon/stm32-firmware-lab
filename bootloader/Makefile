CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
MACH    = cortex-m3

BUILD   = build

SRC = main.c \
			flash.c \
      ../src/core/startup.c \
      ../src/core/crc.c \
      ../src/drivers/clock.c \
      ../src/drivers/port_pin.c

OBJS = $(BUILD)/main.o \
			 $(BUILD)/flash.o \
       $(BUILD)/startup.o \
       $(BUILD)/crc.o \
       $(BUILD)/clock.o \
       $(BUILD)/port_pin.o

DEPS = $(OBJS:.o=.d)

INCLUDES = -I. -I../src/core -I../src/drivers
CFLAGS   = -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Os -Wall -g -MMD -MP $(INCLUDES)
LDFLAGS  = -mcpu=$(MACH) -mthumb -mfloat-abi=soft -T link.ld -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,-Map=bootloader.map

.PHONY: all clean

all: bootloader.elf bootloader.bin

$(BUILD)/main.o: main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/flash.o: flash.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/startup.o: ../src/core/startup.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/crc.o: ../src/core/crc.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/clock.o: ../src/drivers/clock.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/port_pin.o: ../src/drivers/port_pin.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bootloader.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

bootloader.bin: bootloader.elf
	$(OBJCOPY) -O binary $< $@

-include $(DEPS)

clean:
	rm -rf $(BUILD) *.elf *.map *.bin
