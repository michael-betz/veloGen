EESchema Schematic File Version 2
LIBS:veloGen-rescue
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
LIBS:veloGen-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title "veloGen"
Date ""
Rev "0"
Comp "Compu Global Hyper Meganet"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L C C10
U 1 1 58D19908
P 1750 3250
F 0 "C10" H 1550 3150 50  0000 L CNN
F 1 "10u" H 1550 3050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 1788 3100 50  0001 C CNN
F 3 "" H 1750 3250 50  0001 C CNN
	1    1750 3250
	1    0    0    -1  
$EndComp
$Comp
L D_Schottky D6
U 1 1 58D1990B
P 7300 2500
F 0 "D6" H 7250 2400 50  0000 L CNN
F 1 "STPS3L40UF" H 7050 2600 50  0000 L CNN
F 2 "Diodes_SMD:D_SMB_Standard" H 7300 2500 50  0001 C CNN
F 3 "http://www.st.com/resource/en/datasheet/stps3l40.pdf" H 7300 2500 50  0001 C CNN
	1    7300 2500
	-1   0    0    1   
$EndComp
Wire Wire Line
	1600 4050 8200 4050
Wire Wire Line
	1750 2500 1750 3100
Wire Wire Line
	1750 3400 1750 4050
Connection ~ 1750 2500
Connection ~ 1750 4050
Wire Wire Line
	1600 2500 6250 2500
Wire Wire Line
	1600 4950 4400 4950
$Comp
L GND #PWR23
U 1 1 58D19CF5
P 4050 4100
F 0 "#PWR23" H 4050 3850 50  0001 C CNN
F 1 "GND" H 3900 4000 50  0000 C CNN
F 2 "" H 4050 4100 50  0001 C CNN
F 3 "" H 4050 4100 50  0001 C CNN
	1    4050 4100
	1    0    0    -1  
$EndComp
Text HLabel 1600 4950 0    60   Input ~ 0
buckPwm
Text HLabel 1600 2500 0    60   Input ~ 0
vIn
Text HLabel 8200 2500 2    60   Output ~ 0
vOut
Text Notes 7100 6750 0    60   ~ 0
30 V to 4.2 V buck converter
$Comp
L C C11
U 1 1 58D2C50B
P 1900 2850
F 0 "C11" H 1850 2950 50  0000 R CNN
F 1 "470n" H 1850 3050 50  0000 R CNN
F 2 "Capacitors_SMD:C_0805" H 1938 2700 50  0001 C CNN
F 3 "" H 1900 2850 50  0001 C CNN
	1    1900 2850
	-1   0    0    1   
$EndComp
Wire Wire Line
	1900 3000 1900 4050
Connection ~ 1900 4050
Wire Wire Line
	1900 2700 1900 2500
Connection ~ 1900 2500
$Comp
L Transformer_1P_1S T1
U 1 1 58F6E738
P 6650 2700
F 0 "T1" H 6650 3078 50  0000 C CNN
F 1 "10u  10u k=0.99" H 6650 2987 50  0000 C CNN
F 2 "" H 6650 2700 50  0001 C CNN
F 3 "" H 6650 2700 50  0001 C CNN
	1    6650 2700
	1    0    0    -1  
$EndComp
$Comp
L STL6N3LLH6 Q3
U 1 1 58F6E849
P 6100 3150
F 0 "Q3" H 6306 3196 50  0000 L CNN
F 1 "STL6N3LLH6" H 6306 3105 50  0000 L CNN
F 2 "myStuff:PowerFLAT_2x2" H 6350 3050 50  0001 L CIN
F 3 "http://www.st.com/resource/en/datasheet/stl6n3llh6.pdf" H 6350 2950 50  0001 L CNN
	1    6100 3150
	1    0    0    -1  
$EndComp
$Comp
L R R16
U 1 1 58F6E9C1
P 6200 3700
F 0 "R16" H 6270 3746 50  0000 L CNN
F 1 "0.11R" H 6270 3655 50  0000 L CNN
F 2 "" V 6130 3700 50  0001 C CNN
F 3 "" H 6200 3700 50  0001 C CNN
	1    6200 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3350 6200 3550
Wire Wire Line
	4050 4050 4050 4100
Connection ~ 4050 4050
Wire Wire Line
	6200 3850 6200 4050
Connection ~ 6200 4050
Wire Wire Line
	6200 2050 6200 2950
Wire Wire Line
	6200 2900 6250 2900
