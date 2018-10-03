################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DigitalIoPin.cpp \
../src/Fmutex.cpp \
../src/Parser.cpp \
../src/XYPlotterLPC1549_new.cpp \
../src/cr_cpp_config.cpp \
../src/cr_startup_lpc15xx.cpp 

C_SRCS += \
../src/ITM_write.c \
../src/cdc_desc.c \
../src/cdc_main.c \
../src/cdc_vcom.c \
../src/crp.c \
../src/sysinit.c 

OBJS += \
./src/DigitalIoPin.o \
./src/Fmutex.o \
./src/ITM_write.o \
./src/Parser.o \
./src/XYPlotterLPC1549_new.o \
./src/cdc_desc.o \
./src/cdc_main.o \
./src/cdc_vcom.o \
./src/cr_cpp_config.o \
./src/cr_startup_lpc15xx.o \
./src/crp.o \
./src/sysinit.o 

CPP_DEPS += \
./src/DigitalIoPin.d \
./src/Fmutex.d \
./src/Parser.d \
./src/XYPlotterLPC1549_new.d \
./src/cr_cpp_config.d \
./src/cr_startup_lpc15xx.d 

C_DEPS += \
./src/ITM_write.d \
./src/cdc_desc.d \
./src/cdc_main.d \
./src/cdc_vcom.d \
./src/crp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -std=c++11 -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\XYPlotterLPC1549\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_board_nxp_lpcxpresso_1549\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_chip_15xx\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_chip_15xx\inc\usbd" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\src\include" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\src\portable\GCC\ARM_CM3" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_chip_15xx\inc\usbd" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\XYPlotterLPC1549\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_board_nxp_lpcxpresso_1549\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_chip_15xx\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\lpc_chip_15xx\inc\usbd" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\inc" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\src\include" -I"C:\Users\pavel\Desktop\koulu\Emdedded_Systems_Project\Plotter_Project\sorsa\FreeRTOS\src\portable\GCC\ARM_CM3" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


