source ../../common/scripts/common.tcl

create_system Video_In_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source
set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add video in core
add_instance Video_In altera_up_avalon_video_decoder

set_instance_parameter_value Video_In video_source "On-board Video In (NTSC or PAL)"

set_interface_property video_in EXPORT_OF Video_In.external_interface

add_connection Sys_Clk.clk			Video_In.clk
add_connection Sys_Clk.clk_reset	Video_In.reset
###############################################################################


###############################################################################
# Add chroma resampler
add_instance Video_In_Chroma_Resampler altera_up_avalon_video_chroma_resampler

set_instance_parameter_value Video_In_Chroma_Resampler input_type	"YCrCb 422"
set_instance_parameter_value Video_In_Chroma_Resampler output_type	"YCrCb 444"

add_connection Sys_Clk.clk						Video_In_Chroma_Resampler.clk
add_connection Sys_Clk.clk_reset				Video_In_Chroma_Resampler.reset
add_connection Video_In.avalon_decoder_source	Video_In_Chroma_Resampler.avalon_chroma_sink
###############################################################################


###############################################################################
# Add edge detection subsystem
add_instance Video_In_Edge_Detection_Subsystem Video_In_Edge_Detection_Subsystem

set_interface_property video_in_edge_detection_control_slave EXPORT_OF Video_In_Edge_Detection_Subsystem.edge_detection_control_slave

add_connection Sys_Clk.clk										Video_In_Edge_Detection_Subsystem.sys_clk
add_connection Sys_Clk.clk_reset								Video_In_Edge_Detection_Subsystem.sys_reset
add_connection Video_In_Chroma_Resampler.avalon_chroma_source	Video_In_Edge_Detection_Subsystem.video_stream_sink
###############################################################################


###############################################################################
# Add color space converter
add_instance Video_In_CSC altera_up_avalon_video_csc

set_instance_parameter_value Video_In_CSC csc_type "444 YCrCb to 24-bit RGB"

add_connection Sys_Clk.clk												Video_In_CSC.clk
add_connection Sys_Clk.clk_reset										Video_In_CSC.reset
add_connection Video_In_Edge_Detection_Subsystem.video_stream_source	Video_In_CSC.avalon_csc_sink
###############################################################################


###############################################################################
# Add rgb resampler
add_instance Video_In_RGB_Resampler altera_up_avalon_video_rgb_resampler

set_instance_parameter_value Video_In_RGB_Resampler input_type	"24-bit RGB"
set_instance_parameter_value Video_In_RGB_Resampler output_type	"16-bit RGB"

add_connection Sys_Clk.clk						Video_In_RGB_Resampler.clk
add_connection Sys_Clk.clk_reset				Video_In_RGB_Resampler.reset
add_connection Video_In_CSC.avalon_csc_source	Video_In_RGB_Resampler.avalon_rgb_sink
###############################################################################


###############################################################################
# Add clipper
add_instance Video_In_Clipper altera_up_avalon_video_clipper

set_instance_parameter_value Video_In_Clipper width_in			720
set_instance_parameter_value Video_In_Clipper height_in			244
set_instance_parameter_value Video_In_Clipper drop_left			40
set_instance_parameter_value Video_In_Clipper drop_right		40
set_instance_parameter_value Video_In_Clipper drop_top			2
set_instance_parameter_value Video_In_Clipper drop_bottom		2
set_instance_parameter_value Video_In_Clipper add_left			0
set_instance_parameter_value Video_In_Clipper add_right			0
set_instance_parameter_value Video_In_Clipper add_top			0
set_instance_parameter_value Video_In_Clipper add_bottom		0
set_instance_parameter_value Video_In_Clipper add_value_plane_1	0
set_instance_parameter_value Video_In_Clipper color_bits		16
set_instance_parameter_value Video_In_Clipper color_planes		1

add_connection Sys_Clk.clk								Video_In_Clipper.clk
add_connection Sys_Clk.clk_reset						Video_In_Clipper.reset
add_connection Video_In_RGB_Resampler.avalon_rgb_source	Video_In_Clipper.avalon_clipper_sink
###############################################################################


###############################################################################
# Add scaler
add_instance Video_In_Scaler altera_up_avalon_video_scaler

set_instance_parameter_value Video_In_Scaler width_scaling	0.5
set_instance_parameter_value Video_In_Scaler height_scaling	1
set_instance_parameter_value Video_In_Scaler width_in		640
set_instance_parameter_value Video_In_Scaler height_in		240
set_instance_parameter_value Video_In_Scaler color_bits		16
set_instance_parameter_value Video_In_Scaler color_planes	1

add_connection Sys_Clk.clk								Video_In_Scaler.clk
add_connection Sys_Clk.clk_reset						Video_In_Scaler.reset
add_connection Video_In_Clipper.avalon_clipper_source	Video_In_Scaler.avalon_scaler_sink
###############################################################################


###############################################################################
# Add dma controller
add_instance Video_In_DMA altera_up_avalon_video_dma_controller

set_instance_parameter_value Video_In_DMA mode					"From Stream to Memory" 
set_instance_parameter_value Video_In_DMA addr_mode				"X-Y"
set_instance_parameter_value Video_In_DMA start_address			0x08000000
set_instance_parameter_value Video_In_DMA back_start_address	0x08000000
set_instance_parameter_value Video_In_DMA width					320
set_instance_parameter_value Video_In_DMA height				240
set_instance_parameter_value Video_In_DMA color_bits			16
set_instance_parameter_value Video_In_DMA color_planes			1
set_instance_parameter_value Video_In_DMA dma_enabled			false

set_interface_property video_in_dma_control_slave	EXPORT_OF Video_In_DMA.avalon_dma_control_slave
set_interface_property video_in_dma_master			EXPORT_OF Video_In_DMA.avalon_dma_master

add_connection Sys_Clk.clk							Video_In_DMA.clk
add_connection Sys_Clk.clk_reset					Video_In_DMA.reset
add_connection Video_In_Scaler.avalon_scaler_source	Video_In_DMA.avalon_dma_sink
###############################################################################

save_system Video_In_Subsystem.qsys

