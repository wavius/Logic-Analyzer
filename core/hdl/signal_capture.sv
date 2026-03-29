module signal_capture #(
  parameter BUFFER_SIZE = 1024
)(
  input  logic        clk,
  input  logic        nreset,

  // Avalon-MM slave 
  input  logic [2:0]  address,      // 3-bit address for up to 8 registers
  input  logic        write,
  input  logic [31:0] writedata,
  input  logic        read,
  input  logic        chipselect,
  output logic [31:0] readdata,

  // Channel input 
  input  logic [15:0] channel_in
);

  // Internal registers
  logic [31:0] control_reg;     // Address 0
  logic [31:0] trigger_config;  // Address 2

  // Internal pointers
  logic [15:0] post_trigger_length;            
  logic [15:0] post_trigger_count;             
  logic [15:0] pre_trigger_count;              
  logic [$clog2(BUFFER_SIZE)-1:0] buffer_ptr;  
  logic [$clog2(BUFFER_SIZE)-1:0] trigger_ptr; 
  logic [$clog2(BUFFER_SIZE)-1:0] read_pointer; // Auto-incrementing read pointer

  // Memory buffer
  logic [15:0] buffer [0:BUFFER_SIZE-1];

  // Status flags 
  logic run;
  logic buffer_full;
  logic triggered;
  
  assign run = control_reg[0];

  // Rising edge trigger 
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
      // Writes
      if (write) begin
        case (address)
          3'h0: control_reg    <= writedata;
          3'h2: trigger_config <= writedata;
          3'h3: read_pointer   <= '0; // Write to Address 3 resets the read pointer
        endcase
      end
      
      // Auto-incerement pointer on read
      if (read && address == 3'h3) begin
        if (read_pointer < BUFFER_SIZE - 1) begin
          read_pointer <= read_pointer + 1'b1;
        end
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
        3'h4: readdata <= {16'b0, trigger_ptr};
        3'h5: readdata <= {post_trigger_count, pre_trigger_count};
        default: readdata <= 32'hDEADBEEF;
      endcase
    end else begin
      readdata <= 32'h0; // Drive 0 when not selected to keep the bus quiet
    end
  end

  // FSM enum
  typedef enum logic [1:0] {
    IDLE         = 2'b00, 
    PRE_TRIGGER  = 2'b01, 
    POST_TRIGGER = 2'b10,
    DONE         = 2'b11 
  } state_t;
  
  state_t current_state, next_state;

  // FSM combinational logic
  always_comb begin
    next_state = current_state;
    case(current_state) 
      IDLE: begin
        if (run) next_state = PRE_TRIGGER; 
      end

      PRE_TRIGGER: begin
        if (rising_edge_detected) next_state = POST_TRIGGER; 
      end

      POST_TRIGGER: begin
        if (post_trigger_count >= post_trigger_length) next_state = DONE;
      end

      DONE: begin
        if (!run) next_state = IDLE; // Wait for CPU to clear the run bit
      end
    endcase
  end

  // FSM sequential logic
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
          buffer_ptr          <= '0; // Reset write pointer on new run
        end

        PRE_TRIGGER: begin
          buffer[buffer_ptr] <= channel_in;
          buffer_ptr         <= buffer_ptr + 1'b1;
          trigger_ptr        <= buffer_ptr;  
          
          // Limit pre trigger samples
          if (pre_trigger_count < (BUFFER_SIZE / 2)) begin
            pre_trigger_count <= pre_trigger_count + 1'b1;
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