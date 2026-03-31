# This Makefile generates and compiles the DE1-SoC Nios V/m Computer
# To be run in the Nios II Command shell

TCLSRCS = gen_audio_subsystem.tcl gen_char_buf_subsystem.tcl gen_vga_subsystem.tcl gen_video_in_edge_detection_subsystem.tcl gen_video_in_subsystem.tcl gen_niosvm_computer_system.tcl
QSYSSRC = Computer_System.qsys
QP_NAME = DE1_SoC_NiosVm_Computer
GEN_RBF = 0

include paths.mk

