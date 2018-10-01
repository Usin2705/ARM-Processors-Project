################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: XYPlotterLPC1549.axf

# Tool invocations
XYPlotterLPC1549.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU C++ Linker'
	arm-none-eabi-c++ -nostdlib -L"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_board_nxp_lpcxpresso_1549\Debug" -L"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_chip_15xx\Debug" -L"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\Debug" -Xlinker -Map="XYPlotterLPC1549.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m3 -mthumb -T "XYPlotterLPC1549_Debug.ld" -o "XYPlotterLPC1549.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) XYPlotterLPC1549.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "XYPlotterLPC1549.axf"; # arm-none-eabi-objcopy -v -O binary "XYPlotterLPC1549.axf" "XYPlotterLPC1549.bin" ; # checksum -p LPC1549 -d "XYPlotterLPC1549.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
