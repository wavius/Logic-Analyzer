package require -exact qsys 15.0


###############################################################################
# Default system settings
#
# Audio Setting
set ifup_default_audio_clk_freq				12.288
set ifup_default_audio_gen_lrclk			0
set ifup_default_audio_in_enabled			true
set ifup_default_audio_out_enabled			true
#
# VGA Setting
set ifup_default_vga_buffer_width			320
set ifup_default_vga_buffer_height			240
set ifup_default_vga_width_scaling			2
set ifup_default_vga_height_scaling			2
set ifup_default_vga_bits_per_color			16
set ifup_default_vga_rgb_resampler_input	"16-bit RGB"
###############################################################################


###############################################################################
# System generation flags
#
# Avalon master types
set ifup_flag_nios2_0_data_master			0x00000001
set ifup_flag_nios2_0_instruction_master	0x00000002
set ifup_flag_nios2_1_data_master			0x00000004
set ifup_flag_nios2_1_instruction_master	0x00000008
set ifup_flag_hps_h2f_master				0x00000010
set ifup_flag_hps_h2f_lw_master				0x00000020
set ifup_flag_jtag_to_fpga_bridge			0x00000100
set ifup_flag_video_out_dma_addr_translator	0x00001000
set ifup_flag_char_buf_dma_addr_translator	0x00002000
set ifup_flag_lcd_dma_addr_translator		0x00004000
set ifup_flag_video_in_dma_addr_translator	0x00010000
set ifup_flag_camera_dma_addr_translator	0x00020000
set ifup_flag_niosV_0_data_manager			0x00100000
set ifup_flag_niosV_0_instruction_manager	0x00200000
set ifup_flag_niosV_1_data_manager			0x00400000
set ifup_flag_niosV_1_instruction_manager	0x00800000
#
# Avalon slave types
set ifup_flag_hps_f2h_slave					0x00000001
set ifup_flag_video_in_memory				0x00000010
set ifup_flag_video_out_memory				0x00000020
#
# Avalon irq receiver types
set ifup_flag_nios2_0_irq_receiver			0x00000001
set ifup_flag_nios2_1_irq_receiver			0x00000002
set ifup_flag_hps_arm_0_irq_receiver		0x00000010
set ifup_flag_hps_arm_1_irq_receiver		0x00000020
set ifup_flag_niosV_0_irq_receiver			0x00000100
set ifup_flag_niosV_1_irq_receiver			0x00000200
#
# Avalon master common flag sets
set ifup_flag_set_fpga_data_masters			[ expr $ifup_flag_nios2_0_data_master | $ifup_flag_nios2_1_data_master | $ifup_flag_jtag_to_fpga_bridge | $ifup_flag_niosV_0_data_manager | $ifup_flag_niosV_1_data_manager ]
set ifup_flag_set_fpga_instruction_masters	[ expr $ifup_flag_nios2_0_instruction_master | $ifup_flag_nios2_1_instruction_master | $ifup_flag_niosV_0_instruction_manager | $ifup_flag_niosV_1_instruction_manager ]
set ifup_flag_set_fpga_peripherals			[ expr $ifup_flag_set_fpga_data_masters | $ifup_flag_hps_h2f_lw_master ]
#
# Avalon irq receiver types
set ifup_flag_set_main_irq_receivers		[ expr $ifup_flag_nios2_0_irq_receiver | $ifup_flag_nios2_1_irq_receiver | $ifup_flag_hps_arm_0_irq_receiver | $ifup_flag_niosV_0_irq_receiver | $ifup_flag_niosV_1_irq_receiver ]
###############################################################################


###############################################################################
# Connection lists
set system_clock				{}
set reset_sources				{}
set video_vga_clock				{}
set video_lcd_clock				{}
set video_reset					{}
array set memory_mapped_masters	{}
array set memory_mapped_slaves	{}
array set irq_receivers			{}
#
proc ifup_connect_system_clock { clock_sink } {
	global system_clock
	add_connection $system_clock $clock_sink
}
#
proc ifup_connect_reset_sources { reset_sink } {
	global reset_sources
	foreach reset_source $reset_sources {
		add_connection $reset_source $reset_sink
	}	
}
#
proc ifup_connect_video_vga_clock { clock_sink } {
	global video_vga_clock
	add_connection $video_vga_clock $clock_sink
}
#
proc ifup_connect_video_lcd_clock { clock_sink } {
	global video_lcd_clock
	add_connection $video_lcd_clock $clock_sink
}
#
proc ifup_connect_video_reset { reset_sink } {
	global video_reset
	add_connection $video_reset $reset_sink
}
#
proc ifup_connect_mm_interface { master slave offset } {
	add_connection					$master $slave
	set_connection_parameter_value "$master/$slave" baseAddress $offset
}
#
proc ifup_connect_slave_to_mm_masters { slave offset flags } {
	global memory_mapped_masters
	foreach master [array names memory_mapped_masters] {
		set masters_settings		$memory_mapped_masters($master)
		set masters_flags			[ lindex $masters_settings 0 ]
		set masters_start_address	[ lindex $masters_settings 1 ]	
		if { [ expr $masters_flags & $flags ] } {
			ifup_connect_mm_interface $master $slave [ format "0x%x" [ expr $offset - $masters_start_address ] ]
		}
	}	
}
#
proc ifup_connect_master_to_mm_slaves { master flags } {
	global memory_mapped_slaves
	foreach slave [array names memory_mapped_slaves] {
		set slaves_settings	$memory_mapped_slaves($slave)
		set slaves_flags	[ lindex $slaves_settings 0 ]
		set slaves_offset	[ lindex $slaves_settings 1 ]	
		if { [ expr $slaves_flags & $flags ] } {
			ifup_connect_mm_interface $master $slave $slaves_offset
		}
	}	
}
#
proc ifup_connect_irq_receivers { irq_sender irq_number flags } {
	global irq_receivers
	foreach receiver [array names irq_receivers] {
		if { [ expr $irq_receivers($receiver) & $flags ] } {
			add_connection $receiver $irq_sender
			set_connection_parameter_value "$receiver/$irq_sender" irqNumber $irq_number
		}
	}
}
###############################################################################


