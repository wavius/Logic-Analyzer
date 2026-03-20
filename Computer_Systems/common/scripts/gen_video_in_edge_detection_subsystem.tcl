source ../../common/scripts/common.tcl

create_system Video_In_Edge_Detection_Subsystem

source project_properties.tcl

###############################################################################
# Add system clock
add_instance Sys_Clk clock_source
set_instance_parameter_value Sys_Clk clockFrequencyKnown false

set_interface_property sys_clk		EXPORT_OF Sys_Clk.clk_in
set_interface_property sys_reset	EXPORT_OF Sys_Clk.clk_in_reset
###############################################################################


###############################################################################
# Add video splitter
add_instance Video_Stream_Splitter altera_up_avalon_video_stream_router

set_instance_parameter_value Video_Stream_Splitter router_type	"Split"
set_instance_parameter_value Video_Stream_Splitter sync			true
set_instance_parameter_value Video_Stream_Splitter color_bits	8
set_instance_parameter_value Video_Stream_Splitter color_planes	3

set_interface_property video_stream_sink EXPORT_OF Video_Stream_Splitter.avalon_stream_router_sink

add_connection Sys_Clk.clk			Video_Stream_Splitter.clk
add_connection Sys_Clk.clk_reset	Video_Stream_Splitter.reset
###############################################################################


###############################################################################
# Add edge detection router controller
add_instance Edge_Detection_Router_Controller altera_avalon_pio

set_instance_parameter_value Edge_Detection_Router_Controller direction				Output
set_instance_parameter_value Edge_Detection_Router_Controller width					1
set_instance_parameter_value Edge_Detection_Router_Controller bitModifyingOutReg	false

set_interface_property edge_detection_control_slave EXPORT_OF Edge_Detection_Router_Controller.s1

add_connection Sys_Clk.clk								Edge_Detection_Router_Controller.clk
add_connection Sys_Clk.clk_reset						Edge_Detection_Router_Controller.reset
add_connection Video_Stream_Splitter.external_interface	Edge_Detection_Router_Controller.external_connection
###############################################################################


###############################################################################
# Add chroma resampler
add_instance Chroma_Filter altera_up_avalon_video_chroma_resampler

set_instance_parameter_value Chroma_Filter input_type	"YCrCb 444"
set_instance_parameter_value Chroma_Filter output_type	"Y only"

add_connection Sys_Clk.clk											Chroma_Filter.clk
add_connection Sys_Clk.clk_reset									Chroma_Filter.reset
add_connection Video_Stream_Splitter.avalon_stream_router_source_1	Chroma_Filter.avalon_chroma_sink
###############################################################################


###############################################################################
# Add edge detection
add_instance Edge_Detection altera_up_avalon_video_edge_detection

set_instance_parameter_value Edge_Detection width		720
set_instance_parameter_value Edge_Detection intensity	1

add_connection Sys_Clk.clk							Edge_Detection.clk
add_connection Sys_Clk.clk_reset					Edge_Detection.reset
add_connection Chroma_Filter.avalon_chroma_source	Edge_Detection.avalon_edge_detection_sink
###############################################################################


###############################################################################
# Add chroma resampler
add_instance Chroma_Upsampler altera_up_avalon_video_chroma_resampler

set_instance_parameter_value Chroma_Upsampler input_type	"Y only"
set_instance_parameter_value Chroma_Upsampler output_type	"YCrCb 444"

add_connection Sys_Clk.clk									Chroma_Upsampler.clk
add_connection Sys_Clk.clk_reset							Chroma_Upsampler.reset
add_connection Edge_Detection.avalon_edge_detection_source	Chroma_Upsampler.avalon_chroma_sink
###############################################################################


###############################################################################
# Add video merger
add_instance Video_Stream_Merger altera_up_avalon_video_stream_router

set_instance_parameter_value Video_Stream_Merger router_type	"Merge"
set_instance_parameter_value Video_Stream_Merger sync			true
set_instance_parameter_value Video_Stream_Merger color_bits		8
set_instance_parameter_value Video_Stream_Merger color_planes	3

set_interface_property video_stream_source EXPORT_OF Video_Stream_Merger.avalon_stream_router_source

add_connection Sys_Clk.clk											Video_Stream_Merger.clk
add_connection Sys_Clk.clk_reset									Video_Stream_Merger.reset
add_connection Video_Stream_Splitter.avalon_stream_router_source_0	Video_Stream_Merger.avalon_stream_router_sink_0
add_connection Chroma_Upsampler.avalon_chroma_source				Video_Stream_Merger.avalon_stream_router_sink_1
add_connection Video_Stream_Splitter.avalon_sync_source				Video_Stream_Merger.avalon_sync_sink
###############################################################################

save_system Video_In_Edge_Detection_Subsystem.qsys

