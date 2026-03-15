module channel_capture #(
  parameter BUFFER_SIZE
)(
  input logic clock_50M,
  input logic resetn,
  
  // Memory mapped registers
  input logic  [31:0] address_reg,    // [15:0] = address bits, [31] = read enable
  output logic [31:0] data_reg,       // Output data

  // Status registers
  input logic  [31:0] status_in_reg,  // [0] = RUN, [1] = BUFFER_FULL, [2] = TRIGGERED
  output logic [31:0] status_out_reg, // [0] = RUN, [1] = BUFFER_FULL, [2] = TRIGGERED

  // Trigger registers
  input logic  [31:0] trigger_control_reg, // [15:0] = trigger type, [31:16] = trigger channel
  output logic [31:0] trigger_data_reg,    // [15:0] = trigger index, 
  output logic [31:0] trigger_capture_reg, // [15:0] = pre trigger samples, [31:16] = post trigger samples

  // Channel input
  input logic [15:0] channel_in
);

  // --- Internal Signals ---
  logic [15:0] read_address    = address_reg[15:0];  
  logic        read_enable     = address_reg[31];    
  logic [15:0] trigger_type    = trigger_control_reg[15:0];  
  logic [15:0] trigger_channel = trigger_control_reg[31:16];
 
  logic run = status_in_reg[0]; // set on capture start
  logic buffer_full;            // set on full buffer after trigger
  logic triggered;              // set on trigger

  // --- Buffer and Pointer Declarations ---
  logic [15:0] post_trigger_length;            // max number of post trigger samples
  logic [15:0] post_trigger_count;             // number of post trigger samples
  logic [15:0] pre_trigger_count;              // number of pre trigger samples
  logic [$clog2(BUFFER_SIZE)-1:0] buffer_ptr;  // pointer to buffer index
  logic [$clog2(BUFFER_SIZE)-1:0] trigger_ptr; // pointer to buffer index on trigger
  logic [15:0] buffer [BUFFER_SIZE-1:0];

  // --- Trigger Edge Detection Logic ---
  logic trigger_channel_data = channel_in[trigger_channel];
  logic trigger_current_data;
  logic trigger_past_data;

  always_ff @(posedge clock_50M) begin
    trigger_current_data <= trigger_channel_data;
    trigger_past_data    <= trigger_current_data;
  end

  // --- FSM Definitions ---
  typedef enum logic [1:0] {
    IDLE         = 2'b00, 
    PRE_TRIGGER  = 2'b01, 
    POST_TRIGGER = 2'b10,
    DONE         = 2'b11 
  } state_t;
  state_t current_state, next_state;

  // --- Combinational Logic ---
  always_comb begin
    next_state = current_state;
    case(current_state) 
      IDLE: begin
        if (run == 1) next_state = PRE_TRIGGER; 
        else          next_state = IDLE;        
      end

      PRE_TRIGGER: begin
        if (trigger_channel_data && !trigger_past_data) next_state = POST_TRIGGER; 
        else                                            next_state = PRE_TRIGGER;
      end

      POST_TRIGGER: begin
        if (post_trigger_count >= post_trigger_length) next_state = DONE;
        else                                           next_state = POST_TRIGGER;
      end

      DONE: begin
        if (run == 0) next_state = IDLE; 
        else          next_state = DONE; 
      end
    endcase
  end

  // --- Sequential Logic ---
  always_ff @(posedge clock_50M, negedge nreset) begin
    if (!nreset) begin
      current_state <= IDLE;
      buffer_ptr    <= 0;
    end else begin
      current_state <= next_state;
      case(current_state)
        IDLE: begin
          buffer_full         <= 0; 
          triggered           <= 0; 
          post_trigger_count  <= 0;
          pre_trigger_count   <= 0;
          post_trigger_length <= '1; // set all bits to one to prevent race condition
        end

        PRE_TRIGGER: begin
          buffer_ptr          <= buffer_ptr + 1;
          trigger_ptr         <= buffer_ptr;   
          buffer[buffer_ptr]  <= channel_in;
          if (pre_trigger_count < BUFFER_SIZE / 2) begin
            pre_trigger_count   <= pre_trigger_count + 1; // cap pre trigger samples to half the buffer
          end
        end

        POST_TRIGGER: begin
          post_trigger_length <= BUFFER_SIZE - pre_trigger_count; // calculate remaining space in buffer after pre trigger samples
          triggered          <= 1;            
          buffer_ptr         <= buffer_ptr + 1;
          buffer[buffer_ptr] <= channel_in;
          post_trigger_count <= post_trigger_count + 1;
        end

        DONE: begin
          buffer_full <= 1; 
          trigger_capture_reg <= {post_trigger_count, pre_trigger_count};
          if (read_enable) begin
            data_reg <= {16'b0, buffer[read_address]}; 
          end
        end
      endcase
    end
  end

  // --- Output Assignments ---
  assign status_out_reg = {29'b0, triggered, buffer_full, run};
  assign trigger_data_reg = {16'b0, trigger_ptr};

endmodule
