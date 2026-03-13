module top (
    // Inputs
    input wire	      CLOCK_50,
    input wire	[3:0] KEY,

    // Outputs
    output wire [6:0] HEX0,
    output wire [6:0] HEX1,
    output wire [6:0] HEX2,
    output wire [6:0] HEX3,
    output wire [6:0] HEX4,
    output wire [6:0] HEX5,
    output wire [9:0] LEDR,

    // Input channels
    input wire IN_CH_1,
    input wire IN_CH_2,
    input wire IN_CH_3,
    input wire IN_CH_4,

    // Output channels
    output wire OUT_CH_1,
    output wire OUT_CH_2,
    output wire OUT_CH_3,
    output wire OUT_CH_4
);


  signal_generator() S0 





endmodule
