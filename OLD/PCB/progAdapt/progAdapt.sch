EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:myStuff
LIBS:progAdapt-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Conn_01x06 J1
U 1 1 59D80D8F
P 3300 2500
F 0 "J1" H 3300 2850 50  0000 L CNN
F 1 "Conn_01x06" H 3150 2100 50  0000 L CNN
F 2 "myStuff:myPogoPads" H 3300 2500 50  0001 C CNN
F 3 "" H 3300 2500 50  0001 C CNN
	1    3300 2500
	1    0    0    -1  
$EndComp
Text Notes 3400 2800 0    60   ~ 0
Rst\nRXD\nTXD\n5 V\nGND\nGND
$Comp
L TTL_232R_3V3 P1
U 1 1 59D80F83
P 2100 2200
F 0 "P1" H 2072 2497 60  0000 R CNN
F 1 "TTL_232R_3V3" H 2072 2603 60  0000 R CNN
F 2 "Socket_Strips:Socket_Strip_Straight_1x06_Pitch2.54mm" H 2100 2200 60  0001 C CNN
F 3 "" H 2100 2200 60  0001 C CNN
	1    2100 2200
	1    0    0    1   
$EndComp
Wire Wire Line
	2650 2300 3100 2300
Wire Wire Line
	2650 2400 3100 2400
Wire Wire Line
	2650 2500 3100 2500
Wire Wire Line
	2650 2600 3100 2600
Wire Wire Line
	2650 2800 3100 2800
Wire Wire Line
	3100 2700 2900 2700
Wire Wire Line
	2900 2700 2900 2850
Connection ~ 2900 2800
NoConn ~ 2650 2700
$Comp
L GND #PWR?
U 1 1 59D81091
P 2900 2850
F 0 "#PWR?" H 2900 2600 50  0001 C CNN
F 1 "GND" H 2905 2677 50  0000 C CNN
F 2 "" H 2900 2850 50  0001 C CNN
F 3 "" H 2900 2850 50  0001 C CNN
	1    2900 2850
	1    0    0    -1  
$EndComp
$EndSCHEMATC
