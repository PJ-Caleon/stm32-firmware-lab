# STM32F103 project layout

```
core/      chip-level, rarely edited
  stm32f103.h   register structs + bit names (the ONLY place with addresses/bit positions)
  startup.c     vector table, Reset_Handler
  syscalls.c    newlib glue (_write -> uart_putc, so printf works)
drivers/   one peripheral = one .c/.h pair, logic only
  clock         RCC/Flash/PLL, SysTick (millis, delay_ms), DWT cycle counter
  port_pin      EVERY pin mode + GPIO/AFIO clocks, led_on()/led_off()
  exti          PA0 edge interrupt -> exti_flag + interrupt_timestamp
  dma           generic DMA1 channel driver (knows nothing about UART)
  uart          USART1: RX DMA ring buffer, TX DMA queue
app/       what the product does; never touches a register
  button        debounce (500 ms) on top of exti
  stateMachine  reaction game
  shell         "status" and "led on" (auto-off after 3 s)
  main.c        init order + super-loop, nothing else
tools/stress_test.py
link.ld, Makefile  (Makefile builds every .c in core/ drivers/ app/ automatically)
```

## Rules that keep it modular

1. `app/` may call `drivers/`. `drivers/` never call `app/`. Nothing but `core/` knows registers.
2. A driver may use a lower-level driver (`uart` -> `dma`), never the reverse.
3. Buffers belong to the module that interprets them (`tx_q`, `dma_rx_buffer` are private to `uart.c`).
4. A pin is configured in `port_pin.c` and nowhere else.
5. A new peripheral = new `drivers/name.c/.h` + one `name_init()` call in `main.c`.

## Where to look when something breaks

| Symptom                                   | File                          |
|-------------------------------------------|-------------------------------|
| No output at all / garbage characters     | clock.c (PLL), port_pin.c (PA9), uart.c (BRR) |
| Typing does nothing, output works         | uart.c RX path, dma.c, stm32f103.h DMA struct |
| Output cut off / repeated / out of order  | uart.c TX queue               |
| Button ignored or double-triggers         | exti.c, button.c (debounce)   |
| LED does the wrong thing                  | port_pin.c, then who calls led_on/off (shell.c, stateMachine.c) |
| Hard fault / stuck in Default_Handler     | startup.c vector table vs. the handler name |
| Command parsing                           | shell.c                       |
