module signal_generator #(
  // Signal frequencies 
  parameter FREQ_HEARTBEAT, // Heartbeat frequency
  parameter FREQ_CLOCK,     // Clock frequency
  parameter BURST_PATTERN,  // Burst pattern
  parameter FREQ_BURST,     // Burst frequency
  parameter FREQ_PULSE      // Frequency of individual pulses within burst
)(
  // Inputs
  input  logic clock_50m,
  input  logic nreset,
  
  // Outputs
  output logic heartbeat, // 1MHz 
  output logic burst,     // 10KHz bursts with 1MHz pulse
  output logic clock,     // 1MHz clock
  output logic logic1,    // Set to logic voltage
  output logic logic0     // Set to GND
); 

  // Local parameters
  localparam SYS_CLOCK       = 50_000_000; // 50MHz
  localparam BURST_WIDTH     = 8;          // Number of bits in BURST_PATTERN
  
  // Counter targets
  localparam COUNT_HEARTBEAT = SYS_CLOCK / FREQ_HEARTBEAT;
  localparam COUNT_CLOCK     = SYS_CLOCK / FREQ_CLOCK;
  localparam COUNT_BURST     = SYS_CLOCK / FREQ_BURST;
  localparam COUNT_PULSE     = SYS_CLOCK / FREQ_PULSE;

  // heartbeat generator
  logic [$clog2(COUNT_HEARTBEAT)-1:0] q0; 
  always_ff @(posedge clock_50m, negedge nreset) begin
    if (!nreset) begin
      q0        <= 0;
      heartbeat <= 0;
    end else begin
      if (q0 == COUNT_HEARTBEAT - 1) begin
        q0        <= 0;
        heartbeat <= 1; // Create a pulse that is high for one clock cycle
      end else begin
        q0        <= q0 + 1;
        heartbeat <= 0;
      end
    end
  end

  // clock generator
  logic [$clog2(COUNT_CLOCK)-1:0] q1;
  always_ff @(posedge clock_50m, negedge nreset) begin
    if (!nreset) begin
      q1    <= 0;
      clock <= 0;
    end else begin
      if (q1 == ((COUNT_CLOCK / 2) - 1)) begin
        q1    <= 0;
        clock <= ~clock; // Create a clock signal by toggling high and low
      end else begin
        q1 <= q1 + 1;
      end
    end
  end

  // burst generator
  logic [$clog2(COUNT_BURST)-1:0] q2;     // Burst frequency counter
  logic [$clog2(COUNT_PULSE)-1:0] p2;     // Pulse frequency counter
  logic [$clog2(BURST_WIDTH + 1)-1:0] l2; // Number of bits required to store BURST_WIDTH
  logic [BURST_WIDTH-1:0] pattern;        // Pattern width counter
  always_ff @(posedge clock_50m, negedge nreset) begin
    if (!nreset) begin
      q2      <= 0;
      p2      <= 0;
      l2      <= 0;
      pattern <= BURST_PATTERN;
      burst   <= 0;
    end
    else begin
      // Burst timer
      if (q2 == (COUNT_BURST - 1)) begin
        q2      <= 0;             // Reset burst counter
        l2      <= 0;             // Reset width counter
        pattern <= BURST_PATTERN; // Reset pattern
      end else begin
        q2 <= q2 + 1;
      end

      // Pulse timer and shift logic
      if (l2 < BURST_WIDTH) begin
        if (p2 == COUNT_PULSE - 1) begin
          burst   <= pattern[BURST_WIDTH-1]; // Set burst to MSB in pattern
          pattern <= pattern << 1;           // Bit shift pattern to get next bit
          l2      <= l2 + 1;                 // Increment width counter 
          p2      <= 0;                      // Reset pulse frequency counter
        end else begin
          p2 <= p2 + 1;
        end  
      end else begin
        burst <= 0;
      end
    end
  end

  // Logic 1
  assign logic1 = 1;

  // Logic 0
  assign logic0 = 0;

endmodule
