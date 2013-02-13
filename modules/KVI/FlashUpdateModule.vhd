-------------------------------------------------------------------------------
-- Title      : Flash Update Module
-- Project    : White Rabbit Remote Flash Update
-------------------------------------------------------------------------------
-- File       : FlashUpdateModule.vhd
-- Author     : Peter Schakel
-- Company    : KVI
-- Created    : 2012-11-21
-- Last update: 2012-11-21
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description:
--
-- Writes data in external Serial Flash
-- Accesses Remote System Upgrade module to reconfigure FPGA with updated configuration.
-- 
-- 
-- Generics
--
-- Inputs
--     clk_sys_i : 125MHz Whishbone bus clock
--     clk_cal_i : Clock for serial flash access
--     rst_n_i : reset: low active
--     gpio_slave_i : Record with Whishbone Bus signals
--     watchdog_reset_i : Reset watchdog timer
--
-- Outputs
--     gpio_slave_o : Record with Whishbone Bus signals
--
-- Components
--     flash_access : Module to access external flash
--     flash_update : Module for checking and loading new configuration
--     posedge_to_pulse : Makes one pulse from a rising edge in a different clock domain
--
--
-------------------------------------------------------------------------------
-- Copyright (c) 2012 KVI / Peter Schakel
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all ;
use ieee.std_logic_arith.all ;

library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;

entity FlashUpdateModule is
	port(
		clk_sys_i                              : in std_logic;
		clk_cal_i                              : in std_logic;
		rst_n_i                                : in std_logic;
		gpio_slave_i                           : in t_wishbone_slave_in;
		gpio_slave_o                           : out t_wishbone_slave_out;
		watchdog_reset_i                       : in std_logic
    );
end FlashUpdateModule;

architecture struct of FlashUpdateModule is

