EESchema Schematic File Version 4
EELAYER 30 0
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
L velogen2-rescue:Conn_01x02_Male-Connector J1
U 1 1 5F56A285
P 1900 1800
F 0 "J1" V 1850 1800 50  0000 R CNN
F 1 "DYN" V 1750 1800 50  0000 R CNN
F 2 "Connector_JST:JST_XH_S2B-XH-A-1_1x02_P2.50mm_Horizontal" H 1900 1800 50  0001 C CNN
F 3 "~" H 1900 1800 50  0001 C CNN
	1    1900 1800
	0    -1   -1   0   
$EndComp
$Comp
L velogen2-rescue:D_Schottky-Device D1
U 1 1 5F56A4FF
P 1550 1400
F 0 "D1" V 1504 1480 50  0000 L CNN
F 1 "SS32" V 1595 1480 50  0000 L CNN
F 2 "Diode_SMD:D_SMC" H 1550 1400 50  0001 C CNN
F 3 "~" H 1550 1400 50  0001 C CNN
	1    1550 1400
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:D_Schottky-Device D3
U 1 1 5F56B156
P 2300 1400
F 0 "D3" V 2254 1480 50  0000 L CNN
F 1 "SS32" V 2345 1480 50  0000 L CNN
F 2 "Diode_SMD:D_SMC" H 2300 1400 50  0001 C CNN
F 3 "~" H 2300 1400 50  0001 C CNN
	1    2300 1400
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:D_Schottky-Device D2
U 1 1 5F56B54E
P 1550 1800
F 0 "D2" V 1504 1880 50  0000 L CNN
F 1 "SS32" V 1595 1880 50  0000 L CNN
F 2 "Diode_SMD:D_SMC" H 1550 1800 50  0001 C CNN
F 3 "~" H 1550 1800 50  0001 C CNN
	1    1550 1800
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:D_Schottky-Device D4
U 1 1 5F56B7DE
P 2300 1800
F 0 "D4" V 2254 1880 50  0000 L CNN
F 1 "SS32" V 2345 1880 50  0000 L CNN
F 2 "Diode_SMD:D_SMC" H 2300 1800 50  0001 C CNN
F 3 "~" H 2300 1800 50  0001 C CNN
	1    2300 1800
	0    1    1    0   
$EndComp
Wire Wire Line
	1550 1550 1550 1600
Wire Wire Line
	1550 1600 1900 1600
Connection ~ 1550 1600
Wire Wire Line
	1550 1600 1550 1650
Wire Wire Line
	2000 1600 2300 1600
Wire Wire Line
	2300 1600 2300 1550
Wire Wire Line
	2300 1650 2300 1600
Connection ~ 2300 1600
$Comp
L velogen2-rescue:GND-power #PWR0101
U 1 1 5F56E28B
P 1550 1950
F 0 "#PWR0101" H 1550 1700 50  0001 C CNN
F 1 "GND" H 1555 1777 50  0000 C CNN
F 2 "" H 1550 1950 50  0001 C CNN
F 3 "" H 1550 1950 50  0001 C CNN
	1    1550 1950
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0102
U 1 1 5F56E6EF
P 2300 1950
F 0 "#PWR0102" H 2300 1700 50  0001 C CNN
F 1 "GND" H 2305 1777 50  0000 C CNN
F 2 "" H 2300 1950 50  0001 C CNN
F 3 "" H 2300 1950 50  0001 C CNN
	1    2300 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	1550 1250 1550 1200
Wire Wire Line
	1550 1200 2300 1200
Wire Wire Line
	2300 1200 2300 1250
Connection ~ 2300 1200
$Comp
L velogen2-rescue:Q_PMOS_GDS-Device Q2
U 1 1 5F56F31B
P 3600 1300
F 0 "Q2" V 3942 1300 50  0000 C CNN
F 1 "DMP10H400SE" V 3851 1300 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 3800 1400 50  0001 C CNN
F 3 "https://www.diodes.com/assets/Datasheets/DMP10H400SE.pdf" H 3600 1300 50  0001 C CNN
	1    3600 1300
	0    1    -1   0   
$EndComp
$Comp
L velogen2-rescue:D_Zener-Device D5
U 1 1 5F571419
P 2900 1400
F 0 "D5" V 2854 1480 50  0000 L CNN
F 1 "10V" V 2945 1480 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-123" H 2900 1400 50  0001 C CNN
F 3 "~" H 2900 1400 50  0001 C CNN
	1    2900 1400
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:R-Device R1
U 1 1 5F572215
P 3250 1400
F 0 "R1" H 3320 1446 50  0000 L CNN
F 1 "470k" H 3320 1355 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3180 1400 50  0001 C CNN
F 3 "~" H 3250 1400 50  0001 C CNN
	1    3250 1400
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:R-Device R2
U 1 1 5F573AB3
P 3250 1800
F 0 "R2" H 3320 1846 50  0000 L CNN
F 1 "47k" H 3320 1755 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3180 1800 50  0001 C CNN
F 3 "~" H 3250 1800 50  0001 C CNN
	1    3250 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 1200 2900 1200