###############################################################################
# Add IP component functions: Clocks
#
# Clock Source
proc ifup_add_clock_source { instance_name export_name clk_freq } {
	add_instance $instance_name clock_source

	set_instance_parameter_value $instance_name clockFrequency			$clk_freq
	set_instance_parameter_value $instance_name clockFrequencyKnown		true
	set_instance_parameter_value $instance_name resetSynchronousEdges	"NONE"

	set_interface_property ${export_name}_clk_ref	EXPORT_OF $instance_name.clk_in
	set_interface_property ${export_name}_clk		EXPORT_OF $instance_name.clk_in_reset
}
#
# System Clock Source
proc ifup_add_system_clock_source { clk_freq } {
	set instance_name System_CLK

	ifup_add_clock_source $instance_name sys $clk_freq

	# Update connection lists
	global system_clock
	global reset_sources
	set		system_clock	$instance_name.clk
	lappend reset_sources	$instance_name.clk_reset
}
#
# VGA Clock Source
proc ifup_add_vga_clock_source { clk_freq } {
	set instance_name VGA_CLK

	ifup_add_clock_source $instance_name vga $clk_freq

	# Update connection lists
	global	video_vga_clock
	global	video_reset
	set		video_vga_clock	$instance_name.clk
	set		video_reset		$instance_name.clk_reset
}
#
# System PLL
proc ifup_add_system_pll { include_sdram_clk sys_clk_freq } {
	global device_family
	global board_name

	set instance_name System_PLL

	add_instance $instance_name altera_up_avalon_sys_sdram_pll

	set_instance_parameter_value $instance_name gui_refclk		50.0
	set_instance_parameter_value $instance_name gui_outclk		$sys_clk_freq
	set_instance_parameter_value $instance_name $device_family	$board_name

	set_interface_property system_pll_ref_clk	EXPORT_OF $instance_name.ref_clk
	set_interface_property system_pll_ref_reset	EXPORT_OF $instance_name.ref_reset
	if {$include_sdram_clk == 1} {
		set_interface_property sdram_clk		EXPORT_OF $instance_name.sdram_clk
	}

	# Update connection lists
	global system_clock
	global reset_sources
	set		system_clock	$instance_name.sys_clk
	lappend reset_sources	$instance_name.reset_source
}
#
# Video PLL
proc ifup_add_video_pll { include_video_in_clk camera include_vga_clk resolution include_lcd_clk lcd } {
	set instance_name Video_PLL

	add_instance $instance_name altera_up_avalon_video_pll

	set_instance_parameter_value $instance_name gui_refclk			50.0
	if { $include_video_in_clk == 1 } {
		set_instance_parameter_value $instance_name video_in_clk_en	true
		set_instance_parameter_value $instance_name camera			$camera
	} else {
		set_instance_parameter_value $instance_name video_in_clk_en	false
	}
	if { $include_vga_clk == 1 } {
		set_instance_parameter_value $instance_name vga_clk_en		true
		set_instance_parameter_value $instance_name gui_resolution	$resolution
	} else {
		set_instance_parameter_value $instance_name vga_clk_en		false
	}
	if { $include_lcd_clk == 1 } {
		set_instance_parameter_value $instance_name lcd_clk_en		true
		set_instance_parameter_value $instance_name lcd				$lcd
	} else {
		set_instance_parameter_value $instance_name lcd_clk_en		false
	}


	set_interface_property video_pll_ref_clk	EXPORT_OF $instance_name.ref_clk
	set_interface_property video_pll_ref_reset	EXPORT_OF $instance_name.ref_reset
	if { $include_video_in_clk == 1 } {
		set_interface_property camera_clk		EXPORT_OF $instance_name.video_in_clk
	}

	# Update connection lists
	global video_vga_clock
	global video_lcd_clock
	global video_reset
	set	video_vga_clock	$instance_name.vga_clk
	set	video_lcd_clock	$instance_name.lcd_clk
	set	video_reset	$instance_name.reset_source
}
###############################################################################


