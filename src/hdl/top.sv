module top (
    // Inputs 
    input  logic       CLOCK_50,
    input  logic [3:0] KEY,

    // Outputs
    output logic [6:0] HEX0,
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

    // Instantiate the signal generator
    signal_generator #(
      .FREQ_HEARTBEAT (1_000_000),    // HEARTBEAT frequency
  
      .FREQ_CLOCK     (1_000_000),    // CLOCK frequency
  
      .BURST_PATTERN  (8'b0110_1010), // BURST pattern
      .FREQ_BURST     (100_000),      // BURST frequency
      .FREQ_PULSE;    (1_000_000)     // Frequency of individual pulses within BURST
    ) S0 (
        .CLOCK_50M (CLOCK_50),
        .NRESET    (KEY[0]),  
        
        .HEARTBEAT (OUT_CH_1),  // 1MHz Pulse
        .BURST     (OUT_CH_2),  // 100kHz Bursts
        .CLOCK     (OUT_CH_3),  // 1MHz Square wave
        .LOGIC1    (OUT_CH_4),  // Constant High
        .LOGIC0    (OUT_CH_5)   // Constant Low
    );

    assign {HEX5, HEX4, HEX3, HEX2, HEX1, HEX0} = '1;
    
    // Optional: Map LEDs to see activity
    assign LEDR[0] = KEY[0];     // Reset status
    assign LEDR[1] = OUT_CH_1;   // Heartbeat visual
    assign LEDR[2] = OUT_CH_2;   // Burst visual
    assign LEDR[9:3] = '0;

endmodule