Wire Wire Line
	2900 1250 2900 1200
Connection ~ 2900 1200
Wire Wire Line
	2900 1200 3250 1200
Wire Wire Line
	3250 1250 3250 1200
Connection ~ 3250 1200
Wire Wire Line
	3250 1200 3400 1200
Wire Wire Line
	2900 1550 2900 1600
Wire Wire Line
	2900 1600 3250 1600
Wire Wire Line
	3250 1550 3250 1600
Connection ~ 3250 1600
Wire Wire Line
	3250 1650 3250 1600
Wire Wire Line
	3250 1950 3250 2000
Wire Wire Line
	3600 1500 3600 1600
Wire Wire Line
	3600 1600 3250 1600
$Comp
L velogen2-rescue:GND-power #PWR0103
U 1 1 5F583012
P 3250 2550
F 0 "#PWR0103" H 3250 2300 50  0001 C CNN
F 1 "GND" H 3255 2377 50  0000 C CNN
F 2 "" H 3250 2550 50  0001 C CNN
F 3 "" H 3250 2550 50  0001 C CNN
	1    3250 2550
	1    0    0    -1  
$EndComp
Text Notes 1550 1100 0    50   ~ 0
Max. input voltage: 100 V
$Comp
L velogen2-rescue:BSS123-Transistor_FET Q1
U 1 1 5F5845F4
P 3350 2200
F 0 "Q1" H 3555 2246 50  0000 L CNN
F 1 "BSS123" H 3555 2155 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3550 2125 50  0001 L CIN
F 3 "http://www.diodes.com/assets/Datasheets/ds30366.pdf" H 3350 2200 50  0001 L CNN
	1    3350 2200
	-1   0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0104
U 1 1 5F5834B3
P 9400 1400
F 0 "#PWR0104" H 9400 1150 50  0001 C CNN
F 1 "GND" H 9405 1227 50  0000 C CNN
F 2 "" H 9400 1400 50  0001 C CNN
F 3 "" H 9400 1400 50  0001 C CNN
	1    9400 1400
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:Conn_01x02_Male-Connector J6
U 1 1 5F5692A0
P 9600 1300
F 0 "J6" H 9572 1274 50  0000 R CNN
F 1 "BAT" H 9572 1183 50  0000 R CNN
F 2 "Connector_JST:JST_XH_S2B-XH-A-1_1x02_P2.50mm_Horizontal" H 9600 1300 50  0001 C CNN
F 3 "~" H 9600 1300 50  0001 C CNN
	1    9600 1300
	-1   0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+BATT-power #PWR0105
U 1 1 5F58E239
P 4100 1200
F 0 "#PWR0105" H 4100 1050 50  0001 C CNN
F 1 "+BATT" H 4115 1373 50  0000 C CNN
F 2 "" H 4100 1200 50  0001 C CNN
F 3 "" H 4100 1200 50  0001 C CNN
	1    4100 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 1200 4100 1200
$Comp
L velogen2-rescue:C-Device C2
U 1 1 5F58F463
P 4100 1400
F 0 "C2" H 4215 1446 50  0000 L CNN
F 1 "10u" H 4215 1355 50  0000 L CNN
F 2 "Capacitor_SMD:C_1210_3225Metric" H 4138 1250 50  0001 C CNN
F 3 "~" H 4100 1400 50  0001 C CNN
	1    4100 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 1250 4100 1200
Connection ~ 4100 1200
$Comp
L velogen2-rescue:GND-power #PWR0106
U 1 1 5F58FC7B
P 4100 1550
F 0 "#PWR0106" H 4100 1300 50  0001 C CNN
F 1 "GND" H 4105 1377 50  0000 C CNN
F 2 "" H 4100 1550 50  0001 C CNN
F 3 "" H 4100 1550 50  0001 C CNN
	1    4100 1550
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:R-Device R3
U 1 1 5F5917E1
P 3600 2400
F 0 "R3" H 3670 2446 50  0000 L CNN
F 1 "47k" H 3670 2355 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3530 2400 50  0001 C CNN
F 3 "~" H 3600 2400 50  0001 C CNN
	1    3600 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 2200 3600 2200
