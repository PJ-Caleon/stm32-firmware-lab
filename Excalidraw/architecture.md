# Architecture

STM32F103C6T6 (Blue Pill) firmware. Layered so that `app/` depends on `drivers/`, and `drivers/` and `app/` depend on `core/`.

## Folder Structure

```mermaid
flowchart LR
    src["src"] --> link["link.ld"]
    src --> core["core/"]
    src --> app["app/"]
    src --> drivers["drivers/"]
    src --> test["test/"]
```

## core/

```mermaid
flowchart LR
    core["core/"] --> crc["crc<br/>Error detecting code<br/>16-bit and 32-bit checksums"]
    core --> fault["fault_handler<br/>Flags usage, bus, memory manage<br/>and hard faults<br/>GDB shows which line is stalling<br/>Halts during debug"]
    core --> ring["ring_buffer<br/>FIFO queue: enqueue and dequeue<br/>Pointer section disables interrupts<br/>and re-enables them after"]
    core --> startup["startup<br/>Vector table in ROM: stack, reset handler,<br/>hardfault. Reset handler copies .data,<br/>zeroes .bss, then calls main"]
    core --> syscall["syscall<br/>Overrides printf's write function<br/>so output reaches the terminal<br/>Use instead of uartputs when the<br/>string has variables"]
    core --> wdt["watchdog<br/>Timer that resets the MCU if it is<br/>not kicked within a set time"]
    core --> regs["stm32f103<br/>Register memory map, register and bit<br/>macro helpers. Built from the STM manual"]
```

## app/

```mermaid
flowchart LR
    app["app/"] --> button["button<br/>Clock-based press timing<br/>Distinguishes click from hold"]
    app --> art["haimiya / malfoid / nixos<br/>128x64 pixel art for the OLED<br/>Stored in ROM"]
    app --> oled["oled<br/>Font stored in ROM<br/>Sends commands over I2C<br/>Memory reset clears the screen<br/>Draws via framebuffer<br/>Fixed hex init commands"]
    app --> frame["oled_frame<br/>Frame and title template<br/>for the state machine<br/>Built on the oled commands"]
    app --> pot["potentiometer<br/>Reads PA2 as ADC value<br/>Maps raw reading to 0 - 4095"]
    app --> shell["shell<br/>Commands from PC to MCU over UART<br/>Used to debug physical parts"]
    app --> sm["stateMachine<br/>3 states: uptime, led toggle, images<br/>Switched via potentiometer"]
    app --> main["main<br/>Inits watchdog, core timings,<br/>hardware and state machine"]
```

## drivers/

```mermaid
flowchart LR
    drivers["drivers/"] --> adc["adc<br/>Built-in ADC1 channel 2<br/>Inits clock and links to DMA"]
    drivers --> clock["clock<br/>72 MHz system clock<br/>millis and delay_ms<br/>DWT cycle counter since startup"]
    drivers --> dma["dma<br/>Moves peripheral data to memory<br/>over the bus without the CPU"]
    drivers --> exti["exti<br/>16 external interrupt lines<br/>Selects which pins can interrupt<br/>EXTI0 has the highest priority"]
    drivers --> port["port_pin<br/>GPIO clocks and pin setup<br/>Input, output, pull-up, push-pull<br/>Config listed at image/PortBitConfig"]
    drivers --> pwm["pwm<br/>Duty cycle from the potentiometer<br/>Controls LED brightness"]
    drivers --> spi["spi<br/>Master generates the clock<br/>Chip select per device<br/>MOSI and MISO lines"]
    drivers --> uart["uart<br/>115200 baud, configurable<br/>Buffer queue, DMA, TX and RX pins"]
    drivers --> i2c["i2c<br/>Two wires: SCL and SDA<br/>Master and slave clocks<br/>must be configured the same"]
```

## link.ld and test/

```mermaid
flowchart LR
    link["link.ld<br/>Places compiler output into the<br/>STM32 ROM and RAM regions<br/>Fixed address layout for each"]
    test["test/<br/>Unit tests for independent functions<br/>Mocks and stubs with Unity framework"]
```

## State Machine
- Far left: Toggle LED
- Middle: Uptime
- Far right: Image

```mermaid
stateDiagram-v2
    [*] --> ToggleLed

    ToggleLed: Toggle LED
    ToggleLed: Display "Led: On"
    ToggleLed: Button click toggles LED
    ToggleLed: Button hold blinks LED

    Uptime: Uptime
    Uptime: Display "Time: 30 s"

    state Image {
        [*] --> Img1
        Img1 --> Img2: button
        Img2 --> Img3: button
        Img3 --> Img1: button
    }

    ToggleLed --> Uptime: pot to middle
    Uptime --> Image: pot to right
    Image --> ToggleLed: pot to far left
    Uptime --> ToggleLed: pot to far left
    Image --> Uptime: pot to middle
```