component wb_FlashUpdate is
  port (
-- 
    rst_n_i                                  : in     std_logic;
-- 
    wb_clk_i                                 : in     std_logic;
-- 
    wb_addr_i                                : in     std_logic_vector(2 downto 0);
-- 
    wb_data_i                                : in     std_logic_vector(31 downto 0);
-- 
    wb_data_o                                : out    std_logic_vector(31 downto 0);
-- 
    wb_cyc_i                                 : in     std_logic;
-- 
    wb_sel_i                                 : in     std_logic_vector(3 downto 0);
-- 
    wb_stb_i                                 : in     std_logic;
-- 
    wb_we_i                                  : in     std_logic;
-- 
    wb_ack_o                                 : out    std_logic;
-- Port for std_logic_vector field: 'parameter data' in reg: 'Flash parameters'
    wbflash_params_data_o                    : out    std_logic_vector(23 downto 0);
-- Port for std_logic_vector field: 'parameter address' in reg: 'Flash parameters'
    wbflash_params_address_o                 : out    std_logic_vector(2 downto 0);
-- Ports for PASS_THROUGH field: 'parameter write' in reg: 'Flash parameters'
    wbflash_params_write_o                   : out    std_logic_vector(0 downto 0);
    wbflash_params_write_wr_o                : out    std_logic;
-- Ports for PASS_THROUGH field: 'parameter read request' in reg: 'Flash parameters'
    wbflash_params_request_o                 : out    std_logic_vector(0 downto 0);
    wbflash_params_request_wr_o              : out    std_logic;
-- Ports for PASS_THROUGH field: 'start reconfiguration' in reg: 'Flash parameters'
    wbflash_params_reconf_o                  : out    std_logic_vector(2 downto 0);
    wbflash_params_reconf_wr_o               : out    std_logic;
-- Port for std_logic_vector field: 'data read' in reg: 'Flash parameters read'
    wbflash_params_read_data_i               : in     std_logic_vector(23 downto 0);
-- Port for std_logic_vector field: 'addres read' in reg: 'Flash parameters read'
    wbflash_params_read_address_i            : in     std_logic_vector(2 downto 0);
-- Port for std_logic_vector field: 'configuration busy' in reg: 'Flash parameters read'
    wbflash_params_read_busy_i               : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'configuration data error' in reg: 'Flash parameters read'
    wbflash_params_read_error_i              : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'configuration write error' in reg: 'Flash parameters read'
    wbflash_params_read_illegal_i            : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'illegal flash erase' in reg: 'Flash parameters read'
    wbflash_params_read_erase_error_i        : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'flash data' in reg: 'Flash data'
    wbflash_flash_data_data_o                : out    std_logic_vector(7 downto 0);
-- Port for std_logic_vector field: 'flash address' in reg: 'Flash data'
    wbflash_flash_data_address_o             : out    std_logic_vector(23 downto 0);
-- Ports for PASS_THROUGH field: 'Access to flash, disable in wb_FlashUpdate.vhd' in reg: 'Flash data'
--//    wbflash_flash_data_access_o              : out    std_logic_vector;
    wbflash_flash_data_access_wr_o           : out    std_logic;
-- Port for std_logic_vector field: 'data from flash' in reg: 'Flash read'
    wbflash_flash_read_data_i                : in     std_logic_vector(7 downto 0);
-- Port for std_logic_vector field: 'data from flash is valid' in reg: 'Flash read'
    wbflash_flash_read_valid_i               : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'accessing flash is busy' in reg: 'Flash read'
    wbflash_flash_read_busy_i                : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'error accessing flash' in reg: 'Flash read'
    wbflash_flash_read_error_i               : in     std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'Enable data access' in reg: 'Flash access'
    wbflash_flash_access_enable_o            : out    std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'Enable reading from flash' in reg: 'Flash access'
    wbflash_flash_access_read_enable_o       : out    std_logic_vector(0 downto 0);
-- Port for std_logic_vector field: 'Enable writing to flash' in reg: 'Flash access'
    wbflash_flash_access_write_enable_o      : out    std_logic_vector(2 downto 0);
-- Port for std_logic_vector field: 'Enable erasing sector' in reg: 'Flash access'
    wbflash_flash_access_erase_enable_o      : out    std_logic_vector(2 downto 0);
-- Ports for PASS_THROUGH field: 'Read ID from flash' in reg: 'Flash access'
    wbflash_flash_access_id_o                : out    std_logic_vector(0 downto 0);
    wbflash_flash_access_id_wr_o             : out    std_logic;
-- Ports for PASS_THROUGH field: 'Read status form flash' in reg: 'Flash access'
    wbflash_flash_access_status_o            : out    std_logic_vector(0 downto 0);
    wbflash_flash_access_status_wr_o         : out    std_logic

	 );
end component;


 component  flash_access IS 
	 PORT 
	 ( 
		 addr	:	in  std_logic_vector (23 DOWNTO 0);
		 busy	:	out  std_logic;
		 clkin	:	in  std_logic;
		 data_valid	:	out  std_logic;
		 datain	:	in  std_logic_vector (7 DOWNTO 0) := (OTHERS => '0');
		 dataout	:	out  std_logic_vector (7 DOWNTO 0);
		 illegal_erase	:	out  std_logic;
		 illegal_write	:	out  std_logic;
		 rden	:	in  std_logic;
		 rdid_out	:	out  std_logic_vector (7 DOWNTO 0);
		 read	:	in  std_logic := '0';
		 read_rdid	:	in  std_logic := '0';
		 read_status	:	in  std_logic := '0';
		 sector_erase	:	in  std_logic := '0';
		 shift_bytes	:	in  std_logic := '0';
		 status_out	:	out  std_logic_vector (7 DOWNTO 0);
		 wren	:	in  std_logic := '1';
		 write	:	in  std_logic := '0'
	 ); 
 end component;


		 
 component flash_update IS
	PORT
	(
		asmi_busy		: IN std_logic  := '0';
		asmi_data_valid		: IN std_logic  := '0';
		asmi_dataout		: IN std_logic_vector (7 DOWNTO 0) :=  (OTHERS => '0');
		clock		: IN std_logic ;
		data_in		: IN std_logic_vector (23 DOWNTO 0);
		param		: IN std_logic_vector (2 DOWNTO 0);
		read_param		: IN std_logic ;
		reconfig		: IN std_logic ;
		reset		: IN std_logic ;
		reset_timer		: IN std_logic ;
		write_param		: IN std_logic ;
		asmi_addr		: OUT std_logic_vector (23 DOWNTO 0);
		asmi_rden		: OUT std_logic ;
		asmi_read		: OUT std_logic ;
		busy		: OUT std_logic ;
		data_out		: OUT std_logic_vector (23 DOWNTO 0);
		pof_error		: OUT std_logic 
	);
