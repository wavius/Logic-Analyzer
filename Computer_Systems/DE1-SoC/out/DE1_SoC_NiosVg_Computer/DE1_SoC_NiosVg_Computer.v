

module DE1_SoC_NiosVg_Computer (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pins
	CLOCK_50,
	CLOCK2_50,
	CLOCK3_50,
	CLOCK4_50,

	// SDRAM
	DRAM_ADDR,
	DRAM_BA,
	DRAM_CAS_N,
	DRAM_CKE,
	DRAM_CLK,
	DRAM_CS_N,
	DRAM_DQ,
	DRAM_LDQM,
	DRAM_RAS_N,
	DRAM_UDQM,
	DRAM_WE_N,

	// I2C Bus for Configuration of the Audio and Video-In Chips
	FPGA_I2C_SCLK,
	FPGA_I2C_SDAT,

	// 40-Pin Headers
	GPIO_0,
	GPIO_1,
	
	// IR
	IRDA_RXD,
	IRDA_TXD,

	// PS2 Ports
	PS2_CLK,
	PS2_DAT,
	
	PS2_CLK2,
	PS2_DAT2,

	// VGA
	VGA_B,
	VGA_BLANK_N,
	VGA_CLK,
	VGA_G,
	VGA_HS,
	VGA_R,
	VGA_SYNC_N,
	VGA_VS

);

//=======================================================
//  PARAMETER declarations
//=======================================================


//=======================================================
//  PORT declarations
//=======================================================

////////////////////////////////////
// FPGA Pins
////////////////////////////////////

// Clock pins
input					CLOCK_50;
input					CLOCK2_50;
input					CLOCK3_50;
input					CLOCK4_50;

// SDRAM
output 		[12: 0]		DRAM_ADDR;
output		[ 1: 0]		DRAM_BA;
output					DRAM_CAS_N;
output					DRAM_CKE;
output					DRAM_CLK;
output					DRAM_CS_N;
inout			[15: 0]	DRAM_DQ;
output					DRAM_LDQM;
output					DRAM_RAS_N;
output					DRAM_UDQM;
output					DRAM_WE_N;

// I2C Bus for Configuration of the Audio and Video-In Chips
output					FPGA_I2C_SCLK;
inout					FPGA_I2C_SDAT;

// 40-pin headers
inout			[35: 0]	GPIO_0;
inout			[35: 0]	GPIO_1;

// IR
input					IRDA_RXD;
output					IRDA_TXD;

// PS2 Ports
inout					PS2_CLK;
inout					PS2_DAT;

inout					PS2_CLK2;
inout					PS2_DAT2;

// VGA
output		[ 7: 0]		VGA_B;
output					VGA_BLANK_N;
output					VGA_CLK;
output		[ 7: 0]		VGA_G;
output					VGA_HS;
output		[ 7: 0]		VGA_R;
output					VGA_SYNC_N;
output					VGA_VS;


//=======================================================
//  REG/WIRE declarations
//=======================================================

wire					system_clock;
wire					system_clock_locked;
wire					vga_clock;
wire					vga_clock_locked;

//=======================================================
//  Structural coding
//=======================================================

System_PLL_100 System_PLL (
	.refclk		(CLOCK_50),
	.rst		(1'b0),
	.outclk_0	(system_clock),
	.outclk_1	(DRAM_CLK),
	.locked		(system_clock_locked)
);

VGA_PLL VGA_PLL (
	.refclk		(CLOCK2_50),
	.rst		(1'b0),
	.outclk_0	(vga_clock),
	.locked		(vga_clock_locked)
);

Computer_System The_System (
	////////////////////////////////////
	// FPGA Side
	////////////////////////////////////

	// Global signals
	.sys_clk_ref_clk						(system_clock),
	.sys_clk_reset_n						(system_clock_locked),
	.vga_clk_ref_clk						(vga_clock),
	.vga_clk_reset_n						(1'b1),


	// AV Config
	.av_config_SCLK							(FPGA_I2C_SCLK),
	.av_config_SDAT							(FPGA_I2C_SDAT),

	// Expansion JP1
	//.expansion_jp1_export					({GPIO_0[35:19], GPIO_0[17], GPIO_0[15:3], GPIO_0[1]}),

	// Expansion JP2
	//.expansion_jp2_export					({GPIO_1[35:19], GPIO_1[17], GPIO_1[15:3], GPIO_1[1]}),
	
	// Logic Analyzer Input Channels
	.logic_analyzer_0_conduit_end_data_in   ({GPIO_0[17:12], GPIO_0[9:0]}), // JP1
	.logic_analyzer_0_conduit_end_data_out  ({GPIO_1[17:12], GPIO_1[9:0]}), // JP2

	
	// PS2 Ports
	.ps2_port_CLK							(PS2_CLK),
	.ps2_port_DAT							(PS2_DAT),
	.ps2_port_dual_CLK						(PS2_CLK2),
	.ps2_port_dual_DAT						(PS2_DAT2),

	// IrDA
	.irda_RXD								(IRDA_RXD),
	.irda_TXD								(IRDA_TXD),

	// VGA Subsystem
	.vga_CLK								(VGA_CLK),
	.vga_BLANK								(VGA_BLANK_N),
	.vga_SYNC								(VGA_SYNC_N),
	.vga_HS									(VGA_HS),
	.vga_VS									(VGA_VS),
	.vga_R									(VGA_R),
	.vga_G									(VGA_G),
	.vga_B									(VGA_B),
	
	// SDRAM
	.sdram_addr								(DRAM_ADDR),
	.sdram_ba								(DRAM_BA),
	.sdram_cas_n							(DRAM_CAS_N),
	.sdram_cke								(DRAM_CKE),
	.sdram_cs_n								(DRAM_CS_N),
	.sdram_dq								(DRAM_DQ),
	.sdram_dqm								({DRAM_UDQM,DRAM_LDQM}),
	.sdram_ras_n							(DRAM_RAS_N),
	.sdram_we_n								(DRAM_WE_N)
);

endmodule
