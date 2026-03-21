`timescale 1ns/1ps

module tb_signal_capture();
    // System signals
    logic clk = 0;
    logic nreset = 0;
    
    // Avalon-MM signals
    logic [2:0]  address;
    logic        write;
    logic [31:0] writedata;
    logic        read;
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

    // Connect the 10MHz clock to Channel 1
    assign channel_in[15:0] = clk_10mhz;

    // --- Test Sequence ---
    initial begin
        // 1. Initialize and Reset
        nreset = 0;
        address = 0; 
        write = 0; 
        read = 0; 
        writedata = 0;
        
        #20 nreset = 1; // Release reset
        #20;

        $display("--- Starting Logic Analyzer Simulation ---");

        // 2. Configure Trigger for Channel 1
        // Write 1 to Address 2 (trigger_config)
        @(posedge clk);
        address = 3'h2;
        writedata = 32'h1; 
        write = 1;
        @(posedge clk);
        write = 0;
        $display("Trigger configured for Channel 1.");

        // 3. Start the Capture (Set RUN bit)
        // Write 1 to Address 0 (control_reg)
        @(posedge clk);
        address = 3'h0;
        writedata = 32'h1; 
        write = 1;
        @(posedge clk);
        write = 0;
        $display("Run bit set. Waiting for trigger and buffer full...");

        // 4. Poll the Status Register until buffer_full (Bit 1) is high
        // Address 1 is the Status Reg: {29'b0, triggered, buffer_full, run}
        readdata = 0;
        while ((readdata & 32'h2) == 0) begin
            @(posedge clk);
            address = 3'h1;
            read = 1;
            @(posedge clk); // Wait 1 cycle for readdata to update
            read = 0;
        end
        $display("Buffer is FULL! Capture complete.");

        // 5. Reset the Read Pointer
        // Your Verilog resets read_pointer when any write happens to Address 3
        @(posedge clk);
        address = 3'h3;
        writedata = 32'h0; // Data doesn't matter, just the write event
        write = 1;
        @(posedge clk);
        write = 0;
        $display("Read pointer reset.");

        // 6. Read from the Buffer Window (Address 3)
        // Let's read the first 20 samples to verify the 10MHz clock was captured
        $display("--- Dumping first 20 samples ---");
        for (int i = 0; i < 20; i++) begin
            @(posedge clk);
            address = 3'h3;
            read = 1;
            
            @(posedge clk); // Data is available here
            $display("Sample [%0d]: 16'b%016b", i, readdata[15:0]);
            
            read = 0;
        end

        $display("--- Simulation Complete ---");
        #100 $stop;
    end
endmodule