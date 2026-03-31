// (C) 2001-2025 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// $Id: //acds/rel/25.1std/ip/merlin/altera_irq_mapper/altera_irq_mapper.sv.terp#1 $
// $Revision: #1 $
// $Date: 2025/03/10 $
// $Author: psgswbuild $

// -------------------------------------------------------
// Altera IRQ Mapper
//
// Parameters
//   NUM_RCVRS        : 10
//   SENDER_IRW_WIDTH : 16
//   IRQ_MAP          : 0:5,1:6,2:7,3:9,4:2,5:11,6:12,7:8,8:0,9:1
//
// -------------------------------------------------------

`timescale 1 ns / 1 ns

module Computer_System_irq_mapper
(
    // -------------------
    // Clock & Reset
    // -------------------
    input clk,
    input reset,

    // -------------------
    // IRQ Receivers
    // -------------------
    input                receiver0_irq,
    input                receiver1_irq,
    input                receiver2_irq,
    input                receiver3_irq,
    input                receiver4_irq,
    input                receiver5_irq,
    input                receiver6_irq,
    input                receiver7_irq,
    input                receiver8_irq,
    input                receiver9_irq,

    // -------------------
    // Command Source (Output)
    // -------------------
    output reg [15 : 0] sender_irq
);


    always @* begin
	sender_irq = 0;

        sender_irq[5] = receiver0_irq;
        sender_irq[6] = receiver1_irq;
        sender_irq[7] = receiver2_irq;
        sender_irq[9] = receiver3_irq;
        sender_irq[2] = receiver4_irq;
        sender_irq[11] = receiver5_irq;
        sender_irq[12] = receiver6_irq;
        sender_irq[8] = receiver7_irq;
        sender_irq[0] = receiver8_irq;
        sender_irq[1] = receiver9_irq;
    end

endmodule