Wire Wire Line
	3600 2200 3600 2250
$Comp
L velogen2-rescue:GND-power #PWR0107
U 1 1 5F592124
P 3600 2550
F 0 "#PWR0107" H 3600 2300 50  0001 C CNN
F 1 "GND" H 3605 2377 50  0000 C CNN
F 2 "" H 3600 2550 50  0001 C CNN
F 3 "" H 3600 2550 50  0001 C CNN
	1    3600 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 2200 3950 2200
Connection ~ 3600 2200
$Comp
L velogen2-rescue:C-Device C4
U 1 1 5F592D1B
P 8950 1450
F 0 "C4" H 9065 1496 50  0000 L CNN
F 1 "10u" H 9065 1405 50  0000 L CNN
F 2 "Capacitor_SMD:C_1210_3225Metric" H 8988 1300 50  0001 C CNN
F 3 "~" H 8950 1450 50  0001 C CNN
	1    8950 1450
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0108
U 1 1 5F5933FD
P 8950 1600
F 0 "#PWR0108" H 8950 1350 50  0001 C CNN
F 1 "GND" H 8955 1427 50  0000 C CNN
F 2 "" H 8950 1600 50  0001 C CNN
F 3 "" H 8950 1600 50  0001 C CNN
	1    8950 1600
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:INA219BxD-Analog_ADC U3
U 1 1 5F594FA9
P 7200 1850
F 0 "U3" V 7400 2250 50  0000 L CNN
F 1 "INA219BxD" V 7500 2250 50  0000 L CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 8000 1500 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/ina219.pdf" H 7550 1750 50  0001 C CNN
	1    7200 1850
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:R-Device R4
U 1 1 5F597149
P 7200 1300
F 0 "R4" V 6993 1300 50  0000 C CNN
F 1 "R050" V 7084 1300 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 7130 1300 50  0001 C CNN
F 3 "~" H 7200 1300 50  0001 C CNN
	1    7200 1300
	0    1    1    0   
$EndComp
Wire Wire Line
	7050 1300 7000 1300
Wire Wire Line
	6100 1300 6100 1250
$Comp
L velogen2-rescue:+BATT-power #PWR0109
U 1 1 5F5999D2
P 6100 1250
F 0 "#PWR0109" H 6100 1100 50  0001 C CNN
F 1 "+BATT" H 6115 1423 50  0000 C CNN
F 2 "" H 6100 1250 50  0001 C CNN
F 3 "" H 6100 1250 50  0001 C CNN
	1    6100 1250
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0110
U 1 1 5F599CB9
P 6800 1850
F 0 "#PWR0110" H 6800 1600 50  0001 C CNN
F 1 "GND" H 6805 1677 50  0000 C CNN
F 2 "" H 6800 1850 50  0001 C CNN
F 3 "" H 6800 1850 50  0001 C CNN
	1    6800 1850
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+3V3-power #PWR0111
U 1 1 5F59A64B
P 7650 1800
F 0 "#PWR0111" H 7650 1650 50  0001 C CNN
F 1 "+3V3" H 7665 1973 50  0000 C CNN
F 2 "" H 7650 1800 50  0001 C CNN
F 3 "" H 7650 1800 50  0001 C CNN
	1    7650 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	7650 1800 7650 1850
Wire Wire Line
	7650 1850 7600 1850
$Comp
L velogen2-rescue:+3V3-power #PWR0112
U 1 1 5F59B554
P 6450 1700
F 0 "#PWR0112" H 6450 1550 50  0001 C CNN
F 1 "+3V3" H 6465 1873 50  0000 C CNN
F 2 "" H 6450 1700 50  0001 C CNN
F 3 "" H 6450 1700 50  0001 C CNN
	1    6450 1700
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:C-Device C3
U 1 1 5F59BE11
P 6450 1850
F 0 "C3" H 6565 1896 50  0000 L CNN
F 1 "1u" H 6565 1805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 6488 1700 50  0001 C CNN
F 3 "~" H 6450 1850 50  0001 C CNN
	1    6450 1850
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0113
U 1 1 5F59C3A2
P 6450 2000
F 0 "#PWR0113" H 6450 1750 50  0001 C CNN
F 1 "GND" H 6455 1827 50  0000 C CNN
F 2 "" H 6450 2000 50  0001 C CNN
F 3 "" H 6450 2000 50  0001 C CNN
	1    6450 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 1450 7000 1450
Wire Wire Line
	7000 1450 7000 1300
Connection ~ 7000 1300
Wire Wire Line
	7000 1300 6100 1300
Wire Wire Line
	7300 1450 7400 1450