END component;
 
component posedge_to_pulse is
	port (
		clock_in                               : in  std_logic;
		clock_out                              : in  std_logic;
		en_clk                                 : in  std_logic;
		signal_in                              : in  std_logic;
		pulse                                  : out std_logic
	);
end component;

signal reset_s                               : std_logic := '0';

signal wbflash_params_write_s                : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_params_write_wr_s             : std_logic := '0';
signal flash_write_param_sysclk_s            : std_logic := '0';

signal wbflash_params_request_s              : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_params_request_wr_s           : std_logic := '0';
signal flash_read_param_sysclk_s             : std_logic := '0';
		
signal wbflash_params_reconf_s               : std_logic_vector(2 downto 0) := (others => '0');
signal wbflash_params_reconf_wr_s            : std_logic := '0';
signal flash_reconfig_sysclk_s               : std_logic := '0';

signal wbflash_params_read_busy_s            : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_params_read_error_s           : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_params_read_illegal_s         : std_logic_vector(0 downto 0) := (others => '0');


signal wbflash_flash_read_valid_s            : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_read_busy_s             : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_read_error_s            : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_access_enable_s         : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_access_read_enable_s    : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_access_write_enable_s   : std_logic_vector(2 downto 0) := (others => '0');
signal wbflash_flash_access_id_s             : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_access_status_s         : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_access_erase_enable_s   : std_logic_vector(2 downto 0) := (others => '0');
signal wbflash_params_read_erase_error_s     : std_logic_vector(0 downto 0) := (others => '0');
signal wbflash_flash_access_id_wr_s          : std_logic := '0';	
signal wbflash_flash_access_status_wr_s      : std_logic := '0';	
		
signal flash_enable_reading_s                : std_logic := '0';	
signal flash_enable_reading_calclk_s         : std_logic := '0';	
signal flash_enable_writing_s                : std_logic := '0';		
signal flash_sector_enable_erasing_s         : std_logic := '0';

signal asmi_busy_s                           : std_logic := '0';
signal asmi_data_valid_s                     : std_logic := '0';
signal asmi_dataout_s                        : std_logic_vector(7 downto 0) := (others => '0');

signal flash_param_in_s                      : std_logic_vector(23 downto 0) := (others => '0');
signal flash_param_s                         : std_logic_vector(2 downto 0) := (others => '0');
signal flash_read_param_s                    : std_logic := '0';
signal flash_reconfig_s                      : std_logic := '0';
signal watchdog_reset_timer_s                : std_logic := '0';
signal flash_write_param_s                   : std_logic := '0';
signal asmi_addr_s                           : std_logic_vector(23 downto 0) := (others => '0');
signal asmi_rden_s                           : std_logic := '0';
signal asmi_read_s                           : std_logic := '0';
signal flash_param_busy_s                    : std_logic := '0';
signal flash_rden_s                          : std_logic := '0';
signal flash_wren_s                          : std_logic := '0';
signal flash_param_out_s                     : std_logic_vector(23 downto 0) := (others => '0');
signal flash_pof_error_s                     : std_logic := '0';
signal flash_data_valid_s                    : std_logic := '0';