###############################################################################
# Add IP component functions: Processors
#
# Arm A9
proc ifup_add_arm_a9_hps {} {
	set instance_name ARM_A9_HPS

	add_instance $instance_name altera_hps

	set_instance_parameter_value $instance_name CTL_ENABLE_BURST_INTERRUPT						true
	set_instance_parameter_value $instance_name CTL_ENABLE_BURST_TERMINATE						true
	set_instance_parameter_value $instance_name EMAC1_Mode										"RGMII"
	set_instance_parameter_value $instance_name EMAC1_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name F2SCLK_COLDRST_Enable							false
	set_instance_parameter_value $instance_name F2SCLK_DBGRST_Enable							false
	set_instance_parameter_value $instance_name F2SCLK_WARMRST_Enable							false
	set_instance_parameter_value $instance_name F2SDRAM_Type									""
	set_instance_parameter_value $instance_name F2SDRAM_Width									""
	set_instance_parameter_value $instance_name F2SINTERRUPT_Enable								true
	set_instance_parameter_value $instance_name FPGA_PERIPHERAL_OUTPUT_CLOCK_FREQ_EMAC0_GTX_CLK	100.0
	set_instance_parameter_value $instance_name FPGA_PERIPHERAL_OUTPUT_CLOCK_FREQ_EMAC1_GTX_CLK	100.0
	set_instance_parameter_value $instance_name FPGA_PERIPHERAL_OUTPUT_CLOCK_FREQ_EMAC0_MD_CLK	100.0
	set_instance_parameter_value $instance_name FPGA_PERIPHERAL_OUTPUT_CLOCK_FREQ_EMAC1_MD_CLK	100.0
	set_instance_parameter_value $instance_name I2C0_Mode										"I2C"
	set_instance_parameter_value $instance_name I2C0_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name I2C1_Mode										"I2C"
	set_instance_parameter_value $instance_name I2C1_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name MAX_PENDING_RD_CMD								16
	set_instance_parameter_value $instance_name MAX_PENDING_WR_CMD								8
	set_instance_parameter_value $instance_name MEM_CLK_FREQ									400.0
	set_instance_parameter_value $instance_name MEM_CLK_FREQ_MAX								800.0
	set_instance_parameter_value $instance_name MEM_COL_ADDR_WIDTH								10
	set_instance_parameter_value $instance_name MEM_DQ_WIDTH									32
	set_instance_parameter_value $instance_name MEM_DRV_STR										"RZQ/6"
	set_instance_parameter_value $instance_name MEM_ROW_ADDR_WIDTH								15
	set_instance_parameter_value $instance_name MEM_RTT_NOM										"RZQ/6"
	set_instance_parameter_value $instance_name MEM_RTT_WR										"Dynamic ODT off"
	set_instance_parameter_value $instance_name MEM_TINIT_US									500
	set_instance_parameter_value $instance_name MEM_TMRD_CK										4
	set_instance_parameter_value $instance_name MEM_TREFI_US									7.8
	set_instance_parameter_value $instance_name MEM_TWTR										4
	set_instance_parameter_value $instance_name MEM_WTCL										7
	set_instance_parameter_value $instance_name MPU_EVENTS_Enable								false
	set_instance_parameter_value $instance_name REF_CLK_FREQ									25
	set_instance_parameter_value $instance_name SDIO_Mode										"4-bit Data"
	set_instance_parameter_value $instance_name SDIO_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name SPIM1_Mode										"Single Slave Select"
	set_instance_parameter_value $instance_name SPIM1_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name STM_Enable										true
	set_instance_parameter_value $instance_name UART0_Mode										"No Flow Control"
	set_instance_parameter_value $instance_name UART0_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name USB1_Mode										"SDR"
	set_instance_parameter_value $instance_name USB1_PinMuxing									"HPS I/O Set 0"
	set_instance_parameter_value $instance_name desired_mpu_clk_mhz								800.0
	set_instance_parameter_value $instance_name use_default_mpu_clk								true

	set_interface_property memory EXPORT_OF $instance_name.memory
	set_interface_property hps_io EXPORT_OF $instance_name.hps_io

	ifup_connect_system_clock	$instance_name.h2f_axi_clock
	ifup_connect_system_clock	$instance_name.f2h_axi_clock
	ifup_connect_system_clock	$instance_name.h2f_lw_axi_clock

	lock_avalon_base_address $instance_name.f2h_axi_slave
	
	# Update connection lists
	global reset_sources
	global memory_mapped_masters
	global memory_mapped_slaves
	global irq_receivers
	global ifup_flag_hps_h2f_master
	global ifup_flag_hps_h2f_lw_master
	global ifup_flag_hps_f2h_slave
	global ifup_flag_hps_arm_0_irq_receiver
	global ifup_flag_hps_arm_1_irq_receiver
	lappend reset_sources $instance_name.h2f_reset
	set		memory_mapped_masters($instance_name.h2f_axi_master)	[ list $ifup_flag_hps_h2f_master	0x00000000 ]
	set		memory_mapped_masters($instance_name.h2f_lw_axi_master)	[ list $ifup_flag_hps_h2f_lw_master	0xFF200000 ]
	set		memory_mapped_slaves($instance_name.f2h_axi_slave)		[ list $ifup_flag_hps_f2h_slave		0x00000000 ]
	set		irq_receivers($instance_name.f2h_irq0)					$ifup_flag_hps_arm_0_irq_receiver
	set		irq_receivers($instance_name.f2h_irq1)					$ifup_flag_hps_arm_1_irq_receiver
}
#
# Nios II
proc ifup_add_niosII { instance_name resetSlave resetOffset exceptionSlave exceptionOffset cpuID include_fp data_master_flag instruction_master_flag irq_receiver_flags } {
	add_instance $instance_name altera_nios2_gen2

	set_instance_parameter_value $instance_name resetSlave							$resetSlave
	set_instance_parameter_value $instance_name resetOffset							$resetOffset
	set_instance_parameter_value $instance_name exceptionSlave						$exceptionSlave
	set_instance_parameter_value $instance_name exceptionOffset						$exceptionOffset
	set_instance_parameter_value $instance_name cpuID								$cpuID
	set_instance_parameter_value $instance_name dataAddrWidth						32
	set_instance_parameter_value $instance_name debug_hwbreakpoint					2
	set_instance_parameter_value $instance_name debug_datatrigger					2
	set_instance_parameter_value $instance_name debug_traceType						"instruction_trace"
	set_instance_parameter_value $instance_name dcache_size							0
	set_instance_parameter_value $instance_name mul_32_impl							2
	set_instance_parameter_value $instance_name mul_64_impl							1
	set_instance_parameter_value $instance_name mul_shift_choice					1
	set_instance_parameter_value $instance_name shift_rot_impl						1
	set_instance_parameter_value $instance_name dividerType							"srt2"
	set_instance_parameter_value $instance_name resetrequest_enabled				false
	set_instance_parameter_value $instance_name setting_support31bitdcachebypass	false

	ifup_connect_system_clock	$instance_name.clk
	ifup_connect_reset_sources	$instance_name.reset
	add_connection				$instance_name.debug_reset_request	$instance_name.reset
	ifup_connect_mm_interface	$instance_name.data_master			$instance_name.debug_mem_slave 0x0a000000
	ifup_connect_mm_interface	$instance_name.instruction_master	$instance_name.debug_mem_slave 0x0a000000

	lock_avalon_base_address	$instance_name.debug_mem_slave

	if { $include_fp == 1 } {
		set fp_instance_name [ format "%s_Floating_Point" $instance_name ]

		add_instance $fp_instance_name altera_nios_custom_instr_floating_point

		set_instance_parameter_value $fp_instance_name useDivider 1

		add_connection $instance_name.custom_instruction_master $fp_instance_name.s1
	}
	
	# Update connection lists
	global memory_mapped_masters
	global irq_receivers
	set	memory_mapped_masters($instance_name.data_master)			 [ list $data_master_flag			0x00000000 ]
	set	memory_mapped_masters($instance_name.instruction_master)	 [ list $instruction_master_flag	0x00000000 ]
	set	irq_receivers($instance_name.irq)									$irq_receiver_flags
}
#
# Nios V/m
proc ifup_add_niosVm { instance_name data_master_flag instruction_master_flag irq_receiver_flags } {
	add_instance $instance_name intel_niosv_m
	
	set_instance_parameter_value $instance_name enableDebug							true
	set_instance_parameter_value $instance_name enableDebugReset					true
	set_instance_parameter_value $instance_name enableECCLite						false
	set_instance_parameter_value $instance_name useResetReq							false
	set_instance_parameter_value $instance_name resetSlave							"Absolute"
	set_instance_parameter_value $instance_name resetOffset							0

	ifup_connect_system_clock	$instance_name.clk
	ifup_connect_reset_sources	$instance_name.reset
	ifup_connect_reset_sources	$instance_name.ndm_reset_in
	add_connection				$instance_name.dbg_reset_out		$instance_name.ndm_reset_in
	ifup_connect_mm_interface	$instance_name.data_manager			$instance_name.timer_sw_agent	0xFF202100
	ifup_connect_mm_interface	$instance_name.instruction_manager	$instance_name.dm_agent			0x0a000000
	ifup_connect_mm_interface	$instance_name.data_manager			$instance_name.dm_agent			0x0a000000

	lock_avalon_base_address	$instance_name.timer_sw_agent
	lock_avalon_base_address	$instance_name.dm_agent

	# Update connection lists
	global memory_mapped_masters
	global irq_receivers
	set	memory_mapped_masters($instance_name.data_manager)			 [ list $data_master_flag			0x00000000 ]
	set	memory_mapped_masters($instance_name.instruction_manager)	 [ list $instruction_master_flag	0x00000000 ]
	set	irq_receivers($instance_name.platform_irq_rx)						$irq_receiver_flags
}
#
# Nios V/g
proc ifup_add_niosVg { instance_name data_master_flag instruction_master_flag irq_receiver_flags } {
	add_instance $instance_name intel_niosv_g
	
	set_instance_parameter_value $instance_name dataCacheSize						1024
	set_instance_parameter_value $instance_name instCacheSize						1024
	set_instance_parameter_value $instance_name enableFPU							true
	set_instance_parameter_value $instance_name enableDebug							true
	set_instance_parameter_value $instance_name enableDebugReset					true
	set_instance_parameter_value $instance_name enableECCLite						false
	set_instance_parameter_value $instance_name useResetReq							false
	set_instance_parameter_value $instance_name resetSlave							"Absolute"
	set_instance_parameter_value $instance_name resetOffset							0
	set_instance_parameter_value $instance_name peripheralRegionABase				-16777216
	set_instance_parameter_value $instance_name peripheralRegionASize				16777216
	set_instance_parameter_value $instance_name peripheralRegionBBase				134217728
	set_instance_parameter_value $instance_name peripheralRegionBSize				134217728

	ifup_connect_system_clock	$instance_name.clk
	ifup_connect_reset_sources	$instance_name.reset
	ifup_connect_reset_sources	$instance_name.ndm_reset_in
	add_connection				$instance_name.dbg_reset_out		$instance_name.ndm_reset_in
	ifup_connect_mm_interface	$instance_name.data_manager			$instance_name.timer_sw_agent	0xFF202100
	ifup_connect_mm_interface	$instance_name.instruction_manager	$instance_name.dm_agent			0x0a000000
	ifup_connect_mm_interface	$instance_name.data_manager			$instance_name.dm_agent			0x0a000000

	lock_avalon_base_address	$instance_name.timer_sw_agent
	lock_avalon_base_address	$instance_name.dm_agent

	# Update connection lists
	global memory_mapped_masters
	global irq_receivers
	set	memory_mapped_masters($instance_name.data_manager)			 [ list $data_master_flag			0x00000000 ]
	set	memory_mapped_masters($instance_name.instruction_manager)	 [ list $instruction_master_flag	0x00000000 ]
	set	irq_receivers($instance_name.platform_irq_rx)						$irq_receiver_flags
}
###############################################################################


