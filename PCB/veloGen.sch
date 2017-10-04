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
Sheet 1 2
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
L CONN_01X02 J2
U 1 1 58CD9C31
P 2550 2900
F 0 "J2" V 2400 2800 50  0000 R CNN
F 1 "Gen. IN" V 2250 3050 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02_Pitch2.54mm" H 2550 2900 50  0001 C CNN
F 3 "" H 2550 2900 50  0001 C CNN
	1    2550 2900
	0    -1   -1   0   
$EndComp
$Comp
L D_Schottky D1
U 1 1 58CDA218
P 2250 2700
F 0 "D1" V 2250 2779 50  0000 L CNN
F 1 "B5819WS" V 2295 2779 50  0001 L CNN
F 2 "Diodes_SMD:D_SOD-323" H 2250 2700 50  0001 C CNN
F 3 "" H 2250 2700 50  0001 C CNN
	1    2250 2700
	0    1    1    0   
$EndComp
$Comp
L D_Schottky D3
U 1 1 58CDA413
P 2850 2700
F 0 "D3" V 2850 2779 50  0000 L CNN
F 1 "B5819WS" V 2895 2779 50  0001 L CNN
F 2 "Diodes_SMD:D_SOD-323" H 2850 2700 50  0001 C CNN
F 3 "" H 2850 2700 50  0001 C CNN
	1    2850 2700
	0    1    1    0   
$EndComp
$Comp
L D_Schottky D2
U 1 1 58CDA4A3
P 2250 3350
F 0 "D2" V 2250 3429 50  0000 L CNN
F 1 "B5819WS" V 2295 3429 50  0001 L CNN
F 2 "Diodes_SMD:D_SOD-323" H 2250 3350 50  0001 C CNN
F 3 "" H 2250 3350 50  0001 C CNN
	1    2250 3350
	0    1    1    0   
$EndComp
$Comp
L D_Schottky D4
U 1 1 58CDA4A9
P 2850 3350
F 0 "D4" V 2850 3429 50  0000 L CNN
F 1 "B5819WS" V 2895 3429 50  0001 L CNN
F 2 "Diodes_SMD:D_SOD-323" H 2850 3350 50  0001 C CNN
F 3 "" H 2850 3350 50  0001 C CNN
	1    2850 3350
	0    1    1    0   
$EndComp
$Comp
L MAX8881 U4
U 1 1 58CDD6F1
P 9950 2700
F 0 "U4" H 9750 2300 50  0000 C CNN
F 1 "MAX8881" H 10100 2300 50  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-6" H 9950 3050 50  0001 C CIN
F 3 "" H 9950 2700 50  0000 C CNN
	1    9950 2700
	1    0    0    -1  
$EndComp
$Comp
L C C8
U 1 1 58CDD8CD
P 9250 3150
F 0 "C8" H 9050 2950 50  0000 L CNN
F 1 "470n" H 9050 2850 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 9288 3000 50  0001 C CNN
F 3 "" H 9250 3150 50  0001 C CNN
	1    9250 3150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR015
U 1 1 58CDDA95
P 7000 3600
F 0 "#PWR015" H 7000 3350 50  0001 C CNN
F 1 "GND" H 7005 3427 50  0000 C CNN
F 2 "" H 7000 3600 50  0001 C CNN
F 3 "" H 7000 3600 50  0001 C CNN
	1    7000 3600
	1    0    0    -1  
$EndComp
NoConn ~ 10300 2600
$Comp
L +3V3 #PWR022
U 1 1 58CDF05C
P 10600 2500
F 0 "#PWR022" H 10600 2350 50  0001 C CNN
F 1 "+3V3" H 10615 2673 50  0000 C CNN
F 2 "" H 10600 2500 50  0001 C CNN
F 3 "" H 10600 2500 50  0001 C CNN
	1    10600 2500
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR014
U 1 1 58D0C801
P 7000 2450
F 0 "#PWR014" H 7000 2300 50  0001 C CNN
F 1 "+BATT" H 7150 2550 50  0000 C CNN
F 2 "" H 7000 2450 50  0001 C CNN
F 3 "" H 7000 2450 50  0001 C CNN
	1    7000 2450
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 58D1A675
P 3600 2850
F 0 "R4" H 3670 2896 50  0000 L CNN
F 1 "30k1" H 3670 2805 50  0000 L CNN
F 2 "Resistors_SMD:R_0805" V 3530 2850 50  0001 C CNN
F 3 "" H 3600 2850 50  0001 C CNN
	1    3600 2850
	1    0    0    -1  
