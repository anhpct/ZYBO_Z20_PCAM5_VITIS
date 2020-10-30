connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Zybo Z7 210351A822C1A" && level==0} -index 1
fpga -file C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/Practice_G/_ide/bitstream/design_1_sys_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/design_1_sys_wrapper/export/design_1_sys_wrapper/hw/design_1_sys_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/Practice_G/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/Users/HPZBOOK15G3/Desktop/FPGA/Study/4_Practice/4.6_Practice_G/Worksapce/Practice_G/Debug/Practice_G.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