###############################################################################
# Add IP component functions: Bridges
#
# JTAG Bridge for HPS
proc ifup_add_jtag_to_hps_bridge {} {
	global ifup_flag_hps_f2h_slave
	
	set instance_name JTAG_to_HPS_Bridge

	add_instance $instance_name altera_jtag_avalon_master

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.clk_reset
	ifup_connect_master_to_mm_slaves	$instance_name.master $ifup_flag_hps_f2h_slave
}
#
# JTAG Bridge for FPGA
proc ifup_add_jtag_to_fpga_bridge {} {
	set instance_name JTAG_to_FPGA_Bridge

	add_instance $instance_name altera_jtag_avalon_master

	ifup_connect_system_clock	$instance_name.clk
	ifup_connect_reset_sources	$instance_name.clk_reset
	
	# Update connection lists
	global memory_mapped_masters
	global ifup_flag_jtag_to_fpga_bridge
	set	memory_mapped_masters($instance_name.master) [ list $ifup_flag_jtag_to_fpga_bridge 0x00000000 ]
}
#
# DMA Address Translation
proc ifup_add_dma_address_translation { instance_name offset connection_flag } {
	global ifup_flag_hps_h2f_lw_master
	
	add_instance $instance_name altera_up_avalon_video_dma_ctrl_addr_trans

	ifup_connect_system_clock			$instance_name.clock
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.slave	$offset	$ifup_flag_hps_h2f_lw_master

	lock_avalon_base_address $instance_name.slave
	
	# Update connection lists
	global memory_mapped_masters
	set memory_mapped_masters($instance_name.master)	[ list $connection_flag $offset ]
}
#
# Address span extender
proc ifup_add_address_span_extender { instance_name address width offset } {
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_hps_f2h_slave

	add_instance $instance_name altera_address_span_extender

	set_instance_parameter_value $instance_name ENABLE_SLAVE_PORT	false
	set_instance_parameter_value $instance_name MASTER_ADDRESS_DEF	$address
	set_instance_parameter_value $instance_name SLAVE_ADDRESS_WIDTH	$width

	ifup_connect_system_clock			$instance_name.clock
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.windowed_slave	$offset	$ifup_flag_set_fpga_data_masters
	ifup_connect_master_to_mm_slaves	$instance_name.expanded_master	$ifup_flag_hps_f2h_slave

	lock_avalon_base_address $instance_name.windowed_slave
}
###############################################################################