$EndComp
$Comp
L R R5
U 1 1 58D1A8A9
P 3600 3250
F 0 "R5" H 3700 3150 50  0000 L CNN
F 1 "1k15" H 3700 3050 50  0000 L CNN
F 2 "Resistors_SMD:R_0805" V 3530 3250 50  0001 C CNN
F 3 "" H 3600 3250 50  0001 C CNN
	1    3600 3250
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 58D1AAA9
P 3400 3250
F 0 "C2" H 3250 3150 50  0000 L CNN
F 1 "47n" H 3250 3050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 3438 3100 50  0001 C CNN
F 3 "" H 3400 3250 50  0001 C CNN
	1    3400 3250
	1    0    0    -1  
$EndComp
Text Label 3650 3050 0    60   ~ 0
Vgen
$Comp
L R R6
U 1 1 58D1DCAF
P 6050 2850
F 0 "R6" H 6120 2896 50  0000 L CNN
F 1 "1M150" H 6120 2805 50  0000 L CNN
F 2 "Resistors_SMD:R_0805" V 5980 2850 50  0001 C CNN
F 3 "" H 6050 2850 50  0001 C CNN
	1    6050 2850
	1    0    0    -1  
$EndComp
$Comp
L R R7
U 1 1 58D1DCB5
P 6050 3250
F 0 "R7" H 6150 3200 50  0000 L CNN
F 1 "365k" H 6150 3100 50  0000 L CNN
F 2 "Resistors_SMD:R_0805" V 5980 3250 50  0001 C CNN
F 3 "" H 6050 3250 50  0001 C CNN
	1    6050 3250
	1    0    0    -1  
$EndComp
$Comp
L C C5
U 1 1 58D1DCBD
P 5850 3250
F 0 "C5" H 5650 3150 50  0000 L CNN
F 1 "47n" H 5650 3050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 5888 3100 50  0001 C CNN
F 3 "" H 5850 3250 50  0001 C CNN
	1    5850 3250
	1    0    0    -1  
$EndComp
Text Label 6100 3050 0    60   ~ 0
Vbatt
Text Label 4250 2700 0    60   ~ 0
sepicPwm
$Comp
L +3V3 #PWR04
U 1 1 58D24DE5
P 2550 5050
F 0 "#PWR04" H 2550 4900 50  0001 C CNN
F 1 "+3V3" H 2550 5200 50  0000 C CNN
F 2 "" H 2550 5050 50  0001 C CNN
F 3 "" H 2550 5050 50  0001 C CNN
	1    2550 5050
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 58D24F30
P 2750 5100
F 0 "C1" V 2550 5150 50  0000 R CNN
F 1 "470n" V 2550 5400 50  0000 R CNN
F 2 "Capacitors_SMD:C_0805" H 2788 4950 50  0001 C CNN
F 3 "" H 2750 5100 50  0001 C CNN
	1    2750 5100
	0    1    1    0   
$EndComp
$Comp
L GND #PWR05
U 1 1 58D25BFF
P 2950 5100
F 0 "#PWR05" H 2950 4850 50  0001 C CNN
F 1 "GND" H 2950 5150 50  0000 C CNN
F 2 "" H 2950 5100 50  0001 C CNN
F 3 "" H 2950 5100 50  0001 C CNN
	1    2950 5100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR012
U 1 1 58D26A6B
P 5050 5000
F 0 "#PWR012" H 5050 4750 50  0001 C CNN
F 1 "GND" H 5050 5050 50  0000 C CNN
F 2 "" H 5050 5000 50  0001 C CNN
F 3 "" H 5050 5000 50  0001 C CNN
	1    5050 5000
	1    0    0    -1  
