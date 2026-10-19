CC=arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
MACH=cortex-m3
CFLAGS = -c -mcpu=$(MACH) -mthumb -std=gnu11 -O0 -Wall
LDFLAGS = -T link.ld -nostdlib -Wl,-Map=app.map 

.PHONY: all clean 

all: app.elf app.bin

%.o: %.c stm32f103.h
	$(CC) $(CFLAGS)	$<	-o	$@ 

app.elf:	main.o	startup.o 
	$(CC)	$(LDFLAGS) $^	-o	$@ 

app.bin: app.elf
	$(OBJCOPY) -O binary $< $@ 

clean:
	rm -rf *.o *.elf *.map *.bin 