###############################################################################
# Add IP component functions: Memory
#
# SDRAM
proc ifup_add_sdram { columns rows data_width } {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_instruction_masters

	set instance_name	SDRAM
	set offset			0x00000000
	set bus_connection_flags [ expr $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_instruction_masters | $ifup_flag_set_fpga_data_masters ]

	add_instance $instance_name altera_avalon_new_sdram_controller

	set_instance_parameter_value $instance_name columnWidth				$columns
	set_instance_parameter_value $instance_name rowWidth				$rows
	set_instance_parameter_value $instance_name dataWidth				$data_width
	set_instance_parameter_value $instance_name generateSimulationModel	true

	set_interface_property sdram	EXPORT_OF $instance_name.wire

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.s1	 $offset $bus_connection_flags

	lock_avalon_base_address $instance_name.s1
	
	# Update connection lists
	global memory_mapped_slaves
	global ifup_flag_video_in_memory
	global ifup_flag_video_out_memory
	set memory_mapped_slaves($instance_name.s1) [ list [ expr $ifup_flag_video_in_memory | $ifup_flag_video_out_memory ] $offset ]
}
proc ifup_add_sdram_64mb {} {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_instruction_masters

	set instance_name	SDRAM
	set offset			0x00000000
	set bus_connection_flags [ expr $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_instruction_masters | $ifup_flag_set_fpga_data_masters ]

	add_instance $instance_name sdram_64mb

	set_interface_property sdram	EXPORT_OF $instance_name.wire

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.s1	 $offset $bus_connection_flags

	lock_avalon_base_address $instance_name.s1
	
	# Update connection lists
	global memory_mapped_slaves
	global ifup_flag_video_in_memory
	global ifup_flag_video_out_memory
	set memory_mapped_slaves($instance_name.s1) [ list [ expr $ifup_flag_video_in_memory | $ifup_flag_video_out_memory ] $offset ]
}
#
# SRAM
proc ifup_add_sram {} {
	global board_name
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_instruction_masters

	set instance_name			SRAM
	set export_name				sram
	set offset					0x08000000
	set bus_connection_flags	[ expr $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_instruction_masters | $ifup_flag_set_fpga_data_masters ]

	add_instance $instance_name altera_up_avalon_sram

	set_interface_property $export_name	EXPORT_OF $instance_name.external_interface

	set_instance_parameter_value $instance_name board			$board_name
	set_instance_parameter_value $instance_name pixel_buffer	true

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_sram_slave	$offset	$bus_connection_flags

	lock_avalon_base_address $instance_name.avalon_sram_slave
	
	# Update connection lists
	global memory_mapped_slaves
	global ifup_flag_video_in_memory
	global ifup_flag_video_out_memory
	set memory_mapped_slaves($instance_name.avalon_sram_slave) [ list [ expr $ifup_flag_video_in_memory | $ifup_flag_video_out_memory ] $offset ]
}
#
# SSRAM
proc ifup_add_ssram {} {
	global board_name
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_instruction_masters

	set instance_name			SSRAM
	set export_name				ssram
	set offset					0x08000000
	set bus_connection_flags	[ expr $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_instruction_masters | $ifup_flag_set_fpga_data_masters ]

	add_instance $instance_name altera_up_avalon_ssram

	set_interface_property $export_name	EXPORT_OF $instance_name.external_interface

	set_instance_parameter_value $instance_name board			$board_name
	set_instance_parameter_value $instance_name pixel_buffer	true

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_ssram_slave	$offset	$bus_connection_flags

	lock_avalon_base_address $instance_name.avalon_ssram_slave
	
	# Update connection lists
	global memory_mapped_slaves
	global ifup_flag_video_in_memory
	global ifup_flag_video_out_memory
	set memory_mapped_slaves($instance_name.avalon_ssram_slave) [ list [ expr $ifup_flag_video_in_memory | $ifup_flag_video_out_memory ] $offset ]
}
#
# Onchip Memory
proc ifup_add_onchip_memory { instance_name memory_size offset } {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_instruction_masters

	add_instance $instance_name altera_avalon_onchip_memory2

	set_instance_parameter_value $instance_name dualPort				true
	set_instance_parameter_value $instance_name memorySize				$memory_size
	set_instance_parameter_value $instance_name singleClockOperation	true

	ifup_connect_system_clock			$instance_name.clk1
	ifup_connect_reset_sources			$instance_name.reset1
	ifup_connect_slave_to_mm_masters	$instance_name.s1		$offset	[ expr $ifup_flag_set_fpga_data_masters | $ifup_flag_hps_h2f_master ]
	ifup_connect_slave_to_mm_masters	$instance_name.s2		$offset	$ifup_flag_set_fpga_instruction_masters

	lock_avalon_base_address $instance_name.s1
	lock_avalon_base_address $instance_name.s2
	
	# Update connection lists
	global memory_mapped_slaves
	global ifup_flag_video_in_memory
	global ifup_flag_video_out_memory
	set memory_mapped_slaves($instance_name.s2) [ list $ifup_flag_video_out_memory	$offset ]
	set memory_mapped_slaves($instance_name.s1) [ list $ifup_flag_video_in_memory	$offset ]
}
#
# SD Card Memory
proc ifup_add_sd_card {} {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters

	set instance_name	SD_Card
	set offset			0x0B000000

	add_instance $instance_name Altera_UP_SD_Card_Avalon_Interface

	set_interface_property sd_card EXPORT_OF $instance_name.conduit_end

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_sdcard_slave	$offset	[ expr $ifup_flag_set_fpga_data_masters | $ifup_flag_hps_h2f_master ]

	lock_avalon_base_address $instance_name.avalon_sdcard_slave
}
#
# Flash Memory
proc ifup_add_flash_ctrl {} {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_instruction_masters

	set instance_name	Flash
	set export_name		flash
	set offset_data		0x0C000000
	set offset_ctrl		0x0BFF0000

	add_instance $instance_name Altera_UP_Flash_Memory_IP_Core_Avalon_Interface

	set_instance_parameter_value $instance_name FLASH_MEMORY_ADDRESS_WIDTH	23

	set_interface_property $export_name EXPORT_OF $instance_name.conduit_end

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.flash_data			$offset_data	[ expr $ifup_flag_set_fpga_data_masters | $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_instruction_masters ]
	ifup_connect_slave_to_mm_masters	$instance_name.flash_erase_control	$offset_ctrl	[ expr $ifup_flag_set_fpga_data_masters | $ifup_flag_hps_h2f_master ]

	lock_avalon_base_address $instance_name.flash_data
	lock_avalon_base_address $instance_name.flash_erase_control
}
###############################################################################


