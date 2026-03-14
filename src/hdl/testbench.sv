`timescale 1ns/1ps

module testbench;

    // Use 'logic' for all testbench signals (SystemVerilog standard)
    logic        CLOCK_50;
    logic [3:0]  KEY;

    // Observe outputs from 'top'
    logic [9:0]  LEDR;
    logic [6:0]  HEX0, HEX1, HEX2, HEX3, HEX4, HEX5;
    
    // Logic Analyzer Channels
    logic        IN_CH_1,  IN_CH_2,  IN_CH_3,  IN_CH_4;
    logic        OUT_CH_1, OUT_CH_2, OUT_CH_3, OUT_CH_4, OUT_CH_5;

    // Instantiate DUT (matching your 'top' module)
    top DUT (
        .CLOCK_50 (CLOCK_50),
        .KEY      (KEY),

        .LEDR     (LEDR),
        .HEX0     (HEX0), .HEX1 (HEX1), .HEX2 (HEX2), 
        .HEX3     (HEX3), .HEX4 (HEX4), .HEX5 (HEX5),

        // Connect these to observe the generator output
        .OUT_CH_1 (OUT_CH_1), // HEARTBEAT
        .OUT_CH_2 (OUT_CH_2), // BURST
        .OUT_CH_3 (OUT_CH_3), // CLOCK
        .OUT_CH_4 (OUT_CH_4), // LOGIC1
        .OUT_CH_5 (OUT_CH_5), // LOGIC0

        // Inputs (unused for now, tied to 0)
        .IN_CH_1  (1'b0),
        .IN_CH_2  (1'b0),
        .IN_CH_3  (1'b0),
        .IN_CH_4  (1'b0)
    );

    // 50 MHz clock: Period is 20ns (10ns high, 10ns low)
    initial CLOCK_50 = 0;
    always #10 CLOCK_50 = ~CLOCK_50;

    // Simulation stimulus
    initial begin
        // --- Setup ---
        KEY = 4'b1111; // KEYs are active-low, 1 is idle
        
        // --- Reset Sequence ---
        $display("Starting simulation: Resetting system...");
        #5;
        KEY[0] = 0;    // Press Reset
        #100;
        KEY[0] = 1;    // Release Reset
        $display("Reset released. Observing signal generation...");

        // --- Observation Window ---
        // 100kHz = 10,000ns period, wait for a few cycles
        
        #50000; // Wait 50 microseconds
        
        $display("Simulation complete. Check waves for BURST and HEARTBEAT.");
        $stop; // Pause simulation so you can look at the waveform
    end

    // Optional: Monitor significant changes in the console
    initial begin
        $monitor("Time: %0t | BURST: %b | HEARTBEAT: %b", $time, OUT_CH_2, OUT_CH_1);
    end

endmodule
