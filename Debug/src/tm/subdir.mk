################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/tm/tm_stm32f4_disco.c \
../src/tm/tm_stm32f4_dma.c \
../src/tm/tm_stm32f4_dma2d_graphic.c \
../src/tm/tm_stm32f4_fonts.c \
../src/tm/tm_stm32f4_gpio.c \
../src/tm/tm_stm32f4_i2c.c \
../src/tm/tm_stm32f4_ili9341_button.c \
../src/tm/tm_stm32f4_ili9341_ltdc.c \
../src/tm/tm_stm32f4_sdram.c \
../src/tm/tm_stm32f4_spi.c 

OBJS += \
./src/tm/tm_stm32f4_disco.o \
./src/tm/tm_stm32f4_dma.o \
./src/tm/tm_stm32f4_dma2d_graphic.o \
./src/tm/tm_stm32f4_fonts.o \
./src/tm/tm_stm32f4_gpio.o \
./src/tm/tm_stm32f4_i2c.o \
./src/tm/tm_stm32f4_ili9341_button.o \
./src/tm/tm_stm32f4_ili9341_ltdc.o \
./src/tm/tm_stm32f4_sdram.o \
./src/tm/tm_stm32f4_spi.o 

C_DEPS += \
./src/tm/tm_stm32f4_disco.d \
./src/tm/tm_stm32f4_dma.d \
./src/tm/tm_stm32f4_dma2d_graphic.d \
./src/tm/tm_stm32f4_fonts.d \
./src/tm/tm_stm32f4_gpio.d \
./src/tm/tm_stm32f4_i2c.d \
./src/tm/tm_stm32f4_ili9341_button.d \
./src/tm/tm_stm32f4_ili9341_ltdc.d \
./src/tm/tm_stm32f4_sdram.d \
./src/tm/tm_stm32f4_spi.d 


# Each subdirectory must supply rules for building sources it contributes
src/tm/%.o: ../src/tm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -ffunction-sections -Wall  -g -DARM_MATH_CM4 -DTM_DISCO_STM32F429_DISCOVERY -DSTM32F4XX -DSTM32F429xx -DUSE_STDPERIPH_DRIVER -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\ra8875" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\si570" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\button" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\codec" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\encoder" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\i2c" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\inc" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\system\include\stm32f4" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\tm" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\ui" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\si5351" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


