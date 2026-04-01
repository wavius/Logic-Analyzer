module signal_capture #(
  parameter BUFFER_SIZE = 4096 // <-- MATCHES C CODE
)(
  input  logic        clk,
  input  logic        nreset,

  // Avalon-MM slave 
  input  logic [2:0]  address,      
  input  logic        write,
  input  logic [31:0] writedata,
  input  logic        read,
  input  logic        chipselect,
  output logic [31:0] readdata,

  // Channel input 
  input  logic [15:0] channel_in
);

  logic [31:0] control_reg;     
  logic [31:0] trigger_config;  

  logic [15:0] post_trigger_length;            
  logic [15:0] post_trigger_count;             
  logic [15:0] pre_trigger_count;              
  logic [$clog2(BUFFER_SIZE)-1:0] buffer_ptr;  
  logic [$clog2(BUFFER_SIZE)-1:0] trigger_ptr; 
  logic [$clog2(BUFFER_SIZE)-1:0] read_pointer; 

  logic [15:0] buffer [0:BUFFER_SIZE-1];

  logic run;
  logic buffer_full;
  logic triggered;
  
  assign run = control_reg[0];

  logic [15:0] trigger_channel;
  logic        trigger_current_data;
  logic        trigger_past_data;
  logic        rising_edge_detected;

  assign trigger_channel = trigger_config[15:0];
  assign trigger_current_data = channel_in[trigger_channel];

  always_ff @(posedge clk) begin
    if (!nreset) begin
      trigger_past_data <= 0;
    end else begin
      trigger_past_data <= trigger_current_data;
    end
  end

  assign rising_edge_detected = (trigger_current_data && !trigger_past_data);

  // Avalon write logic
  always_ff @(posedge clk or negedge nreset) begin
    if (!nreset) begin
      control_reg    <= 32'h0;
      trigger_config <= 32'h0;
      read_pointer   <= '0;
    end else begin
      if (write) begin
        case (address)
          3'h0: control_reg    <= writedata;
          3'h2: trigger_config <= writedata;
          3'h3: read_pointer   <= '0; 
        endcase
      end
      
      // FIX: Allow pointer to wrap naturally, removing the hard stop
      if (read && address == 3'h3) begin
        read_pointer <= read_pointer + 1'b1;
      end
    end
  end

  // Avalon read logic
  always_ff @(posedge clk or negedge nreset) begin
    if (!nreset) begin
      readdata <= 32'h0;
    end else if (chipselect && read) begin
      case (address)
        3'h0: readdata <= control_reg;
        3'h1: readdata <= {29'b0, triggered, buffer_full, run};
        3'h2: readdata <= trigger_config;
        3'h3: readdata <= {16'b0, buffer[read_pointer]};
        3'h4: readdata <= 32'(trigger_ptr); // FIX: Safe zero-extend for 12-bit ptr
        3'h5: readdata <= {post_trigger_count, pre_trigger_count};
        default: readdata <= 32'hDEADBEEF;
      endcase
    end else begin
      readdata <= 32'h0; 
    end
  end

  typedef enum logic [1:0] {
    IDLE         = 2'b00, 
    PRE_TRIGGER  = 2'b01, 
    POST_TRIGGER = 2'b10,
    DONE         = 2'b11 
  } state_t;
  
  state_t current_state, next_state;

  always_comb begin
    next_state = current_state;
    case(current_state) 
      IDLE:         if (run) next_state = PRE_TRIGGER; 
      PRE_TRIGGER:  if (rising_edge_detected) next_state = POST_TRIGGER; 
      POST_TRIGGER: if (post_trigger_count >= post_trigger_length) next_state = DONE;
      DONE:         if (!run) next_state = IDLE; 
    endcase
  end

  always_ff @(posedge clk or negedge nreset) begin
    if (!nreset) begin
      current_state <= IDLE;
      buffer_ptr    <= '0;
      buffer_full   <= 1'b0;
      triggered     <= 1'b0;
      post_trigger_count  <= '0;
      pre_trigger_count   <= '0;
      post_trigger_length <= '1;
      trigger_ptr         <= '0;
    end else begin
      current_state <= next_state;
      
      case(current_state)
        IDLE: begin
          buffer_full         <= 1'b0; 
          triggered           <= 1'b0; 
          post_trigger_count  <= '0;
          pre_trigger_count   <= '0;
          post_trigger_length <= '1; 
          // FIX: Removed buffer_ptr <= '0 so the circular buffer stays intact
        end

        PRE_TRIGGER: begin
          buffer[buffer_ptr] <= channel_in;
          buffer_ptr         <= buffer_ptr + 1'b1;
          
          if (pre_trigger_count < (BUFFER_SIZE / 2)) begin
            pre_trigger_count <= pre_trigger_count + 1'b1;
          end

          // FIX: Snap the pointer ONLY when the edge happens
          if (rising_edge_detected) begin
            trigger_ptr <= buffer_ptr;
          end
        end

        POST_TRIGGER: begin
          triggered           <= 1'b1;            
          post_trigger_length <= BUFFER_SIZE - pre_trigger_count; 
          
          buffer[buffer_ptr]  <= channel_in;
          buffer_ptr          <= buffer_ptr + 1'b1;
          post_trigger_count  <= post_trigger_count + 1'b1;
        end

        DONE: begin
          buffer_full <= 1'b1; 
        end
      endcase
    end
  end

endmodule