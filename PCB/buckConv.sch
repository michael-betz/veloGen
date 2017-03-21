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
L C C?
U 1 1 58D19908
P 3450 3250
F 0 "C?" H 3550 3150 50  0000 L CNN
F 1 "10u" H 3550 3050 50  0000 L CNN
F 2 "" H 3488 3100 50  0001 C CNN
F 3 "" H 3450 3250 50  0001 C CNN
	1    3450 3250
	1    0    0    -1  
$EndComp
$Comp
L L L?
U 1 1 58D1990A
P 4950 2500
F 0 "L?" V 5140 2500 50  0000 C CNN
F 1 "L" V 5049 2500 50  0000 C CNN
F 2 "" H 4950 2500 50  0001 C CNN
F 3 "" H 4950 2500 50  0001 C CNN
	1    4950 2500
	0    -1   -1   0   
$EndComp
$Comp
L D_Schottky D?
U 1 1 58D1990B
P 4700 3200
F 0 "D?" H 4900 3100 50  0000 L CNN
F 1 "STPS3L40UF" H 4350 3100 50  0000 L CNN
F 2 "Diodes_SMD:D_SMB_Standard" H 4700 3200 50  0001 C CNN
F 3 "http://www.st.com/resource/en/datasheet/stps3l40.pdf" H 4700 3200 50  0001 C CNN
	1    4700 3200
	0    1    1    0   
$EndComp
$Comp
L C C?
U 1 1 58D1990C
P 5550 3250
F 0 "C?" H 5600 3150 50  0000 L CNN
F 1 "10u" H 5600 3050 50  0000 L CNN
F 2 "" H 5588 3100 50  0001 C CNN
F 3 "" H 5550 3250 50  0001 C CNN
	1    5550 3250
	1    0    0    -1  
$EndComp
$Comp
L STL6N3LLH6 Q?
U 1 1 58D19914
P 4300 2600
F 0 "Q?" V 4643 2600 50  0000 C CNN
F 1 "STL6N3LLH6" V 4552 2600 50  0000 C CNN
F 2 "myStuff:PowerFLAT_2x2" H 4550 2500 50  0001 L CIN
F 3 "http://www.st.com/resource/en/datasheet/stl6n3llh6.pdf" H 4550 2400 50  0001 L CNN
	1    4300 2600
	0    -1   -1   0   
$EndComp
$Comp
L C C?
U 1 1 58D19918
P 2450 4450
F 0 "C?" H 2400 4350 50  0000 R CNN
F 1 "470n" H 2400 4550 50  0000 R CNN
F 2 "" H 2488 4300 50  0001 C CNN
F 3 "" H 2450 4450 50  0001 C CNN
	1    2450 4450
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58D19919
P 2600 4650
F 0 "#PWR?" H 2600 4400 50  0001 C CNN
F 1 "GND" H 2450 4600 50  0000 C CNN
F 2 "" H 2600 4650 50  0001 C CNN
F 3 "" H 2600 4650 50  0001 C CNN
	1    2600 4650
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 58D1991A
P 3800 4300
F 0 "C?" H 3685 4254 50  0000 R CNN
F 1 "470n" H 3650 4350 50  0000 R CNN
F 2 "" H 3838 4150 50  0001 C CNN
F 3 "" H 3800 4300 50  0001 C CNN
	1    3800 4300
	0    1    1    0   
$EndComp
$Comp
L D_Schottky D?
U 1 1 58D1991B
P 3150 4050
F 0 "D?" H 3250 3950 50  0000 L CNN
F 1 "B5819WS" V 3195 4129 50  0001 L CNN
F 2 "" H 3150 4050 50  0001 C CNN
F 3 "" H 3150 4050 50  0001 C CNN
	1    3150 4050
	-1   0    0    1   
$EndComp
$Comp
L LTC4440 U?
U 1 1 58D1991C
P 2950 4550
F 0 "U?" H 3050 4500 60  0000 C CNN
F 1 "LTC4440-5" H 3150 4400 60  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-6_Handsoldering" H 3750 4350 60  0001 C CNN
F 3 "http://www.linear.com/docs/1706" H 3450 4250 60  0001 C CNN
	1    2950 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1600 3550 3450 3550
Wire Wire Line
	3450 3550 3800 3550
Wire Wire Line
	3800 3550 4700 3550
Wire Wire Line
	4700 3550 5250 3550
Wire Wire Line
	5250 3550 5550 3550
Wire Wire Line
	5550 3550 8200 3550
Wire Wire Line
	3450 2500 3450 3100
Wire Wire Line
	3450 3400 3450 3550
Wire Wire Line
	4000 3550 4000 3650
