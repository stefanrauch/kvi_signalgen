-- megafunction wizard: %ALTREMOTE_UPDATE%
-- GENERATION: STANDARD
-- VERSION: WM1.0
-- MODULE: altremote_update 

-- ============================================================
-- File Name: flash_update.vhd
-- Megafunction Name(s):
-- 			altremote_update
--
-- Simulation Library Files(s):
-- 			arriaii;lpm
-- ============================================================
-- ************************************************************
-- THIS IS A WIZARD-GENERATED FILE. DO NOT EDIT THIS FILE!
--
-- 11.1 Build 173 11/01/2011 SJ Full Version
-- ************************************************************


--Copyright (C) 1991-2011 Altera Corporation
--Your use of Altera Corporation's design tools, logic functions 
--and other software and tools, and its AMPP partner logic 
--functions, and any output files from any of the foregoing 
--(including device programming or simulation files), and any 
--associated documentation or information are expressly subject 
--to the terms and conditions of the Altera Program License 
--Subscription Agreement, Altera MegaCore Function License 
--Agreement, or other applicable license agreement, including, 
--without limitation, that your use is for the sole purpose of 
--programming logic devices manufactured by Altera and sold by 
--Altera or its authorized distributors.  Please refer to the 
--applicable agreement for further details.


LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY flash_update IS
	PORT
	(
		asmi_busy		: IN STD_LOGIC  := '0';
		asmi_data_valid		: IN STD_LOGIC  := '0';
		asmi_dataout		: IN STD_LOGIC_VECTOR (7 DOWNTO 0) :=  (OTHERS => '0');
		clock		: IN STD_LOGIC ;
		data_in		: IN STD_LOGIC_VECTOR (23 DOWNTO 0);
		param		: IN STD_LOGIC_VECTOR (2 DOWNTO 0);
		read_param		: IN STD_LOGIC ;
		reconfig		: IN STD_LOGIC ;
		reset		: IN STD_LOGIC ;
		reset_timer		: IN STD_LOGIC ;
		write_param		: IN STD_LOGIC ;
		asmi_addr		: OUT STD_LOGIC_VECTOR (23 DOWNTO 0);
		asmi_rden		: OUT STD_LOGIC ;
		asmi_read		: OUT STD_LOGIC ;
		busy		: OUT STD_LOGIC ;
		data_out		: OUT STD_LOGIC_VECTOR (23 DOWNTO 0);
		pof_error		: OUT STD_LOGIC 
	);
END flash_update;


ARCHITECTURE SYN OF flash_update IS

	SIGNAL sub_wire0	: STD_LOGIC ;
	SIGNAL sub_wire1	: STD_LOGIC_VECTOR (23 DOWNTO 0);
	SIGNAL sub_wire2	: STD_LOGIC ;
	SIGNAL sub_wire3	: STD_LOGIC ;
	SIGNAL sub_wire4	: STD_LOGIC ;
	SIGNAL sub_wire5	: STD_LOGIC_VECTOR (23 DOWNTO 0);



	COMPONENT altremote_update
	GENERIC (
		check_app_pof		: STRING;
		intended_device_family		: STRING;
		in_data_width		: NATURAL;
		operation_mode		: STRING;
		out_data_width		: NATURAL;
		lpm_type		: STRING
	);
	PORT (
			data_in	: IN STD_LOGIC_VECTOR (23 DOWNTO 0);
			asmi_read	: OUT STD_LOGIC ;
			data_out	: OUT STD_LOGIC_VECTOR (23 DOWNTO 0);
			param	: IN STD_LOGIC_VECTOR (2 DOWNTO 0);
			reconfig	: IN STD_LOGIC ;
			asmi_data_valid	: IN STD_LOGIC ;
			asmi_rden	: OUT STD_LOGIC ;
			pof_error	: OUT STD_LOGIC ;
			asmi_dataout	: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
			busy	: OUT STD_LOGIC ;
			reset	: IN STD_LOGIC ;
			reset_timer	: IN STD_LOGIC ;
			write_param	: IN STD_LOGIC ;
			asmi_addr	: OUT STD_LOGIC_VECTOR (23 DOWNTO 0);
			asmi_busy	: IN STD_LOGIC ;
			clock	: IN STD_LOGIC ;
			read_param	: IN STD_LOGIC 
	);
	END COMPONENT;

