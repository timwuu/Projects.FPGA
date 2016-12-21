
`timescale 1 ps / 1 ps
module  max10_top (
        //Reset and Clocks
        input           max10_resetn,
        input           clk50m_max10,
        input           clk100m_lpddr2,

        //User IO: LED/PB/DIPSW
        inout  [9:0]    user_io,
        output [4:0]    user_led,
        input  [3:0]    user_pb,
        input  [5:0]    user_dipsw,

        //PMOD
        inout  [7:0]    pmoda_d,
        inout  [7:0]    pmodb_d,

        //QSPI
        output          flash_clk,
        inout  [3:0]    flash_d,
        output          flash_csn,
        output          flash_resetn,

        //USB 
        input           usb_clk,
        input           usb_resetn,
        inout  [7:0]    usb_data,
        inout  [1:0]    usb_addr,
        output          usb_full,
        output          usb_empty,
        input           usb_wrn,
        input           usb_rdn,
        input           usb_oen,
        inout           usb_scl,
        inout           usb_sda,

        //HDMI TX
        output          hdmi_video_clk,
        output [23:0]   hdmi_video_din,
        output          hdmi_hsync,
        output          hdmi_vsync,
        output          hdmi_video_data_en,
        input           hdmi_intr,
        inout           hdmi_sda,
        output          hdmi_scl,

        //LPDDR2
        output [9:0]    lpddr2_ca,
        inout  [15:0]   lpddr2_dq,
        output [1:0]    lpddr2_dm,
        inout           lpddr2_dqs1n,
        inout           lpddr2_dqs1,
        inout           lpddr2_dqs0n,
        inout           lpddr2_dqs0,
        output          lpddr2_cke,
        output          lpddr2_csn,
        inout           lpddr2_ckn,
        inout           lpddr2_ck,

        /*** Rx MIPI Channel ***/
        
        /* Low power lane data */
        input           ov10640_rx_clk_lp_p,
        input           ov10640_rx_data_0_lp_p, 
        input           ov10640_rx_data_1_lp_p,
        input           ov10640_rx_data_2_lp_p,
        input           ov10640_rx_data_3_lp_p,
        
        input           ov10640_rx_clk_lp_n,
        input           ov10640_rx_data_0_lp_n, 
        input           ov10640_rx_data_1_lp_n,
        input           ov10640_rx_data_2_lp_n,
        input           ov10640_rx_data_3_lp_n,
        
        /* Differential lane data  */
        input           ov10640_rx_clk_hs_p,
        input           ov10640_rx_data_0_hs_p,
        input           ov10640_rx_data_1_hs_p,
        input           ov10640_rx_data_2_hs_p,
        input           ov10640_rx_data_3_hs_p,
        
        //input  rx_clk_hs_n,
        input           ov10640_rx_data_0_hs_n,
        input           ov10640_rx_data_1_hs_n,
        input           ov10640_rx_data_2_hs_n,
        input           ov10640_rx_data_3_hs_n,
        
        /*** Tx MIPI Channel ***/
        
        /* Low power lane data */
        output          mipi_tx_clk_lp_p,
        output          mipi_tx_data_0_lp_p, 
        output          mipi_tx_data_1_lp_p,
        output          mipi_tx_data_2_lp_p,
        output          mipi_tx_data_3_lp_p,
        
        output          mipi_tx_clk_lp_n,
        output          mipi_tx_data_0_lp_n, 
        output          mipi_tx_data_1_lp_n,
        output          mipi_tx_data_2_lp_n,
        output          mipi_tx_data_3_lp_n,
        
        /* Differential lane data  */
        output          mipi_tx_clk_hs_p,
        output          mipi_tx_data_0_hs_p,
        output          mipi_tx_data_1_hs_p,
        output          mipi_tx_data_2_hs_p,
        output          mipi_tx_data_3_hs_p,
        
        output          mipi_tx_clk_hs_n,
        output          mipi_tx_data_0_hs_n,
        output          mipi_tx_data_1_hs_n,
        output          mipi_tx_data_2_hs_n,
        output          mipi_tx_data_3_hs_n
        );

endmodule