$Comp
L C C13
U 1 1 58F6EE0F
P 6650 2050
F 0 "C13" V 6398 2050 50  0000 C CNN
F 1 "1u" V 6489 2050 50  0000 C CNN
F 2 "" H 6688 1900 50  0001 C CNN
F 3 "" H 6650 2050 50  0001 C CNN
	1    6650 2050
	0    1    1    0   
$EndComp
Wire Wire Line
	6500 2050 6200 2050
Connection ~ 6200 2900
Wire Wire Line
	6800 2050 7100 2050
Wire Wire Line
	7100 2050 7100 2500
Connection ~ 7100 2500
Wire Wire Line
	7050 2900 7100 2900
Wire Wire Line
	7100 2900 7100 3550
Wire Wire Line
	7050 2500 7150 2500
$Comp
L C C14
U 1 1 58F6F4C6
P 7900 3250
F 0 "C14" H 7700 3150 50  0000 L CNN
F 1 "10u" H 7700 3050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 7938 3100 50  0001 C CNN
F 3 "" H 7900 3250 50  0001 C CNN
	1    7900 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7900 2500 7900 3100
$Comp
L C C15
U 1 1 58F6F4CD
P 8050 2850
F 0 "C15" H 8000 2950 50  0000 R CNN
F 1 "470n" H 8000 3050 50  0000 R CNN
F 2 "Capacitors_SMD:C_0805" H 8088 2700 50  0001 C CNN
F 3 "" H 8050 2850 50  0001 C CNN
	1    8050 2850
	-1   0    0    1   
$EndComp
Wire Wire Line
	8050 2500 8050 2700
Wire Wire Line
	7900 3400 7900 4050
Connection ~ 7900 4050
Wire Wire Line
	8050 3000 8050 4050
Connection ~ 8050 4050
Wire Wire Line
	7450 2500 8200 2500
Connection ~ 7900 2500
Connection ~ 8050 2500
$Comp
L MCP1630 U5
U 1 1 58F6FD39
P 4800 3400
F 0 "U5" H 4750 3950 60  0000 C CNN
F 1 "MCP1630" H 4900 3450 60  0000 C CNN
F 2 "Housings_SSOP:MSOP-8_3x3mm_Pitch0.65mm" H 4850 3350 60  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/21896b.pdf" H 5300 3250 60  0001 C CNN
	1    4800 3400
	1    0    0    -1  
$EndComp
$Comp
L R R15
U 1 1 58F7007F
P 5650 3150
F 0 "R15" V 5700 3300 50  0000 C CNN
F 1 "10R" V 5650 3150 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 5580 3150 50  0001 C CNN
F 3 "" H 5650 3150 50  0001 C CNN
	1    5650 3150
	0    1    1    0   
$EndComp
Wire Wire Line
	5900 3150 5800 3150
Wire Wire Line
	5500 3150 5400 3150
$Comp
L GND #PWR24
U 1 1 58F706AE
P 5450 3300
F 0 "#PWR24" H 5450 3050 50  0001 C CNN
F 1 "GND" H 5350 3300 50  0000 C CNN
F 2 "" H 5450 3300 50  0001 C CNN
F 3 "" H 5450 3300 50  0001 C CNN
	1    5450 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 3250 5400 3250
Wire Wire Line
	5450 3300 5450 3250
Wire Wire Line
	4400 4950 4400 3250
Wire Wire Line
	4400 3250 4450 3250
$Comp
L +BATT #PWR25
U 1 1 58F7103E
P 5500 3050
F 0 "#PWR25" H 5500 2900 50  0001 C CNN
F 1 "+BATT" V 5500 3150 50  0000 L CNN
F 2 "" H 5500 3050 50  0001 C CNN
F 3 "" H 5500 3050 50  0001 C CNN
	1    5500 3050
	0    1    1    0   
$EndComp
Wire Wire Line
	5500 3050 5400 3050
Text Notes 1650 4800 0    60   ~ 0
When buckPwm is high, Vext is forced low\nOn neg. edge: Vext high until cycle current limit kicks in\n ... or until buckPwm goes high again
$Comp
L R R17
U 1 1 58F71873
P 7100 3700
F 0 "R17" H 7170 3746 50  0000 L CNN
F 1 "0.11R" H 7170 3655 50  0000 L CNN
F 2 "" V 7030 3700 50  0001 C CNN
F 3 "" H 7100 3700 50  0001 C CNN
	1    7100 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 3850 7100 4050
Connection ~ 7100 4050
$EndSCHEMATC
