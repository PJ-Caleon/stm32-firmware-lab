cmake_minimum_required(VERSION 3.21)

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

    file(GLOB TEST_SOURCES 
      src/test/*.c
    )

    add_executable(unit_tests
      ${TEST_SOURCES}
    )

    target_include_directories(unit_tests PRIVATE
      src/test
    )

    add_test(NAME RunUnitTests COMMAND unit_tests)
endif()