signal flash_data_in_sysclk_s                : std_logic_vector(7 downto 0) := (others => '0');
signal flash_data_s                          : std_logic_vector(7 downto 0) := (others => '0');
signal flash_write_address_s                 : std_logic_vector(23 downto 0) := (others => '0');
signal flash_asmi_addr_s                     : std_logic_vector(23 downto 0) := (others => '0');
signal flash_addr_s                          : std_logic_vector(23 downto 0) := (others => '0');
signal flash_access_s                        : std_logic := '0';
signal flash_write_s                         : std_logic := '0';
signal flash_write_sysclk_s                  : std_logic := '0';
signal flash_write_enable_n_s                : std_logic := '0';
signal flash_write_pulse_s                   : std_logic := '0';
signal flash_write_enable_delayed_s          : std_logic := '0';
signal flash_shift_bytes_s                   : std_logic := '0';

signal flash_illegal_write_s                 : std_logic := '0';
signal flash_read_s                          : std_logic := '0';
signal flash_read_sysclk_s                   : std_logic := '0';
signal flash_read_calclk_s                   : std_logic := '0';
signal flash_readenable_s                    : std_logic := '0';
signal flash_startreading_s                  : std_logic := '0';
signal rdfifo_reset_n_s                      : std_logic := '0';
signal rdfifo_write_s                        : std_logic := '0';
signal rdfifo_read_s                         : std_logic := '0';
signal rdfifo_full_s                         : std_logic := '0';
signal rdfifo_empty_s                        : std_logic := '0';
signal rdfifo_bufferoverrun_s                : std_logic := '0';
signal rdfifo_data_out_s                     : std_logic_vector(7 downto 0) := (others => '0');
 
 
signal flash_id_s                            : std_logic_vector(7 downto 0) := (others => '0');
signal flash_read_id_s                       : std_logic := '0';
signal flash_status_s                        : std_logic_vector(7 downto 0) := (others => '0');
signal flash_read_status_s                   : std_logic := '0';
signal flash_read_id_sysclk_s                : std_logic := '0';
signal flash_read_status_sysclk_s            : std_logic := '0';

signal flash_sector_erase_s                  : std_logic := '0';
signal flash_illegal_erase_s                 : std_logic := '0';
signal flash_sector_erase_sysclk_s           : std_logic := '0';

signal wrfifo_full_s                         : std_logic := '0';
signal wrfifo_read_s                         : std_logic := '0';
signal wrfifo_empty_s                        : std_logic := '0';
signal flash_endofwrite_s                    : std_logic := '0';
signal flash_endofwrite_occured_s            : std_logic := '0';
signal flash_data_in_calclk_s                : std_logic_vector(7 downto 0) := (others => '0');
 		 
type flashmode_type is (data,id,status);
signal flashmode : flashmode_type := data;

-- keep for SignalTap
attribute keep: boolean;
attribute keep of flash_asmi_addr_s: signal is true;	
attribute keep of flash_data_in_sysclk_s: signal is true;	
attribute keep of flash_data_in_calclk_s: signal is true;	
attribute keep of flash_read_s: signal is true;	
attribute keep of flash_enable_writing_s: signal is true;	
attribute keep of flash_write_s: signal is true;	
attribute keep of flash_data_valid_s: signal is true;	
attribute keep of flash_illegal_write_s: signal is true;	
attribute keep of flash_wren_s: signal is true;	

		
attribute keep of asmi_busy_s: signal is true;	
attribute keep of asmi_data_valid_s: signal is true;	
attribute keep of asmi_dataout_s: signal is true;	
attribute keep of flash_param_in_s: signal is true;	
attribute keep of flash_param_s: signal is true;	
attribute keep of flash_read_param_s: signal is true;	
attribute keep of flash_rden_s: signal is true;	
attribute keep of flash_reconfig_s: signal is true;	
attribute keep of watchdog_reset_timer_s: signal is true;	
attribute keep of flash_write_param_s: signal is true;	
attribute keep of asmi_addr_s: signal is true;	
attribute keep of asmi_rden_s: signal is true;	
attribute keep of asmi_read_s: signal is true;	
attribute keep of flash_param_busy_s: signal is true;	
attribute keep of flash_param_out_s: signal is true;	
attribute keep of flash_pof_error_s: signal is true;	
attribute keep of reset_s: signal is true;	

attribute keep of wbflash_flash_read_valid_s: signal is true;	
attribute keep of wbflash_flash_access_enable_s: signal is true;	