Wire Wire Line
	7400 1450 7400 1300
Connection ~ 7400 1300
Wire Wire Line
	7400 1300 7350 1300
$Comp
L velogen2-rescue:GND-power #PWR0114
U 1 1 5F59FF3D
P 7000 2250
F 0 "#PWR0114" H 7000 2000 50  0001 C CNN
F 1 "GND" V 7005 2077 50  0000 C CNN
F 2 "" H 7000 2250 50  0001 C CNN
F 3 "" H 7000 2250 50  0001 C CNN
	1    7000 2250
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0115
U 1 1 5F5A02AD
P 7100 2250
F 0 "#PWR0115" H 7100 2000 50  0001 C CNN
F 1 "GND" V 7105 2077 50  0000 C CNN
F 2 "" H 7100 2250 50  0001 C CNN
F 3 "" H 7100 2250 50  0001 C CNN
	1    7100 2250
	1    0    0    -1  
$EndComp
Connection ~ 8950 1300
Wire Wire Line
	8950 1300 9400 1300
Wire Wire Line
	7400 1300 8950 1300
$Comp
L velogen2-rescue:DCDC_mod-myStuff U1
U 1 1 5F5ACFB9
P 2200 5050
F 0 "U1" H 2072 5578 60  0000 R CNN
F 1 "DCDC_mod" H 2072 5472 60  0000 R CNN
F 2 "myStuff:ebay_dcdc" H 2250 5100 60  0001 C CNN
F 3 "" H 1800 5650 60  0001 C CNN
	1    2200 5050
	-1   0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+BATT-power #PWR0116
U 1 1 5F5B0986
P 1700 4650
F 0 "#PWR0116" H 1700 4500 50  0001 C CNN
F 1 "+BATT" V 1715 4777 50  0000 L CNN
F 2 "" H 1700 4650 50  0001 C CNN
F 3 "" H 1700 4650 50  0001 C CNN
	1    1700 4650
	0    -1   -1   0   
$EndComp
$Comp
L velogen2-rescue:+3V3-power #PWR0117
U 1 1 5F5B1979
P 1700 4350
F 0 "#PWR0117" H 1700 4200 50  0001 C CNN
F 1 "+3V3" V 1715 4478 50  0000 L CNN
F 2 "" H 1700 4350 50  0001 C CNN
F 3 "" H 1700 4350 50  0001 C CNN
	1    1700 4350
	0    -1   -1   0   
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0118
U 1 1 5F5B284C
P 1700 4500
F 0 "#PWR0118" H 1700 4250 50  0001 C CNN
F 1 "GND" V 1705 4372 50  0000 R CNN
F 2 "" H 1700 4500 50  0001 C CNN
F 3 "" H 1700 4500 50  0001 C CNN
	1    1700 4500
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:SW_SPST-Switch SW1
U 1 1 5F5B427A
P 1550 5150
F 0 "SW1" V 1504 5248 50  0000 L CNN
F 1 "SW_SPST" V 1595 5248 50  0000 L CNN
F 2 "Connector_JST:JST_EH_B2B-EH-A_1x02_P2.50mm_Vertical" H 1550 5150 50  0001 C CNN
F 3 "~" H 1550 5150 50  0001 C CNN
	1    1550 5150
	0    1    1    0   
$EndComp
Wire Wire Line
	1700 4800 1550 4800
Wire Wire Line
	1550 4800 1550 4950