BEGIN
	asmi_read    <= sub_wire0;
	data_out    <= sub_wire1(23 DOWNTO 0);
	asmi_rden    <= sub_wire2;
	pof_error    <= sub_wire3;
	busy    <= sub_wire4;
	asmi_addr    <= sub_wire5(23 DOWNTO 0);

	altremote_update_component : altremote_update
	GENERIC MAP (
		check_app_pof => "true",
		intended_device_family => "Arria II GX",
		in_data_width => 24,
		operation_mode => "REMOTE",
		out_data_width => 24,
		lpm_type => "altremote_update"
	)
	PORT MAP (
		data_in => data_in,
		param => param,
		reconfig => reconfig,
		asmi_data_valid => asmi_data_valid,
		asmi_dataout => asmi_dataout,
		reset => reset,
		reset_timer => reset_timer,
		write_param => write_param,
		asmi_busy => asmi_busy,
		clock => clock,
		read_param => read_param,
		asmi_read => sub_wire0,
		data_out => sub_wire1,
		asmi_rden => sub_wire2,
		pof_error => sub_wire3,
		busy => sub_wire4,
		asmi_addr => sub_wire5
	);



END SYN;

-- ============================================================
-- CNX file retrieval info
-- ============================================================
-- Retrieval info: PRIVATE: INTENDED_DEVICE_FAMILY STRING "Arria II GX"
-- Retrieval info: PRIVATE: SIM_INIT_CONFIG_COMBO STRING "FACTORY"
-- Retrieval info: PRIVATE: SIM_INIT_PAGE_SELECT_COMBO STRING "0"
-- Retrieval info: PRIVATE: SIM_INIT_STAT_BIT0_CHECK STRING "0"
-- Retrieval info: PRIVATE: SIM_INIT_STAT_BIT1_CHECK STRING "0"
-- Retrieval info: PRIVATE: SIM_INIT_STAT_BIT2_CHECK STRING "0"
-- Retrieval info: PRIVATE: SIM_INIT_STAT_BIT3_CHECK STRING "0"
-- Retrieval info: PRIVATE: SIM_INIT_STAT_BIT4_CHECK STRING "0"
-- Retrieval info: PRIVATE: SIM_INIT_WATCHDOG_VALUE_EDIT STRING "1"
-- Retrieval info: PRIVATE: SUPPORT_WRITE_CHECK STRING "1"
-- Retrieval info: PRIVATE: SYNTH_WRAPPER_GEN_POSTFIX STRING "0"
-- Retrieval info: PRIVATE: WATCHDOG_ENABLE_CHECK STRING "0"
-- Retrieval info: CONSTANT: CHECK_APP_POF STRING "true"
-- Retrieval info: CONSTANT: INTENDED_DEVICE_FAMILY STRING "Arria II GX"
-- Retrieval info: CONSTANT: IN_DATA_WIDTH NUMERIC "24"
-- Retrieval info: CONSTANT: OPERATION_MODE STRING "REMOTE"
-- Retrieval info: CONSTANT: OUT_DATA_WIDTH NUMERIC "24"
-- Retrieval info: USED_PORT: asmi_addr 0 0 24 0 OUTPUT NODEFVAL "asmi_addr[23..0]"
-- Retrieval info: USED_PORT: asmi_busy 0 0 0 0 INPUT GND "asmi_busy"
-- Retrieval info: USED_PORT: asmi_data_valid 0 0 0 0 INPUT GND "asmi_data_valid"
-- Retrieval info: USED_PORT: asmi_dataout 0 0 8 0 INPUT GND "asmi_dataout[7..0]"
-- Retrieval info: USED_PORT: asmi_rden 0 0 0 0 OUTPUT NODEFVAL "asmi_rden"
-- Retrieval info: USED_PORT: asmi_read 0 0 0 0 OUTPUT NODEFVAL "asmi_read"
-- Retrieval info: USED_PORT: busy 0 0 0 0 OUTPUT NODEFVAL "busy"
-- Retrieval info: USED_PORT: clock 0 0 0 0 INPUT NODEFVAL "clock"
-- Retrieval info: USED_PORT: data_in 0 0 24 0 INPUT NODEFVAL "data_in[23..0]"
-- Retrieval info: USED_PORT: data_out 0 0 24 0 OUTPUT NODEFVAL "data_out[23..0]"
-- Retrieval info: USED_PORT: param 0 0 3 0 INPUT NODEFVAL "param[2..0]"
-- Retrieval info: USED_PORT: pof_error 0 0 0 0 OUTPUT NODEFVAL "pof_error"
-- Retrieval info: USED_PORT: read_param 0 0 0 0 INPUT NODEFVAL "read_param"
-- Retrieval info: USED_PORT: reconfig 0 0 0 0 INPUT NODEFVAL "reconfig"
-- Retrieval info: USED_PORT: reset 0 0 0 0 INPUT NODEFVAL "reset"
-- Retrieval info: USED_PORT: reset_timer 0 0 0 0 INPUT NODEFVAL "reset_timer"
-- Retrieval info: USED_PORT: write_param 0 0 0 0 INPUT NODEFVAL "write_param"
-- Retrieval info: CONNECT: @asmi_busy 0 0 0 0 asmi_busy 0 0 0 0
-- Retrieval info: CONNECT: @asmi_data_valid 0 0 0 0 asmi_data_valid 0 0 0 0
-- Retrieval info: CONNECT: @asmi_dataout 0 0 8 0 asmi_dataout 0 0 8 0
-- Retrieval info: CONNECT: @clock 0 0 0 0 clock 0 0 0 0
-- Retrieval info: CONNECT: @data_in 0 0 24 0 data_in 0 0 24 0
-- Retrieval info: CONNECT: @param 0 0 3 0 param 0 0 3 0
-- Retrieval info: CONNECT: @read_param 0 0 0 0 read_param 0 0 0 0
-- Retrieval info: CONNECT: @reconfig 0 0 0 0 reconfig 0 0 0 0
-- Retrieval info: CONNECT: @reset 0 0 0 0 reset 0 0 0 0
-- Retrieval info: CONNECT: @reset_timer 0 0 0 0 reset_timer 0 0 0 0
-- Retrieval info: CONNECT: @write_param 0 0 0 0 write_param 0 0 0 0
-- Retrieval info: CONNECT: asmi_addr 0 0 24 0 @asmi_addr 0 0 24 0
-- Retrieval info: CONNECT: asmi_rden 0 0 0 0 @asmi_rden 0 0 0 0
-- Retrieval info: CONNECT: asmi_read 0 0 0 0 @asmi_read 0 0 0 0
-- Retrieval info: CONNECT: busy 0 0 0 0 @busy 0 0 0 0
-- Retrieval info: CONNECT: data_out 0 0 24 0 @data_out 0 0 24 0
-- Retrieval info: CONNECT: pof_error 0 0 0 0 @pof_error 0 0 0 0
-- Retrieval info: GEN_FILE: TYPE_NORMAL flash_update.vhd TRUE
-- Retrieval info: GEN_FILE: TYPE_NORMAL flash_update.inc FALSE
-- Retrieval info: GEN_FILE: TYPE_NORMAL flash_update.cmp TRUE
-- Retrieval info: GEN_FILE: TYPE_NORMAL flash_update.bsf FALSE
-- Retrieval info: GEN_FILE: TYPE_NORMAL flash_update_inst.vhd FALSE
-- Retrieval info: LIB_FILE: arriaii
-- Retrieval info: LIB_FILE: lpm
