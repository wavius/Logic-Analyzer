source ../../common/scripts/common.tcl

create_system LCD_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source
set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add lcd clock
add_instance LCD_Clk clock_source
set_instance_parameter_value LCD_Clk clockFrequencyKnown false

set_interface_property lcd_clk		EXPORT_OF LCD_Clk.clk_in
set_interface_property lcd_reset	EXPORT_OF LCD_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add pixel dma for the lcd
add_instance LCD_Pixel_DMA altera_up_avalon_video_dma_controller

set_instance_parameter_value LCD_Pixel_DMA addr_mode			"X-Y"
set_instance_parameter_value LCD_Pixel_DMA start_address		0x08100000
set_instance_parameter_value LCD_Pixel_DMA back_start_address	0x08100000
set_instance_parameter_value LCD_Pixel_DMA width				400
set_instance_parameter_value LCD_Pixel_DMA height				240
set_instance_parameter_value LCD_Pixel_DMA color_bits			16
set_instance_parameter_value LCD_Pixel_DMA color_planes			1
set_instance_parameter_value LCD_Pixel_DMA dma_enabled			true
set_instance_parameter_value LCD_Pixel_DMA mode					"From Memory to Stream"

set_interface_property pixel_dma_master			EXPORT_OF LCD_Pixel_DMA.avalon_dma_master
set_interface_property pixel_dma_control_slave	EXPORT_OF LCD_Pixel_DMA.avalon_dma_control_slave

add_connection Sys_Clk.clk			LCD_Pixel_DMA.clk
add_connection Sys_Clk.clk_reset	LCD_Pixel_DMA.reset
###############################################################################


###############################################################################
# Add pixel fifo for the lcd
add_instance LCD_Pixel_FIFO altera_up_avalon_video_dual_clock_buffer

set_instance_parameter_value LCD_Pixel_FIFO color_bits		16
set_instance_parameter_value LCD_Pixel_FIFO color_planes	1

add_connection Sys_Clk.clk							LCD_Pixel_FIFO.clock_stream_in
add_connection Sys_Clk.clk_reset					LCD_Pixel_FIFO.reset_stream_in
add_connection Sys_Clk.clk							LCD_Pixel_FIFO.clock_stream_out
add_connection Sys_Clk.clk_reset					LCD_Pixel_FIFO.reset_stream_out
add_connection LCD_Pixel_DMA.avalon_pixel_source	LCD_Pixel_FIFO.avalon_dc_buffer_sink
###############################################################################


###############################################################################
# Add pixel rgb resampler for the lcd
add_instance LCD_Pixel_RGB_Resampler altera_up_avalon_video_rgb_resampler

set_instance_parameter_value LCD_Pixel_RGB_Resampler input_type		"16-bit RGB"
set_instance_parameter_value LCD_Pixel_RGB_Resampler output_type	"30-bit RGB"

set_interface_property rgb_slave	EXPORT_OF LCD_Pixel_RGB_Resampler.avalon_rgb_slave

add_connection Sys_Clk.clk								LCD_Pixel_RGB_Resampler.clk
add_connection Sys_Clk.clk_reset						LCD_Pixel_RGB_Resampler.reset
add_connection LCD_Pixel_FIFO.avalon_dc_buffer_source	LCD_Pixel_RGB_Resampler.avalon_rgb_sink
###############################################################################


###############################################################################
# Add pixel scaler for the lcd
add_instance LCD_Pixel_Scaler altera_up_avalon_video_scaler

set_instance_parameter_value LCD_Pixel_Scaler width_scaling		2
set_instance_parameter_value LCD_Pixel_Scaler height_scaling	2
set_instance_parameter_value LCD_Pixel_Scaler width_in			400
set_instance_parameter_value LCD_Pixel_Scaler height_in			240
set_instance_parameter_value LCD_Pixel_Scaler color_bits		10
set_instance_parameter_value LCD_Pixel_Scaler color_planes		3

add_connection Sys_Clk.clk									LCD_Pixel_Scaler.clk
add_connection Sys_Clk.clk_reset							LCD_Pixel_Scaler.reset
add_connection LCD_Pixel_RGB_Resampler.avalon_rgb_source	LCD_Pixel_Scaler.avalon_scaler_sink
###############################################################################


###############################################################################
# Add LCD Char Buf Subsystem
add_instance LCD_Char_Buf_Subsystem LCD_Char_Buf_Subsystem

set_interface_property char_buffer_slave			EXPORT_OF LCD_Char_Buf_Subsystem.char_buffer_slave
set_interface_property char_buffer_control_slave	EXPORT_OF LCD_Char_Buf_Subsystem.char_buffer_control_slave

add_connection Sys_Clk.clk			LCD_Char_Buf_Subsystem.sys_clk
add_connection Sys_Clk.clk_reset	LCD_Char_Buf_Subsystem.sys_reset
###############################################################################


###############################################################################
# Add alpha blender for the lcd
add_instance LCD_Alpha_Blender altera_up_avalon_video_alpha_blender

set_instance_parameter_value LCD_Alpha_Blender mode "Simple"

add_connection Sys_Clk.clk									LCD_Alpha_Blender.clk
add_connection Sys_Clk.clk_reset							LCD_Alpha_Blender.reset
add_connection LCD_Pixel_Scaler.avalon_scaler_source		LCD_Alpha_Blender.avalon_background_sink
add_connection LCD_Char_Buf_Subsystem.avalon_char_source	LCD_Alpha_Blender.avalon_foreground_sink
###############################################################################


###############################################################################
# Add pixel fifo for the lcd
add_instance LCD_Dual_Clock_FIFO altera_up_avalon_video_dual_clock_buffer

set_instance_parameter_value LCD_Dual_Clock_FIFO color_bits		10
set_instance_parameter_value LCD_Dual_Clock_FIFO color_planes	3

add_connection Sys_Clk.clk								LCD_Dual_Clock_FIFO.clock_stream_in
add_connection Sys_Clk.clk_reset						LCD_Dual_Clock_FIFO.reset_stream_in
add_connection LCD_Clk.clk								LCD_Dual_Clock_FIFO.clock_stream_out
add_connection LCD_Clk.clk_reset						LCD_Dual_Clock_FIFO.reset_stream_out
add_connection LCD_Alpha_Blender.avalon_blended_source	LCD_Dual_Clock_FIFO.avalon_dc_buffer_sink
###############################################################################


###############################################################################
# Add the lcd controller
add_instance LCD_Controller altera_up_avalon_video_vga_controller

set_instance_parameter_value LCD_Controller board			"DE2-115"
set_instance_parameter_value LCD_Controller device			"7\" LCD on VEEK-MT and MTL/MTL2 Modules"

set_interface_property lcd EXPORT_OF LCD_Controller.external_interface

add_connection LCD_Clk.clk									LCD_Controller.clk
add_connection LCD_Clk.clk_reset							LCD_Controller.reset
add_connection LCD_Dual_Clock_FIFO.avalon_dc_buffer_source	LCD_Controller.avalon_vga_sink
###############################################################################

save_system LCD_Subsystem.qsys