Connection ~ 3450 2500
Wire Wire Line
	4500 2500 4550 2500
Wire Wire Line
	4550 2500 4700 2500
Wire Wire Line
	4700 2500 4800 2500
Wire Wire Line
	4700 3550 4700 3350
Connection ~ 3450 3550
Wire Wire Line
	1600 2500 3450 2500
Wire Wire Line
	3450 2500 3800 2500
Wire Wire Line
	3800 2500 4100 2500
Wire Wire Line
	4700 2500 4700 3050
Connection ~ 4700 2500
Wire Wire Line
	5550 3100 5550 2500
Wire Wire Line
	5100 2500 5250 2500
Wire Wire Line
	5250 2500 5550 2500
Wire Wire Line
	5550 2500 5800 2500
Wire Wire Line
	5800 2500 8050 2500
Wire Wire Line
	8050 2500 8200 2500
Wire Wire Line
	5550 3550 5550 3400
Connection ~ 4700 3550
Connection ~ 5550 3550
Connection ~ 5550 2500
Wire Wire Line
	2450 4300 2600 4300
Wire Wire Line
	2600 4300 2750 4300
Wire Wire Line
	2600 4400 2600 4600
Wire Wire Line
	2600 4600 2600 4650
Wire Wire Line
	2600 4400 2750 4400
Connection ~ 2600 4600
Wire Wire Line
	3000 4050 2600 4050
Wire Wire Line
	2600 3850 2600 4050
Wire Wire Line
	2600 4050 2600 4300
Connection ~ 2600 4300
Wire Wire Line
	3300 4050 3600 4050
Wire Wire Line
	3600 4050 3600 4300
Wire Wire Line
	3550 4300 3600 4300
Wire Wire Line
	3600 4300 3650 4300
Connection ~ 3600 4300
Wire Wire Line
	3550 4500 4000 4500
Wire Wire Line
	4000 4500 4350 4500
Wire Wire Line
	3550 4400 4300 4400
Wire Wire Line
	4300 4400 4300 2800
Connection ~ 4550 2500
Wire Wire Line
	4550 2500 4550 2750
Wire Wire Line
	4550 2750 4350 2750
Wire Wire Line
	4350 2750 4350 4500
Wire Wire Line
	3950 4300 4000 4300
Wire Wire Line
	4000 4300 4000 4500
Connection ~ 4000 4500
Connection ~ 2600 4050
Wire Wire Line
	2600 4600 2450 4600
Wire Wire Line
	2750 4950 2750 4500
Wire Wire Line
	1600 4950 2750 4950
$Comp
L GND #PWR?
U 1 1 58D19CF5
P 4000 3650
F 0 "#PWR?" H 4000 3400 50  0001 C CNN
F 1 "GND" H 3850 3550 50  0000 C CNN
F 2 "" H 4000 3650 50  0001 C CNN
F 3 "" H 4000 3650 50  0001 C CNN
	1    4000 3650
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
Wire Wire Line
	2600 3850 5800 3850
Wire Wire Line
	5800 3850 5800 2500
Connection ~ 5800 2500
$Comp
L PWR_FLAG #FLG?
U 1 1 58D1D6FC
P 8050 2450
F 0 "#FLG?" H 8050 2525 50  0001 C CNN
F 1 "PWR_FLAG" H 8050 2624 50  0000 C CNN
F 2 "" H 8050 2450 50  0000 C CNN
F 3 "" H 8050 2450 50  0000 C CNN
	1    8050 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	8050 2450 8050 2500
Connection ~ 8050 2500
$Comp
L C C?
U 1 1 58D2C0CB
P 5250 3250
F 0 "C?" H 5200 3350 50  0000 R CNN
F 1 "470n" H 5200 3450 50  0000 R CNN
F 2 "" H 5288 3100 50  0001 C CNN
F 3 "" H 5250 3250 50  0001 C CNN
	1    5250 3250
	-1   0    0    1   
$EndComp
Wire Wire Line
	5250 3400 5250 3550
Connection ~ 5250 3550
Wire Wire Line
	5250 3100 5250 2500
Connection ~ 5250 2500
$Comp
L C C?
U 1 1 58D2C50B
P 3800 3250
F 0 "C?" H 3750 3350 50  0000 R CNN
F 1 "470n" H 3750 3450 50  0000 R CNN
F 2 "" H 3838 3100 50  0001 C CNN
F 3 "" H 3800 3250 50  0001 C CNN
	1    3800 3250
	-1   0    0    1   
$EndComp
Wire Wire Line
	3800 3400 3800 3550
Connection ~ 3800 3550
Wire Wire Line
	3800 3100 3800 2500
Connection ~ 3800 2500
$EndSCHEMATC
