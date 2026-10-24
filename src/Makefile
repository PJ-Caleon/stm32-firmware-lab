CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
MACH    = cortex-m3

# Every .c file inside these folders is built automatically.
# Adding a new driver = drop two files in a folder, no Makefile edit.
SRC_DIRS = core drivers app
BUILD    = build

SRC  = $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))
OBJS = $(patsubst %.c,$(BUILD)/%.o,$(SRC))
DEPS = $(OBJS:.o=.d)

INCLUDES = $(addprefix -I,$(SRC_DIRS))
CFLAGS   = -mcpu=$(MACH) -mthumb -std=gnu11 -O0 -Wall -g -MMD -MP $(INCLUDES)
LDFLAGS  = -mcpu=$(MACH) -mthumb -T link.ld -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,-Map=app.map

.PHONY: all clean

all: app.elf app.bin

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

app.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

app.bin: app.elf
	$(OBJCOPY) -O binary $< $@

-include $(DEPS)

clean:
	rm -rf $(BUILD) *.elf *.map *.bin