attribute keep of flash_id_s: signal is true;	
attribute keep of flash_read_id_s: signal is true;	
attribute keep of flash_status_s: signal is true;	
attribute keep of flash_read_status_s: signal is true;	
attribute keep of flash_shift_bytes_s: signal is true;	



attribute keep of flash_write_sysclk_s: signal is true;	
attribute keep of wrfifo_full_s: signal is true;	
attribute keep of wrfifo_read_s: signal is true;	
attribute keep of wrfifo_empty_s: signal is true;	
attribute keep of flash_write_pulse_s: signal is true;	
attribute keep of flash_endofwrite_occured_s: signal is true;	


attribute keep of flash_enable_reading_s: signal is true;	
attribute keep of flash_enable_reading_calclk_s: signal is true;	
attribute keep of flash_startreading_s: signal is true;	
attribute keep of flash_readenable_s: signal is true;	
attribute keep of rdfifo_reset_n_s: signal is true;	
attribute keep of rdfifo_write_s: signal is true;	
attribute keep of rdfifo_full_s: signal is true;	
attribute keep of rdfifo_data_out_s: signal is true;	
attribute keep of rdfifo_read_s: signal is true;	
attribute keep of rdfifo_empty_s: signal is true;	
attribute keep of rdfifo_bufferoverrun_s: signal is true;	





			

begin

watchdog_reset_timer_s <= watchdog_reset_i;

flash_write_param_sysclk_s <= '1' when (wbflash_params_write_s(0)='1') and (wbflash_params_write_wr_s='1') else '0';
sync_param_write: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_write_param_sysclk_s,
		pulse => flash_write_param_s);

		
flash_read_param_sysclk_s <= '1' when (wbflash_params_request_s(0)='1') and (wbflash_params_request_wr_s='1') else '0';
sync_param_read: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_read_param_sysclk_s,
		pulse => flash_read_param_s);

flash_reconfig_sysclk_s <= '1' when (wbflash_params_reconf_s="101") and (wbflash_params_reconf_wr_s='1') else '0';
sync_reconfig: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_reconfig_sysclk_s,
		pulse => flash_reconfig_s);



wb_FlashUpdate1: wb_FlashUpdate port map(
		rst_n_i => rst_n_i,
		wb_clk_i => clk_sys_i,
		wb_addr_i => gpio_slave_i.adr(4 downto 2),
		wb_data_i => gpio_slave_i.dat,
		wb_data_o => gpio_slave_o.dat,
		wb_cyc_i => gpio_slave_i.cyc,
		wb_sel_i => gpio_slave_i.sel,
		wb_stb_i => gpio_slave_i.stb,
		wb_we_i => gpio_slave_i.we,
		wb_ack_o => gpio_slave_o.ack ,
		wbflash_params_data_o => flash_param_in_s,
		wbflash_params_address_o => flash_param_s,
		wbflash_params_write_o => wbflash_params_write_s,
		wbflash_params_write_wr_o => wbflash_params_write_wr_s,
		wbflash_params_request_o => wbflash_params_request_s,
		wbflash_params_request_wr_o => wbflash_params_request_wr_s,
		wbflash_params_reconf_o => wbflash_params_reconf_s,
		wbflash_params_reconf_wr_o => wbflash_params_reconf_wr_s,
		wbflash_params_read_data_i => flash_param_out_s,
		wbflash_params_read_address_i => flash_param_s,
		wbflash_params_read_busy_i => wbflash_params_read_busy_s,
		wbflash_params_read_error_i => wbflash_params_read_error_s,
		wbflash_params_read_illegal_i => wbflash_params_read_illegal_s,
		wbflash_params_read_erase_error_i => wbflash_params_read_erase_error_s,
		wbflash_flash_data_data_o => flash_data_in_sysclk_s,
		wbflash_flash_data_address_o => flash_addr_s,
--		wbflash_flash_data_access_o => open,
		wbflash_flash_data_access_wr_o => flash_access_s,
		wbflash_flash_read_data_i => flash_data_s,
		wbflash_flash_read_valid_i => wbflash_flash_read_valid_s,
		wbflash_flash_read_busy_i => wbflash_flash_read_busy_s,
		wbflash_flash_read_error_i => wbflash_flash_read_error_s,
		wbflash_flash_access_enable_o => wbflash_flash_access_enable_s,
		wbflash_flash_access_read_enable_o => wbflash_flash_access_read_enable_s,
		wbflash_flash_access_write_enable_o => wbflash_flash_access_write_enable_s,
		wbflash_flash_access_erase_enable_o => wbflash_flash_access_erase_enable_s,
		wbflash_flash_access_id_o => wbflash_flash_access_id_s,
		wbflash_flash_access_id_wr_o => wbflash_flash_access_id_wr_s,
		wbflash_flash_access_status_o => wbflash_flash_access_status_s,
		wbflash_flash_access_status_wr_o => wbflash_flash_access_status_wr_s		
		);		

	 
