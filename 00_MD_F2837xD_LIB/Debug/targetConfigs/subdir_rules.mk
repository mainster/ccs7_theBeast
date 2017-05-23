################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
targetConfigs/md_gpio.obj: ../targetConfigs/md_gpio.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/media/data/CODES/ccs7_theBeast/MD_F2837xD_LIB" --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="targetConfigs/md_gpio.d" --obj_directory="targetConfigs" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


