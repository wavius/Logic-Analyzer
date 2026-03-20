source ../../common/scripts/common.tcl

create_system VGA_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source
set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add vga clock
add_instance VGA_Clk clock_source
set_instance_parameter_value VGA_Clk clockFrequencyKnown false

set_interface_property vga_clk		EXPORT_OF VGA_Clk.clk_in
set_interface_property vga_reset	EXPORT_OF VGA_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add pixel dma for the vga
add_instance VGA_Pixel_DMA altera_up_avalon_video_dma_controller

set_instance_parameter_value VGA_Pixel_DMA addr_mode			"X-Y"
set_instance_parameter_value VGA_Pixel_DMA start_address		0x08000000
set_instance_parameter_value VGA_Pixel_DMA back_start_address	0x08000000
set_instance_parameter_value VGA_Pixel_DMA width				$ifup_default_vga_buffer_width
set_instance_parameter_value VGA_Pixel_DMA height				$ifup_default_vga_buffer_height
set_instance_parameter_value VGA_Pixel_DMA color_bits			$ifup_default_vga_bits_per_color
set_instance_parameter_value VGA_Pixel_DMA color_planes			1
set_instance_parameter_value VGA_Pixel_DMA dma_enabled			true
set_instance_parameter_value VGA_Pixel_DMA mode					"From Memory to Stream"

set_interface_property pixel_dma_master			EXPORT_OF VGA_Pixel_DMA.avalon_dma_master
set_interface_property pixel_dma_control_slave	EXPORT_OF VGA_Pixel_DMA.avalon_dma_control_slave

add_connection Sys_Clk.clk			VGA_Pixel_DMA.clk
add_connection Sys_Clk.clk_reset	VGA_Pixel_DMA.reset
###############################################################################


###############################################################################
# Add pixel fifo for the vga
add_instance VGA_Pixel_FIFO altera_up_avalon_video_dual_clock_buffer

set_instance_parameter_value VGA_Pixel_FIFO color_bits		$ifup_default_vga_bits_per_color
set_instance_parameter_value VGA_Pixel_FIFO color_planes	1

add_connection Sys_Clk.clk							VGA_Pixel_FIFO.clock_stream_in
add_connection Sys_Clk.clk_reset					VGA_Pixel_FIFO.reset_stream_in
add_connection Sys_Clk.clk							VGA_Pixel_FIFO.clock_stream_out
add_connection Sys_Clk.clk_reset					VGA_Pixel_FIFO.reset_stream_out
add_connection VGA_Pixel_DMA.avalon_pixel_source	VGA_Pixel_FIFO.avalon_dc_buffer_sink
###############################################################################


###############################################################################
# Add pixel rgb resampler for the vga
add_instance VGA_Pixel_RGB_Resampler altera_up_avalon_video_rgb_resampler

set_instance_parameter_value VGA_Pixel_RGB_Resampler input_type		$ifup_default_vga_rgb_resampler_input
set_instance_parameter_value VGA_Pixel_RGB_Resampler output_type	"30-bit RGB"

set_interface_property rgb_slave	EXPORT_OF VGA_Pixel_RGB_Resampler.avalon_rgb_slave

add_connection Sys_Clk.clk								VGA_Pixel_RGB_Resampler.clk
add_connection Sys_Clk.clk_reset						VGA_Pixel_RGB_Resampler.reset
add_connection VGA_Pixel_FIFO.avalon_dc_buffer_source	VGA_Pixel_RGB_Resampler.avalon_rgb_sink
###############################################################################


###############################################################################
# Add pixel scaler for the vga
add_instance VGA_Pixel_Scaler altera_up_avalon_video_scaler

set_instance_parameter_value VGA_Pixel_Scaler width_scaling		$ifup_default_vga_width_scaling
set_instance_parameter_value VGA_Pixel_Scaler height_scaling	$ifup_default_vga_height_scaling
set_instance_parameter_value VGA_Pixel_Scaler width_in			$ifup_default_vga_buffer_width
set_instance_parameter_value VGA_Pixel_Scaler height_in			$ifup_default_vga_buffer_height
set_instance_parameter_value VGA_Pixel_Scaler color_bits		10
set_instance_parameter_value VGA_Pixel_Scaler color_planes		3

add_connection Sys_Clk.clk									VGA_Pixel_Scaler.clk
add_connection Sys_Clk.clk_reset							VGA_Pixel_Scaler.reset
add_connection VGA_Pixel_RGB_Resampler.avalon_rgb_source	VGA_Pixel_Scaler.avalon_scaler_sink
###############################################################################


###############################################################################
# Add Char Buf Subsystem
add_instance Char_Buf_Subsystem Char_Buf_Subsystem

set_interface_property char_buffer_slave			EXPORT_OF Char_Buf_Subsystem.char_buffer_slave
set_interface_property char_buffer_control_slave	EXPORT_OF Char_Buf_Subsystem.char_buffer_control_slave

add_connection Sys_Clk.clk			Char_Buf_Subsystem.sys_clk
add_connection Sys_Clk.clk_reset	Char_Buf_Subsystem.sys_reset
###############################################################################


###############################################################################
# Add alpha blender for the vga
add_instance VGA_Alpha_Blender altera_up_avalon_video_alpha_blender

set_instance_parameter_value VGA_Alpha_Blender mode "Simple"

add_connection Sys_Clk.clk								VGA_Alpha_Blender.clk
add_connection Sys_Clk.clk_reset						VGA_Alpha_Blender.reset
add_connection VGA_Pixel_Scaler.avalon_scaler_source	VGA_Alpha_Blender.avalon_background_sink
add_connection Char_Buf_Subsystem.avalon_char_source	VGA_Alpha_Blender.avalon_foreground_sink
###############################################################################


###############################################################################
# Add pixel fifo for the vga
add_instance VGA_Dual_Clock_FIFO altera_up_avalon_video_dual_clock_buffer

set_instance_parameter_value VGA_Dual_Clock_FIFO color_bits		10
set_instance_parameter_value VGA_Dual_Clock_FIFO color_planes	3

add_connection Sys_Clk.clk								VGA_Dual_Clock_FIFO.clock_stream_in
add_connection Sys_Clk.clk_reset						VGA_Dual_Clock_FIFO.reset_stream_in
add_connection VGA_Clk.clk								VGA_Dual_Clock_FIFO.clock_stream_out
add_connection VGA_Clk.clk_reset						VGA_Dual_Clock_FIFO.reset_stream_out
add_connection VGA_Alpha_Blender.avalon_blended_source	VGA_Dual_Clock_FIFO.avalon_dc_buffer_sink
###############################################################################


###############################################################################
# Add the vga controller
add_instance VGA_Controller altera_up_avalon_video_vga_controller

set_instance_parameter_value VGA_Controller board			$board_name
set_instance_parameter_value VGA_Controller device			"VGA Connector"
set_instance_parameter_value VGA_Controller resolution		"VGA 640x480"

set_interface_property vga EXPORT_OF VGA_Controller.external_interface

add_connection VGA_Clk.clk									VGA_Controller.clk
add_connection VGA_Clk.clk_reset							VGA_Controller.reset
add_connection VGA_Dual_Clock_FIFO.avalon_dc_buffer_source	VGA_Controller.avalon_vga_sink
###############################################################################

save_system VGA_Subsystem.qsys

