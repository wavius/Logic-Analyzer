# Generates the DE1-SoC ARM/Nios II Computer

###############################################################################
# Add components to the system
#
# Add clocks
ifup_add_system_clock_source	100000000
ifup_add_vga_clock_source		25000000
#
# Add processors
ifup_add_arm_a9_hps
ifup_add_niosII		 Nios2			"SDRAM.s1" 0x00000000 "SDRAM.s1" 0x00000020 0 1 $ifup_flag_nios2_0_data_master $ifup_flag_nios2_0_instruction_master $ifup_flag_nios2_0_irq_receiver
ifup_add_niosII		 Nios2_2nd_Core	"SDRAM.s1" 0x02000000 "SDRAM.s1" 0x02000020 1 1 $ifup_flag_nios2_1_data_master $ifup_flag_nios2_1_instruction_master $ifup_flag_nios2_1_irq_receiver
#
# Add bridges
ifup_add_jtag_to_hps_bridge
ifup_add_jtag_to_fpga_bridge
#
# Add address span extenders
ifup_add_address_span_extender	F2H_Mem_Window_00000000	0x00000000	28	0x40000000
#
# Add memories
ifup_add_sdram_64mb
ifup_add_onchip_memory	Onchip_SRAM 262144 0x08000000
#
# Add gpio
ifup_add_gpio	LEDs			leds			Output	false	RISING  10 0xFF200000 -1
ifup_add_gpio	HEX3_HEX0		hex3_hex0		Output	false	RISING  32 0xFF200020 -1
ifup_add_gpio	HEX5_HEX4		hex5_hex4		Output	false	RISING  16 0xFF200030 -1
ifup_add_gpio	Slider_Switches	slider_switches	Input	false	RISING  10 0xFF200040 -1
ifup_add_gpio	Pushbuttons		pushbuttons		Input	true	FALLING  4 0xFF200050  1
ifup_add_gpio	Expansion_JP1	expansion_jp1	Bidir	true	FALLING 32 0xFF200060 11
ifup_add_gpio	Expansion_JP2	expansion_jp2	Bidir	true	FALLING 32 0xFF200070 12
ifup_add_ps2	PS2_Port		ps2_port		0xFF200100 7
ifup_add_ps2	PS2_Port_Dual	ps2_port_dual	0xFF200108 23
#
# Add communications
ifup_add_jtag_uart	JTAG_UART			0xFF201000 [ expr $ifup_flag_nios2_0_data_master | $ifup_flag_jtag_to_fpga_bridge ] 8 $ifup_flag_nios2_0_irq_receiver
ifup_add_jtag_uart	JTAG_UART_2nd_Core	0xFF201000 $ifup_flag_nios2_1_data_master 8 $ifup_flag_nios2_1_irq_receiver
ifup_add_jtag_uart	JTAG_UART_for_ARM_0	0xFF201000 $ifup_flag_hps_h2f_lw_master 8 $ifup_flag_hps_arm_0_irq_receiver
ifup_add_jtag_uart	JTAG_UART_for_ARM_1	0xFF201008 $ifup_flag_hps_h2f_lw_master 8 $ifup_flag_hps_arm_1_irq_receiver
ifup_add_irda
#
# Add timers
ifup_add_interval_timer	Interval_Timer				0xFF202000 [ expr $ifup_flag_nios2_0_data_master | $ifup_flag_jtag_to_fpga_bridge | $ifup_flag_hps_h2f_lw_master ] 0 [ expr $ifup_flag_nios2_0_irq_receiver | $ifup_flag_hps_arm_0_irq_receiver ]
ifup_add_interval_timer	Interval_Timer_2			0xFF202020 [ expr $ifup_flag_nios2_0_data_master | $ifup_flag_jtag_to_fpga_bridge | $ifup_flag_hps_h2f_lw_master ] 2 [ expr $ifup_flag_nios2_0_irq_receiver | $ifup_flag_hps_arm_0_irq_receiver ]
#ifup_add_interval_timer	Interval_Timer				0xFF202000 [ expr $ifup_flag_nios2_0_data_master | $ifup_flag_jtag_to_fpga_bridge ] 0 $ifup_flag_nios2_0_irq_receiver
#ifup_add_interval_timer	Interval_Timer_2			0xFF202020 [ expr $ifup_flag_nios2_0_data_master | $ifup_flag_jtag_to_fpga_bridge ] 2 $ifup_flag_nios2_0_irq_receiver
ifup_add_interval_timer	Interval_Timer_2nd_Core		0xFF202000 $ifup_flag_nios2_1_data_master 0 $ifup_flag_nios2_1_irq_receiver
ifup_add_interval_timer	Interval_Timer_2nd_Core_2	0xFF202020 $ifup_flag_nios2_1_data_master 2 $ifup_flag_nios2_1_irq_receiver
#ifup_add_interval_timer	Interval_Timer_hps				0xFF202000 $ifup_flag_hps_h2f_lw_master 0 $ifup_flag_hps_arm_0_irq_receiver
#ifup_add_interval_timer	Interval_Timer_hps_2			0xFF202020 $ifup_flag_hps_h2f_lw_master 2 $ifup_flag_hps_arm_0_irq_receiver
#
# Add sys id
ifup_add_system_id
#
# Add av cores
ifup_add_av_config
ifup_add_adc 12.5
ifup_add_dma_address_translation	Pixel_DMA_Addr_Translation		0xFF203020 $ifup_flag_video_out_dma_addr_translator
ifup_add_dma_address_translation	Char_DMA_Addr_Translation		0xFF203030 $ifup_flag_char_buf_dma_addr_translator
ifup_add_vga_subsystem
ifup_add_audio_subsystem	6
ifup_add_dma_address_translation	Video_In_DMA_Addr_Translation	0xFF203060 $ifup_flag_video_in_dma_addr_translator
ifup_add_video_in_subsystem
#
# Add address span extenders
ifup_add_address_span_extender	F2H_Mem_Window_FF600000	0xFF600000	19	0xFF600000
ifup_add_address_span_extender	F2H_Mem_Window_FF800000	0xFF800000	21	0xFF800000
###############################################################################


