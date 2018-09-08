################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/pcp/PlanificadorPCP.c \
../src/pcp/PlanificadorPCP_th.c 

OBJS += \
./src/pcp/PlanificadorPCP.o \
./src/pcp/PlanificadorPCP_th.o 

C_DEPS += \
./src/pcp/PlanificadorPCP.d \
./src/pcp/PlanificadorPCP_th.d 


# Each subdirectory must supply rules for building sources it contributes
src/pcp/%.o: ../src/pcp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../protocolo_kernel_cpu" -I"../../msgUMV" -I"../../parser" -I"../../commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


