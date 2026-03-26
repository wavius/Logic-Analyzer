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


// $Id: //acds/rel/24.1std/ip/merlin/altera_irq_mapper/altera_irq_mapper.sv.terp#1 $
// $Revision: #1 $
// $Date: 2023/12/11 $
// $Author: psgswbuild $

// -------------------------------------------------------
// Altera IRQ Mapper
//
// Parameters
//   NUM_RCVRS        : 8
//   SENDER_IRW_WIDTH : 16
//   IRQ_MAP          : 0:6,1:7,2:9,3:11,4:12,5:8,6:0,7:1
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

    // -------------------
    // Command Source (Output)
    // -------------------
    output reg [15 : 0] sender_irq
);


    always @* begin
	sender_irq = 0;

        sender_irq[6] = receiver0_irq;
        sender_irq[7] = receiver1_irq;
        sender_irq[9] = receiver2_irq;
        sender_irq[11] = receiver3_irq;
        sender_irq[12] = receiver4_irq;
        sender_irq[8] = receiver5_irq;
        sender_irq[0] = receiver6_irq;
        sender_irq[1] = receiver7_irq;
    end

endmodule