$Comp
L velogen2-rescue:GND-power #PWR0119
U 1 1 5F5B57B8
P 1550 5350
F 0 "#PWR0119" H 1550 5100 50  0001 C CNN
F 1 "GND" H 1555 5177 50  0000 C CNN
F 2 "" H 1550 5350 50  0001 C CNN
F 3 "" H 1550 5350 50  0001 C CNN
	1    1550 5350
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:ESP32-WROOM-32-RF_Module U2
U 1 1 5F5B67E7
P 5650 4750
F 0 "U2" H 5650 6650 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 5650 6550 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 5650 3250 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 5350 4800 50  0001 C CNN
	1    5650 4750
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+3V3-power #PWR0120
U 1 1 5F5B8692
P 5650 3350
F 0 "#PWR0120" H 5650 3200 50  0001 C CNN
F 1 "+3V3" H 5665 3523 50  0000 C CNN
F 2 "" H 5650 3350 50  0001 C CNN
F 3 "" H 5650 3350 50  0001 C CNN
	1    5650 3350
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0121
U 1 1 5F5B8DDE
P 5650 6150
F 0 "#PWR0121" H 5650 5900 50  0001 C CNN
F 1 "GND" H 5655 5977 50  0000 C CNN
F 2 "" H 5650 6150 50  0001 C CNN
F 3 "" H 5650 6150 50  0001 C CNN
	1    5650 6150
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+3V3-power #PWR0122
U 1 1 5F5B94A9
P 750 4450
F 0 "#PWR0122" H 750 4300 50  0001 C CNN
F 1 "+3V3" H 765 4623 50  0000 C CNN
F 2 "" H 750 4450 50  0001 C CNN
F 3 "" H 750 4450 50  0001 C CNN
	1    750  4450
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:C-Device C1
U 1 1 5F5B94B3
P 750 4600
F 0 "C1" H 865 4646 50  0000 L CNN
F 1 "1u" H 865 4555 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 788 4450 50  0001 C CNN
F 3 "~" H 750 4600 50  0001 C CNN
	1    750  4600
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0123
U 1 1 5F5B94BD
P 750 4750
F 0 "#PWR0123" H 750 4500 50  0001 C CNN
F 1 "GND" H 755 4577 50  0000 C CNN
F 2 "" H 750 4750 50  0001 C CNN
F 3 "" H 750 4750 50  0001 C CNN
	1    750  4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7400 2250 7400 2450
Wire Wire Line
	7400 2450 7550 2450
Connection ~ 7400 2450
$Comp
L velogen2-rescue:R-Device R5
U 1 1 5F5BF3F7
P 7700 2450
F 0 "R5" V 7600 2350 50  0000 C CNN
F 1 "4k7" V 7600 2500 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 7630 2450 50  0001 C CNN
F 3 "~" H 7700 2450 50  0001 C CNN
	1    7700 2450
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:+3V3-power #PWR0124
U 1 1 5F5BF87F
P 7950 2450
F 0 "#PWR0124" H 7950 2300 50  0001 C CNN
F 1 "+3V3" V 7965 2578 50  0000 L CNN
F 2 "" H 7950 2450 50  0001 C CNN
F 3 "" H 7950 2450 50  0001 C CNN
	1    7950 2450
	0    1    1    0   
$EndComp
Wire Wire Line
	7950 2450 7850 2450
Wire Wire Line
	7400 2450 7400 2650
Text Label 6750 4350 2    50   ~ 0
SCL
Text Label 7400 2650 1    50   ~ 0
SDA
Wire Wire Line
	7300 2250 7300 2650
Wire Wire Line
	6250 4350 6750 4350
Wire Wire Line
	6250 4150 6750 4150
Text Label 6750 4150 2    50   ~ 0
SDA
Text Label 7300 2650 1    50   ~ 0
SCL
Wire Wire Line
	6250 4550 6750 4550
Wire Wire Line
	6250 4250 6750 4250
Wire Wire Line
	6250 3950 6750 3950
Wire Wire Line
	6250 4050 6750 4050
Text Label 6750 4550 2    50   ~ 0
EN_DYN
Text Label 3950 2200 2    50   ~ 0
EN_DYN
Wire Wire Line
	3250 2550 3250 2400
$Comp
L velogen2-rescue:Conn_01x03_Male-Connector J3
U 1 1 5F5CC84D
P 9400 3450
F 0 "J3" H 9372 3474 50  0000 R CNN
F 1 "AUX0" H 9372 3383 50  0000 R CNN
F 2 "Connector_JST:JST_XH_S3B-XH-A-1_1x03_P2.50mm_Horizontal" H 9400 3450 50  0001 C CNN
F 3 "~" H 9400 3450 50  0001 C CNN
	1    9400 3450
	-1   0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+BATT-power #PWR0125
U 1 1 5F5CD5DE
P 9200 3450
F 0 "#PWR0125" H 9200 3300 50  0001 C CNN
F 1 "+BATT" V 9215 3577 50  0000 L CNN
F 2 "" H 9200 3450 50  0001 C CNN
F 3 "" H 9200 3450 50  0001 C CNN
	1    9200 3450
	0    -1   -1   0   
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0126
U 1 1 5F5CE12A
P 9200 3550
F 0 "#PWR0126" H 9200 3300 50  0001 C CNN
F 1 "GND" V 9205 3422 50  0000 R CNN
F 2 "" H 9200 3550 50  0001 C CNN
F 3 "" H 9200 3550 50  0001 C CNN
	1    9200 3550
	0    1    1    0   
$EndComp
Wire Wire Line
	9200 3350 8900 3350
