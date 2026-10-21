CC=arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
MACH=cortex-m3
CFLAGS = -c -mcpu=$(MACH) -mthumb -std=gnu11 -O0 -Wall -g -MMD -MP
LDFLAGS = -T link.ld -nostdlib -Wl,-Map=app.map

SRC = startup.c clock.c main.c
OBJS = $(SRC:.c=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean 

all: app.elf app.bin

%.o: %.c 
	$(CC) $(CFLAGS)	$<	-o	$@ 

app.elf:	$(OBJS) 
	$(CC)	$(LDFLAGS) $^	-o	$@ 

app.bin: app.elf
	$(OBJCOPY) -O binary $< $@ 

-include $(DEPS)

clean:
	rm -rf *.o *.elf *.map *.bin *.d
