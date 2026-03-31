# Generates the DE1-SoC Nios V/m Computer

###############################################################################
# Add components to the system
#
# Add clocks
ifup_add_system_clock_source	100000000
ifup_add_vga_clock_source		25000000
#
# Add processors
ifup_add_niosVm		NiosVm			$ifup_flag_niosV_0_data_manager $ifup_flag_niosV_0_instruction_manager $ifup_flag_niosV_0_irq_receiver
#ifup_add_niosVm		NiosVm_2nd_Core	$ifup_flag_niosV_1_data_manager $ifup_flag_niosV_1_instruction_manager $ifup_flag_niosV_1_irq_receiver
#
# Add bridges
ifup_add_jtag_to_fpga_bridge
#
# Add address span extenders
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
ifup_add_gpio	Pushbuttons		pushbuttons		Input	true	FALLING  4 0xFF200050  2
ifup_add_gpio	Expansion_JP1	expansion_jp1	Bidir	true	FALLING 32 0xFF200060 11
ifup_add_gpio	Expansion_JP2	expansion_jp2	Bidir	true	FALLING 32 0xFF200070 12
ifup_add_ps2	PS2_Port		ps2_port		0xFF200100 6
ifup_add_ps2	PS2_Port_Dual	ps2_port_dual	0xFF200108 7
#
# Add communications
ifup_add_jtag_uart	JTAG_UART_NiosV			0xFF201000 [ expr $ifup_flag_niosV_0_data_manager | $ifup_flag_jtag_to_fpga_bridge ] 8 $ifup_flag_niosV_0_irq_receiver
#ifup_add_jtag_uart	JTAG_UART_NiosV_2nd_Core	0xFF201000 $ifup_flag_niosV_1_data_manager 8 $ifup_flag_niosV_1_irq_receiver
ifup_add_irda
#
# Add timers
ifup_add_interval_timer	Interval_Timer_NiosV				0xFF202000 [ expr $ifup_flag_niosV_0_data_manager | $ifup_flag_jtag_to_fpga_bridge ] 0 $ifup_flag_niosV_0_irq_receiver
ifup_add_interval_timer	Interval_Timer_NiosV_2				0xFF202020 [ expr $ifup_flag_niosV_0_data_manager | $ifup_flag_jtag_to_fpga_bridge ] 1 $ifup_flag_niosV_0_irq_receiver
#ifup_add_interval_timer	Interval_Timer_NiosV_2nd_Core		0xFF202000 $ifup_flag_niosV_1_data_manager 0 $ifup_flag_niosV_1_irq_receiver
#ifup_add_interval_timer	Interval_Timer_NiosV_2nd_Core_2	0xFF202020 $ifup_flag_niosV_1_data_manager 1 $ifup_flag_niosV_1_irq_receiver
#
# Add sys id
ifup_add_system_id
#
# Add av cores
ifup_add_av_config
ifup_add_adc 12.5
ifup_add_vga_subsystem
ifup_add_audio_subsystem 5
ifup_add_video_in_subsystem
###############################################################################


###############################################################################
# Modify component parameters that are specific to this system
#
###############################################################################


