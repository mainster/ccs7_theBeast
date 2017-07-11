################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
F2837xD_Adc.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_Adc.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_Adc.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_CodeStartBranch.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_CodeStartBranch.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_CodeStartBranch.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_DefaultISR.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_DefaultISR.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_DefaultISR.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_EPwm.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_EPwm.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_EPwm.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_GlobalVariableDefs.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/source/F2837xD_GlobalVariableDefs.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_GlobalVariableDefs.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_Gpio.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_Gpio.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_Gpio.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_PieCtrl.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_PieCtrl.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_PieCtrl.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_PieVect.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_PieVect.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_PieVect.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_SysCtrl.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_SysCtrl.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_SysCtrl.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

F2837xD_usDelay.obj: /opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/source/F2837xD_usDelay.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="F2837xD_usDelay.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

examples_setup.obj: /opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common/examples_setup.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="examples_setup.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-c2000_17.3.0.STS/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_common/include" --include_path="/opt/ti/controlSUITE/device_support/F2837xD/v150/F2837xD_headers/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/include" --include_path="/opt/ti/controlSUITE/libs/dsp/FPU/v1_50_00_00/examples/common" --define=CPU1 --define=ADCA -g --diag_warning=225 --preproc_with_compile --preproc_dependency="main.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