###############################################################################
# Modify component parameters that are specific to this system
#
# Specific parameter setting for the HPS component for the DE1-SoC board
set_instance_parameter_value ARM_A9_HPS GPIO_Enable [list No No No No No No No No No Yes No No No No No No No No No No No No No No No No No No No No No No No No No Yes No No No No Yes Yes No No No No No No Yes No No No No Yes Yes No No No No No No Yes No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No No]
set_instance_parameter_value ARM_A9_HPS MEM_TCL								7
set_instance_parameter_value ARM_A9_HPS MEM_TFAW_NS							45.0
set_instance_parameter_value ARM_A9_HPS MEM_TRAS_NS							36.0
set_instance_parameter_value ARM_A9_HPS MEM_TRCD_NS							13.125
set_instance_parameter_value ARM_A9_HPS MEM_TRFC_NS							300.0
set_instance_parameter_value ARM_A9_HPS MEM_TRP_NS							13.125
set_instance_parameter_value ARM_A9_HPS MEM_VENDOR							"Micron"
set_instance_parameter_value ARM_A9_HPS QSPI_Mode							"1 SS"
set_instance_parameter_value ARM_A9_HPS QSPI_PinMuxing						"HPS I/O Set 0"
set_instance_parameter_value ARM_A9_HPS S2F_Width							3
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_AC_SKEW				0.02
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_AC_TO_CK_SKEW			0.01
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_DQ_TO_DQS_SKEW			0.05
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_MAX_CK_DELAY			0.03
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_MAX_DQS_DELAY			0.02
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_SKEW_BETWEEN_DIMMS		0.05
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_SKEW_BETWEEN_DQS		0.06
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_SKEW_CKDQS_DIMM_MAX	0.12
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_SKEW_CKDQS_DIMM_MIN	0.06
set_instance_parameter_value ARM_A9_HPS TIMING_BOARD_SKEW_WITHIN_DQS		0.01
set_instance_parameter_value ARM_A9_HPS TIMING_TDH							65
set_instance_parameter_value ARM_A9_HPS TIMING_TDQSCK						255
set_instance_parameter_value ARM_A9_HPS TIMING_TDQSQ						125
set_instance_parameter_value ARM_A9_HPS TIMING_TDS							30
set_instance_parameter_value ARM_A9_HPS TIMING_TIH							140
set_instance_parameter_value ARM_A9_HPS TIMING_TIS							190
set_instance_parameter_value ARM_A9_HPS TIMING_TQSH							0.4
###############################################################################

