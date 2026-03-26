# 1. Create and map the work library
vlib work
vmap work work

# 2. Compile all SystemVerilog files
vlog -sv signal_capture.sv
vlog -sv testbench.sv

# 3. Launch the simulation with visibility (+acc)
vsim -voptargs="+acc" work.tb_signal_capture

# 4. Add Waves
add wave -noupdate -divider "TESTBENCH PINS"
add wave -noupdate -color "Yellow" /tb_signal_capture/clk
add wave -noupdate -color "Cyan"   /tb_signal_capture/nreset
add wave -noupdate -hex            /tb_signal_capture/channel_in

add wave -noupdate -divider "AVALON-MM BUS"
add wave -noupdate -group "Avalon_Interface" -hex /tb_signal_capture/address
add wave -noupdate -group "Avalon_Interface"      /tb_signal_capture/write
add wave -noupdate -group "Avalon_Interface" -hex /tb_signal_capture/writedata
add wave -noupdate -group "Avalon_Interface"      /tb_signal_capture/read
add wave -noupdate -group "Avalon_Interface"      /tb_signal_capture/chipselect
add wave -noupdate -group "Avalon_Interface" -hex /tb_signal_capture/readdata

add wave -noupdate -divider "LA REGISTERS (HPS FACING)"
add wave -noupdate -hex /tb_signal_capture/uut/control_reg
add wave -noupdate -hex /tb_signal_capture/uut/trigger_config
add wave -noupdate -hex /tb_signal_capture/uut/read_pointer
add wave -noupdate      /tb_signal_capture/uut/run
add wave -noupdate      /tb_signal_capture/uut/buffer_full
add wave -noupdate      /tb_signal_capture/uut/triggered

add wave -noupdate -divider "LA INTERNAL LOGIC"
add wave -noupdate -label "State" /tb_signal_capture/uut/current_state
add wave -noupdate -group "Trigger_Detect" /tb_signal_capture/uut/trigger_channel
add wave -noupdate -group "Trigger_Detect" /tb_signal_capture/uut/trigger_current_data
add wave -noupdate -group "Trigger_Detect" /tb_signal_capture/uut/trigger_past_data
add wave -noupdate -group "Trigger_Detect" /tb_signal_capture/uut/rising_edge_detected

add wave -noupdate -group "Counters" -decimal /tb_signal_capture/uut/buffer_ptr
add wave -noupdate -group "Counters" -decimal /tb_signal_capture/uut/pre_trigger_count
add wave -noupdate -group "Counters" -decimal /tb_signal_capture/uut/post_trigger_count
add wave -noupdate -group "Counters" -decimal /tb_signal_capture/uut/post_trigger_length

# Add specific internal elements of interest
add wave -position insertpoint sim:/tb_signal_capture/uut/buffer[10]

# Add everything else just in case, nested in a group so it doesn't ruin the formatting
add wave -noupdate -group "Other_Internal_Signals" /tb_signal_capture/uut/*

# 5. Configure Wave Window
configure wave -namecolwidth 250
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1

# 6. Run the simulation
# (Running -all since the testbench has a $stop command built in)
run -all

# 7. Zoom to fit
wave zoom full