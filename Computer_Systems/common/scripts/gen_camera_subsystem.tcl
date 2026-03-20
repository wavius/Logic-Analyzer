source ../../common/scripts/common.tcl

create_system Camera_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source
set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add Audio/Video Configuration
add_instance Camera_Config altera_up_avalon_audio_and_video_config

set_instance_parameter_value Camera_Config device			"5 Megapixel Camera (TRDB_D5M)"
set_instance_parameter_value Camera_Config eai				true
set_instance_parameter_value Camera_Config d5m_resolution	"1280 x 720"
set_instance_parameter_value Camera_Config exposure			true

set_interface_property camera_config			EXPORT_OF Camera_Config.external_interface
set_interface_property avalon_av_config_slave	EXPORT_OF Camera_Config.avalon_av_config_slave

add_connection Sys_Clk.clk			Camera_Config.clk
add_connection Sys_Clk.clk_reset	Camera_Config.reset
###############################################################################


###############################################################################
# Add video in core
add_instance Camera_In altera_up_avalon_video_decoder

set_instance_parameter_value Camera_In video_source "5MP Digital Camera (THDB_D5M)"

set_interface_property camera_in EXPORT_OF Camera_In.external_interface

add_connection Sys_Clk.clk			Camera_In.clk
add_connection Sys_Clk.clk_reset	Camera_In.reset
###############################################################################


###############################################################################
# Add bayer pattern resampler
add_instance Camera_Bayer_Resampler altera_up_avalon_video_bayer_resampler

set_instance_parameter_value Camera_Bayer_Resampler device				"5MP Digital Camera (THDB_D5M)"
set_instance_parameter_value Camera_Bayer_Resampler use_custom_format	false
set_instance_parameter_value Camera_Bayer_Resampler custom_width_in		1280
set_instance_parameter_value Camera_Bayer_Resampler custom_height_in	720

add_connection Sys_Clk.clk						Camera_Bayer_Resampler.clk
add_connection Sys_Clk.clk_reset				Camera_Bayer_Resampler.reset
add_connection Camera_In.avalon_decoder_source	Camera_Bayer_Resampler.avalon_bayer_sink
###############################################################################


###############################################################################
# Add edge detection subsystem
add_instance Edge_Detection_Subsystem Camera_Edge_Detection_Subsystem

set_interface_property edge_detection_control_slave EXPORT_OF Edge_Detection_Subsystem.edge_detection_control_slave

add_connection Sys_Clk.clk									Edge_Detection_Subsystem.sys_clk
add_connection Sys_Clk.clk_reset							Edge_Detection_Subsystem.sys_reset
add_connection Camera_Bayer_Resampler.avalon_bayer_source	Edge_Detection_Subsystem.video_stream_sink
###############################################################################


###############################################################################
# Add scaler
add_instance Camera_Scaler altera_up_avalon_video_scaler

set_instance_parameter_value Camera_Scaler width_scaling	0.75
set_instance_parameter_value Camera_Scaler height_scaling	0.75
set_instance_parameter_value Camera_Scaler width_in			640
set_instance_parameter_value Camera_Scaler height_in		360
set_instance_parameter_value Camera_Scaler color_bits		8
set_instance_parameter_value Camera_Scaler color_planes		3

add_connection Sys_Clk.clk									Camera_Scaler.clk
add_connection Sys_Clk.clk_reset							Camera_Scaler.reset
add_connection Edge_Detection_Subsystem.video_stream_source	Camera_Scaler.avalon_scaler_sink
###############################################################################


###############################################################################
# Add clipper
add_instance Camera_Clipper altera_up_avalon_video_clipper

set_instance_parameter_value Camera_Clipper width_in			480
set_instance_parameter_value Camera_Clipper height_in			270
set_instance_parameter_value Camera_Clipper drop_left			40
set_instance_parameter_value Camera_Clipper drop_right			40
set_instance_parameter_value Camera_Clipper drop_top			15
set_instance_parameter_value Camera_Clipper drop_bottom			15
set_instance_parameter_value Camera_Clipper add_left			0
set_instance_parameter_value Camera_Clipper add_right			0
set_instance_parameter_value Camera_Clipper add_top				0
set_instance_parameter_value Camera_Clipper add_bottom			0
set_instance_parameter_value Camera_Clipper add_value_plane_1	0
set_instance_parameter_value Camera_Clipper color_bits			8
set_instance_parameter_value Camera_Clipper color_planes		3

add_connection Sys_Clk.clk							Camera_Clipper.clk
add_connection Sys_Clk.clk_reset					Camera_Clipper.reset
add_connection Camera_Scaler.avalon_scaler_source	Camera_Clipper.avalon_clipper_sink
###############################################################################


###############################################################################
# Add rgb resampler
add_instance Camera_RGB_Resampler altera_up_avalon_video_rgb_resampler

set_instance_parameter_value Camera_RGB_Resampler input_type	"24-bit RGB"
set_instance_parameter_value Camera_RGB_Resampler output_type	"16-bit RGB"

add_connection Sys_Clk.clk							Camera_RGB_Resampler.clk
add_connection Sys_Clk.clk_reset					Camera_RGB_Resampler.reset
add_connection Camera_Clipper.avalon_clipper_source	Camera_RGB_Resampler.avalon_rgb_sink
###############################################################################


###############################################################################
# Add dma controller
add_instance Camera_DMA altera_up_avalon_video_dma_controller

set_instance_parameter_value Camera_DMA mode						"From Stream to Memory" 
set_instance_parameter_value Camera_DMA addr_mode				"X-Y"
set_instance_parameter_value Camera_DMA start_address			0x08100000
set_instance_parameter_value Camera_DMA back_start_address	0x08100000
set_instance_parameter_value Camera_DMA width					400
set_instance_parameter_value Camera_DMA height					240
set_instance_parameter_value Camera_DMA color_bits				16
set_instance_parameter_value Camera_DMA color_planes			1
set_instance_parameter_value Camera_DMA dma_enabled			false

set_interface_property camera_dma_control_slave	EXPORT_OF Camera_DMA.avalon_dma_control_slave
set_interface_property camera_dma_master			EXPORT_OF Camera_DMA.avalon_dma_master

add_connection Sys_Clk.clk										Camera_DMA.clk
add_connection Sys_Clk.clk_reset								Camera_DMA.reset
add_connection Camera_RGB_Resampler.avalon_rgb_source	Camera_DMA.avalon_dma_sink
###############################################################################

save_system Camera_Subsystem.qsys