wbflash_params_read_busy_s(0) <= flash_param_busy_s;
wbflash_params_read_error_s(0) <= flash_pof_error_s;


wbflash_flash_read_busy_s(0) <= '1' when (asmi_busy_s='1') or (wrfifo_full_s='1') else '0';
error_process: process(clk_cal_i)
begin
	if rising_edge(clk_cal_i) then
		if reset_s='1' or flash_write_s='1' then 
			wbflash_params_read_illegal_s(0) <= '0';
		elsif flash_illegal_write_s='1' then
			wbflash_params_read_illegal_s(0) <= '1';
		end if;
		if reset_s='1' or flash_sector_erase_s='1' then 
			wbflash_params_read_erase_error_s(0) <= '0';
		elsif flash_illegal_erase_s='1' then
			wbflash_params_read_erase_error_s(0) <= '1';
		end if;
	end if;
end process;


flash_data_s <= 
	flash_id_s when flashmode=id else
	flash_status_s when flashmode=status else	
	rdfifo_data_out_s ;
select_data_process: process(clk_cal_i)
begin
	if rising_edge(clk_cal_i) then
		if reset_s='1' then
			flashmode <= data;
		elsif flash_read_id_s='1' then
			flashmode <= id;
		elsif flash_read_status_s='1' then
			flashmode <= status;
		elsif flash_read_s='1' or flash_write_s='1' or flash_wren_s='1' or flash_rden_s='1' or flash_sector_erase_s='1' then
			flashmode <= data;
		end if;		
	end if;
end process;

-- make delayed write enable signal to have the address be valid until the write pulse
delay_write_enable_process: process(clk_cal_i)
begin
	if rising_edge(clk_cal_i) then
		if flash_enable_writing_s='1' then
			flash_write_enable_delayed_s<= '1';
		elsif flash_write_pulse_s='1' then
			flash_write_enable_delayed_s<= '0';
		end if;
	end if;
end process;

flash_enable_writing_s <= '1' when wbflash_flash_access_write_enable_s="101" and flash_enable_reading_s='0' and flash_sector_enable_erasing_s='0'
		and wbflash_flash_access_enable_s(0)='1' 
	else '0';
flash_enable_reading_s <= '1' when wbflash_flash_access_read_enable_s(0)='1' and flash_enable_writing_s='0' and flash_sector_enable_erasing_s='0'
		and wbflash_flash_access_enable_s(0)='1' 
	else '0';
flash_sector_enable_erasing_s <= '1' when wbflash_flash_access_erase_enable_s="101" and flash_enable_writing_s='0' and flash_enable_reading_s='0'
		and wbflash_flash_access_enable_s(0)='1' 
	else '0';

	
flash_write_sysclk_s <= '1' when (flash_enable_writing_s='1') and (flash_access_s='1') else '0';
flash_write_enable_n_s <= not flash_enable_writing_s;
sync_flash_enable_writing_s: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_write_enable_n_s,
		pulse => flash_endofwrite_s);
		
flash_sector_erase_sysclk_s <= '1' when (flash_sector_enable_erasing_s='1') and (flash_access_s='1') else '0';	
sync_flash_erase: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_sector_erase_sysclk_s,
		pulse => flash_sector_erase_s);
		
		
		
