`timescale 1ns/1ps

module tb_signal_capture();
    // System signals
    logic clk = 0;
    logic nreset = 0;
    
    // Avalon-MM signals
    logic [2:0]  address = 0;
    logic        write = 0;
    logic [31:0] writedata = 0;
    logic        read = 0;
    logic        chipselect = 1; // FIXED: Added chipselect and tied it high
    logic [31:0] readdata;
    
    // Conduit signal
    logic [15:0] channel_in;

    // Instantiate the Logic Analyzer
    signal_capture uut (.*);

    // --- Clock Generation ---
    // 100 MHz System Clock (10ns period)
    always #5 clk = ~clk; 

    // 10 MHz Test Signal (100ns period)
    logic clk_10mhz = 0;
    always #50 clk_10mhz = ~clk_10mhz;

    // FIXED: Replicate the 10MHz clock to ALL channels to prevent trigger mismatches
    assign channel_in = {16{clk_10mhz}};

    // --- Test Sequence ---
    initial begin
        // 1. Initialize and Reset
        nreset = 0;
        
        #20 nreset = 1; // Release reset
        #20;

        $display("--- Starting Logic Analyzer Simulation ---");

        // 2. Configure Trigger for Channel 1
        @(posedge clk);
        address = 3'h2;
        writedata = 32'h1; 
        write = 1;
        @(posedge clk);
        write = 0;
        $display("Trigger configured for Channel 1.");

        // 3. Start the Capture (Set RUN bit)
        @(posedge clk);
        address = 3'h0;
        writedata = 32'h1; 
        write = 1;
        @(posedge clk);
        write = 0;
        $display("Run bit set. Waiting for trigger and buffer full...");

        // 4. Poll the Status Register until buffer_full (Bit 1) is high
        address = 3'h1;
        read = 1;
        @(posedge clk); 
        while ((readdata & 32'h2) == 0) begin
            @(posedge clk); // Wait 1 cycle and check again
        end
        read = 0;
        $display("Buffer is FULL! Capture complete.");

        // 5. Reset the Read Pointer
        @(posedge clk);
        address = 3'h3;
        writedata = 32'h0; 
        write = 1;
        @(posedge clk);
        write = 0;
        $display("Read pointer reset.");

        // 6. Read from the Buffer Window (Address 3)
        $display("--- Dumping first 20 samples ---");
        
        // Assert address and read once, then stream the data out
        address = 3'h3;
        read = 1;
        for (int i = 0; i < 20; i++) begin
            #1; // Wait a delta cycle for combinational readdata to settle
            $display("Sample [%0d]: 16'b%016b", i, readdata[15:0]);
            @(posedge clk); // Auto-increment happens here
        end
        read = 0;

        $display("--- Simulation Complete ---");
        #100 $stop;
    end
endmodule