
module Computer_System (
	av_config_SDAT,
	av_config_SCLK,
	expansion_jp1_export,
	expansion_jp2_export,
	irda_TXD,
	irda_RXD,
	logic_analyzer_0_conduit_end_data_in,
	logic_analyzer_0_conduit_end_data_out,
	ps2_port_CLK,
	ps2_port_DAT,
	ps2_port_dual_CLK,
	ps2_port_dual_DAT,
	sdram_ba,
	sdram_addr,
	sdram_cas_n,
	sdram_cke,
	sdram_cs_n,
	sdram_dq,
	sdram_dqm,
	sdram_ras_n,
	sdram_we_n,
	sys_clk_reset_n,
	sys_clk_ref_clk,
	vga_CLK,
	vga_HS,
	vga_VS,
	vga_BLANK,
	vga_SYNC,
	vga_R,
	vga_G,
	vga_B,
	vga_clk_reset_n,
	vga_clk_ref_clk);	

	inout		av_config_SDAT;
	output		av_config_SCLK;
	inout	[31:0]	expansion_jp1_export;
	inout	[31:0]	expansion_jp2_export;
	output		irda_TXD;
	input		irda_RXD;
	input	[15:0]	logic_analyzer_0_conduit_end_data_in;
	output	[15:0]	logic_analyzer_0_conduit_end_data_out;
	inout		ps2_port_CLK;
	inout		ps2_port_DAT;
	inout		ps2_port_dual_CLK;
	inout		ps2_port_dual_DAT;
	output	[1:0]	sdram_ba;
	output	[12:0]	sdram_addr;
	output		sdram_cas_n;
	output		sdram_cke;
	output		sdram_cs_n;
	inout	[15:0]	sdram_dq;
	output	[1:0]	sdram_dqm;
	output		sdram_ras_n;
	output		sdram_we_n;
	input		sys_clk_reset_n;
	input		sys_clk_ref_clk;
	output		vga_CLK;
	output		vga_HS;
	output		vga_VS;
	output		vga_BLANK;
	output		vga_SYNC;
	output	[7:0]	vga_R;
	output	[7:0]	vga_G;
	output	[7:0]	vga_B;
	input		vga_clk_reset_n;
	input		vga_clk_ref_clk;
endmodule