flash_read_sysclk_s <= '1' when (flash_enable_reading_s='1') and (flash_access_s='1') else '0';
sync_flash_read: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_read_sysclk_s,
		pulse => flash_read_calclk_s);

flash_read_id_sysclk_s <= '1' when (flash_enable_reading_s='0') and (flash_enable_writing_s='0') 
	and (wbflash_flash_access_enable_s(0)='1') 
	and (wbflash_flash_access_id_s(0)='1') and (wbflash_flash_access_id_wr_s='1') else '0';
sync_flash_id_read: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_read_id_sysclk_s,
		pulse => flash_read_id_s);

flash_read_status_sysclk_s <= '1' when (flash_enable_reading_s='0') and (flash_enable_writing_s='0') 
	and (wbflash_flash_access_enable_s(0)='1') 
	and (wbflash_flash_access_status_s(0)='1') and (wbflash_flash_access_status_wr_s='1') else '0';
sync_flash_status_read: posedge_to_pulse port map(
		clock_in => clk_sys_i,
		clock_out => clk_cal_i,
		en_clk => '1',
		signal_in => flash_read_status_sysclk_s,
		pulse => flash_read_status_s);

		
rdfifo_write_s <= '1' when (flash_data_valid_s='1') and (flash_enable_reading_s='1') else '0';	
-- process to check for buffer overrun
process(clk_cal_i)
begin
	if (rst_n_i='0') then
		rdfifo_bufferoverrun_s <= '0';
	elsif rising_edge(clk_cal_i) then
		if (flash_startreading_s='1') then
			rdfifo_bufferoverrun_s <= '0';
		elsif (rdfifo_read_s='1') and (rdfifo_full_s='1') then
			rdfifo_bufferoverrun_s <= '1';
		end if;
	end if;
end process;	 

-- process to generate read signals for flash
process(clk_cal_i)
begin
	if (rst_n_i='0') then
		flash_startreading_s <= '0';
		flash_readenable_s <= '0';
	elsif rising_edge(clk_cal_i) then
		if flash_enable_reading_s='0' then
			flash_startreading_s <= '0';
			flash_readenable_s <= '0';
		elsif (flash_read_calclk_s='1') and (flash_readenable_s='0') then 
			flash_startreading_s <= '1';
			flash_readenable_s <= '1';
		else
			flash_startreading_s <= '0';
		end if;
	end if;
end process;


readfifo: generic_async_fifo 
	generic map (
		g_data_width => 8,
		g_size => 1024
    )
	port map(
		rst_n_i => rdfifo_reset_n_s,
		clk_wr_i => clk_cal_i,
		d_i => asmi_dataout_s,
		we_i => rdfifo_write_s,
		wr_full_o => rdfifo_full_s,
		clk_rd_i => clk_sys_i,
		q_o => rdfifo_data_out_s,
		rd_i => rdfifo_read_s,
		rd_empty_o => rdfifo_empty_s
	);
rdfifo_read_s <= '1' when (rdfifo_empty_s='0') and (flash_read_sysclk_s='1') else '0';
wbflash_flash_read_valid_s(0) <= '1' when rdfifo_empty_s='0' else '0';
rdfifo_reset_n_s <= '0' when (rst_n_i='0') or (flash_enable_reading_s='0') else '1'; -- clear fifo if not reading 
wbflash_flash_read_error_s(0) <= rdfifo_bufferoverrun_s;



writefifo: generic_async_fifo 
	generic map (
		g_data_width => 8,
		g_size => 256
    )
	port map(
    rst_n_i => rst_n_i,
    clk_wr_i => clk_sys_i,
    d_i => flash_data_in_sysclk_s,
    we_i => flash_write_sysclk_s,
    wr_full_o => wrfifo_full_s,
    clk_rd_i => clk_cal_i,
    q_o => flash_data_in_calclk_s,
    rd_i => wrfifo_read_s,
    rd_empty_o => wrfifo_empty_s
    );
