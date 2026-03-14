module sig_gen (
  // Inputs
  input logic CLOCK_50M,
  input logic NRESET,
  
  // Outputs
  output logic HEARTBEAT, // 1MHz 
  output logic BURST,     // 10KHz bursts with 1MHz pulses
  output logic CLOCK,     // 1MHz
  output logic LOGIC1,    // Set to logic voltage
  output logic LOGIC0     // Set to GND
);
  // Local parameters
  localparam SYS_CLOCK       = 50_000_000; // 50MHz
  
  // Signal frequencies
  localparam FREQ_HEARTBEAT  = 1_000_000; // HEARTBEAT frequency
  
  localparam FREQ_CLOCK      = 1_000_000; // CLOCK frequency
  
  localparam BURST_PATTERN   = 8'b1001_1010; // BURST pattern
  localparam BURST_WIDTH     = 8;           // Number of bits in BURST_PATTERN
  localparam FREQ_BURST      = 100_000;     // BURST frequency
  localparam FREQ_PULSE      = 1_000_000;   // Frequency of individual pulses within BURST
  
  // Counter targets
  localparam COUNT_HEARTBEAT = SYS_CLOCK / FREQ_HEARTBEAT;
  localparam COUNT_CLOCK     = SYS_CLOCK / FREQ_CLOCK;
  localparam COUNT_BURST     = SYS_CLOCK / FREQ_BURST;
  localparam COUNT_PULSE     = SYS_CLOCK / FREQ_PULSE;

  // HEARBEAT generator
  logic [$clog2(COUNT_HEARTBEAT)-1:0] Q0; // ceiling log_2 (COUNT_HEARTBEAT)
  always_ff @(posedge CLOCK_50M, negedge NRESET) begin
    if (!NRESET) begin
      Q0        <= 0;
      HEARTBEAT <= 0;
    end else begin
      if (Q0 == COUNT_HEARTBEAT - 1) begin
        Q0        <= 0;
        HEARTBEAT <= 1; // Create a pulse that is high for one clock cycle
      end else begin
        Q0        <= Q0 + 1;
        HEARTBEAT <= 0;
      end
    end
  end

  // CLOCK generator
  logic [$clog2(COUNT_CLOCK)-1:0] Q1;
  always_ff @(posedge CLOCK_50M, negedge NRESET) begin
    if (!NRESET) begin
      Q1    <= 0;
      CLOCK <= 0;
    end else begin
      if (Q1 == ((COUNT_CLOCK / 2) - 1)) begin
        Q1    <= 0;
        CLOCK <= ~CLOCK; // Create a clock signal by toggling high and low
      end else begin
        Q1 <= Q1 + 1;
      end
    end
  end

  // BURST generator
  logic [$clog2(COUNT_BURST)-1:0] Q2;     // Burst frequency counter
  logic [$clog2(COUNT_PULSE)-1:0] P2;     // Pulse frequency counter
  logic [$clog2(BURST_WIDTH + 1)-1:0] L2; // Number of bits required to store BURST_WIDTH
  logic [BURST_WIDTH-1:0] PATTERN;        // Patern width counter
  always_ff @(posedge CLOCK_50M, negedge NRESET) begin
    if (!NRESET) begin
      Q2      <= 0;
      P2      <= 0;
      L2      <= 0;
      PATTERN <= BURST_PATTERN;
      BURST   <= 0;
    end
    else begin
      // Burst timer
      if (Q2 == (COUNT_BURST - 1)) begin
        Q2      <= 0;             // Reset burst counter
        L2      <= 0;             // Reset width counter
        PATTERN <= BURST_PATTERN; // Reset pattern
      end else begin
        Q2 <= Q2 + 1;
      end

      // Pulse timer and shift logic
      if (L2 < BURST_WIDTH) begin
        if (P2 == COUNT_PULSE - 1) begin
          BURST   <= PATTERN[BURST_WIDTH-1]; // Set burst to MSB in pattern
          PATTERN <= PATTERN << 1;           // Bit shift pattern to get next bit
          L2      <= L2 + 1;                 // Increment width counter 
          P2      <= 0;                      // Reset pulse frequency counter
        end else begin
          P2 <= P2 + 1;
        end  
      end else begin
        BURST <= 0;
      end
    end
  end

  // Logic 1
  assign LOGIC1 = 1;

  // Logic 0
  assign LOGIC0 = 0;

endmodule