###############################################################################
# Add IP component functions: IO
#
# GPIO 
proc ifup_add_gpio { instance_name export_name direction captureEdge edge_type bits offset irq } {
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	add_instance $instance_name altera_avalon_pio

	set_instance_parameter_value $instance_name bitClearingEdgeCapReg		$captureEdge
	set_instance_parameter_value $instance_name bitModifyingOutReg			false
	set_instance_parameter_value $instance_name captureEdge					$captureEdge
	set_instance_parameter_value $instance_name direction					$direction
	set_instance_parameter_value $instance_name edgeType					$edge_type
	if { $irq == -1 } {
		set_instance_parameter_value $instance_name generateIRQ				false
		set_instance_parameter_value $instance_name irqType					"LEVEL"
	} else {
		set_instance_parameter_value $instance_name generateIRQ				true
		set_instance_parameter_value $instance_name irqType					"EDGE"
	}
	set_instance_parameter_value $instance_name resetValue					0
	if { $direction == "Output" } {
		set_instance_parameter_value $instance_name simDoTestBenchWiring	false
	} else {
		set_instance_parameter_value $instance_name simDoTestBenchWiring	true
	}
	set_instance_parameter_value $instance_name simDrivenValue				0
	set_instance_parameter_value $instance_name width						$bits

	set_interface_property $export_name EXPORT_OF $instance_name.external_connection

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.s1	$offset	$ifup_flag_set_fpga_peripherals
	if { $irq != -1 } {
		ifup_connect_irq_receivers		$instance_name.irq	$irq	$ifup_flag_set_main_irq_receivers
	}

	lock_avalon_base_address $instance_name.s1
}
#
# PS2
proc ifup_add_ps2 { instance_name export_name offset irq } {
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	add_instance $instance_name altera_up_avalon_ps2

	set_interface_property $export_name EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_ps2_slave	$offset	$ifup_flag_set_fpga_peripherals
	ifup_connect_irq_receivers			$instance_name.interrupt		$irq	$ifup_flag_set_main_irq_receivers

	lock_avalon_base_address $instance_name.avalon_ps2_slave
}
#
# ADC
proc ifup_add_adc { sclk_freq {num_channels 8} {enable_external_interface true} } {
	global board_name
	global ifup_flag_set_fpga_peripherals

	set instance_name	ADC
	set export_name		adc
	set offset			0xFF204000

	add_instance $instance_name altera_up_avalon_adc

	set_instance_parameter_value $instance_name board		$board_name
	set_instance_parameter_value $instance_name board_rev	"Autodetect"
	set_instance_parameter_value $instance_name sclk_freq	$sclk_freq
	set_instance_parameter_value $instance_name numch_		$num_channels

	if { [string is true $enable_external_interface] } {
		set_interface_property $export_name EXPORT_OF $instance_name.external_interface
	}

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.adc_slave	$offset	$ifup_flag_set_fpga_peripherals

	lock_avalon_base_address $instance_name.adc_slave
}
#
# Accelerometer
proc ifup_add_accelerometer {} {
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	set instance_name	Accelerometer
	set export_name		accelerometer
	set offset			0xFF204020
	set irq				15

	add_instance $instance_name altera_up_avalon_accelerometer_spi

	set_interface_property $export_name EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_accelerometer_spi_mode_slave	$offset	$ifup_flag_set_fpga_peripherals
	ifup_connect_irq_receivers			$instance_name.interrupt							$irq	$ifup_flag_set_main_irq_receivers

	lock_avalon_base_address $instance_name.avalon_accelerometer_spi_mode_slave
}
#
# USB
proc ifup_add_usb {} {
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	set instance_name	USB
	set export_name		usb
	set offset			0xFF200110
	set irq				17

	add_instance $instance_name altera_up_avalon_usb

	set_interface_property $export_name EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_usb_slave	$offset	$ifup_flag_set_fpga_peripherals
	ifup_connect_irq_receivers			$instance_name.interrupt		$irq	$ifup_flag_set_main_irq_receivers

	lock_avalon_base_address $instance_name.avalon_usb_slave
}
###############################################################################