$EndComp
Text Label 5100 6300 0    60   ~ 0
sepicPwm
Text Label 2500 5400 0    60   ~ 0
Vgen
Text Label 2500 5500 0    60   ~ 0
Vbatt
$Comp
L ESP-WROOM-32 U1
U 1 1 58D2AD95
P 3900 5750
F 0 "U1" H 3950 6500 60  0000 C CNN
F 1 "ESP-WROOM-32" H 3950 6350 60  0000 C CNN
F 2 "myStuff:ESP-WROOM-32" H 2050 4350 60  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp_wroom_32_datasheet_en.pdf" H 3700 4200 60  0001 C CNN
	1    3900 5750
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 58D11F44
P 2000 3100
F 0 "R1" V 1793 3100 50  0000 C CNN
F 1 "47k" V 1884 3100 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 1930 3100 50  0001 C CNN
F 3 "" H 2000 3100 50  0001 C CNN
	1    2000 3100
	0    1    1    0   
$EndComp
Text Label 1250 3100 0    60   ~ 0
speedPulse
Text Label 5100 6000 0    60   ~ 0
speedPulse
Text Notes 7050 6750 0    60   ~ 0
A minimalistic bluetooth bike computer\nand hub dynamo power manager
$Sheet
S 4800 2350 600  650 
U 58D19599
F0 "SepicConv." 60
F1 "sepicConv.sch" 60
F2 "buckPwm" I L 4800 2700 60 
F3 "vIn" I L 4800 2500 60 
F4 "vOut" O R 5400 2500 60 
F5 "maxSwCurr" I L 4800 2900 60 
$EndSheet
Text Label 5100 6100 0    60   ~ 0
vExtEnable
Text Label 5100 5900 0    60   ~ 0
extScl
Text Label 5100 5800 0    60   ~ 0
extSda
$Comp
L R R2
U 1 1 58D23995
P 2350 5100
F 0 "R2" V 2143 5100 50  0000 C CNN
F 1 "470k" V 2234 5100 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2280 5100 50  0001 C CNN
F 3 "" H 2350 5100 50  0001 C CNN
	1    2350 5100
	0    1    1    0   
$EndComp
Text Label 2500 5300 0    60   ~ 0
rstBtn
$Comp
L GND #PWR06
U 1 1 58D25783
P 3550 6800
F 0 "#PWR06" H 3550 6550 50  0001 C CNN
F 1 "GND" H 3400 6750 50  0000 C CNN
F 2 "" H 3550 6800 50  0001 C CNN
F 3 "" H 3550 6800 50  0001 C CNN
	1    3550 6800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR013
U 1 1 58D266BB
P 6550 5850
F 0 "#PWR013" H 6550 5600 50  0001 C CNN
F 1 "GND" V 6550 5650 50  0000 C CNN
F 2 "" H 6550 5850 50  0001 C CNN
F 3 "" H 6550 5850 50  0001 C CNN
	1    6550 5850
	1    0    0    -1  
$EndComp
Text Notes 3100 3700 0    60   ~ 0
30 V --> 1.1 V
NoConn ~ 3000 6000
NoConn ~ 3000 6100
NoConn ~ 3000 6200
NoConn ~ 3000 6300
NoConn ~ 3650 6750
NoConn ~ 3750 6750
NoConn ~ 3850 6750
NoConn ~ 3950 6750
NoConn ~ 4050 6750
NoConn ~ 4150 6750
NoConn ~ 4250 6750
NoConn ~ 4350 6750
NoConn ~ 4450 6750
NoConn ~ 4900 5600
NoConn ~ 4900 5200
$Comp
L R R14
U 1 1 58E35502
P 2200 6400
F 0 "R14" V 1993 6400 50  0000 C CNN
F 1 "470R" V 2084 6400 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2130 6400 50  0001 C CNN
F 3 "" H 2200 6400 50  0001 C CNN
	1    2200 6400
	0    1    1    0   
$EndComp
NoConn ~ 4900 5300
Wire Wire Line
	3000 6400 2350 6400
Wire Wire Line
	2150 3100 2500 3100
Wire Wire Line
	2250 2850 2250 3200
