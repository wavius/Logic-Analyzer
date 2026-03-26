	component Computer_System is
		port (
			av_config_SDAT                        : inout std_logic                     := 'X';             -- SDAT
			av_config_SCLK                        : out   std_logic;                                        -- SCLK
			expansion_jp1_export                  : inout std_logic_vector(31 downto 0) := (others => 'X'); -- export
			expansion_jp2_export                  : inout std_logic_vector(31 downto 0) := (others => 'X'); -- export
			irda_TXD                              : out   std_logic;                                        -- TXD
			irda_RXD                              : in    std_logic                     := 'X';             -- RXD
			logic_analyzer_0_conduit_end_data_in  : in    std_logic_vector(15 downto 0) := (others => 'X'); -- data_in
			logic_analyzer_0_conduit_end_data_out : out   std_logic_vector(15 downto 0);                    -- data_out
			ps2_port_CLK                          : inout std_logic                     := 'X';             -- CLK
			ps2_port_DAT                          : inout std_logic                     := 'X';             -- DAT
			ps2_port_dual_CLK                     : inout std_logic                     := 'X';             -- CLK
			ps2_port_dual_DAT                     : inout std_logic                     := 'X';             -- DAT
			sdram_ba                              : out   std_logic_vector(1 downto 0);                     -- ba
			sdram_addr                            : out   std_logic_vector(12 downto 0);                    -- addr
			sdram_cas_n                           : out   std_logic;                                        -- cas_n
			sdram_cke                             : out   std_logic;                                        -- cke
			sdram_cs_n                            : out   std_logic;                                        -- cs_n
			sdram_dq                              : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
			sdram_dqm                             : out   std_logic_vector(1 downto 0);                     -- dqm
			sdram_ras_n                           : out   std_logic;                                        -- ras_n
			sdram_we_n                            : out   std_logic;                                        -- we_n
			sys_clk_reset_n                       : in    std_logic                     := 'X';             -- reset_n
			sys_clk_ref_clk                       : in    std_logic                     := 'X';             -- clk
			vga_CLK                               : out   std_logic;                                        -- CLK
			vga_HS                                : out   std_logic;                                        -- HS
			vga_VS                                : out   std_logic;                                        -- VS
			vga_BLANK                             : out   std_logic;                                        -- BLANK
			vga_SYNC                              : out   std_logic;                                        -- SYNC
			vga_R                                 : out   std_logic_vector(7 downto 0);                     -- R
			vga_G                                 : out   std_logic_vector(7 downto 0);                     -- G
			vga_B                                 : out   std_logic_vector(7 downto 0);                     -- B
			vga_clk_reset_n                       : in    std_logic                     := 'X';             -- reset_n
			vga_clk_ref_clk                       : in    std_logic                     := 'X'              -- clk
		);
	end component Computer_System;

	u0 : component Computer_System
		port map (
			av_config_SDAT                        => CONNECTED_TO_av_config_SDAT,                        --                    av_config.SDAT
			av_config_SCLK                        => CONNECTED_TO_av_config_SCLK,                        --                             .SCLK
			expansion_jp1_export                  => CONNECTED_TO_expansion_jp1_export,                  --                expansion_jp1.export
			expansion_jp2_export                  => CONNECTED_TO_expansion_jp2_export,                  --                expansion_jp2.export
			irda_TXD                              => CONNECTED_TO_irda_TXD,                              --                         irda.TXD
			irda_RXD                              => CONNECTED_TO_irda_RXD,                              --                             .RXD
			logic_analyzer_0_conduit_end_data_in  => CONNECTED_TO_logic_analyzer_0_conduit_end_data_in,  -- logic_analyzer_0_conduit_end.data_in
			logic_analyzer_0_conduit_end_data_out => CONNECTED_TO_logic_analyzer_0_conduit_end_data_out, --                             .data_out
			ps2_port_CLK                          => CONNECTED_TO_ps2_port_CLK,                          --                     ps2_port.CLK
			ps2_port_DAT                          => CONNECTED_TO_ps2_port_DAT,                          --                             .DAT
			ps2_port_dual_CLK                     => CONNECTED_TO_ps2_port_dual_CLK,                     --                ps2_port_dual.CLK
			ps2_port_dual_DAT                     => CONNECTED_TO_ps2_port_dual_DAT,                     --                             .DAT
			sdram_ba                              => CONNECTED_TO_sdram_ba,                              --                        sdram.ba
			sdram_addr                            => CONNECTED_TO_sdram_addr,                            --                             .addr
			sdram_cas_n                           => CONNECTED_TO_sdram_cas_n,                           --                             .cas_n
			sdram_cke                             => CONNECTED_TO_sdram_cke,                             --                             .cke
			sdram_cs_n                            => CONNECTED_TO_sdram_cs_n,                            --                             .cs_n
			sdram_dq                              => CONNECTED_TO_sdram_dq,                              --                             .dq
			sdram_dqm                             => CONNECTED_TO_sdram_dqm,                             --                             .dqm
			sdram_ras_n                           => CONNECTED_TO_sdram_ras_n,                           --                             .ras_n
			sdram_we_n                            => CONNECTED_TO_sdram_we_n,                            --                             .we_n
			sys_clk_reset_n                       => CONNECTED_TO_sys_clk_reset_n,                       --                      sys_clk.reset_n
			sys_clk_ref_clk                       => CONNECTED_TO_sys_clk_ref_clk,                       --                  sys_clk_ref.clk
			vga_CLK                               => CONNECTED_TO_vga_CLK,                               --                          vga.CLK
			vga_HS                                => CONNECTED_TO_vga_HS,                                --                             .HS
			vga_VS                                => CONNECTED_TO_vga_VS,                                --                             .VS
			vga_BLANK                             => CONNECTED_TO_vga_BLANK,                             --                             .BLANK
			vga_SYNC                              => CONNECTED_TO_vga_SYNC,                              --                             .SYNC
			vga_R                                 => CONNECTED_TO_vga_R,                                 --                             .R
			vga_G                                 => CONNECTED_TO_vga_G,                                 --                             .G
			vga_B                                 => CONNECTED_TO_vga_B,                                 --                             .B
			vga_clk_reset_n                       => CONNECTED_TO_vga_clk_reset_n,                       --                      vga_clk.reset_n
			vga_clk_ref_clk                       => CONNECTED_TO_vga_clk_ref_clk                        --                  vga_clk_ref.clk
		);

