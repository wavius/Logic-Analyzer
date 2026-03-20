module top (
    input  logic        CLOCK_50,
    input  logic [3:0]  KEY,

    input  logic [31:0] pio_la_address_reg,
    input  logic [31:0] pio_la_status_in,
    input  logic [31:0] pio_la_trigger_ctrl,
    output logic [31:0] pio_la_data_reg,
    output logic [31:0] pio_la_status_out,
    output logic [31:0] pio_la_trigger_capture,

    input  logic [31:0] pio_la_address_reg,
    input  logic [31:0] pio_la_status_in,
    input  logic [31:0] pio_la_trigger_ctrl,
    output logic [31:0] pio_la_data_reg,
    output logic [31:0] pio_la_status_out,
    output logic [31:0] pio_la_trigger_capture,

    // FPGA Pins
    output logic [6:0]  HEX0, HEX1, HEX2, HEX3, HEX4, HEX5,
    output logic [9:0]  LEDR,
    input  logic        IN_CH_1, IN_CH_2, IN_CH_3, IN_CH_4,
    output logic        OUT_CH_1, OUT_CH_2, OUT_CH_3, OUT_CH_4, OUT_CH_5 
);

    // Signal generator
    signal_generator #(
      .FREQ_HEARTBEAT (1_000_000),
      .FREQ_CLOCK     (1_000_000),
      .BURST_PATTERN  (8'b0110_1010),
      .FREQ_BURST     (100_000),
      .FREQ_PULSE     (1_000_000),
      .FREQ_SYS_CLOCK (50_000_000)
    ) s0 (
        .clock_50m (CLOCK_50),
        .nreset    (KEY[0]), 
        .heartbeat (OUT_CH_1), 
        .burst     (OUT_CH_2), 
        .clock     (OUT_CH_3), 
        .logic1    (OUT_CH_4), 
        .logic0    (OUT_CH_5)   
    );

    // Channel capture
    channel_capture #(
        .BUFFER_SIZE(4096)
    ) la0 (
        .clock_50M            (CLOCK_50),
        .nreset               (KEY[0]),
        
        // Connected directly to the new top-level ports
        .address_reg          (pio_la_address_reg),
        .data_reg             (pio_la_data_reg),
        .status_in_reg        (pio_la_status_in),
        .status_out_reg       (pio_la_status_out),
        .trigger_control_reg  (pio_la_trigger_ctrl),
        .trigger_capture_reg  (pio_la_trigger_capture),
        .trigger_data_reg     (), // Leave open or add port if needed

        .channel_in           ({12'b0, IN_CH_4, IN_CH_3, IN_CH_2, IN_CH_1})
    );

    // Assignments
    assign {HEX5, HEX4, HEX3, HEX2, HEX1, HEX0} = '1;
    assign LEDR[0]   = KEY[0];
    assign LEDR[1]   = OUT_CH_1;
    assign LEDR[2]   = pio_la_status_out[1]; // Buffer Full
    assign LEDR[3]   = pio_la_status_out[2]; // Triggered
    assign LEDR[9:4] = '0;

endmodule