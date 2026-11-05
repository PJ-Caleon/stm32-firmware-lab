cmake_minimum_required(VERSION 3.21)

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(stm32-firmware-lab LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Cross compiling 
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    set(MCU_OPTIONS -mcpu=cortex-m3 -mthumb)

    file(GLOB_RECURSE FIRMWARE_SOURCES 
        src/app/*.c
        src/core/*.c
        src/drivers/*.c
    )

    add_executable(firmware.elf ${FIRMWARE_SOURCES})

    target_include_directories(firmware.elf PRIVATE
      src/app 
      src/core 
      src/drivers
    )

    target_compile_options(firmware.elf PRIVATE
      ${MCU_OPTIONS}
      -Wall 
      -Wextra 
      -g 
      -Wno-conversion
      -Wno-sign-conversion 
      -Wno-shadow
    )

    target_link_options(firmware.elf PRIVATE
        ${MCU_OPTIONS}
        -specs=nano.specs
        -specs=nosys.specs
        -T${CMAKE_SOURCE_DIR}/src/link.ld
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
    )

    # Post-build size and hex generation
    add_custom_command(TARGET firmware.elf POST_BUILD
        COMMAND arm-none-eabi-size firmware.elf
        COMMAND arm-none-eabi-objcopy -O ihex firmware.elf firmware.hex
        COMMAND arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
        COMMENT "Generating firmware binaries..."
    )

else()
    message(STATUS "Configuring for Host Test Target...")
    enable_testing()

    set(UNITY_DIR "src/test/Unity/src")

    function(add_stm32_test TEST_NAME TEST_SOURCE EXTRA_SOURCES)
        add_executable(${TEST_NAME}
            ${TEST_SOURCE}
            ${UNITY_DIR}/unity.c
            ${EXTRA_SOURCES}
        )

        target_include_directories(${TEST_NAME} PRIVATE
            ${UNITY_DIR}
            src/test
            src/app
            src/core
            src/drivers
        )

        target_compile_options(${TEST_NAME} PRIVATE
            -Wall -Wextra -g 
            -fsanitize=address -fsanitize=undefined 
            -fno-omit-frame-pointer
            -Wno-conversion -Wno-sign-conversion -Wno-shadow
        )

        target_link_options(${TEST_NAME} PRIVATE
            -fsanitize=address -fsanitize=undefined
        )

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endfunction()
    
    # Define each test uniquely with all required source dependencies
    add_stm32_test(test_clock src/test/test_clock.c "src/drivers/clock.c")
    add_stm32_test(test_crc src/test/test_crc.c "src/core/crc.c")
    add_stm32_test(test_oled src/test/test_oled.c "src/app/oled.c;src/app/oled_frame.c")
    add_stm32_test(test_potentiometer src/test/test_potentiometer.c "src/drivers/adc.c;src/app/potentiometer.c;src/drivers/dma.c;src/drivers/clock.c")
    add_stm32_test(test_ring_buffer src/test/test_ring_buffer.c "src/core/ring_buffer.c")
    add_stm32_test(test_shell src/test/test_shell.c "src/app/shell.c")
    add_stm32_test(test_uart src/test/test_uart.c "src/drivers/uart.c")

endif()
