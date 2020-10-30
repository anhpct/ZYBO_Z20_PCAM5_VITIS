#
# This file was created by Xilinx Vitis tool.
#


#
# tools
#

RM = rm -f
RMDIR = rm -rf

#
# variables
#

SDCARD_GEN = sdcard_gen
GENERATE_BIF = ::scw::generate_bif

XPFM_PATH = C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/design_1_sys_wrapper/export/design_1_sys_wrapper/design_1_sys_wrapper.xpfm
SYS_CONFIG = design_1_sys_wrapper
DOMAINS = standalone_domain
BIF_PATH = C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/Practice_G_system/Debug/system.bif

BITSTREAM_PATH = C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/Practice_G/_ide/bitstream/design_1_sys_wrapper.bit
SD_FILES = --elf C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/Practice_G/Debug/Practice_G.elf,ps7_cortexa9_0

#
# primary build targets
#

all: sd_card

sd_card:
	-@echo Generating bif file for the system project
	generate_system_bif.bat 57719 ${XPFM_PATH} ${DOMAINS} ${BIF_PATH}
	${SDCARD_GEN} --xpfm ${XPFM_PATH} --sys_config ${SYS_CONFIG} --bif ${BIF_PATH} --bitstream ${BITSTREAM_PATH} ${SD_FILES}

clean:
	$(RMDIR) ./_sds
	${RMDIR} sd_card
	${RM} system.bif

.PHONY: all sd_card clean