Text Label 8900 3350 0    50   ~ 0
EN0
$Comp
L velogen2-rescue:Conn_01x03_Male-Connector J4
U 1 1 5F5D0AF5
P 9400 3900
F 0 "J4" H 9372 3924 50  0000 R CNN
F 1 "AUX1" H 9372 3833 50  0000 R CNN
F 2 "Connector_JST:JST_XH_S3B-XH-A-1_1x03_P2.50mm_Horizontal" H 9400 3900 50  0001 C CNN
F 3 "~" H 9400 3900 50  0001 C CNN
	1    9400 3900
	-1   0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+BATT-power #PWR0127
U 1 1 5F5D0AFB
P 9200 3900
F 0 "#PWR0127" H 9200 3750 50  0001 C CNN
F 1 "+BATT" V 9215 4027 50  0000 L CNN
F 2 "" H 9200 3900 50  0001 C CNN
F 3 "" H 9200 3900 50  0001 C CNN
	1    9200 3900
	0    -1   -1   0   
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0128
U 1 1 5F5D0B01
P 9200 4000
F 0 "#PWR0128" H 9200 3750 50  0001 C CNN
F 1 "GND" V 9205 3872 50  0000 R CNN
F 2 "" H 9200 4000 50  0001 C CNN
F 3 "" H 9200 4000 50  0001 C CNN
	1    9200 4000
	0    1    1    0   
$EndComp
Wire Wire Line
	9200 3800 8900 3800
Text Label 8900 3800 0    50   ~ 0
EN1
$Comp
L velogen2-rescue:Conn_01x03_Male-Connector J5
U 1 1 5F5D2A28
P 9400 4350
F 0 "J5" H 9372 4374 50  0000 R CNN
F 1 "AUX2" H 9372 4283 50  0000 R CNN
F 2 "Connector_JST:JST_XH_S3B-XH-A-1_1x03_P2.50mm_Horizontal" H 9400 4350 50  0001 C CNN
F 3 "~" H 9400 4350 50  0001 C CNN
	1    9400 4350
	-1   0    0    -1  
$EndComp
$Comp
L velogen2-rescue:+BATT-power #PWR0129
U 1 1 5F5D2A2E
P 9200 4350
F 0 "#PWR0129" H 9200 4200 50  0001 C CNN
F 1 "+BATT" V 9215 4477 50  0000 L CNN
F 2 "" H 9200 4350 50  0001 C CNN
F 3 "" H 9200 4350 50  0001 C CNN
	1    9200 4350
	0    -1   -1   0   
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0130
U 1 1 5F5D2A34
P 9200 4450
F 0 "#PWR0130" H 9200 4200 50  0001 C CNN
F 1 "GND" V 9205 4322 50  0000 R CNN
F 2 "" H 9200 4450 50  0001 C CNN
F 3 "" H 9200 4450 50  0001 C CNN
	1    9200 4450
	0    1    1    0   
$EndComp
Wire Wire Line
	9200 4250 8900 4250
Text Label 8900 4250 0    50   ~ 0
EN2
Text Label 6750 4250 2    50   ~ 0
EN0
Text Label 6750 3950 2    50   ~ 0
EN1
Text Label 6750 4050 2    50   ~ 0
EN2
$Comp
L velogen2-rescue:Conn_01x05_Male-Connector J2
U 1 1 5F5D7932
P 6950 5150
F 0 "J2" H 6922 5174 50  0000 R CNN
F 1 "Conn_01x05_Male" H 6922 5083 50  0000 R CNN
F 2 "Connector_JST:JST_XH_S5B-XH-A-1_1x05_P2.50mm_Horizontal" H 6950 5150 50  0001 C CNN
F 3 "~" H 6950 5150 50  0001 C CNN
	1    6950 5150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6750 4950 6250 4950
Wire Wire Line
	6750 5050 6250 5050
Wire Wire Line
	6750 5150 6250 5150
Wire Wire Line
	6750 5250 6250 5250
Wire Wire Line
	6750 5350 6250 5350
