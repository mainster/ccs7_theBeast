################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./src/md_epwm.obj" \
"./src/md_gpio.obj" \
"./src/md_helpers.obj" \
"./src/md_sci.obj" \
"./targetConfigs/md_gpio.obj" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir_vars.mk
-include targetConfigs/subdir_vars.mk
-include src/subdir_rules.mk
-include targetConfigs/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
00_MD_F2837xD_LIB.out \

EXE_OUTPUTS__QUOTED += \
"00_MD_F2837xD_LIB.out" \

BIN_OUTPUTS += \
00_MD_F2837xD_LIB.hex \

BIN_OUTPUTS__QUOTED += \
"00_MD_F2837xD_LIB.hex" \


# All Target
all: $(OBJS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "00_MD_F2837xD_LIB.out"

# Tool invocations
00_MD_F2837xD_LIB.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C2000 Linker'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -g --diag_warning=225 --diag_wrap=off --display_error_number -z -m"00_MD_F2837xD_LIB.map" --stack_size=0x200 --warn_sections -i"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/lib" -i"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --reread_libs --diag_wrap=off --display_error_number --xml_link_info="00_MD_F2837xD_LIB_linkInfo.xml" --rom_model -o "00_MD_F2837xD_LIB.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

00_MD_F2837xD_LIB.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: C2000 Hex Utility'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/hex2000"  -o "00_MD_F2837xD_LIB.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "src/md_epwm.obj" "src/md_gpio.obj" "src/md_helpers.obj" "src/md_sci.obj" "targetConfigs/md_gpio.obj" 
	-$(RM) "src/md_epwm.d" "src/md_gpio.d" "src/md_helpers.d" "src/md_sci.d" "targetConfigs/md_gpio.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