###############################################################################
# Add IP component functions: Communications
#
# JTAG UART
proc ifup_add_jtag_uart { instance_name offset connection_flags irq irq_flags} {
	add_instance $instance_name altera_avalon_jtag_uart

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_jtag_slave	$offset	$connection_flags
	ifup_connect_irq_receivers			$instance_name.irq					$irq	$irq_flags

	lock_avalon_base_address $instance_name.avalon_jtag_slave
}
#
# RS232 Serial Port
proc ifup_add_rs232 {} {
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	set instance_name	Serial_Port
	set export_name		serial_port
	set offset			0xFF201010
	set irq				10

	add_instance $instance_name altera_up_avalon_rs232

	set_instance_parameter_value $instance_name avalon_bus_type	"Memory Mapped"
	set_instance_parameter_value $instance_name baud			115200
	set_instance_parameter_value $instance_name parity			Odd
	set_instance_parameter_value $instance_name data_bits		8
	set_instance_parameter_value $instance_name stop_bits		1

	set_interface_property $export_name EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_rs232_slave	$offset	$ifup_flag_set_fpga_peripherals
	ifup_connect_irq_receivers			$instance_name.interrupt			$irq	$ifup_flag_set_main_irq_receivers

	lock_avalon_base_address $instance_name.avalon_rs232_slave
}
#
# IrDA
proc ifup_add_irda {} {
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	set instance_name	IrDA
	set offset			0xFF201020
	set irq				9

	add_instance $instance_name altera_up_avalon_irda

	set_interface_property irda EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_irda_slave	$offset	$ifup_flag_set_fpga_peripherals
	ifup_connect_irq_receivers			$instance_name.interrupt			$irq	$ifup_flag_set_main_irq_receivers

	lock_avalon_base_address $instance_name.avalon_irda_slave
}
###############################################################################


###############################################################################
# Add IP component functions: System Components
#
# Interval Timer
proc ifup_add_interval_timer { instance_name offset connection_flags irq irq_flags } {
	add_instance $instance_name altera_avalon_timer

	set_instance_parameter_value $instance_name period 125.0

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.s1	$offset	$connection_flags
	ifup_connect_irq_receivers			$instance_name.irq	$irq	$irq_flags

	lock_avalon_base_address $instance_name.s1
}
#
# System ID
proc ifup_add_system_id {} {
	global ifup_flag_set_fpga_peripherals

	set instance_name	SysID

	add_instance $instance_name altera_avalon_sysid_qsys

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.control_slave	0xFF202040	$ifup_flag_set_fpga_peripherals

	lock_avalon_base_address $instance_name.control_slave
}
###############################################################################


