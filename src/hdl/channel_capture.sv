module channel_capture (
  input logic CLOCK_50M;
  input logic RESETN;

  input logic [31:0] channel_inputs;

  input logic [31:0] cpu_read_address;
  output logic [31:0] cpu_read_data_out;

);
  

endmodule
