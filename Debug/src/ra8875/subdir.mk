################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ra8875/gsl1680.c \
../src/ra8875/ui_ra8875.c \
../src/ra8875/ui_ra8875_fonts.c 

OBJS += \
./src/ra8875/gsl1680.o \
./src/ra8875/ui_ra8875.o \
./src/ra8875/ui_ra8875_fonts.o 

C_DEPS += \
./src/ra8875/gsl1680.d \
./src/ra8875/ui_ra8875.d \
./src/ra8875/ui_ra8875_fonts.d 


# Each subdirectory must supply rules for building sources it contributes
src/ra8875/%.o: ../src/ra8875/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -ffunction-sections -Wall  -g -DARM_MATH_CM4 -DTM_DISCO_STM32F429_DISCOVERY -DSTM32F4XX -DSTM32F429xx -DUSE_STDPERIPH_DRIVER -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\ra8875" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\si570" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\button" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\codec" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\encoder" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\i2c" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\inc" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\system\include\stm32f4" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\tm" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\ui" -I"C:\Users\gmtii\workspace\disco-noFB-noUGUI\src\si5351" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


