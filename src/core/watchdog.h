#ifndef WATCHDOG_H
#define WATCHDOG_H

#define FLAG_SHELL (1UL << 0)
#define FLAG_OLED (1UL << 1)
#define FLAG_BUTTON (1UL << 2)

#define EXPECTED_FLAGS (FLAG_SHELL | FLAG_OLED | FLAG_BUTTON)

void watchdog_init(void);
void watchdog_pat(void);
void supervisor_kick_bit(uint32_t flag_mask);
void supervisor_task(void);

#endif