$Comp
L velogen2-rescue:TestPoint-Connector TP3
U 1 1 5F5E7DAE
P 6750 5550
F 0 "TP3" V 6750 5750 50  0000 L CNN
F 1 "TOUCH" V 6750 5950 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D1.5mm_Drill0.7mm" H 6950 5550 50  0001 C CNN
F 3 "~" H 6950 5550 50  0001 C CNN
	1    6750 5550
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:TestPoint-Connector TP4
U 1 1 5F5E8BE4
P 6750 5650
F 0 "TP4" V 6750 5850 50  0000 L CNN
F 1 "TOUCH" V 6750 6050 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D1.5mm_Drill0.7mm" H 6950 5650 50  0001 C CNN
F 3 "~" H 6950 5650 50  0001 C CNN
	1    6750 5650
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:TestPoint-Connector TP2
U 1 1 5F5E8EE6
P 6750 5450
F 0 "TP2" V 6750 5650 50  0000 L CNN
F 1 "TOUCH" V 6750 5850 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D1.5mm_Drill0.7mm" H 6950 5450 50  0001 C CNN
F 3 "~" H 6950 5450 50  0001 C CNN
	1    6750 5450
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:TestPoint-Connector TP1
U 1 1 5F5EABC4
P 6750 4450
F 0 "TP1" V 6750 4650 50  0000 L CNN
F 1 "TOUCH" V 6750 4850 50  0000 L CNN
F 2 "TestPoint:TestPoint_THTPad_D1.5mm_Drill0.7mm" H 6950 4450 50  0001 C CNN
F 3 "~" H 6950 4450 50  0001 C CNN
	1    6750 4450
	0    1    1    0   
$EndComp
$Comp
L velogen2-rescue:DISP_I2C_EBAY-myStuff U4
U 1 1 5F58867B
P 2700 7100
F 0 "U4" H 3178 7411 60  0000 L CNN
F 1 "DISP_I2C_EBAY" H 3178 7305 60  0000 L CNN
F 2 "myStuff:DISP_I2C_EBAY" H 2350 7050 60  0001 C CNN
F 3 "" H 2700 7100 60  0001 C CNN
	1    2700 7100
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR0131
U 1 1 5F589608
P 2200 6350
F 0 "#PWR0131" H 2200 6100 50  0001 C CNN
F 1 "GND" H 2205 6177 50  0000 C CNN
F 2 "" H 2200 6350 50  0001 C CNN
F 3 "" H 2200 6350 50  0001 C CNN
	1    2200 6350
	1    0    0    -1  
$EndComp
Wire Wire Line
	2200 6350 2200 6300
Wire Wire Line
	2200 6300 2400 6300
Wire Wire Line
	2400 6300 2400 6400
$Comp
L velogen2-rescue:+3V3-power #PWR0132
U 1 1 5F58BAE2
P 2600 6400
F 0 "#PWR0132" H 2600 6250 50  0001 C CNN
F 1 "+3V3" H 2550 6550 50  0000 C CNN
F 2 "" H 2600 6400 50  0001 C CNN
F 3 "" H 2600 6400 50  0001 C CNN
	1    2600 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 6400 2800 6200
Wire Wire Line
	3000 6400 3000 6200
Text Label 2800 6200 3    50   ~ 0
SCL
Text Label 3000 6200 3    50   ~ 0
SDA
$Comp
L velogen2-rescue:POGO_PROG-myStuff U5
U 1 1 5F5C1A33
P 9250 5450
F 0 "U5" H 9072 5496 50  0000 R CNN
F 1 "POGO_PROG" H 9072 5405 50  0000 R CNN
F 2 "myStuff:myPogoPads" H 9250 4800 50  0001 C CNN
F 3 "" H 9250 5450 50  0001 C CNN
	1    9250 5450
	-1   0    0    -1  
$EndComp
Wire Wire Line
	8900 5200 8650 5200
Wire Wire Line
	8900 5300 8650 5300
Wire Wire Line
	8900 5400 8650 5400
Wire Wire Line
	8900 5500 8950 5500
Wire Wire Line
	8900 5600 8650 5600
Text Label 8650 5200 0    50   ~ 0
prog_rst
Text Label 8650 5300 0    50   ~ 0
prog_rx
Text Label 8650 5400 0    50   ~ 0
prog_tx
$Comp
L velogen2-rescue:+BATT-power #PWR03
U 1 1 5F5CBC4F
P 8950 5500
F 0 "#PWR03" H 8950 5350 50  0001 C CNN
F 1 "+BATT" V 8950 5750 50  0000 C CNN
F 2 "" H 8950 5500 50  0001 C CNN
F 3 "" H 8950 5500 50  0001 C CNN
	1    8950 5500
	0    -1   -1   0   
$EndComp
Text Label 8650 5600 0    50   ~ 0
prog_btn
$Comp
L velogen2-rescue:GND-power #PWR02
U 1 1 5F5CECF1
P 8900 5700
F 0 "#PWR02" H 8900 5450 50  0001 C CNN
F 1 "GND" H 8905 5572 50  0000 R CNN
F 2 "" H 8900 5700 50  0001 C CNN
F 3 "" H 8900 5700 50  0001 C CNN
	1    8900 5700
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:R-Device R7
U 1 1 5F5CF834
P 5000 3350
F 0 "R7" H 5100 3450 50  0000 L CNN
F 1 "470k" H 5100 3350 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4930 3350 50  0001 C CNN
F 3 "~" H 5000 3350 50  0001 C CNN
	1    5000 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3500 5000 3550
