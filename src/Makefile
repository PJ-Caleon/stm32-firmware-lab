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

# Unity Test
HOST_CC   = gcc
TEST_DIR  = test
UNITY_DIR = $(TEST_DIR)/Unity/src
UNITY_SRC = $(UNITY_DIR)/unity.c

TEST_SRCS = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BUILD)/test/%,$(TEST_SRCS))

TEST_CFLAGS = -std=gnu11 -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer -I$(UNITY_DIR) $(INCLUDES)

# test_add -> finds add.c in core/, drivers/, or app/
find_src = $(firstword $(foreach d,$(SRC_DIRS),$(wildcard $(d)/$(1).c)))

.PHONY: test
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		echo "==> $$t"; \
		./$$t || exit 1; \
	done

.SECONDEXPANSION:
$(BUILD)/test/test_%: $(TEST_DIR)/test_%.c $(UNITY_SRC) $$(call find_src,$$*)
	@mkdir -p $(dir $@)
	$(HOST_CC) $(TEST_CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD) *.elf *.map *.bin