Connection ~ 2250 3100
Wire Wire Line
	2600 3100 2850 3100
Wire Wire Line
	2850 2850 2850 3200
Connection ~ 2850 3100
Wire Wire Line
	2250 2550 2250 2500
Wire Wire Line
	2850 2500 2850 2550
Wire Wire Line
	2250 3500 2250 3550
Wire Wire Line
	1250 3550 9700 3550
Wire Wire Line
	2850 3550 2850 3500
Connection ~ 2850 2500
Connection ~ 2850 3550
Wire Wire Line
	5400 2500 10400 2500
Connection ~ 7000 2500
Wire Wire Line
	10400 2750 10300 2750
Wire Wire Line
	9250 2500 9250 3000
Connection ~ 9250 2500
Wire Wire Line
	9600 3000 10600 3000
Wire Wire Line
	3600 3550 3600 3400
Wire Wire Line
	3600 3000 3600 3100
Wire Wire Line
	3400 3550 3400 3400
Wire Wire Line
	3400 3050 3850 3050
Wire Wire Line
	3400 3050 3400 3100
Connection ~ 3600 3050
Wire Wire Line
	6050 3000 6050 3100
Wire Wire Line
	5850 3050 6300 3050
Connection ~ 6050 3050
Wire Wire Line
	2500 5100 2600 5100
Wire Wire Line
	2550 5200 3000 5200
Wire Wire Line
	2550 5050 2550 5200
Connection ~ 2550 5100
Wire Wire Line
	2900 5100 3000 5100
Connection ~ 2950 5100
Wire Wire Line
	4900 5000 5050 5000
Wire Wire Line
	4900 5100 4950 5100
Wire Wire Line
	4950 5100 4950 5000
Connection ~ 4950 5000
Wire Wire Line
	10600 3000 10600 2500
Wire Wire Line
	3600 2500 3600 2700
Connection ~ 3600 2500
Wire Wire Line
	1850 3100 1250 3100
Connection ~ 3400 3550
Connection ~ 3600 3550
Wire Wire Line
	4250 2700 4800 2700
Wire Wire Line
	2250 2500 4800 2500
Connection ~ 9500 3550
Wire Wire Line
	9250 3550 9250 3300
Connection ~ 9250 3550
Wire Wire Line
	9600 2900 9600 3000
Wire Wire Line
	9500 2750 9600 2750
Wire Wire Line
	9500 3550 9500 2750
Wire Wire Line
	9500 2600 9600 2600
Wire Wire Line
	9500 2500 9500 2600
Connection ~ 9500 2500
Wire Wire Line
	10400 2500 10400 2750
Wire Wire Line
	10300 2900 10400 2900
Wire Wire Line
	10400 2900 10400 3000
Connection ~ 10400 3000
Wire Wire Line
	2200 5100 2150 5100
Wire Wire Line
	2150 4750 2150 5300
Wire Wire Line
	3000 5400 2500 5400
Wire Wire Line
	3000 5500 2500 5500
Wire Wire Line
	3550 6800 3550 6750
Wire Wire Line
	4900 6400 6350 6400
Wire Wire Line
	4900 5400 6600 5400
Wire Wire Line
	4900 5500 6600 5500
Wire Wire Line
	4900 6000 5550 6000
Wire Wire Line
	4900 6200 5550 6200
Wire Wire Line
	4900 5900 5550 5900
Wire Wire Line
	4900 5800 5550 5800
$Comp
L GND #PWR03
U 1 1 58E3C87D
P 1650 6400
F 0 "#PWR03" H 1650 6150 50  0001 C CNN
F 1 "GND" V 1650 6200 50  0000 C CNN
F 2 "" H 1650 6400 50  0001 C CNN
F 3 "" H 1650 6400 50  0001 C CNN
	1    1650 6400
	0    1    1    0   
$EndComp
$Comp
L LED D5
U 1 1 58E350A9
P 1850 6400
F 0 "D5" H 2000 6300 50  0000 C CNN
F 1 "LED" H 1850 6300 50  0000 C CNN
F 2 "LEDs:LED_0805" H 1850 6400 50  0001 C CNN
F 3 "" H 1850 6400 50  0001 C CNN
	1    1850 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 6400 2000 6400
