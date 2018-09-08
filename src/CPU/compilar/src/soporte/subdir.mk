################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/soporte/arch_confg_cpu.c 

OBJS += \
./src/soporte/arch_confg_cpu.o 

C_DEPS += \
./src/soporte/arch_confg_cpu.d 


# Each subdirectory must supply rules for building sources it contributes
src/soporte/%.o: ../src/soporte/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../protocolo_kernel_cpu" -I"../../msgUMV" -I"../../parser" -I"../../commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


