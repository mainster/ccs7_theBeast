################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
DCL/DCL_PID_C4.obj: ../DCL/DCL_PID_C4.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -O0 --opt_for_speed=0 --fp_mode=relaxed --include_path="/opt/ti/c2000ware/libraries/control/DCL/c28/include" --include_path="/opt/ti/c2000ware/device_support/f2837xd/common/include" --include_path="/opt/ti/c2000ware/device_support/f2837xd/headers/include" --include_path="/opt/ti/c2000ware/driverlib/f2837xd/driverlib" --include_path="/home/mainster/CODES_local/ccs7_theBeast/06_adcEpwmSoc_dclClaPid" --include_path="/home/mainster/CODES_local/ccs7_theBeast/06_adcEpwmSoc_dclClaPid/device" --include_path="/inc" --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.1.LTS/include" --advice:performance=all --define=DEBUG --define=CPU1 --undefine=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="DCL/DCL_PID_C4.d" --obj_directory="DCL" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

DCL/DCL_clamp_C1.obj: ../DCL/DCL_clamp_C1.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -O0 --opt_for_speed=0 --fp_mode=relaxed --include_path="/opt/ti/c2000ware/libraries/control/DCL/c28/include" --include_path="/opt/ti/c2000ware/device_support/f2837xd/common/include" --include_path="/opt/ti/c2000ware/device_support/f2837xd/headers/include" --include_path="/opt/ti/c2000ware/driverlib/f2837xd/driverlib" --include_path="/home/mainster/CODES_local/ccs7_theBeast/06_adcEpwmSoc_dclClaPid" --include_path="/home/mainster/CODES_local/ccs7_theBeast/06_adcEpwmSoc_dclClaPid/device" --include_path="/inc" --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.1.LTS/include" --advice:performance=all --define=DEBUG --define=CPU1 --undefine=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="DCL/DCL_clamp_C1.d" --obj_directory="DCL" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


