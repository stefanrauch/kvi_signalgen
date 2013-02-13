onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /patterngenerator_tb/reset
add wave -noupdate /patterngenerator_tb/data_in
add wave -noupdate /patterngenerator_tb/data_write
add wave -noupdate /patterngenerator_tb/data_enable
add wave -noupdate /patterngenerator_tb/enable
add wave -noupdate /patterngenerator_tb/start
add wave -noupdate /patterngenerator_tb/busy
add wave -noupdate -expand /patterngenerator_tb/pattern_out
add wave -noupdate /patterngenerator_tb/uut/whiterabbit_clock
add wave -noupdate /patterngenerator_tb/uut/wishbone_clock
add wave -noupdate /patterngenerator_tb/uut/reset
add wave -noupdate /patterngenerator_tb/uut/data_in
add wave -noupdate /patterngenerator_tb/uut/data_write
add wave -noupdate /patterngenerator_tb/uut/data_enable
add wave -noupdate /patterngenerator_tb/uut/enable
add wave -noupdate /patterngenerator_tb/uut/start
add wave -noupdate /patterngenerator_tb/uut/busy
add wave -noupdate -expand /patterngenerator_tb/uut/pattern_out
add wave -noupdate /patterngenerator_tb/uut/mem_readaddress_i
add wave -noupdate /patterngenerator_tb/uut/mem_writeaddress_i
add wave -noupdate /patterngenerator_tb/uut/mem_raddr
add wave -noupdate /patterngenerator_tb/uut/mem_waddr
add wave -noupdate /patterngenerator_tb/uut/mem_writeenable_i
add wave -noupdate -expand /patterngenerator_tb/uut/mem_data_out_i
add wave -noupdate /patterngenerator_tb/uut/data_written_i
add wave -noupdate /patterngenerator_tb/uut/nrofvalsmin1_i
add wave -noupdate /patterngenerator_tb/uut/busy_i
add wave -noupdate /patterngenerator_tb/uut/start_i
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ps} 0}
configure wave -namecolwidth 247
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {1804783 ps}
