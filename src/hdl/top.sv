module top (
    // Inputs 
    input  logic        CLOCK_50,
    input  logic [3:0] KEY,

    // Outputs 
    output logic [6:0] HEX1,
    output logic [6:0] HEX2,
    output logic [6:0] HEX3,
    output logic [6:0] HEX4,
    output logic [6:0] HEX5,
    output logic [9:0] LEDR,

    // Input channels
    input  logic IN_CH_1,
    input  logic IN_CH_2,
    input  logic IN_CH_3,
    input  logic IN_CH_4,

    // Output channels
    output logic OUT_CH_1,
    output logic OUT_CH_2,
    output logic OUT_CH_3,
    output logic OUT_CH_4,
    output logic OUT_CH_5 
);

    // Signal generator
    signal_generator #(
      .FREQ_HEARTBEAT (1_000_000),
      .FREQ_CLOCK     (1_000_000),
      .BURST_PATTERN  (8'b0110_1010),
      .FREQ_BURST     (100_000),
      .FREQ_PULSE     (1_000_000)
    ) s0 (
        .clock_50m (CLOCK_50),
        .nreset    (KEY[0]), 

        .heartbeat (OUT_CH_1), 
        .burst     (OUT_CH_2), 
        .clock     (OUT_CH_3), 
        .logic1    (OUT_CH_4), 
        .logic0    (OUT_CH_5)  
    );

    // --- 2. Logic Analyzer (Channel Capture) Instance ---
    
    // Internal wires for CPU interaction (Placeholders for Nios V connections)
    logic [31:0] la_address_reg;
    logic [31:0] la_data_reg;
    logic [31:0] la_status_in;
    logic [31:0] la_status_out;
    logic [31:0] la_trigger_ctrl;
    logic [31:0] la_trigger_data;
    logic [31:0] la_trigger_capture;

    // Pack physical input pins into a vector for the capture module
    logic [15:0] capture_input_vector;
    assign capture_input_vector = {12'b0, IN_CH_4, IN_CH_3, IN_CH_2, IN_CH_1};

    channel_capture #(
        .BUFFER_SIZE(4096)
    ) la0 (
        .clock_50M            (CLOCK_50),
        .resetn               (KEY[0]),
        
        // Memory Mapped Registers
        .address_reg          (la_address_reg),
        .data_reg             (la_data_reg),
        .status_in_reg        (la_status_in),
        .status_out_reg       (la_status_out),
        
        // Triggering Registers
        .trigger_control_reg  (la_trigger_ctrl),
        .trigger_data_reg     (la_trigger_data),
        .trigger_capture_reg  (la_trigger_capture),

        // Signal Input
        .channel_in           (capture_input_vector)
    );

    // Assignments

    assign {HEX5, HEX4, HEX3, HEX2, HEX1, HEX0} = '1;
    
    // Map LEDs to monitor status
    assign LEDR[0]   = KEY[0];           // Reset status
    assign LEDR[1]   = OUT_CH_1;         // Signal Gen Heartbeat
    assign LEDR[2]   = la_status_out[1]; // Buffer Full (Captured)
    assign LEDR[3]   = la_status_out[2]; // Triggered
    assign LEDR[9:4] = '0;

endmodule
