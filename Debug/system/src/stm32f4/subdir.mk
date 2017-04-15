################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/stm32f4/misc.c \
../system/src/stm32f4/stm32f4xx_adc.c \
../system/src/stm32f4/stm32f4xx_dma.c \
../system/src/stm32f4/stm32f4xx_dma2d.c \
../system/src/stm32f4/stm32f4xx_fmc.c \
../system/src/stm32f4/stm32f4xx_gpio.c \
../system/src/stm32f4/stm32f4xx_i2c.c \
../system/src/stm32f4/stm32f4xx_ltdc.c \
../system/src/stm32f4/stm32f4xx_rcc.c \
../system/src/stm32f4/stm32f4xx_spi.c \
../system/src/stm32f4/stm32f4xx_tim.c \
../system/src/stm32f4/stm32f4xx_usart.c 

OBJS += \
./system/src/stm32f4/misc.o \
./system/src/stm32f4/stm32f4xx_adc.o \
./system/src/stm32f4/stm32f4xx_dma.o \
./system/src/stm32f4/stm32f4xx_dma2d.o \
./system/src/stm32f4/stm32f4xx_fmc.o \
./system/src/stm32f4/stm32f4xx_gpio.o \
./system/src/stm32f4/stm32f4xx_i2c.o \
./system/src/stm32f4/stm32f4xx_ltdc.o \
./system/src/stm32f4/stm32f4xx_rcc.o \
./system/src/stm32f4/stm32f4xx_spi.o \
./system/src/stm32f4/stm32f4xx_tim.o \
./system/src/stm32f4/stm32f4xx_usart.o 

C_DEPS += \
./system/src/stm32f4/misc.d \
./system/src/stm32f4/stm32f4xx_adc.d \
./system/src/stm32f4/stm32f4xx_dma.d \
./system/src/stm32f4/stm32f4xx_dma2d.d \
./system/src/stm32f4/stm32f4xx_fmc.d \
./system/src/stm32f4/stm32f4xx_gpio.d \
./system/src/stm32f4/stm32f4xx_i2c.d \
./system/src/stm32f4/stm32f4xx_ltdc.d \
./system/src/stm32f4/stm32f4xx_rcc.d \
./system/src/stm32f4/stm32f4xx_spi.d \
./system/src/stm32f4/stm32f4xx_tim.d \
./system/src/stm32f4/stm32f4xx_usart.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/stm32f4/%.o: ../system/src/stm32f4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -ffunction-sections -Wall  -g -DARM_MATH_CM4 -DTM_DISCO_STM32F429_DISCOVERY -DSTM32F4XX -DSTM32F429xx -DUSE_STDPERIPH_DRIVER -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\ra8875" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\si570" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\button" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\codec" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\encoder" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\i2c" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\inc" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\system\include\stm32f4" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\tm" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\ui" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\si5351" -std=gnu11 -Wno-bad-function-cast -Wno-conversion -Wno-sign-conversion -Wno-unused-parameter -Wno-sign-compare -Wno-missing-prototypes -Wno-missing-declarations -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


