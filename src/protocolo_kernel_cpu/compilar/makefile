################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
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
all: libprotocolo_kernel_cpu.so

dependents:
	-cd ../../commons/compilar && $(MAKE) all
	-cd ../../parser/compilar && $(MAKE) all
	-cd ../../msgUMV/compilar && $(MAKE) all

# Tool invocations
libprotocolo_kernel_cpu.so: $(OBJS) $(USER_OBJS) ../../commons/compilar/libcommons.so ../../parser/compilar/libparser.so ../../msgUMV/compilar/libmsgUMV.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"../../msgUMV/compilar" -L"../../parser/compilar" -L"../../commons/compilar" -shared -o "libprotocolo_kernel_cpu.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libprotocolo_kernel_cpu.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
../../commons/compilar/libcommons.so:
../../parser/compilar/libparser.so:
../../msgUMV/compilar/libmsgUMV.so:

-include ../makefile.targets