Wire Wire Line
	5000 3550 5050 3550
$Comp
L velogen2-rescue:+3V3-power #PWR01
U 1 1 5F5D218B
P 5000 3200
F 0 "#PWR01" H 5000 3050 50  0001 C CNN
F 1 "+3V3" H 5015 3373 50  0000 C CNN
F 2 "" H 5000 3200 50  0001 C CNN
F 3 "" H 5000 3200 50  0001 C CNN
	1    5000 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3550 4650 3550
Connection ~ 5000 3550
Text Label 4650 3550 0    50   ~ 0
prog_rst
Wire Wire Line
	6250 3650 6750 3650
Text Label 6750 3650 2    50   ~ 0
prog_rx
Wire Wire Line
	6250 3850 6750 3850
Text Label 6750 3850 2    50   ~ 0
prog_tx
Wire Wire Line
	6250 3550 6750 3550
Text Label 6750 3550 2    50   ~ 0
prog_btn
$Comp
L velogen2-rescue:R-Device R6
U 1 1 5F5DEAF2
P 2750 1900
F 0 "R6" V 2650 1900 50  0000 L CNN
F 1 "47k" V 2650 1700 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2680 1900 50  0001 C CNN
F 3 "~" H 2750 1900 50  0001 C CNN
	1    2750 1900
	-1   0    0    1   
$EndComp
Wire Wire Line
	2750 1750 2750 1600
Text Label 2750 2550 1    50   ~ 0
speedPulse
Text Label 6750 3750 2    50   ~ 0
speedPulse
Wire Wire Line
	6250 3750 6750 3750
Wire Wire Line
	6750 4450 6250 4450
Wire Wire Line
	6250 5450 6750 5450
Wire Wire Line
	6250 5550 6750 5550
Wire Wire Line
	6250 5650 6750 5650
Wire Wire Line
	2750 1600 2300 1600
$Comp
L Device:D_Schottky D6
U 1 1 5F65588B
P 2850 3050
F 0 "D6" H 2850 3267 50  0000 C CNN
F 1 "B5819WS" H 2850 3176 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-323" H 2850 3050 50  0001 C CNN
F 3 "https://www.mccsemi.com/pdf/Products/B5817WS-B5819WS(SOD-323).PDF" H 2850 3050 50  0001 C CNN
	1    2850 3050
	0    1    1    0   
$EndComp
$Comp
L Device:D_Schottky D7
U 1 1 5F657E57
P 2850 3650
F 0 "D7" H 2850 3867 50  0000 C CNN
F 1 "B5819WS" H 2850 3776 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-323" H 2850 3650 50  0001 C CNN
F 3 "https://www.mccsemi.com/pdf/Products/B5817WS-B5819WS(SOD-323).PDF" H 2850 3650 50  0001 C CNN
	1    2850 3650
	0    1    1    0   
$EndComp
Wire Wire Line
	2850 3200 2850 3350
Wire Wire Line
	2850 3350 2750 3350
Connection ~ 2850 3350
Wire Wire Line
	2850 3350 2850 3500
Wire Wire Line
	2750 2050 2750 3350
$Comp
L velogen2-rescue:+3V3-power #PWR04
U 1 1 5F688C96
P 2850 2900
F 0 "#PWR04" H 2850 2750 50  0001 C CNN
F 1 "+3V3" H 2865 3073 50  0000 C CNN
F 2 "" H 2850 2900 50  0001 C CNN
F 3 "" H 2850 2900 50  0001 C CNN
	1    2850 2900
	1    0    0    -1  
$EndComp
$Comp
L velogen2-rescue:GND-power #PWR05
U 1 1 5F6962B6
P 2850 3800
F 0 "#PWR05" H 2850 3550 50  0001 C CNN
F 1 "GND" H 2855 3627 50  0000 C CNN
F 2 "" H 2850 3800 50  0001 C CNN
F 3 "" H 2850 3800 50  0001 C CNN
	1    2850 3800
	1    0    0    -1  
$EndComp
NoConn ~ 6250 5750
NoConn ~ 6250 5850
NoConn ~ 6250 4850
NoConn ~ 6250 4750
NoConn ~ 6250 4650
NoConn ~ 5050 3750
NoConn ~ 5050 3850
NoConn ~ 5050 4750
NoConn ~ 5050 4850
NoConn ~ 5050 4950
NoConn ~ 5050 5050
NoConn ~ 5050 5150
NoConn ~ 5050 5250
$EndSCHEMATC