###############################################################################
# Add IP component functions: Audio/Video
#
# Audio/Video Configuration
proc ifup_add_av_config {} {
	global board_name
	global ifup_flag_set_fpga_peripherals

	set instance_name	AV_Config

	add_instance $instance_name altera_up_avalon_audio_and_video_config

	set_instance_parameter_value $instance_name board			$board_name
	set_instance_parameter_value $instance_name eai				true
	set_instance_parameter_value $instance_name line_in_bypass	true
	set_instance_parameter_value $instance_name bit_length		32
	set_instance_parameter_value $instance_name sampling_rate	"8 kHz"

	set_interface_property av_config EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_av_config_slave	0xFF203000	$ifup_flag_set_fpga_peripherals

	lock_avalon_base_address $instance_name.avalon_av_config_slave
}
#
# VGA Subsystem
proc ifup_add_vga_subsystem {} {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_video_out_dma_addr_translator
	global ifup_flag_char_buf_dma_addr_translator
	global ifup_flag_video_out_memory

	set instance_name	VGA_Subsystem

	add_instance $instance_name VGA_Subsystem

	set_interface_property vga			EXPORT_OF $instance_name.vga

	ifup_connect_system_clock			$instance_name.sys_clk
	ifup_connect_reset_sources			$instance_name.sys_reset
	ifup_connect_video_vga_clock		$instance_name.vga_clk
	ifup_connect_video_reset			$instance_name.vga_reset
	ifup_connect_slave_to_mm_masters	$instance_name.char_buffer_control_slave	0xFF203030	[ expr $ifup_flag_char_buf_dma_addr_translator | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_slave_to_mm_masters	$instance_name.char_buffer_slave			0x09000000	[ expr $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_slave_to_mm_masters	$instance_name.pixel_dma_control_slave		0xFF203020	[ expr $ifup_flag_video_out_dma_addr_translator | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_master_to_mm_slaves	$instance_name.pixel_dma_master				$ifup_flag_video_out_memory
	ifup_connect_slave_to_mm_masters	$instance_name.rgb_slave					0xFF203010	$ifup_flag_set_fpga_peripherals

	lock_avalon_base_address $instance_name.char_buffer_control_slave
	lock_avalon_base_address $instance_name.char_buffer_slave
	lock_avalon_base_address $instance_name.pixel_dma_control_slave
	lock_avalon_base_address $instance_name.rgb_slave
}
#
# LCD Subsystem
proc ifup_add_lcd_subsystem {} {
	global ifup_flag_hps_h2f_master
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_lcd_dma_addr_translator
	global ifup_flag_video_out_memory

	set instance_name	LCD_Subsystem

	add_instance $instance_name LCD_Subsystem

	set_interface_property lcd					EXPORT_OF $instance_name.lcd

	ifup_connect_system_clock			$instance_name.sys_clk
	ifup_connect_reset_sources			$instance_name.sys_reset
	ifup_connect_video_lcd_clock		$instance_name.lcd_clk
	ifup_connect_video_reset			$instance_name.lcd_reset
	ifup_connect_slave_to_mm_masters	$instance_name.char_buffer_control_slave	0xFF2030B0	$ifup_flag_set_fpga_peripherals
	ifup_connect_slave_to_mm_masters	$instance_name.char_buffer_slave			0x09002000	[ expr $ifup_flag_hps_h2f_master | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_slave_to_mm_masters	$instance_name.pixel_dma_control_slave		0xFF2030A0	[ expr $ifup_flag_lcd_dma_addr_translator | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_master_to_mm_slaves	$instance_name.pixel_dma_master	$ifup_flag_video_out_memory
	ifup_connect_slave_to_mm_masters	$instance_name.rgb_slave					0xFF203090	$ifup_flag_set_fpga_peripherals

	lock_avalon_base_address $instance_name.char_buffer_control_slave
	lock_avalon_base_address $instance_name.char_buffer_slave
	lock_avalon_base_address $instance_name.pixel_dma_control_slave
}
#
# Audio Subsystem
proc ifup_add_audio_subsystem { irq } {
	global ifup_default_audio_gen_lrclk
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_set_main_irq_receivers

	set instance_name	Audio_Subsystem

	add_instance $instance_name Audio_Subsystem

	set_interface_property audio_pll_ref_clk	EXPORT_OF $instance_name.audio_pll_ref_clk
	set_interface_property audio_pll_ref_reset	EXPORT_OF $instance_name.audio_pll_ref_reset
	set_interface_property audio_pll_clk		EXPORT_OF $instance_name.audio_pll_clk
	if {$ifup_default_audio_gen_lrclk == 1} {
		set_interface_property audio_clks_bclk	EXPORT_OF $instance_name.audio_clks_bclk
		set_interface_property audio_clks_lrclk	EXPORT_OF $instance_name.audio_clks_lrclk
	}
	set_interface_property audio				EXPORT_OF $instance_name.audio

	ifup_connect_system_clock			$instance_name.sys_clk
	ifup_connect_reset_sources			$instance_name.sys_reset
	ifup_connect_slave_to_mm_masters	$instance_name.audio_slave	0xFF203040	$ifup_flag_set_fpga_peripherals
	ifup_connect_irq_receivers			$instance_name.audio_irq	$irq		$ifup_flag_set_main_irq_receivers

	lock_avalon_base_address $instance_name.audio_slave
}
#
# Character LCD Controller
proc ifup_add_char_lcd_ctrl {} {
	global ifup_flag_set_fpga_peripherals

	set instance_name	Char_LCD_16x2
	set export_name		char_lcd
	set offset_data		0xFF203050

	add_instance $instance_name altera_up_avalon_character_lcd

	set_instance_parameter_value $instance_name cursor Both
	
	set_interface_property $export_name	EXPORT_OF $instance_name.external_interface

	ifup_connect_system_clock			$instance_name.clk
	ifup_connect_reset_sources			$instance_name.reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_lcd_slave	$offset_data $ifup_flag_set_fpga_peripherals

	lock_avalon_base_address $instance_name.avalon_lcd_slave
}
#
# Video In Subsystem
proc ifup_add_video_in_subsystem {} {
	global ifup_flag_video_in_dma_addr_translator
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_video_in_memory

	set instance_name	Video_In_Subsystem
	set export_name		video_in

	add_instance $instance_name Video_In_Subsystem

	set_interface_property $export_name	EXPORT_OF $instance_name.video_in

	ifup_connect_system_clock			$instance_name.sys_clk
	ifup_connect_reset_sources			$instance_name.sys_reset
	ifup_connect_slave_to_mm_masters	$instance_name.video_in_edge_detection_control_slave	0xFF203070	$ifup_flag_set_fpga_peripherals
	ifup_connect_slave_to_mm_masters	$instance_name.video_in_dma_control_slave				0xFF203060	[ expr $ifup_flag_video_in_dma_addr_translator | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_master_to_mm_slaves	$instance_name.video_in_dma_master	$ifup_flag_video_in_memory

	lock_avalon_base_address $instance_name.video_in_edge_detection_control_slave
	lock_avalon_base_address $instance_name.video_in_dma_control_slave
}
#
# Camera Subsystem
proc ifup_add_camera_subsystem {} {
	global ifup_flag_camera_dma_addr_translator
	global ifup_flag_set_fpga_data_masters
	global ifup_flag_set_fpga_peripherals
	global ifup_flag_video_in_memory

	set instance_name	Camera_Subsystem

	add_instance $instance_name Camera_Subsystem

	set_interface_property camera_config	EXPORT_OF $instance_name.camera_config
	set_interface_property camera_in		EXPORT_OF $instance_name.camera_in

	ifup_connect_system_clock			$instance_name.sys_clk
	ifup_connect_reset_sources			$instance_name.sys_reset
	ifup_connect_slave_to_mm_masters	$instance_name.avalon_av_config_slave		0xFF203080	$ifup_flag_set_fpga_peripherals
	ifup_connect_slave_to_mm_masters	$instance_name.edge_detection_control_slave	0xFF2030F0	$ifup_flag_set_fpga_peripherals
	ifup_connect_slave_to_mm_masters	$instance_name.camera_dma_control_slave		0xFF2030E0	[ expr $ifup_flag_camera_dma_addr_translator | $ifup_flag_set_fpga_data_masters ]
	ifup_connect_master_to_mm_slaves	$instance_name.camera_dma_master	$ifup_flag_video_in_memory

	lock_avalon_base_address $instance_name.avalon_av_config_slave
	lock_avalon_base_address $instance_name.edge_detection_control_slave
	lock_avalon_base_address $instance_name.camera_dma_control_slave
}
###############################################################################