wrfifo_read_s <= '1' when wrfifo_empty_s='0' else '0';

-- process to make write pulse for page-write
process(clk_cal_i)
begin
	if rising_edge(clk_cal_i) then
		flash_write_s <= wrfifo_read_s;
		if (flash_endofwrite_s='1') and (wrfifo_empty_s='0') then
			flash_endofwrite_occured_s <= '1';
		end if;
		if ((flash_endofwrite_s='1') or (flash_endofwrite_occured_s='1')) and (wrfifo_empty_s='1') then -- make last write pulse			
			flash_write_pulse_s <= '1';
			flash_endofwrite_occured_s <= '0';
		else
			flash_write_pulse_s <= '0';
		end if;
	end if;
end process;	 

-- process to synchronise
process(clk_cal_i)
begin
	if rising_edge(clk_cal_i) then
		flash_enable_reading_calclk_s <= flash_enable_reading_s;
	end if;
end process;	
	
flash_asmi_addr_s <= 
	asmi_addr_s when (flash_enable_writing_s='0') and (flash_write_enable_delayed_s='0') and (flash_enable_reading_s='0') and (flash_sector_enable_erasing_s='0') else
	flash_addr_s;
flash_read_s <= asmi_read_s when flash_enable_reading_calclk_s='0' else flash_startreading_s;
flash_rden_s <= asmi_rden_s when flash_enable_reading_calclk_s='0' else flash_readenable_s;
flash_wren_s <= 
	(flash_write_s or flash_write_pulse_s) when (flash_enable_writing_s='1') or (flash_write_enable_delayed_s='1')
	else flash_sector_erase_s when flash_sector_enable_erasing_s='1'
	else flash_write_pulse_s;
flash_shift_bytes_s <= 
	flash_write_s when (flash_enable_writing_s='1') or (flash_write_enable_delayed_s='1')
	else '0';
asmi_data_valid_s <= flash_data_valid_s when flash_enable_reading_calclk_s='0' else '0';
flash_access_inst : flash_access port map(
		addr => flash_asmi_addr_s,
		clkin => clk_cal_i,
		datain => flash_data_in_calclk_s,
		rden => flash_rden_s,
		rdid_out => flash_id_s,
		read => flash_read_s,
		read_rdid => flash_read_id_s,
		read_status => flash_read_status_s,
		status_out => flash_status_s,
		wren => flash_wren_s,
		write => flash_write_pulse_s,
		sector_erase => flash_sector_erase_s,
		shift_bytes	=> flash_shift_bytes_s,
		busy => asmi_busy_s,
		data_valid => flash_data_valid_s,
		dataout => asmi_dataout_s,
		illegal_write => flash_illegal_write_s,
		illegal_erase => flash_illegal_erase_s
		);


reset_s <= '1' when rst_n_i='0' else '0';


flash_update_inst : flash_update port map(
		asmi_busy => asmi_busy_s, -- Input from altasmi_parallel component.
		asmi_data_valid => asmi_data_valid_s, -- Input from altasmi_parallel component
		asmi_dataout => asmi_dataout_s, -- Input from altasmi_parallel component
		clock => clk_cal_i,
		data_in => flash_param_in_s, --Data input for writing parameter data
		param => flash_param_s, -- parameter to be read or updated.
		read_param => flash_read_param_s, --Read signal for the parameter
		reconfig => flash_reconfig_s, -- Indicates when reconfiguration begins
		reset => reset_s,
		reset_timer => watchdog_reset_timer_s, -- Reset signal for watchdog timer
		write_param => flash_write_param_s, -- Write signal for parameter
		asmi_addr => asmi_addr_s, -- Address signal to altasmi_parallel 
		asmi_rden => asmi_rden_s, -- Read enable signal to altasmi_parallel 
		asmi_read => asmi_read_s, -- Read signal to altasmi_parallel 
		busy => flash_param_busy_s, -- indicates when remote update block is reading or writing
		data_out => flash_param_out_s, -- Data output when reading parameters.
		pof_error => flash_pof_error_s -- Detects and invalid application configuration image
		);


  
end struct;

