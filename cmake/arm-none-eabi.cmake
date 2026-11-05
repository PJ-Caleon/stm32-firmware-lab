cmake_minimum_required(VERSION 3.21)

# System name to Generic 
project(GENERIC
  VERSION 1.0.0
  DESCRIPTION "STM32 Firmware"
)

enable_language(C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_EXTENSIONS OFF)

set(MCU_OPTIONS
  -mcpu=cortex-m3
  -mthumb
)

set(EXTRA_OPTIONS
  -fdata-sections
  -ffunction-sections
)

set(OPTIMIZATION_OPTIONS
  $<$<CONFIG:Debug>:-Og>
)


set(DEPENDENCY_INFO_OPTIONS
  -MMD 
  -MP 
  -MF "$(@:%.o=%.d)"
)

set(DEBUG_INFO_OPTIONS
  -g3
  -gdwarf-2
)

add_compile_options(
  ${MCU_OPTIONS}
  ${EXTRA_OPTIONS}
  ${DEBUG_INFO_OPTIONS}
  ${OPTIMIZATION_OPTIONS}
)

add_link_options(
  ${MCU_OPTIONS}
  -specs=nano.spec
  -T${CMAKE_SOURCE_DIR}/STM32F103C6TX_FLASH.ld 
  -wl,-Map=${PROJECT_NAME}.map,--cref 
  -wl,--gc-sections 
)

link_libraries("-lc -lm -lnosys")

add_subdirectory(Drivers)

target_include_directories(Drivers
  PRIVATE 
    Core/Inc 
)

set(EXECUTABLE ${PROJECT_NAME}.elf)

add_executable(${EXECUTBALE}
  ../src/app/*
  ../src/core/ 
  ../src/drivers/ 
)

target_include_directories(${EXECUTABLE}
  PRIVATE 
    Core/Inc 
)

target_compile_options(${EXECUTABLE}
  PRIVATE
    -Wall 
    -Wextra 
    -Wshadow 
    -Wconversion 
    -Wdouble-promotion 
)

target_link_libraries(${EXECUTABLE}
  PRIVATE
    Drivers
)

size(TARGET ${EXECUTABLE}
  POST_BUILD 
  COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE}
)

generate(TARGET ${EXECUTABLE}
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin
)
