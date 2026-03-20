`timescale 1ns/1ps

module testbench;
    
    // --- Clock and Reset ---
    logic        CLOCK_50;
    logic [3:0]  KEY;

    // --- Observe outputs from 'top' ---
    logic [9:0]  LEDR;
    logic [6:0]  HEX0, HEX1, HEX2, HEX3, HEX4, HEX5;
    
    // --- Logic Analyzer Channels ---
    logic        IN_CH_1,  IN_CH_2,  IN_CH_3,  IN_CH_4;
    logic        OUT_CH_1, OUT_CH_2, OUT_CH_3, OUT_CH_4, OUT_CH_5;

    // ... (logic declarations for ports) ...
    logic [31:0] tb_la_addr, tb_la_status_in, tb_la_trig_ctrl;
    logic [31:0] tb_la_data, tb_la_status_out, tb_la_metadata;

  top DUT (
        // Clock and Reset
        .CLOCK_50               (CLOCK_50),
        .KEY                    (KEY),

        // CPU Bridge Ports (Connected to Testbench logic variables)
        .pio_la_address_reg     (tb_la_addr),
        .pio_la_status_in       (tb_la_status_in),
        .pio_la_trigger_ctrl    (tb_la_trig_ctrl),
        .pio_la_data_reg        (tb_la_data),
        .pio_la_status_out      (tb_la_status_out),
        .pio_la_trigger_capture (tb_la_metadata),

        // 7-Segment Displays
        .HEX0(HEX0), .HEX1(HEX1), .HEX2(HEX2), 
        .HEX3(HEX3), .HEX4(HEX4), .HEX5(HEX5),

        // LEDs
        .LEDR                   (LEDR),

        // Input channels (Loopback: watching our own generator)
        .IN_CH_1                (OUT_CH_1), // Heartbeat
        .IN_CH_2                (OUT_CH_2), // Burst
        .IN_CH_3                (OUT_CH_3), // Clock
        .IN_CH_4                (OUT_CH_4), // Logic 1

        // Output channels (From Signal Generator)
        .OUT_CH_1               (OUT_CH_1),
        .OUT_CH_2               (OUT_CH_2),
        .OUT_CH_3               (OUT_CH_3),
        .OUT_CH_4               (OUT_CH_4),
        .OUT_CH_5               (OUT_CH_5)
    );
    
    // 50 MHz clock generation (20ns period)
    initial begin
        CLOCK_50 = 0;
        forever #10 CLOCK_50 = ~CLOCK_50;
    end

    initial begin
        // --- 1. Initialization ---
        tb_la_addr = 0;
        tb_la_status_in = 0;
        tb_la_trig_ctrl = 0;
        KEY[0] = 1;

        // --- 2. System Reset ---
        #100;
        KEY[0] = 0; // Assert Reset
        #100;
        KEY[0] = 1; // De-assert Reset
        $display("[%0t] Reset complete.", $time);

        // --- 3. Arming the Analyzer ---
        // Set trigger for Channel 0 (Heartbeat)
        tb_la_trig_ctrl = 32'h0; 
        #100;
        // Set RUN bit (bit 0)
        tb_la_status_in = 32'h0000_0001; 
        $display("[%0t] Analyzer armed. Waiting for capture...", $time);

        // --- 4. Wait for Capture Completion ---
        // Bit 1 of status_out is BUFFER_FULL
        wait(tb_la_status_out[1] == 1);
        $display("[%0t] Capture Done! Buffer is full.", $time);

        // --- 5. Read One Instruction (Sample) ---
        // We set bit 31 (Read Enable) and choose address 10
        #100;
        tb_la_addr = {1'b1, 15'b0, 16'd10}; 
        #40; // Wait two clock cycles for sync and memory latency
        $display("[%0t] Read complete. Address 10 Data: 0x%h", $time, tb_la_data);

        // --- 6. Reset RUN bit and Re-arm ---
        #100;
        $display("[%0t] Clearing RUN bit...", $time);
        tb_la_status_in = 32'h0; // Stop/Reset state machine
        
        #500; // Small delay to observe IDLE state in wave
        
        $display("[%0t] Re-arming analyzer for second capture...", $time);
        tb_la_status_in = 32'h1; // RUN = 1 again

        // Final observation window
        #5000;
        $display("[%0t] Simulation complete.", $time);
        $stop;
    end
endmodule