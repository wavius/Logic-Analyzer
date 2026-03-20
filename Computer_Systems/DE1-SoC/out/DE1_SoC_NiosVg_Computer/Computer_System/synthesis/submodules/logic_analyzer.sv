module logic_analyzer (
    input  logic        CLOCK_50,
    input  logic        nreset,

    // Avalon-MM Slave Interface
    input  logic [2:0]  address,    // 8 internal registers
    input  logic        write,
    input  logic [31:0] writedata,
    input  logic        read,
    output logic [31:0] readdata,

    // FPGA Pins
    input  logic [15:0] IN_CH,
    output logic [15:0] OUT_CH 
);

    // Signal Generator
    signal_generator #(
        .FREQ_HEARTBEAT (1_000_000),
        .FREQ_CLOCK     (1_000_000),
        .BURST_PATTERN  (8'b0110_1010),
        .FREQ_BURST     (100_000),
        .FREQ_PULSE     (1_000_000),
        .FREQ_SYS_CLOCK (50_000_000)
    ) SG0 (
        .clock_50m (CLOCK_50),
        .nreset    (nreset), 
        .heartbeat (OUT_CH[0]), 
        .burst     (OUT_CH[1]), 
        .clock     (OUT_CH[2]), 
        .logic1    (OUT_CH[3]), 
        .logic0    (OUT_CH[4])   
    );

    // Unused output channels
    assign OUT_CH[15:5] = 11'b0;

    // Channel Capture 
    signal_capture #(
        .BUFFER_SIZE(4096)
    ) SC0 (
        .clock_50M (CLOCK_50),
        .nreset    (nreset),
        
        .address   (address),
        .write     (write),
        .writedata (writedata),
        .read      (read),
        .readdata  (readdata),

        .channel_in({12'b0, IN_CH[3], IN_CH[2], IN_CH[1], IN_CH[0]})
    );

endmodule