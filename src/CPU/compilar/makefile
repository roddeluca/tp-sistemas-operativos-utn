################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/soporte/subdir.mk
-include src/funcionesAnsisop/subdir.mk
-include src/conexiones/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CPU

dependents:
	-cd ../../commons/compilar && $(MAKE) clean all
	-cd ../../parser/compilar && $(MAKE) clean all
	-cd ../../msgUMV/compilar && $(MAKE) clean all
	-cd ../../protocolo_kernel_cpu/compilar && $(MAKE) clean all

# Tool invocations
CPU: $(OBJS) $(USER_OBJS) ../../commons/compilar/libcommons.so ../../parser/compilar/libparser.so ../../msgUMV/compilar/libmsgUMV.so ../../protocolo_kernel_cpu/compilar/libprotocolo_kernel_cpu.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"../../protocolo_kernel_cpu/compilar" -L"../../msgUMV/compilar" -L"../../parser/compilar" -L"../../commons/compilar" -o "CPU" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) CPU
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
../../commons/compilar/libcommons.so:
../../parser/compilar/libparser.so:
../../msgUMV/compilar/libmsgUMV.so:
../../protocolo_kernel_cpu/compilar/libprotocolo_kernel_cpu.so:

-include ../makefile.targets
