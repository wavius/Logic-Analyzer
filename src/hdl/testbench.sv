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

    // Instantiate DUT
    top DUT (
        .CLOCK_50 (CLOCK_50),
        .KEY      (KEY),
        .LEDR     (LEDR),
        .HEX0(HEX0), .HEX1(HEX1), .HEX2(HEX2), .HEX3(HEX3), .HEX4(HEX4), .HEX5(HEX5),
        .OUT_CH_1 (OUT_CH_1), .OUT_CH_2 (OUT_CH_2), .OUT_CH_3 (OUT_CH_3),
        .OUT_CH_4 (OUT_CH_4), .OUT_CH_5 (OUT_CH_5),

        // Loopback: Connect signal generator outputs to analyzer inputs for testing
        .IN_CH_1  (OUT_CH_1), // Watch Heartbeat
        .IN_CH_2  (OUT_CH_2), // Watch Burst
        .IN_CH_3  (OUT_CH_3), 
        .IN_CH_4  (OUT_CH_4)
    );

    // 50 MHz clock
    initial CLOCK_50 = 0;
    always #10 CLOCK_50 = ~CLOCK_50;

    // Simulation stimulus
    initial begin
        // --- 1. System Reset ---
        KEY = 4'b1111; 
        #5;
        KEY[0] = 0;    // Press Reset
        #100;
        KEY[0] = 1;    // Release Reset
        $display("[%0t] Reset released.", $time);

        // --- 2. Configure Trigger (Via Hierarchical Reference) ---
        // Setting trigger to watch Channel 1 (Heartbeat)
        // [31:16] = Channel (0), [15:0] = Type (Rising edge is hardcoded currently)
        DUT.la_trigger_ctrl = {16'd0, 16'd0}; 
        
        // --- 3. Arm the Logic Analyzer ---
        // Set the RUN bit (bit 0) in the status_in_reg
        #100;
        DUT.la_status_in = 32'h0000_0001; 
        $display("[%0t] Analyzer armed. Waiting for trigger on Heartbeat...", $time);

        // --- 4. Wait for Trigger and Completion ---
        // Wait until status_out_reg indicates TRIGGERED (bit 2) and BUFFER_FULL (bit 1)
        wait(DUT.la_status_out[2] == 1);
        $display("[%0t] *** TRIGGER DETECTED! ***", $time);
        
        wait(DUT.la_status_out[1] == 1);
        $display("[%0t] Capture complete. Buffer is full.", $time);

        // --- 5. Read Back Data ---
        $display("[%0t] Reading trigger capture metadata:", $time);
        $display("      Pre-trigger samples: %0d", DUT.la_trigger_capture[15:0]);
        $display("      Post-trigger samples: %0d", DUT.la_trigger_capture[31:16]);
        $display("      Trigger Index: %0d", DUT.la_trigger_data[15:0]);

        // Simulate a CPU reading the first 5 samples from the buffer
        $display("[%0t] Reading first 5 raw samples from buffer...", $time);
        for (int i = 0; i < 5; i++) begin
            DUT.la_address_reg = {1'b1, 15'b0, i[15:0]}; // Bit 31 = Read Enable, [15:0] = Addr
            #20; // Wait for clock edge
            $display("      Addr %0d: Data = 0x%h", i, DUT.la_data_reg);
        end

        #1000;
        $display("[%0t] Simulation complete.", $time);
        $stop;
    end

    // Monitor for console output
    initial begin
        $monitor("Time: %0t | State: %s | Buffer Ptr: %0d", 
                 $time, DUT.la0.current_state.name(), DUT.la0.buffer_ptr);
    end

endmodule
