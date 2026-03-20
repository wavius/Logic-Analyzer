
module Computer_System (
	adc_sclk,
	adc_cs_n,
	adc_dout,
	adc_din,
	audio_ADCDAT,
	audio_ADCLRCK,
	audio_BCLK,
	audio_DACDAT,
	audio_DACLRCK,
	audio_pll_clk_clk,
	audio_pll_ref_clk_clk,
	audio_pll_ref_reset_reset,
	av_config_SDAT,
	av_config_SCLK,
	expansion_jp2_export,
	hex3_hex0_export,
	hex5_hex4_export,
	irda_TXD,
	irda_RXD,
	leds_export,
	ps2_port_CLK,
	ps2_port_DAT,
	ps2_port_dual_CLK,
	ps2_port_dual_DAT,
	pushbuttons_export,
	sdram_ba,
	sdram_addr,
	sdram_cas_n,
	sdram_cke,
	sdram_cs_n,
	sdram_dq,
	sdram_dqm,
	sdram_ras_n,
	sdram_we_n,
	slider_switches_export,
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
	vga_clk_ref_clk,
	video_in_TD_CLK27,
	video_in_TD_DATA,
	video_in_TD_HS,
	video_in_TD_VS,
	video_in_clk27_reset,
	video_in_TD_RESET,
	video_in_overflow_flag,
	logic_analyzer_0_conduit_end_out_data,
	logic_analyzer_0_conduit_end_in_data);	

	output		adc_sclk;
	output		adc_cs_n;
	input		adc_dout;
	output		adc_din;
	input		audio_ADCDAT;
	input		audio_ADCLRCK;
	input		audio_BCLK;
	output		audio_DACDAT;
	input		audio_DACLRCK;
	output		audio_pll_clk_clk;
	input		audio_pll_ref_clk_clk;
	input		audio_pll_ref_reset_reset;
	inout		av_config_SDAT;
	output		av_config_SCLK;
	inout	[31:0]	expansion_jp2_export;
	output	[31:0]	hex3_hex0_export;
	output	[15:0]	hex5_hex4_export;
	output		irda_TXD;
	input		irda_RXD;
	output	[9:0]	leds_export;
	inout		ps2_port_CLK;
	inout		ps2_port_DAT;
	inout		ps2_port_dual_CLK;
	inout		ps2_port_dual_DAT;
	input	[3:0]	pushbuttons_export;
	output	[1:0]	sdram_ba;
	output	[12:0]	sdram_addr;
	output		sdram_cas_n;
	output		sdram_cke;
	output		sdram_cs_n;
	inout	[15:0]	sdram_dq;
	output	[1:0]	sdram_dqm;
	output		sdram_ras_n;
	output		sdram_we_n;
	input	[9:0]	slider_switches_export;
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
	input		video_in_TD_CLK27;
	input	[7:0]	video_in_TD_DATA;
	input		video_in_TD_HS;
	input		video_in_TD_VS;
	input		video_in_clk27_reset;
	output		video_in_TD_RESET;
	output		video_in_overflow_flag;
	output	[15:0]	logic_analyzer_0_conduit_end_out_data;
	input	[15:0]	logic_analyzer_0_conduit_end_in_data;
endmodule