Wire Wire Line
	1650 6400 1700 6400
Connection ~ 2250 3550
Wire Wire Line
	2150 5300 3000 5300
Wire Wire Line
	6350 5300 6600 5300
Wire Wire Line
	6350 5300 6350 4750
Wire Wire Line
	6350 4750 2150 4750
Connection ~ 2150 5100
Wire Wire Line
	7000 2450 7000 3150
NoConn ~ 3000 5900
NoConn ~ 3000 5800
Wire Wire Line
	5850 3400 5850 3550
Connection ~ 5850 3550
Wire Wire Line
	5850 3100 5850 3050
Wire Wire Line
	6050 3400 6050 3550
Connection ~ 6050 3550
Wire Wire Line
	6050 2700 6050 2500
Connection ~ 6050 2500
Wire Wire Line
	4800 2900 4250 2900
Text Label 4250 2900 0    60   ~ 0
sepicImax
$Comp
L CONN_01X02 J3
U 1 1 59D6FC9E
P 7200 3200
F 0 "J3" V 7050 3100 50  0000 R CNN
F 1 "Bat" V 6900 3350 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02_Pitch2.54mm" H 7200 3200 50  0001 C CNN
F 3 "" H 7200 3200 50  0001 C CNN
	1    7200 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7000 3250 7000 3600
Connection ~ 7000 3550
NoConn ~ 3000 5600
NoConn ~ 3000 5700
Text Label 5100 6200 0    60   ~ 0
sepicImax
Wire Wire Line
	4900 6100 5550 6100
$Comp
L PWR_FLAG #FLG01
U 1 1 59D77339
P 1250 3550
F 0 "#FLG01" H 1250 3625 50  0001 C CNN
F 1 "PWR_FLAG" H 1250 3724 50  0000 C CNN
F 2 "" H 1250 3550 50  0001 C CNN
F 3 "" H 1250 3550 50  0001 C CNN
	1    1250 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 6300 5550 6300
$Comp
L PWR_FLAG #FLG02
U 1 1 59D79473
P 7050 3000
F 0 "#FLG02" H 7050 3075 50  0001 C CNN
F 1 "PWR_FLAG" V 7050 3128 50  0000 L CNN
F 2 "" H 7050 3000 50  0001 C CNN
F 3 "" H 7050 3000 50  0001 C CNN
	1    7050 3000
	0    1    1    0   
$EndComp
Wire Wire Line
	7050 3000 7000 3000
Connection ~ 7000 3000
Text Label 5100 6400 0    60   ~ 0
progFlag
$Comp
L Conn_01x06 J1
U 1 1 59D56F5B
P 6800 5500
F 0 "J1" H 6700 5050 50  0000 L CNN
F 1 "Conn_01x06" H 6700 4950 50  0000 L CNN
F 2 "myStuff:myPogoPads" H 6800 5500 50  0001 C CNN
F 3 "" H 6800 5500 50  0001 C CNN
	1    6800 5500
	1    0    0    -1  
$EndComp
Text Notes 6900 5300 0    60   ~ 0
Rst
Text Notes 6900 5400 0    60   ~ 0
RXD
Text Notes 6900 5500 0    60   ~ 0
TXD
Text Notes 6900 5600 0    60   ~ 0
5 V
Text Notes 6900 5700 0    60   ~ 0
GND
Text Notes 6900 5800 0    60   ~ 0
GND
Wire Wire Line
	6350 6400 6350 5700
Wire Wire Line
	6350 5700 6600 5700
$Comp
L +BATT #PWR01
U 1 1 59D58CC8
P 6550 5600
F 0 "#PWR01" H 6550 5450 50  0001 C CNN
F 1 "+BATT" V 6550 5850 50  0000 C CNN
F 2 "" H 6550 5600 50  0001 C CNN
F 3 "" H 6550 5600 50  0001 C CNN
	1    6550 5600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6550 5600 6600 5600
Wire Wire Line
	6600 5800 6550 5800
Wire Wire Line
	6550 5800 6550 5850
$EndSCHEMATC
