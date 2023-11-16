cmake_minimum_required(VERSION 3.15.3)

#check path
set(SDK_ROOT_PATH "${CMAKE_SOURCE_DIR}/../../../.." CACHE STRING "SDK root path")

if (NOT EXISTS "${SDK_ROOT_PATH}/Library/RT58x")
    message(SEND ERROR "You should Give SDK directory location by -DSDK_ROOT_PATH=....")
endif()

set(TOOL_SCRIPT_PATH "${SDK_ROOT_PATH}/Library/RT58x/Device/GCC/")

set(CMAKE_TOOLCHAIN_FILE "${TOOL_SCRIPT_PATH}/arm-none-eabi-gcc.cmake" CACHE STRING "ARM Cortex M3 toolchain for cmake setting path")    


set(PROJECT_NAME "UART")

project(${PROJECT_NAME} C ASM)

message("SOURCE DIR: ${CMAKE_SOURCE_DIR}")

set(MCU_BASE_LIBRARY_PATH "${SDK_ROOT_PATH}/Library/RT58x/")

set(COMPILER_SETTING_PATH "${TOOL_SCRIPT_PATH}")

set(DRIVE_SOURCE_PATH "${MCU_BASE_LIBRARY_PATH}/Peripheral/")
set(CRYPTO_SOURCE_PATH "${MCU_BASE_LIBRARY_PATH}/Crypto")
set(PHY_SOURCE_PATH "${MCU_BASE_LIBRARY_PATH}/PHY")

if (EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message("FIND arm-none-eabi-gcc.cmake")
else()
    message("what bad OOPS")
endif()


#if no special defined lds, we use default lds
#set(PROJECT_LD "${TOOL_SCRIPT_PATH}/default_gcc_cm3_mcu.ld" CACHE STRING "Project ld setting")

set(PROJECT_LD "default_gcc_cm3_mcu.ld" CACHE STRING "Project lds setting filename")


#check use default project lds or project special lds
if (NOT ${PROJECT_LD} STREQUAL "default_gcc_cm3_mcu.ld")
	set(LD_PATH "${CMAKE_SOURCE_DIR}")
else()
	set(LD_PATH  "${TOOL_SCRIPT_PATH}")
endif()	


if (EXISTS "${LD_PATH}/${PROJECT_LD}")
	message("FIND PROJECT_LD ${LD_PATH}/${PROJECT_LD}")
else()
	message("Can not find PROJECT_LD ${LD_PATH}/${PROJECT_LD}")
endif()	

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/..")

#Here We assume source code is in the same sdk directory.

set(SRCS
    ${SOURCE_DIR}/main.c       
    ${TOOL_SCRIPT_PATH}/gcc_startup_cm3_mcu.S
    ${TOOL_SCRIPT_PATH}/../system_cm3_mcu.c
    ${DRIVE_SOURCE_PATH}/uart_drv.c
    ${DRIVE_SOURCE_PATH}/retarget_drv_keil.c
    ${DRIVE_SOURCE_PATH}/flashctl.c
    ${DRIVE_SOURCE_PATH}/gpio.c
    ${DRIVE_SOURCE_PATH}/sysfun.c
    ${DRIVE_SOURCE_PATH}/sysctrl.c
    ${DRIVE_SOURCE_PATH}/dma.c
    ${DRIVE_SOURCE_PATH}/timer.c
    )   
        

set(EXECUTABLE ${PROJECT_NAME}.out)

add_executable(${EXECUTABLE} "${SRCS}")

#Reserved for Future used
#target_compile_definitions(${EXECUTABLE} PRIVATE
#        -D
#        -D
#        )

target_include_directories(${EXECUTABLE} PUBLIC
    "${SOURCE_DIR}/config"
    "${SDK_ROOT_PATH}/Library/CMSIS/Include"
    "${MCU_BASE_LIBRARY_PATH}/include"
    "${MCU_BASE_LIBRARY_PATH}/Peripheral/Include"
    "${MCU_BASE_LIBRARY_PATH}/PHY/include"
    "${MCU_BASE_LIBRARY_PATH}/CRYPTO/include"
    )

target_compile_options(${EXECUTABLE} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -mabi=aapcs
    -O3
    -fdata-sections
    -ffunction-sections
    -fno-builtin
    -fno-strict-aliasing
    -fshort-enums
    -Wall
    )

target_link_options(${EXECUTABLE} PRIVATE
    -L "${LD_PATH}"
    -T "${PROJECT_LD}"
    -mcpu=cortex-m3
    -mthumb
    -mabi=aapcs
    -specs=nano.specs
    -lc
    -lm
    -lnosys
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    )

# Print executable size
add_custom_command(TARGET ${EXECUTABLE}
    POST_BUILD
    COMMAND arm-none-eabi-size ${EXECUTABLE})

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
    POST_BUILD
    COMMAND arm-none-eabi-objdump -S ${EXECUTABLE} > ${PROJECT_NAME}.lst
    COMMAND arm-none-eabi-objcopy -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
    COMMAND arm-none-eabi-objcopy -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)

