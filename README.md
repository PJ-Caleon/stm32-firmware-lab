This repo is meant to enhance my embedded skills in preparation for my career goals. The project uses STM32F103C6T6 and scoped to be accomplish within 30 days. This was build from m3y54m Embedded Engineering Roadmap trimmed to the software side of C, microcontrollers, protocols, RTOS, debugging, build systems and embedded Linux

# Part List 
- STM32F103C6T6
- ST-Link V2 dongle
- USB-to-TTL adapter, CH340G module
- 830-point breadboard
- 0.96 inch SSD1306 OLED 
- 10 tactile switches, 12 mm 
- B10k potentiometer (WH148)

Each day has a project to commit to this repo with its own documentation and change to the src folder

# Overview of Daily Task 
- Learn (2h)
- Build (4h)
- Debug and Test (1.5h)
- Write-up (1h)

# Daily Documentation 
I make a documentation of my learnings per day and they are all made through excalidraw so to access them. Open them with excalidraw to properly see the documentation 

# Reference
- Test Driven Development: https://pragprog.com/titles/jgade/test-driven-development-for-embedded-c/

## To Compile
```
make
```

## To Flash to board
```
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c 'program app.elf verify reset exit'
```
