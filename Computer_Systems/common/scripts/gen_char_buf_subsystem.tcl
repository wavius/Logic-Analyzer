source ../../common/scripts/common.tcl

create_system Char_Buf_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source

set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add Onchip Memory
add_instance Onchip_SRAM altera_avalon_onchip_memory2

set_instance_parameter_value Onchip_SRAM dualPort				true
set_instance_parameter_value Onchip_SRAM memorySize				8192
set_instance_parameter_value Onchip_SRAM singleClockOperation	true

set_interface_property char_buffer_slave	EXPORT_OF Onchip_SRAM.s1

add_connection Sys_Clk.clk			Onchip_SRAM.clk1
add_connection Sys_Clk.clk_reset	Onchip_SRAM.reset1
###############################################################################


###############################################################################
# Add character buffer dma for the vga
add_instance Char_Buf_DMA altera_up_avalon_video_dma_controller

set_instance_parameter_value Char_Buf_DMA addr_mode				"X-Y"
set_instance_parameter_value Char_Buf_DMA start_address			0x09000000
set_instance_parameter_value Char_Buf_DMA back_start_address	0x09000000
set_instance_parameter_value Char_Buf_DMA width					80
set_instance_parameter_value Char_Buf_DMA height				60
set_instance_parameter_value Char_Buf_DMA color_bits			8
set_instance_parameter_value Char_Buf_DMA color_planes			1
set_instance_parameter_value Char_Buf_DMA dma_enabled			true
set_instance_parameter_value Char_Buf_DMA mode					"From Memory to Stream"

set_interface_property char_buffer_control_slave	EXPORT_OF Char_Buf_DMA.avalon_dma_control_slave

add_connection Sys_Clk.clk						Char_Buf_DMA.clk
add_connection Sys_Clk.clk_reset				Char_Buf_DMA.reset
add_connection Char_Buf_DMA.avalon_dma_master	Onchip_SRAM.s2

set_connection_parameter_value Char_Buf_DMA.avalon_dma_master/Onchip_SRAM.s2	baseAddress "0x09000000"

lock_avalon_base_address	Onchip_SRAM.s2
###############################################################################


###############################################################################
# Add character buffer scaler for the vga
add_instance Char_Buf_Scaler altera_up_avalon_video_scaler

set_instance_parameter_value Char_Buf_Scaler color_bits			8
set_instance_parameter_value Char_Buf_Scaler color_planes		1
set_instance_parameter_value Char_Buf_Scaler width_scaling		8
set_instance_parameter_value Char_Buf_Scaler height_scaling		8
set_instance_parameter_value Char_Buf_Scaler include_channel	true
set_instance_parameter_value Char_Buf_Scaler width_in			80
set_instance_parameter_value Char_Buf_Scaler height_in			60

add_connection Sys_Clk.clk						Char_Buf_Scaler.clk
add_connection Sys_Clk.clk_reset				Char_Buf_Scaler.reset
add_connection Char_Buf_DMA.avalon_pixel_source	Char_Buf_Scaler.avalon_scaler_sink
###############################################################################


###############################################################################
# Add character buffer ASCII to Image Stream
add_instance ASCII_to_Image altera_up_avalon_video_ascii_to_image

set_instance_parameter_value ASCII_to_Image colour_format	"1-bit Black/White"

add_connection Sys_Clk.clk							ASCII_to_Image.clk
add_connection Sys_Clk.clk_reset					ASCII_to_Image.reset
add_connection Char_Buf_Scaler.avalon_scaler_source	ASCII_to_Image.avalon_ascii_sink
###############################################################################


###############################################################################
# Add character buffer rgb resampler for the vga
add_instance Char_Buf_RGB_Resampler altera_up_avalon_video_rgb_resampler

set_instance_parameter_value Char_Buf_RGB_Resampler alpha		1023
set_instance_parameter_value Char_Buf_RGB_Resampler input_type	"1-bit Black/White"
set_instance_parameter_value Char_Buf_RGB_Resampler output_type	"40-bit RGBA"

add_connection Sys_Clk.clk							Char_Buf_RGB_Resampler.clk
add_connection Sys_Clk.clk_reset					Char_Buf_RGB_Resampler.reset
add_connection ASCII_to_Image.avalon_image_source	Char_Buf_RGB_Resampler.avalon_rgb_sink
###############################################################################


###############################################################################
# Add character buffer change alpha
add_instance Set_Black_Transparent altera_up_avalon_video_change_alpha

set_instance_parameter_value Set_Black_Transparent alpha		0
set_instance_parameter_value Set_Black_Transparent color		0
set_instance_parameter_value Set_Black_Transparent color_bits	10
set_instance_parameter_value Set_Black_Transparent color_planes	4

set_interface_property avalon_char_source	EXPORT_OF Set_Black_Transparent.avalon_apply_alpha_source

add_connection Sys_Clk.clk								Set_Black_Transparent.clk
add_connection Sys_Clk.clk_reset						Set_Black_Transparent.reset
add_connection Char_Buf_RGB_Resampler.avalon_rgb_source	Set_Black_Transparent.avalon_apply_alpha_sink
###############################################################################

save_system Char_Buf_Subsystem.qsys

