module e4fpmk7_mmb(
//-----------------on motherboard------------
    //sd card
    inout sd_cd,
    inout [3:0] sd_d,
    inout sd_clk,
    inout sd_cmd,
    //cs534x i2s adc
    output adc_rst_n,
    output adc_mclk,
    inout  adc_sclk,
    inout  adc_lrck,
    input  adc_sdout,
    output [1:0] adc_mode,
    //cs43xx i2s dac
    output dac_sclk,
    output dac_lrck,
    output dac_sdin,
    output dac_mclk,
    //ps2 interface
    inout ps2_clk_1,    ps2_clk_2,
    inout ps2_data_1,   ps2_data_2,
    //UART interface
    input uart_rxd,
    output uart_txd,
    //vga interface
    output          vga_clk,
    output [7:0]    vga_blue, vga_green, vga_red,
    output          vga_sync_n,
    output          vga_psave_n,   //0:7123 in power save mode
    output          vga_blank_n, vga_hs, vga_vs,
    //key
    input [7:0]     key,
    //but
    input [7:0]     but,
    //7-seg display
    output          seg_g, seg_f, seg_e, seg_d, seg_c, seg_b, seg_a, seg_dp,
    output [3:0]    seg_sel,
    //led output
    output [7:0]    led,
    //clock generate (CDCM 61002)
    output [1:0]    clkgen_pr, clkgen_os,
    output [2:0]    clkgen_od,
    //gpio total 17*2
    input [16:0]    gpio0, gpio1;
//--------------on SoM part----------------------
    //eth phy
    output          rgmii_reset_n,
    inout           rgmii_mdio, rgmii_mdc,
    input [3:0]     rgmii_rxd,
    input           rgmii_rxck, rgmii_rxctl,
    output          rgmii_txctl, rgmii_txck,
    output[3:0]     rgmii_txd,
    //  emmc
    inout [7:0]     emmc_dq,
    output          emmc_cmd, emmc_clk,emmc_rst_n,
    //  user flash
    output          qspi_cs_n,
    inout [3:0]     qspi_dq,
    output          qspi_sclk,
    // user led & key
    output          som_led,
    input           som_key,
    // clock
    input       ext_clk_27m,
    input       refclk_200m_p,
    input       refclk_200m_n,
    //usb phy (device)
    input       usbd_clk, usbd_dir, usbd_nxt, usbd_stp,
    inout [7:0] usbd_dq,
    //usb phy (host)
    input       usbm_clk, usbm_dir, usbm_nxt, usbm_stp,
    inout [7:0] usbm_dq,
    //hdmi (or GPIO)
    inout           hdmi_scl, hdmi_sck,
    output [2:0]    tmds_l_p, tmds_l_n,
    output          tmds_clk_p, tmds_clk_n,
    //pwm vid (CAUTION WHEN USE THIS SIGNAL)
    output          pwmvid      //when use pwmvid, the dcdc converter must be set to pwmvid-ctrl mode
                                //the default config is fixed 1V output
);

endmodule