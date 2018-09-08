################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/soporte/ConexionConCPU.c \
../src/soporte/ConexionConUMV.c 

OBJS += \
./src/soporte/ConexionConCPU.o \
./src/soporte/ConexionConUMV.o 

C_DEPS += \
./src/soporte/ConexionConCPU.d \
./src/soporte/ConexionConUMV.d 


# Each subdirectory must supply rules for building sources it contributes
src/soporte/%.o: ../src/soporte/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../protocolo_kernel_cpu" -I"../../msgUMV" -I"../../parser" -I"../../commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


