# 1. Create and map the work library
vlib work
vmap work work

# 2. Compile all SystemVerilog files
vlog -sv channel_capture.sv
vlog -sv signal_generator.sv
vlog -sv top.sv
vlog -sv testbench.sv

# 3. Launch the simulation with visibility (+acc)
vsim -voptargs="+acc" work.testbench

# 4. Add Waves
add wave -noupdate -divider "TOP LEVEL PINS"
add wave -noupdate -color "Yellow" /testbench/CLOCK_50
add wave -noupdate -color "Cyan"   /testbench/KEY
add wave -noupdate -label "Heartbeat_Out" /testbench/DUT/OUT_CH_1
add wave -noupdate -label "Burst_Out"     /testbench/DUT/OUT_CH_2

add wave -noupdate -divider "CPU BRIDGE (PIO)"
add wave -noupdate -group "CPU_Interface" -hex     /testbench/DUT/pio_la_address_reg
add wave -noupdate -group "CPU_Interface" -hex     /testbench/DUT/pio_la_status_in
add wave -noupdate -group "CPU_Interface" -hex     /testbench/DUT/pio_la_status_out
add wave -noupdate -group "CPU_Interface" -hex     /testbench/DUT/pio_la_trigger_ctrl
add wave -noupdate -group "CPU_Interface" -hex     /testbench/DUT/pio_la_data_reg
add wave -noupdate -group "CPU_Interface" -hex     /testbench/DUT/pio_la_trigger_capture

add wave -noupdate -divider "LA INTERNAL LOGIC"
add wave -noupdate -label "State" /testbench/DUT/la0/current_state
add wave -noupdate -group "Trigger_Detect" /testbench/DUT/la0/trigger_channel_data
add wave -noupdate -group "Trigger_Detect" /testbench/DUT/la0/trigger_current_data
add wave -noupdate -group "Trigger_Detect" /testbench/DUT/la0/trigger_past_data

add wave -noupdate -group "Counters" -decimal /testbench/DUT/la0/buffer_ptr
add wave -noupdate -group "Counters" -decimal /testbench/DUT/la0/pre_trigger_count
add wave -noupdate -group "Counters" -decimal /testbench/DUT/la0/post_trigger_count
add wave -noupdate -group "Counters" -decimal /testbench/DUT/la0/post_trigger_length

add wave -position insertpoint /testbench/DUT/la0/*
add wave -position insertpoint sim:/testbench/DUT/la0/buffer[10]

# 5. Configure Wave Window
configure wave -namecolwidth 250
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1

# 6. Run the simulation
# Note: Using run 200us instead of -all to prevent infinite hangs during debug
run 200us

# 7. Zoom to fit
wave zoom full