source ../../common/scripts/common.tcl

create_system Audio_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source
set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add audio pll
add_instance Audio_PLL altera_up_avalon_audio_pll

set_instance_parameter_value Audio_PLL gui_refclk		50.0
set_instance_parameter_value Audio_PLL audio_clk_freq	$ifup_default_audio_clk_freq

set_interface_property audio_pll_ref_clk	EXPORT_OF Audio_PLL.ref_clk
set_interface_property audio_pll_ref_reset	EXPORT_OF Audio_PLL.ref_reset
set_interface_property audio_pll_clk		EXPORT_OF Audio_PLL.audio_clk
set_interface_property audio_pll_reset		EXPORT_OF Audio_PLL.reset_source
###############################################################################


if {$ifup_default_audio_gen_lrclk == 1} {
###############################################################################
# Add audio clock generator
add_instance Audio_LRCLK intel_up_avalon_audio_clks

set_instance_parameter_value Audio_LRCLK dw	32

set_interface_property audio_clks_bclk	EXPORT_OF Audio_LRCLK.AUD_BCLK
set_interface_property audio_clks_lrclk	EXPORT_OF Audio_LRCLK.AUD_LRCLK

add_connection Sys_Clk.clk			Audio_LRCLK.clk
add_connection Sys_Clk.clk_reset	Audio_LRCLK.reset
###############################################################################
}


###############################################################################
# Add audio core
add_instance Audio altera_up_avalon_audio

set_instance_parameter_value Audio avalon_bus_type	"Memory Mapped"
set_instance_parameter_value Audio audio_in			$ifup_default_audio_in_enabled
set_instance_parameter_value Audio audio_out		$ifup_default_audio_out_enabled
set_instance_parameter_value Audio dw				32

set_interface_property audio_slave	EXPORT_OF Audio.avalon_audio_slave
set_interface_property audio_irq	EXPORT_OF Audio.interrupt
set_interface_property audio		EXPORT_OF Audio.external_interface

add_connection Sys_Clk.clk			Audio.clk
add_connection Sys_Clk.clk_reset	Audio.reset
###############################################################################

save_system Audio_Subsystem.qsys

