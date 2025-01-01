################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ADC.c \
../Core/Src/DAC.c \
../Core/Src/DAC1_CH1.c \
../Core/Src/DMA.c \
../Core/Src/Delay.c \
../Core/Src/DelayFilter.c \
../Core/Src/HPF.c \
../Core/Src/Keypad.c \
../Core/Src/LED_Debug.c \
../Core/Src/LPF.c \
../Core/Src/NoiseGate.c \
../Core/Src/TIM.c \
../Core/Src/main.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c 

OBJS += \
./Core/Src/ADC.o \
./Core/Src/DAC.o \
./Core/Src/DAC1_CH1.o \
./Core/Src/DMA.o \
./Core/Src/Delay.o \
./Core/Src/DelayFilter.o \
./Core/Src/HPF.o \
./Core/Src/Keypad.o \
./Core/Src/LED_Debug.o \
./Core/Src/LPF.o \
./Core/Src/NoiseGate.o \
./Core/Src/TIM.o \
./Core/Src/main.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o 

C_DEPS += \
./Core/Src/ADC.d \
./Core/Src/DAC.d \
./Core/Src/DAC1_CH1.d \
./Core/Src/DMA.d \
./Core/Src/Delay.d \
./Core/Src/DelayFilter.d \
./Core/Src/HPF.d \
./Core/Src/Keypad.d \
./Core/Src/LED_Debug.d \
./Core/Src/LPF.d \
./Core/Src/NoiseGate.d \
./Core/Src/TIM.d \
./Core/Src/main.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ADC.cyclo ./Core/Src/ADC.d ./Core/Src/ADC.o ./Core/Src/ADC.su ./Core/Src/DAC.cyclo ./Core/Src/DAC.d ./Core/Src/DAC.o ./Core/Src/DAC.su ./Core/Src/DAC1_CH1.cyclo ./Core/Src/DAC1_CH1.d ./Core/Src/DAC1_CH1.o ./Core/Src/DAC1_CH1.su ./Core/Src/DMA.cyclo ./Core/Src/DMA.d ./Core/Src/DMA.o ./Core/Src/DMA.su ./Core/Src/Delay.cyclo ./Core/Src/Delay.d ./Core/Src/Delay.o ./Core/Src/Delay.su ./Core/Src/DelayFilter.cyclo ./Core/Src/DelayFilter.d ./Core/Src/DelayFilter.o ./Core/Src/DelayFilter.su ./Core/Src/HPF.cyclo ./Core/Src/HPF.d ./Core/Src/HPF.o ./Core/Src/HPF.su ./Core/Src/Keypad.cyclo ./Core/Src/Keypad.d ./Core/Src/Keypad.o ./Core/Src/Keypad.su ./Core/Src/LED_Debug.cyclo ./Core/Src/LED_Debug.d ./Core/Src/LED_Debug.o ./Core/Src/LED_Debug.su ./Core/Src/LPF.cyclo ./Core/Src/LPF.d ./Core/Src/LPF.o ./Core/Src/LPF.su ./Core/Src/NoiseGate.cyclo ./Core/Src/NoiseGate.d ./Core/Src/NoiseGate.o ./Core/Src/NoiseGate.su ./Core/Src/TIM.cyclo ./Core/Src/TIM.d ./Core/Src/TIM.o ./Core/Src/TIM.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su

.PHONY: clean-Core-2f-Src

